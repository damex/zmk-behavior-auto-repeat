// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once faster press past shortened deadline taps at once, then every interval-ms minus
 * interval-step-ms.
 * Exits 1 on late first timed tap or wrong gap.
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

#define EXPIRED_TIMEOUT_LATENCY_MS 1
#define TIMED_TAPS 4
#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t pick_ms = key_record_position_press_ms(A_POSITION, 0);
    int64_t faster_ms = key_record_position_press_ms(FASTER_POSITION, 0);
    mock_check(pick_ms != KEY_RECORD_ABSENT_MS, "A picked");
    mock_check(faster_ms != KEY_RECORD_ABSENT_MS, "faster pressed");
    int64_t faster_after_pick_ms = faster_ms - pick_ms;
    mock_check(faster_after_pick_ms >= INTERVAL_MS - INTERVAL_STEP_MS,
               "faster press lands past shortened deadline");
    mock_check(faster_after_pick_ms < INTERVAL_MS, "faster press lands before original deadline");
    int64_t tap_after_faster_ms = key_record_keycode_ms(KEYCODE_A, true, 1) - faster_ms;
    mock_check(tap_after_faster_ms >= 0, "first timed tap lands after faster press");
    mock_check(tap_after_faster_ms <= EXPIRED_TIMEOUT_LATENCY_MS,
               "first timed tap lands within one tick of faster press");
    tap_check_gaps(KEYCODE_A, 2, TIMED_TAPS, INTERVAL_MS - INTERVAL_STEP_MS);
    printk("PASS: all %u past due checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
