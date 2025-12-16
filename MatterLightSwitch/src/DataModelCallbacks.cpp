/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>
#include "sl_pwm.h"
#include "sl_pwm_instances.h"

#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/level-control/level-control.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

static constexpr chip::EndpointId kOnboardLedEndpoint = 2;

// Last non-zero level so we can restore when turning back on
static uint8_t sOnboardLedLastLevel = 254;

static void SetOnboardLedLevel(uint8_t level)
{
    // LevelControl.CurrentLevel is 0–254. Map to 0–100% duty.
    uint8_t percent = static_cast<uint16_t>(level) * 100u / 254u;

    // If the PWM is configured "active low" in Simplicity Studio,
    // sl_pwm_set_duty_cycle(percent) will already do the right thing.
    // If you accidentally set it as active high, you’d invert: 100 - percent.
    sl_pwm_set_duty_cycle(&sl_pwm_pwm, percent);
    sl_pwm_start(&sl_pwm_pwm);
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath &attributePath,
                                       uint8_t type, uint16_t size, uint8_t *value)
{
    ClusterId clusterId = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    EndpointId endpointId = attributePath.mEndpointId;

    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    // --- Identify logging stays as-is ---
    if (clusterId == Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);
    }

    // Only care about our LED-test light endpoint
    if (endpointId != kOnboardLedEndpoint)
    {
        return;
    }

    // --- OnOff cluster: turn LED on/off at stored level ---
    if (clusterId == OnOff::Id &&
        attributeId == OnOff::Attributes::OnOff::Id)
    {
        bool on = (*value != 0);

        ChipLogProgress(Zcl, "OnOff changed on LED endpoint %u: %s",
                        static_cast<unsigned>(endpointId), on ? "ON" : "OFF");

        if (!on)
        {
            // Off → force level 0
            SetOnboardLedLevel(0);
        }
        else
        {
            // On → restore last known non-zero level
            if (sOnboardLedLastLevel == 0)
            {
                sOnboardLedLastLevel = 254; // default to full bright if never set
            }
            SetOnboardLedLevel(sOnboardLedLastLevel);
        }
    }
    // --- LevelControl cluster: update brightness + remember last level ---
    else if (clusterId == LevelControl::Id &&
             attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        if (size >= 1 && value != nullptr)
        {
            uint8_t level = *value;

            ChipLogProgress(Zcl, "LevelControl.CurrentLevel changed on LED endpoint %u: %u",
                            static_cast<unsigned>(endpointId), level);

            // Track last non-zero level so On/Off can restore something useful
            if (level != 0)
            {
                sOnboardLedLastLevel = level;
            }

            SetOnboardLedLevel(level);
        }
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}
