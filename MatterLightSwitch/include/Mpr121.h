/*
 *
 *    MPR121 capacitive touch controller driver helpers.
 *
 *    Provides basic bring-up and slider centroid helpers for ELE0-ELE5.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <stdbool.h>
#include <stdint.h>

// Initialize MPR121, configure filtering, thresholds and enable ELE0-ELE5.
CHIP_ERROR Mpr121_Init();

// Read touch status bits (bits 0-11 map to ELE0-ELE11). Returns false on I2C error.
bool Mpr121_ReadTouchState(uint16_t * touchBits);

// Read filtered data for a single electrode (10-bit value returned as 16-bit). Returns false on I2C error.
bool Mpr121_ReadFilteredData(uint8_t electrode, uint16_t * value);

// Convenience: read filtered data for all 12 electrodes. Returns false on I2C error.
bool Mpr121_ReadAllFilteredData(uint16_t out[12]);

// Compute normalized slider position (0..1) across ELE0-ELE5 using filtered deltas + centroid.
// Returns true when a valid touch is detected and pos01 is populated.
bool Mpr121_GetSliderPosition(float * pos01);

// Set LED outputs (ELE6..ELE11) as a bar mask: bit0->ELE6 ... bit5->ELE11. HIGH = LED on (sourcing current).
bool Mpr121_SetLedMask(uint8_t mask);

// Convenience: map a 0..254 level to the LED bar (0..6 LEDs lit) and apply via Mpr121_SetLedMask.
bool Mpr121_SetLedBarFromLevel(uint8_t level);
