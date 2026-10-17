// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once a tap of auto_repeat with no key pressed stops every repeating key, all
 * released.
 * Exits 1 on a tap after the lone auto_repeat tap or a key left pressed.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "auto_repeat_keymap.h"
#include "key_record.h"
#include "mock.h"

#define TAPS_BEFORE_STOP_MIN 2
#define VERDICT_MS 400

static void check_stopped_before(uint32_t keycode, int64_t stop_ms) {
    size_t presses = key_record_keycode_count(keycode, true);
    mock_check(presses >= TAPS_BEFORE_STOP_MIN, "key repeated before the lone tap");
    mock_check(key_record_keycode_ms(keycode, true, presses - 1) < stop_ms,
               "no tap after the lone tap");
    mock_check(key_record_keycode_count(keycode, false) == presses, "every press released");
}

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t lone_tap_ms = key_record_position_press_ms(AUTO_REPEAT_POSITION, 1);
    mock_check(lone_tap_ms != KEY_RECORD_ABSENT_MS, "auto_repeat tapped alone");
    check_stopped_before(KEYCODE_A, lone_tap_ms);
    check_stopped_before(KEYCODE_B, lone_tap_ms);
    printk("PASS: all %u lone tap checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
