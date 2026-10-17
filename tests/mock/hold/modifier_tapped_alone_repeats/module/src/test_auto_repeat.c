// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once shift tapped alone while auto_repeat is held taps every interval-ms from its
 * release, each tap held tap-ms.
 * Exits 1 on missing tap, wrong gap or wrong hold.
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
#define VERDICT_MS 300

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t release_ms = key_record_keycode_ms(KEYCODE_LEFT_SHIFT, false, 0);
    mock_check(release_ms != KEY_RECORD_ABSENT_MS, "shift tapped while auto_repeat is held");
    mock_check(key_record_keycode_ms(KEYCODE_LEFT_SHIFT, true, 1) - release_ms == INTERVAL_MS,
               "first timed tap lands interval-ms after shift release");
    tap_check_gaps(KEYCODE_LEFT_SHIFT, 2, TIMED_TAPS, INTERVAL_MS);
    tap_check_holds(KEYCODE_LEFT_SHIFT, 1, TIMED_TAPS, TAP_MS);
    printk("PASS: all %u tapped modifier checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
