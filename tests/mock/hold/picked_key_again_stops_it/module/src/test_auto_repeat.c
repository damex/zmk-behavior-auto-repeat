// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once picking a repeating key again stops it, that press its last, every press
 * released.
 * Exits 1 on a tap after the second pick or a key left pressed.
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

#define TAPS_BEFORE_SECOND_PICK_MIN 3
#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t second_pick_ms = key_record_position_press_ms(A_POSITION, 1);
    mock_check(second_pick_ms != KEY_RECORD_ABSENT_MS, "A picked again");
    mock_check(key_record_keycode_count(KEYCODE_A, true) >= TAPS_BEFORE_SECOND_PICK_MIN,
               "A tapped between the picks");
    tap_check_stopped_at(KEYCODE_A, second_pick_ms);
    printk("PASS: all %u second pick checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
