// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once a key pressed while auto_repeat is held types on that press, then taps every
 * interval-ms, each tap held tap-ms.
 * Exits 1 on a missing first press, a wrong gap or hold, or a missing tap.
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
#include "tap_check.h"

#define TIMED_TAPS 4
#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t pick_ms = key_record_position_press_ms(A_POSITION, 0);
    mock_check(pick_ms != KEY_RECORD_ABSENT_MS, "A pressed while auto_repeat is held");
    mock_check(key_record_keycode_ms(KEYCODE_A, true, 0) == pick_ms, "picking press types A");
    tap_check_gaps(KEYCODE_A, 1, TIMED_TAPS, INTERVAL_MS);
    tap_check_holds(KEYCODE_A, 1, TIMED_TAPS, TAP_MS);
    printk("PASS: all %u picked key checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
