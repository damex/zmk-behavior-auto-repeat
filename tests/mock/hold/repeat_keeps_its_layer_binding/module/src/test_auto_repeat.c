// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once a key picked on a held layer keeps tapping that layer's binding after the
 * layer is released.
 * Exits 1 on a tap of the base layer binding or a wrong gap.
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
    mock_check(key_record_position_press_ms(OTHER_LAYER_POSITION, 0) != KEY_RECORD_ABSENT_MS,
               "other layer held");
    mock_check(key_record_keycode_count(KEYCODE_A, true) == 0, "base layer A never taps");
    tap_check_gaps(KEYCODE_X, 1, TIMED_TAPS, INTERVAL_MS);
    printk("PASS: all %u layer binding checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
