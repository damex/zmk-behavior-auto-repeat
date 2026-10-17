// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once a faster press before the first timed tap brings every tap one step closer.
 * Exits 1 on a gap other than interval-ms minus interval-step-ms.
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

#define TIMED_TAPS 3
#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t faster_ms = key_record_position_press_ms(FASTER_POSITION, 0);
    mock_check(faster_ms != KEY_RECORD_ABSENT_MS, "faster pressed");
    mock_check(faster_ms < key_record_keycode_ms(KEYCODE_A, true, 1),
               "faster press lands before the first timed tap");
    tap_check_gaps(KEYCODE_A, 1, TIMED_TAPS, INTERVAL_MS - INTERVAL_STEP_MS);
    printk("PASS: all %u faster checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
