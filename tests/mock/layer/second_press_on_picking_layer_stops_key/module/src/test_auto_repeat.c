// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once second press of repeating key on picking layer stops it, that press its last,
 * every press released.
 * Exits 1 on tap after second press or key left pressed.
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

#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t second_press_ms = key_record_position_press_ms(B_POSITION, 1);
    mock_check(second_press_ms != KEY_RECORD_ABSENT_MS, "B pressed again");
    mock_check(second_press_ms < key_record_position_press_ms(PICKING_LAYER_POSITION, 1),
               "picking layer still on at second press");
    mock_check(key_record_keycode_ms(KEYCODE_B, true, 1) < second_press_ms,
               "B tapped between presses");
    tap_check_stopped_at(KEYCODE_B, second_press_ms);
    printk("PASS: all %u picking layer stop checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
