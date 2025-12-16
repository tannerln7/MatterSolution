/*
 * IADC driver for button resistor ladder on PC00.
 *
 * Resistor ladder (approx ratios):
 *  - None: near VDD
 *  - Button1 (10k to GND): ~0.175 * VDD
 *  - Button2 (22k to GND): ~0.319 * VDD
 */

#include "IadcButtons.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_iadc.h"
#include "em_core.h"
#include <lib/support/logging/CHIPLogging.h>

namespace {

// ADC input and thresholds
constexpr uint8_t kAdcBits         = 12;
constexpr uint16_t kAdcMax         = (1u << kAdcBits) - 1u; // fixed by IADC_INITSINGLE_DEFAULT alignment

// Ratios from ladder: Button1 ~0.175, Button2 ~0.319, None near 1.0.
constexpr float kRatioBtn1         = 10.0f / (47.0f + 10.0f);
constexpr float kRatioBtn2         = 22.0f / (47.0f + 22.0f);

// Hysteresis bands around expected ratios.
constexpr float kHystFrac          = 0.05f; // +/-5% of full-scale as guard

constexpr uint16_t RatioToCode(float ratio)
{
    float scaled = ratio * static_cast<float>(kAdcMax);
    if (scaled < 0.0f)
    {
        scaled = 0.0f;
    }
    if (scaled > static_cast<float>(kAdcMax))
    {
        scaled = static_cast<float>(kAdcMax);
    }
    return static_cast<uint16_t>(scaled + 0.5f);
}

// Compute window edges.
constexpr uint16_t kBtn1Max = RatioToCode(kRatioBtn1 + kHystFrac);
constexpr uint16_t kBtn2Min = RatioToCode(kRatioBtn2 - kHystFrac);
constexpr uint16_t kBtn2Max = RatioToCode(kRatioBtn2 + kHystFrac);
constexpr uint16_t kNoneMin = RatioToCode(0.8f); // treat top 20% as "none"

constexpr uint8_t kDebounceCount     = 3;

volatile uint16_t sLastRaw           = 0;
volatile bool sNewSample             = false;

ButtonLadderState sStable            = ButtonLadderState::None;
ButtonLadderState sCandidate         = ButtonLadderState::None;
uint8_t sCandidateCount              = 0;

// Map PC00 to IADC positive input.
constexpr IADC_PosInput_t kPosInput = iadcPosInputPortCPin0;

void ConfigureClock()
{
    CMU_ClockEnable(cmuClock_IADC0, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    // Use FSRCO as IADC clock source (matches Silicon Labs reference).
    CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);
}

void ConfigureGpio()
{
    // Disable digital input/output on PC00 for analog.
    GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 0);
    // Route PC00 to IADC via CDBUSALLOC.
    GPIO->CDBUSALLOC_SET = GPIO_CDBUSALLOC_CDEVEN0_ADC0;
}

} // namespace

CHIP_ERROR IadcButtons_Init()
{
    ConfigureClock();
    ConfigureGpio();

    IADC_Init_t init = IADC_INIT_DEFAULT;
    // Keep warm between conversions to reduce latency.
    init.warmup = iadcWarmupKeepWarm;
    uint8_t srcPrescale = IADC_calcSrcClkPrescale(IADC0, 10000000, 0); // target up to 10 MHz
    init.srcClkPrescale = srcPrescale;
    init.timerCycles    = 0;

    IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
    initSingle.dataValidLevel    = iadcFifoCfgDvl1;
    initSingle.showId            = false;
    initSingle.triggerAction     = iadcTriggerActionOnce;
    initSingle.start             = false;

    IADC_SingleInput_t input = IADC_SINGLEINPUT_DEFAULT;
    input.posInput            = kPosInput;
    input.negInput            = iadcNegInputGnd;
    input.configId            = 0;

    IADC_AllConfigs_t allConfigs = IADC_ALLCONFIGS_DEFAULT;
    allConfigs.configs[0].reference      = iadcCfgReferenceVddx;
    allConfigs.configs[0].analogGain     = iadcCfgAnalogGain1x;
    allConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0, 1000000, 0, allConfigs.configs[0].adcMode, srcPrescale);
    allConfigs.configs[0].vRef           = 3300;

    IADC_init(IADC0, &init, &allConfigs);
    IADC_initSingle(IADC0, &initSingle, &input);

    // Clear any stale flags and enable single done interrupt.
    IADC_clearInt(IADC0, _IADC_IF_MASK);
    IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);
    NVIC_ClearPendingIRQ(IADC_IRQn);
    NVIC_EnableIRQ(IADC_IRQn);

    // Prime first conversion if desired.
    IadcButtons_StartSingle();
    return CHIP_NO_ERROR;
}

void IadcButtons_StartSingle()
{
    IADC_command(IADC0, iadcCmdStartSingle);
}

bool IadcButtons_TryGetLatest(uint16_t * raw, bool * isNew)
{
    if (raw == nullptr || isNew == nullptr)
    {
        return false;
    }

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    const bool newSample = sNewSample;
    *raw                 = sLastRaw;
    *isNew               = newSample;
    if (newSample)
    {
        sNewSample = false;
    }
    CORE_EXIT_CRITICAL();
    return true;
}

ButtonLadderState IadcButtons_ClassifyRaw(uint16_t raw)
{
    if (raw >= kNoneMin)
    {
        return ButtonLadderState::None;
    }
    if (raw <= kBtn1Max)
    {
        return ButtonLadderState::Button1;
    }
    if (raw >= kBtn2Min && raw <= kBtn2Max)
    {
        return ButtonLadderState::Button2;
    }
    return ButtonLadderState::Invalid;
}

bool IadcButtons_Update(ButtonLadderState * stableStateOut, bool * changed, uint16_t * lastRawOut)
{
    if (stableStateOut == nullptr || changed == nullptr)
    {
        return false;
    }

    uint16_t raw   = 0;
    bool isNew     = false;
    if (!IadcButtons_TryGetLatest(&raw, &isNew) || !isNew)
    {
        *stableStateOut = sStable;
        *changed        = false;
        if (lastRawOut)
        {
            *lastRawOut = raw;
        }
        return false;
    }

    if (lastRawOut)
    {
        *lastRawOut = raw;
    }
    ButtonLadderState classified = IadcButtons_ClassifyRaw(raw);

    if (classified == ButtonLadderState::Invalid)
    {
        sCandidateCount = 0;
        *stableStateOut = sStable;
        *changed        = false;
        return true;
    }

    if (classified == sCandidate)
    {
        if (sCandidateCount < 0xFF)
        {
            sCandidateCount++;
        }
    }
    else
    {
        sCandidate      = classified;
        sCandidateCount = 1;
    }

    *changed = false;
    if ((sCandidateCount >= kDebounceCount) && (sCandidate != sStable))
    {
        sStable = sCandidate;
        *changed = true;
    }

    *stableStateOut = sStable;
    return true;
}

extern "C" void IADC_IRQHandler(void)
{
    uint32_t flags = IADC_getInt(IADC0);
    if (flags & IADC_IF_SINGLEDONE)
    {
        IADC_Result_t result = IADC_pullSingleFifoResult(IADC0);
        sLastRaw             = result.data;
        sNewSample           = true;
        IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
    }
}
