// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Key position and keycode observer shared by mock cases.
 * Stamps every event it hears with uptime ms and HID report modifiers.
 * Exits 1 once record overflows.
 * Hears position events only after the keymap listener, so it misses any the keymap stops.
 * Hears keycode events after HID listener, so recorded modifiers match sent report.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zmk/keys.h>

#define KEY_RECORD_ABSENT_MS (-1)

/* Uptime of the nth press of position, KEY_RECORD_ABSENT_MS when missing. */
int64_t key_record_position_press_ms(uint32_t position, size_t nth);

/* Uptime of the nth event of keycode in state pressed, KEY_RECORD_ABSENT_MS when missing. */
int64_t key_record_keycode_ms(uint32_t keycode, bool pressed, size_t nth);

size_t key_record_keycode_count(uint32_t keycode, bool pressed);

/* Press to press time from keycode tap nth - 1 to tap nth, KEY_RECORD_ABSENT_MS when missing. */
int64_t key_record_keycode_gap_ms(uint32_t keycode, size_t nth);

/* Press to release time of keycode tap nth, KEY_RECORD_ABSENT_MS when missing. */
int64_t key_record_keycode_hold_ms(uint32_t keycode, size_t nth);

/* HID report after nth event of keycode in state pressed holds modifier, false when missing. */
bool key_record_keycode_has_modifier(uint32_t keycode,
                                     bool pressed,
                                     size_t nth,
                                     zmk_mod_flags_t modifier);
