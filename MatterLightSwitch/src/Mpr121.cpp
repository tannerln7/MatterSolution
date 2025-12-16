/*
 * MPR121 capacitive touch controller helper routines.
 *
 * Bring-up checklist (hardware):
 *  - Route I2C SDA/SCL to PC04/PC05 in Simplicity Studio; IRQ to PC01 (active low) with pull-up.
 *  - Confirm external pull-ups on SDA/SCL and that the device ACKs at 0x5A; log transfer return codes if not.
 *  - Ensure IRQ idles high; only schedule polls when it asserts.
 *  - Tune thresholds if touches never trigger or chatter due to noise/overlay differences.
 */

#include "Mpr121.h"

#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"

#include <FreeRTOS.h>
#include <cmsis_os2.h>
#include <em_i2c.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <stddef.h>

namespace {

constexpr uint8_t kI2cAddress = 0x5A << 1; // emlib expects 8-bit address (7-bit << 1)

constexpr uint8_t kRegTouchStatusL  = 0x00;
constexpr uint8_t kRegTouchStatusH  = 0x01;
constexpr uint8_t kRegFilteredBase  = 0x04;
constexpr uint8_t kRegBaselineBase  = 0x1E;
constexpr uint8_t kRegMhdr          = 0x2B;
constexpr uint8_t kRegNhdr          = 0x2C;
constexpr uint8_t kRegNclr          = 0x2D;
constexpr uint8_t kRegFdlr          = 0x2E;
constexpr uint8_t kRegMhdf          = 0x2F;
constexpr uint8_t kRegNhdf          = 0x30;
constexpr uint8_t kRegNclf          = 0x31;
constexpr uint8_t kRegFdlf          = 0x32;
constexpr uint8_t kRegNhdt          = 0x33;
constexpr uint8_t kRegNclt          = 0x34;
constexpr uint8_t kRegFdlt          = 0x35;
constexpr uint8_t kRegDebounce      = 0x5B;
constexpr uint8_t kRegConfig1       = 0x5C;
constexpr uint8_t kRegConfig2       = 0x5D;
constexpr uint8_t kRegEcr           = 0x5E;
constexpr uint8_t kRegSoftReset     = 0x80;
constexpr uint8_t kRegGpioCtl0      = 0x73;
constexpr uint8_t kRegGpioCtl1      = 0x74;
constexpr uint8_t kRegGpioData      = 0x75;
constexpr uint8_t kRegGpioDir       = 0x76;
constexpr uint8_t kRegGpioEnable    = 0x77;
constexpr uint8_t kRegGpioSet       = 0x78;
constexpr uint8_t kRegGpioClear     = 0x79;

constexpr uint8_t kTouchThreshold   = 12; // datasheet typical range 0x04-0x10
constexpr uint8_t kReleaseThreshold = 6;

constexpr uint8_t kNumElectrodes      = 12;
constexpr uint8_t kSliderElectrodes   = 6; // ELE0-ELE5
constexpr uint16_t kMinSliderOnWeight = 30;
constexpr uint16_t kMinSliderOffWeight = 12;
constexpr float kSmoothingAlpha       = 0.35f;
constexpr uint8_t kLedCount           = 6; // ELE6-ELE11

bool sSliderActive  = false;
float sSmoothedPos  = 0.0f;
bool sInitialized   = false;
bool sLoggedOvcf    = false;
uint8_t sLedState   = 0x00; // cached bits for ELE6-ELE11 (bits2-7 in GPIO regs)

sl_i2cspm_t * GetBus()
{
    return sl_i2cspm_inst0;
}

bool ReadRegisters(uint8_t startReg, uint8_t * data, size_t len)
{
    uint8_t reg = startReg;
    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = kI2cAddress;
    seq.flags       = I2C_FLAG_WRITE_READ;
    seq.buf[0].data = &reg;
    seq.buf[0].len  = static_cast<uint16_t>(1);
    seq.buf[1].data = data;
    seq.buf[1].len  = static_cast<uint16_t>(len);

    I2C_TransferReturn_TypeDef ret = I2CSPM_Transfer(GetBus(), &seq);
    if (ret != i2cTransferDone)
    {
        ChipLogError(DeviceLayer, "MPR121 read 0x%02X failed (%d)", startReg, ret);
        return false;
    }

    return true;
}

bool WriteRegisterChecked(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2] = { reg, value };
    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = kI2cAddress;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buffer;
    seq.buf[0].len  = static_cast<uint16_t>(sizeof(buffer));

    I2C_TransferReturn_TypeDef ret = I2CSPM_Transfer(GetBus(), &seq);
    if (ret != i2cTransferDone)
    {
        ChipLogError(DeviceLayer, "MPR121 write 0x%02X failed (%d)", reg, ret);
        return false;
    }
    return true;
}

bool ReadRegister(uint8_t reg, uint8_t & value)
{
    return ReadRegisters(reg, &value, 1);
}

// Configure ELE6-ELE11 as GPIO outputs (LED drivers) and default them off.
bool ConfigureGpioLeds()
{
    constexpr uint8_t kLedBits = static_cast<uint8_t>(0xFC); // bits2-7 correspond to ELE6-ELE11
    uint8_t regVal             = 0;

    // Ensure control registers are in a known state (leave default drive, just enable GPIO).
    VerifyOrReturnValue(WriteRegisterChecked(kRegGpioCtl0, 0x00), false);
    VerifyOrReturnValue(WriteRegisterChecked(kRegGpioCtl1, 0x00), false);

    // Enable GPIO function and set direction to output for ELE6-ELE11.
    if (!ReadRegister(kRegGpioEnable, regVal))
    {
        return false;
    }
    regVal = static_cast<uint8_t>(regVal | kLedBits);
    VerifyOrReturnValue(WriteRegisterChecked(kRegGpioEnable, regVal), false);

    if (!ReadRegister(kRegGpioDir, regVal))
    {
        return false;
    }
    regVal = static_cast<uint8_t>(regVal | kLedBits);
    VerifyOrReturnValue(WriteRegisterChecked(kRegGpioDir, regVal), false);

    // Turn all LEDs off.
    VerifyOrReturnValue(WriteRegisterChecked(kRegGpioClear, kLedBits), false);
    sLedState = 0;
    return true;
}

} // namespace

CHIP_ERROR Mpr121_Init()
{
    sInitialized = false;
    // Soft reset first to clear any latched faults/config.
    VerifyOrReturnError(WriteRegisterChecked(kRegSoftReset, 0x63), CHIP_ERROR_INTERNAL);
    osDelay(pdMS_TO_TICKS(1));

    // Stop electrode scanning while registers are updated (post-reset).
    VerifyOrReturnError(WriteRegisterChecked(kRegEcr, 0x00), CHIP_ERROR_INTERNAL);

    // Recommended baseline filtering (AN3891).
    const struct
    {
        uint8_t reg;
        uint8_t value;
    } filterSettings[] = {
        { kRegMhdr, 0x01 }, { kRegNhdr, 0x01 }, { kRegNclr, 0x0E }, { kRegFdlr, 0x00 }, // rising
        { kRegMhdf, 0x01 }, { kRegNhdf, 0x05 }, { kRegNclf, 0x01 }, { kRegFdlf, 0x00 }, // falling
        { kRegNhdt, 0x00 }, { kRegNclt, 0x00 }, { kRegFdlt, 0x00 }                        // touched
    };

    for (const auto & setting : filterSettings)
    {
        VerifyOrReturnError(WriteRegisterChecked(setting.reg, setting.value), CHIP_ERROR_INTERNAL);
    }

    // Touch/release thresholds for slider electrodes.
    for (uint8_t i = 0; i < kSliderElectrodes; ++i)
    {
        const uint8_t touchReg = static_cast<uint8_t>(0x41 + (i * 2));
        VerifyOrReturnError(WriteRegisterChecked(touchReg, kTouchThreshold), CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(WriteRegisterChecked(static_cast<uint8_t>(touchReg + 1), kReleaseThreshold), CHIP_ERROR_INTERNAL);
    }

    // Small debounce to suppress chatter (DT/DR nibbles).
    VerifyOrReturnError(WriteRegisterChecked(kRegDebounce, 0x22), CHIP_ERROR_INTERNAL);

    // Global CDC/CDT and filter timing: CONFIG2 = 0x24 (datasheet default timing).
    VerifyOrReturnError(WriteRegisterChecked(kRegConfig1, 0x10), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(WriteRegisterChecked(kRegConfig2, 0x24), CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(ConfigureGpioLeds(), CHIP_ERROR_INTERNAL);

    // Enable ELE0-ELE5, baseline tracking with initial load from first measurement (CL=0b10).
    const uint8_t ecrValue = static_cast<uint8_t>((0x02U << 6) | (0x00U << 4) | kSliderElectrodes);
    VerifyOrReturnError(WriteRegisterChecked(kRegEcr, ecrValue), CHIP_ERROR_INTERNAL);

    sInitialized  = true;
    sSliderActive = false;
    return CHIP_NO_ERROR;
}

bool Mpr121_ReadTouchState(uint16_t * touchBits)
{
    VerifyOrReturnValue(sInitialized, false);

    uint8_t buf[2] = {};
    if (!ReadRegisters(kRegTouchStatusL, buf, sizeof(buf)))
    {
        return false;
    }

    const bool overCurrent = (buf[1] & 0x80) != 0;
    if (overCurrent && !sLoggedOvcf)
    {
        ChipLogError(DeviceLayer, "MPR121 OVCF set (possible overcurrent), reloading config");
    }
    if (overCurrent)
    {
        sLoggedOvcf = true;
        // Per datasheet, clear OVCF by writing '1' to the flag bit before re-enabling electrodes.
        (void) WriteRegisterChecked(kRegTouchStatusH, 0x80);
        (void) Mpr121_Init();
        return false;
    }
    else if (!overCurrent)
    {
        sLoggedOvcf = false;
    }

    if (touchBits)
    {
        *touchBits = static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1] & 0x0F) << 8);
    }

    return true;
}

bool Mpr121_ReadFilteredData(uint8_t electrode, uint16_t * value)
{
    VerifyOrReturnValue(sInitialized && electrode < kNumElectrodes && value != nullptr, false);

    uint8_t raw[2] = {};
    if (!ReadRegisters(static_cast<uint8_t>(kRegFilteredBase + (electrode * 2)), raw, sizeof(raw)))
    {
        return false;
    }

    *value = (static_cast<uint16_t>(raw[1]) << 8) | raw[0];
    return true;
}

bool Mpr121_ReadAllFilteredData(uint16_t out[12])
{
    VerifyOrReturnValue(sInitialized && out != nullptr, false);
    uint8_t raw[kNumElectrodes * 2] = {};
    if (!ReadRegisters(kRegFilteredBase, raw, sizeof(raw)))
    {
        return false;
    }

    for (uint8_t i = 0; i < kNumElectrodes; ++i)
    {
        out[i] = (static_cast<uint16_t>(raw[(i * 2) + 1]) << 8) | raw[i * 2];
    }
    return true;
}

bool Mpr121_GetSliderPosition(float * pos01)
{
    VerifyOrReturnValue(sInitialized && pos01 != nullptr, false);

    // Coherent snapshot: read filtered (0x04..0x0F) + intervening + baselines (0x1E..0x23) in one burst.
    constexpr uint8_t kCoherentLen    = static_cast<uint8_t>((0x23 - kRegFilteredBase) + 1); // 32 bytes
    constexpr uint8_t kBaselineOffset = static_cast<uint8_t>(kRegBaselineBase - kRegFilteredBase); // 0x1A
    uint8_t raw[kCoherentLen]         = {};

    if (!ReadRegisters(kRegFilteredBase, raw, sizeof(raw)))
    {
        return false;
    }

    uint32_t totalWeight = 0;
    float weightedSum    = 0.0f;

    for (uint8_t i = 0; i < kSliderElectrodes; ++i)
    {
        const uint16_t filtered = (static_cast<uint16_t>(raw[(i * 2) + 1]) << 8) | raw[i * 2];
        const uint16_t baseline = static_cast<uint16_t>(raw[kBaselineOffset + i]) << 2;

        int32_t delta = static_cast<int32_t>(baseline) - static_cast<int32_t>(filtered);
        if (delta < 0)
        {
            delta = 0;
        }

        totalWeight += static_cast<uint32_t>(delta);
        weightedSum += static_cast<float>(delta) * static_cast<float>(i);
    }

    if (totalWeight == 0 || (!sSliderActive && totalWeight < kMinSliderOnWeight) ||
        (sSliderActive && totalWeight < kMinSliderOffWeight))
    {
        sSliderActive = false;
        return false;
    }

    const float rawPos = weightedSum / static_cast<float>(totalWeight);
    float pos         = rawPos / static_cast<float>(kSliderElectrodes - 1);

    pos = std::clamp(pos, 0.0f, 1.0f);
    if (!sSliderActive)
    {
        sSliderActive = true;
        sSmoothedPos  = pos;
    }
    else
    {
        sSmoothedPos += kSmoothingAlpha * (pos - sSmoothedPos);
    }

    *pos01 = sSmoothedPos;
    return true;
}

bool Mpr121_SetLedMask(uint8_t mask)
{
    VerifyOrReturnValue(sInitialized, false);
    const uint8_t regMask = static_cast<uint8_t>((mask & ((1U << kLedCount) - 1)) << 2); // shift into ELE6-ELE11 bits

    if (regMask == sLedState)
    {
        return true;
    }

    const uint8_t clearMask = static_cast<uint8_t>(sLedState & static_cast<uint8_t>(~regMask));
    const uint8_t setMask   = static_cast<uint8_t>(regMask & static_cast<uint8_t>(~sLedState));

    if (clearMask && !WriteRegisterChecked(kRegGpioClear, clearMask))
    {
        return false;
    }
    if (setMask && !WriteRegisterChecked(kRegGpioSet, setMask))
    {
        return false;
    }

    sLedState = regMask;
    return true;
}

bool Mpr121_SetLedBarFromLevel(uint8_t level)
{
    // Map 0..254 to 0..6 LEDs. Ensure max lights all.
    const uint16_t scaled   = static_cast<uint16_t>(level) * kLedCount + (254 - 1);
    uint8_t litCount        = static_cast<uint8_t>(scaled / 254);
    if (level == 0)
    {
        litCount = 0;
    }
    else if (level >= 254)
    {
        litCount = kLedCount;
    }

    uint8_t mask = 0;
    if (litCount > 0)
    {
        mask = static_cast<uint8_t>((1U << litCount) - 1U);
    }
    return Mpr121_SetLedMask(mask);
}
