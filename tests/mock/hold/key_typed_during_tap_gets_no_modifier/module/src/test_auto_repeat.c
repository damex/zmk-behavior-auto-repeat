// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once key typed by hand during timed tap of key picked with shift reaches host
 * without shift.
 * Exits 1 on shift in HID report at that key press.
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

#define VERDICT_MS 300

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t typed_ms = key_record_keycode_ms(KEYCODE_C, true, 0);
    mock_check(typed_ms != KEY_RECORD_ABSENT_MS, "C typed");
    mock_check(key_record_keycode_ms(KEYCODE_A, true, 1) < typed_ms,
               "C typed after first timed tap of A presses");
    mock_check(typed_ms < key_record_keycode_ms(KEYCODE_A, false, 1),
               "C typed before first timed tap of A releases");
    mock_check(!key_record_keycode_has_modifier(KEYCODE_C, true, 0, MOD_LSFT),
               "C reaches host without shift");
    printk("PASS: all %u typed key modifier checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
