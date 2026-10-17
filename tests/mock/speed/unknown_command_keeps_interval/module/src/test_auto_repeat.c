// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once unknown speed command returns -ENOTSUP and picked key keeps interval-ms.
 * Exits 1 on other return or changed gap.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <dt-bindings/zmk/auto_repeat.h>
#include <zmk/behavior.h>

#include "auto_repeat_keymap.h"
#include "key_record.h"
#include "mock.h"
#include "tap_check.h"

#define UNKNOWN_SPEED_COMMAND (AUTO_REPEAT_SLOWER + 1)
#define TIMED_TAPS 3
#define VERDICT_MS 400

static int unknown_command_status;

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    mock_check(unknown_command_status == -ENOTSUP, "unknown speed command returns -ENOTSUP");
    mock_check(key_record_position_press_ms(A_POSITION, 0) != KEY_RECORD_ABSENT_MS, "A picked");
    tap_check_gaps(KEYCODE_A, 1, TIMED_TAPS, INTERVAL_MS);
    printk("PASS: all %u unknown command checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(auto_repeat_speed)),
        .param1 = UNKNOWN_SPEED_COMMAND,
    };
    const struct zmk_behavior_binding_event event = {
        .timestamp = k_uptime_get(),
    };
    unknown_command_status = zmk_behavior_invoke_binding(&binding, event, true);
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
