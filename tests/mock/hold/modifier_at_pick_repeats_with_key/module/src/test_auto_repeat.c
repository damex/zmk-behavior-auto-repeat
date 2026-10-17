// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once key picked with shift held carries shift in HID report at every timed tap
 * after shift release.
 * Exits 1 on timed tap without shift.
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

#define TIMED_TAPS 3
#define VERDICT_MS 400

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    mock_check(key_record_keycode_ms(KEYCODE_LEFT_SHIFT, false, 0) <
                   key_record_keycode_ms(KEYCODE_A, true, 1),
               "shift released before the first timed tap");
    for (size_t tap = 1; tap <= TIMED_TAPS; tap++) {
        mock_check(key_record_keycode_ms(KEYCODE_A, true, tap) != KEY_RECORD_ABSENT_MS,
                   "A tapped");
        mock_check(key_record_keycode_has_modifier(KEYCODE_A, true, tap, MOD_LSFT),
                   "report carries shift at each timed tap");
    }
    printk("PASS: all %u modifier checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
