// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Positions, keycodes and timing of the keymap shared by mock cases. */
#pragma once

#include <zephyr/devicetree.h>

#include <dt-bindings/zmk/keys.h>

#define AUTO_REPEAT_POSITION 0
#define A_POSITION 1
#define B_POSITION 2
#define C_POSITION 3
#define LEFT_SHIFT_POSITION 4
#define CLICK_POSITION 5
#define FASTER_POSITION 6
#define SLOWER_POSITION 7
#define OTHER_LAYER_POSITION 8
#define PICKING_LAYER_POSITION 9

#define KEYCODE_A ZMK_HID_USAGE_ID(A)
#define KEYCODE_B ZMK_HID_USAGE_ID(B)
#define KEYCODE_C ZMK_HID_USAGE_ID(C)
#define KEYCODE_X ZMK_HID_USAGE_ID(X)
#define KEYCODE_LEFT_SHIFT ZMK_HID_USAGE_ID(LSHIFT)

#define AUTO_REPEAT_NODE DT_NODELABEL(auto_repeat)
#define INTERVAL_MS DT_PROP(AUTO_REPEAT_NODE, interval_ms)
#define TAP_MS DT_PROP(AUTO_REPEAT_NODE, tap_ms)
#define INTERVAL_STEP_MS DT_PROP(AUTO_REPEAT_NODE, interval_step_ms)
#define INTERVAL_MIN_MS DT_PROP(AUTO_REPEAT_NODE, interval_min_ms)
#define INTERVAL_MAX_MS DT_PROP(AUTO_REPEAT_NODE, interval_max_ms)
