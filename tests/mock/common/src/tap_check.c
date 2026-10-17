// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Timed tap verdicts shared by mock cases. */

#include "tap_check.h"

#include "key_record.h"
#include "mock.h"

void tap_check_gaps(uint32_t keycode, size_t first_tap, size_t last_tap, int64_t gap_ms) {
    for (size_t tap = first_tap; tap <= last_tap; tap++) {
        mock_check(key_record_keycode_gap_ms(keycode, tap) == gap_ms,
                   "taps land the interval apart");
    }
}

void tap_check_holds(uint32_t keycode, size_t first_tap, size_t last_tap, int64_t hold_ms) {
    for (size_t tap = first_tap; tap <= last_tap; tap++) {
        mock_check(key_record_keycode_hold_ms(keycode, tap) == hold_ms, "each tap holds tap-ms");
    }
}

void tap_check_stopped_at(uint32_t keycode, int64_t press_ms) {
    size_t presses = key_record_keycode_count(keycode, true);
    mock_check(presses > 0, "key pressed at least once");
    mock_check(key_record_keycode_ms(keycode, true, presses - 1) == press_ms,
               "no tap after the stop");
    mock_check(key_record_keycode_count(keycode, false) == presses, "every press released");
}
