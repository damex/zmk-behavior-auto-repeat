// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Timed tap verdicts shared by mock cases, over the key record. */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Exits 1 unless taps first_tap through last_tap of keycode each land gap_ms after the last. */
void tap_check_gaps(uint32_t keycode, size_t first_tap, size_t last_tap, int64_t gap_ms);

/* Exits 1 unless taps first_tap through last_tap of keycode each hold hold_ms. */
void tap_check_holds(uint32_t keycode, size_t first_tap, size_t last_tap, int64_t hold_ms);

/* Exits 1 unless the last press of keycode lands at press_ms and every press is released. */
void tap_check_stopped_at(uint32_t keycode, int64_t press_ms);
