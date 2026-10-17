// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once faster presses past the shortest interval, made before any pick, leave taps
 * interval-min-ms apart.
 * Exits 1 on a gap other than interval-min-ms.
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

#define FASTER_PRESSES 4
#define TIMED_TAPS 3
#define VERDICT_MS 400

BUILD_ASSERT((INTERVAL_MS - INTERVAL_MIN_MS) / INTERVAL_STEP_MS < FASTER_PRESSES,
             "faster presses reach past the shortest interval");

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t last_faster_ms = key_record_position_press_ms(FASTER_POSITION, FASTER_PRESSES - 1);
    mock_check(last_faster_ms != KEY_RECORD_ABSENT_MS, "every faster press landed");
    mock_check(last_faster_ms < key_record_position_press_ms(A_POSITION, 0),
               "faster presses land before the pick");
    tap_check_gaps(KEYCODE_A, 1, TIMED_TAPS, INTERVAL_MIN_MS);
    printk("PASS: all %u shortest interval checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
