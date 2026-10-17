// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once shift held alone past tapping-term-ms while auto_repeat is held stays unpicked.
 * Exits 1 on timed tap of shift.
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

#define VERDICT_MS 500

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t press_ms = key_record_keycode_ms(KEYCODE_LEFT_SHIFT, true, 0);
    int64_t release_ms = key_record_keycode_ms(KEYCODE_LEFT_SHIFT, false, 0);
    mock_check(press_ms != KEY_RECORD_ABSENT_MS, "shift pressed while auto_repeat is held");
    mock_check(release_ms - press_ms > TAPPING_TERM_MS, "shift held past tapping-term-ms");
    mock_check(key_record_keycode_count(KEYCODE_LEFT_SHIFT, true) == 1, "shift pressed once");
    printk("PASS: all %u long modifier hold checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
