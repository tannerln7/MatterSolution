/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**********************************************************
 * Includes
 *********************************************************/

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "BindingHandler.h"
#include "LEDWidget.h"
#include "LightSwitchMgr.h"
#include "Mpr121.h"
#include "IadcButtons.h"
#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <algorithm>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include "sl_pwm.h"
#include "sl_pwm_instances.h"

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define SYSTEM_STATE_LED &sl_led_led0

namespace {
constexpr chip::EndpointId kLightSwitchEndpoint   = 1;
constexpr chip::EndpointId kGenericSwitchEndpoint = 2;
constexpr uint32_t kMpr121FastPollMs              = 20;
constexpr uint32_t kMpr121IdlePollMs              = 80;
constexpr uint32_t kMpr121LogIntervalMs           = 60;
constexpr float kMpr121LogDelta                   = 0.02f;
constexpr uint32_t kIadcPollMs                    = 30;

constexpr float AbsDiff(float a, float b)
{
    float d = a - b;
    return (d < 0.0f) ? -d : d;
}
} // namespace

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * AppTask Definitions
 *********************************************************/

AppTask AppTask::sAppTask;

bool AppTask::functionButtonPressed  = false;
bool AppTask::actionButtonPressed    = false;
bool AppTask::actionButtonSuppressed = false;
bool AppTask::isButtonEventTriggered = false;
osTimerId_t AppTask::mpr121Timer     = nullptr;
uint32_t AppTask::mpr121PollIntervalMs = kMpr121IdlePollMs;
uint32_t AppTask::mpr121LastLogMs    = 0;
uint16_t AppTask::mpr121LastTouchBits = 0;
float AppTask::mpr121LastPos         = 0.0f;
uint8_t AppTask::mpr121CurrentLevel  = 0;
osTimerId_t AppTask::iadcTimer       = nullptr;

CHIP_ERROR AppTask::AppInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

    err = LightSwitchMgr::GetInstance().Init(kLightSwitchEndpoint, kGenericSwitchEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("LightSwitchMgr Init failed!");
        appError(err);
    }

    // OPTIONAL: make sure onboard LED PWM starts in a known state
    sl_pwm_set_duty_cycle(&sl_pwm_pwm, 0);   // 0% → off
    sl_pwm_start(&sl_pwm_pwm);

    CHIP_ERROR mprErr = Mpr121_Init();
    if (mprErr != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "MPR121 Init failed: %" CHIP_ERROR_FORMAT, mprErr.Format());
    }

    mpr121Timer = osTimerNew(Mpr121TimerCallback, osTimerPeriodic, this, nullptr);
    if (mpr121Timer == nullptr)
    {
        ChipLogError(AppServer, "MPR121 timer create failed");
    }
    else if (osTimerStart(mpr121Timer, pdMS_TO_TICKS(kMpr121IdlePollMs)) != osOK)
    {
        ChipLogError(AppServer, "MPR121 timer start failed");
    }

    longPressTimer = new Timer(LONG_PRESS_TIMEOUT_MS, OnLongPressTimeout, this);

    if (IadcButtons_Init() != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "IADC init failed");
    }
    else
    {
        IadcButtons_StartSingle();
        iadcTimer = osTimerNew(IadcTimerCallback, osTimerPeriodic, this, nullptr);
        if (iadcTimer == nullptr)
        {
            ChipLogError(AppServer, "IADC timer create failed");
        }
        else if (osTimerStart(iadcTimer, pdMS_TO_TICKS(kIadcPollMs)) != osOK)
        {
            ChipLogError(AppServer, "IADC timer start failed");
        }
    }

    return err;
}

void AppTask::Timer::Start()
{
    // Starts or restarts the function timer
    osStatus_t status = osTimerStart(mHandler, pdMS_TO_TICKS(LONG_PRESS_TIMEOUT_MS));
    if (status != osOK)
    {
        SILABS_LOG("Timer start() failed with error code : %ld", status);
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mIsActive = true;
}

void AppTask::Timer::Timeout()
{
    mIsActive = false;
    if (mCallback)
    {
        mCallback(*this);
    }
}

void AppTask::HandleLongPress()
{
    AppEvent event;
    event.Handler = AppTask::AppEventHandler;

    if (actionButtonPressed)
    {
        actionButtonSuppressed = true;
        // Long press button up : Trigger Level Control Action
        event.Type = AppEvent::kEventType_TriggerLevelControlAction;
        AppTask::GetAppTask().PostEvent(&event);
    }
}

void AppTask::OnLongPressTimeout(AppTask::Timer & timer)
{
    AppTask * app = static_cast<AppTask *>(timer.mContext);
    if (app)
    {
        app->HandleLongPress();
    }
}

AppTask::Timer::Timer(uint32_t timeoutInMs, Callback callback, void * context) : mCallback(callback), mContext(context)
{
    mHandler = osTimerNew(TimerCallback, // timer callback handler
                          osTimerOnce,   // no timer reload (one-shot timer)
                          this,          // pass the app task obj context
                          NULL           // No osTimerAttr_t to provide.
    );

    if (mHandler == NULL)
    {
        SILABS_LOG("Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }
}

AppTask::Timer::~Timer()
{
    if (mHandler)
    {
        osTimerDelete(mHandler);
        mHandler = nullptr;
    }
}

void AppTask::Timer::Stop()
{
    // Abort on osError (-1) as it indicates an unspecified failure with no clear recovery path.
    if (osTimerStop(mHandler) == osError)
    {
        SILABS_LOG("Timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
    mIsActive = false;
}

void AppTask::Timer::TimerCallback(void * timerCbArg)
{
    Timer * timer = reinterpret_cast<Timer *>(timerCbArg);
    if (timer)
    {
        timer->Timeout();
    }
}

void AppTask::Mpr121TimerCallback(void * timerCbArg)
{
    AppEvent event = {};
    event.Type     = AppEvent::kEventType_Mpr121Poll;
    event.Handler  = Mpr121EventHandler;
    AppTask::GetAppTask().PostEvent(&event);
}

void AppTask::IadcTimerCallback(void * timerCbArg)
{
    AppEvent event = {};
    event.Type     = AppEvent::kEventType_IadcPoll;
    event.Handler  = IadcEventHandler;
    AppTask::GetAppTask().PostEvent(&event);
}

void AppTask::UpdateMpr121PollRate(uint32_t intervalMs)
{
    if ((mpr121Timer == nullptr) || (mpr121PollIntervalMs == intervalMs))
    {
        return;
    }

    if (osTimerStop(mpr121Timer) == osError)
    {
        ChipLogError(AppServer, "MPR121 timer stop failed");
        return;
    }
    if (osTimerStart(mpr121Timer, pdMS_TO_TICKS(intervalMs)) != osOK)
    {
        ChipLogError(AppServer, "MPR121 timer restart failed");
        return;
    }

    mpr121PollIntervalMs = intervalMs;
}

void AppTask::UpdateMpr121Level(uint8_t level)
{
    const uint8_t clamped = static_cast<uint8_t>(std::min<uint16_t>(254, level));
    if (clamped == mpr121CurrentLevel)
    {
        return;
    }
    mpr121CurrentLevel = clamped;
    (void) Mpr121_SetLedBarFromLevel(clamped);
}

void AppTask::Mpr121EventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent != nullptr);
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_Mpr121Poll);

    uint16_t touchBits = 0;
    const bool touchOk = Mpr121_ReadTouchState(&touchBits);

    if (!touchOk)
    {
        UpdateMpr121PollRate(kMpr121IdlePollMs);
        return;
    }

    float pos01       = 0.0f;
    const bool hasPos = Mpr121_GetSliderPosition(&pos01);
    const bool active = hasPos || ((touchBits & 0x003F) != 0);

    UpdateMpr121PollRate(active ? kMpr121FastPollMs : kMpr121IdlePollMs);

    const uint32_t nowMs = (osKernelGetTickCount() * 1000U) / osKernelGetTickFreq();

    if (hasPos)
    {
        const bool positionChanged = (AbsDiff(pos01, mpr121LastPos) > kMpr121LogDelta);
        const bool touchChanged    = (touchBits != mpr121LastTouchBits);
        // Map slider position to 0..254 brightness.
        uint8_t level = static_cast<uint8_t>(std::min(254.0f, std::max(0.0f, pos01 * 254.0f)));
        UpdateMpr121Level(level);
        if (positionChanged || touchChanged || ((nowMs - mpr121LastLogMs) >= kMpr121LogIntervalMs))
        {
            SILABS_LOG("MPR121 touch=0x%03X pos=%.3f", touchBits & 0x0FFF, pos01);
            mpr121LastLogMs     = nowMs;
            mpr121LastPos       = pos01;
            mpr121LastTouchBits = touchBits;
        }
    }
    else if (touchBits != mpr121LastTouchBits)
    {
        SILABS_LOG("MPR121 touch=0x%03X (idle)", touchBits & 0x0FFF);
        mpr121LastTouchBits = touchBits;
        mpr121LastLogMs     = nowMs;
    }
}

void AppTask::IadcEventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent != nullptr);
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_IadcPoll);

    // Start next conversion; result will be picked up on the next handler run.
    IadcButtons_StartSingle();

    ButtonLadderState stable = ButtonLadderState::None;
    bool changed             = false;
    if (IadcButtons_Update(&stable, &changed) && changed)
    {
        uint16_t raw  = 0;
        bool isNewRaw = false;
        if (IadcButtons_TryGetLatest(&raw, &isNewRaw))
        {
            const char * stateStr = (stable == ButtonLadderState::Button1) ? "Button1"
                                     : (stable == ButtonLadderState::Button2) ? "Button2"
                                     : "None";
            SILABS_LOG("IADC raw=%u state=%s", raw, stateStr);
        }
    }
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    sAppTask.StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");
    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent event = {};
    event.Handler  = AppTask::AppEventHandler;
    if (btnAction == to_underlying(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        event.Type = (button ? AppEvent::kEventType_ActionButtonPressed : AppEvent::kEventType_FunctionButtonPressed);
    }
    else
    {
        event.Type = (button ? AppEvent::kEventType_ActionButtonReleased : AppEvent::kEventType_FunctionButtonReleased);
    }
    AppTask::GetAppTask().PostEvent(&event);
}

void AppTask::AppEventHandler(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_FunctionButtonPressed:
        functionButtonPressed = true;
        if (actionButtonPressed)
        {
            actionButtonSuppressed = true;
            LightSwitchMgr::GetInstance().changeStepMode();
        }
        else
        {
            isButtonEventTriggered = true;
            // Post button press event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed);
            button_event.Handler            = BaseApplication::ButtonHandler;
            AppTask::GetAppTask().PostEvent(&button_event);
        }
        break;
    case AppEvent::kEventType_FunctionButtonReleased: {
        functionButtonPressed = false;
        if (isButtonEventTriggered)
        {
            isButtonEventTriggered = false;
            // Post button release event to BaseApplication
            AppEvent button_event           = {};
            button_event.Type               = AppEvent::kEventType_Button;
            button_event.ButtonEvent.Action = static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonReleased);
            button_event.Handler            = BaseApplication::ButtonHandler;
            AppTask::GetAppTask().PostEvent(&button_event);
        }
        break;
    }
    case AppEvent::kEventType_ActionButtonPressed:
        actionButtonPressed = true;
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        if (functionButtonPressed)
        {
            actionButtonSuppressed = true;
            LightSwitchMgr::GetInstance().changeStepMode();
        }
        else if (sAppTask.longPressTimer)
        {
            sAppTask.longPressTimer->Start();
        }
        break;
    case AppEvent::kEventType_ActionButtonReleased:
        actionButtonPressed = false;
        if (sAppTask.longPressTimer)
        {
            sAppTask.longPressTimer->Stop();
        }
        if (actionButtonSuppressed)
        {
            actionButtonSuppressed = false;
        }
        else
        {
            aEvent->Type = AppEvent::kEventType_TriggerToggle;
            LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        }
        aEvent->Type = AppEvent::kEventType_ActionButtonReleased;
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        break;
    case AppEvent::kEventType_TriggerLevelControlAction:
        LightSwitchMgr::GetInstance().SwitchActionEventHandler(aEvent->Type);
        break;
    case AppEvent::kEventType_IadcPoll:
        IadcEventHandler(aEvent);
        break;
    default:
        break;
    }
}
