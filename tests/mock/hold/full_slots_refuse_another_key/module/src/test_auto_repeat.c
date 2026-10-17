// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap with max-active 2, real auto-repeat module.
 * Exits 0 once a third key picked with both slots taken types once and never repeats, the
 * first two still repeating.
 * Exits 1 on a repeated third key or a stopped first or second key.
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

#define REPEATED_TAPS_MIN 3
#define VERDICT_MS 300

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    mock_check(key_record_position_press_ms(C_POSITION, 0) != KEY_RECORD_ABSENT_MS,
               "C picked with both slots taken");
    mock_check(key_record_keycode_count(KEYCODE_C, true) == 1, "C types once");
    mock_check(key_record_keycode_count(KEYCODE_A, true) >= REPEATED_TAPS_MIN, "A repeats");
    mock_check(key_record_keycode_count(KEYCODE_B, true) >= REPEATED_TAPS_MIN, "B repeats");
    printk("PASS: all %u full slots checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
