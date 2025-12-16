/*
 * Interrupt-driven IADC sampling for button resistor ladder on PC00.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <stdint.h>

enum class ButtonLadderState : uint8_t
{
    None,
    Button1,
    Button2,
    Invalid
};

CHIP_ERROR IadcButtons_Init();

// Kick off a single conversion (non-blocking).
void IadcButtons_StartSingle();

// Fetch the last raw sample captured by ISR; isNew is true if this sample has not been read since ISR wrote it.
bool IadcButtons_TryGetLatest(uint16_t * raw, bool * isNew);

// Classify a raw ADC sample into ladder state (rough, no debounce).
ButtonLadderState IadcButtons_ClassifyRaw(uint16_t raw);

// Debounce/hysteresis using latest sample; returns true if a new stable state is available. 'changed' indicates transition.
// lastRawOut (optional) returns the raw sample used for the update.
bool IadcButtons_Update(ButtonLadderState * stableStateOut, bool * changed, uint16_t * lastRawOut = nullptr);
