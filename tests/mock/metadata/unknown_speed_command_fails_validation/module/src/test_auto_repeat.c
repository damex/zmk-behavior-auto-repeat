// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap with behavior metadata, real auto-repeat module.
 * Exits 0 once speed binding with unknown command fails ZMK binding validation.
 * Exits 1 on accepted unknown command.
 */

#include <stdint.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/auto_repeat.h>
#include <zmk/behavior.h>

#include "mock.h"

#define UNKNOWN_SPEED_COMMAND (AUTO_REPEAT_SLOWER + 1)
#define VERDICT_MS 100

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(auto_repeat_speed)),
        .param1 = UNKNOWN_SPEED_COMMAND,
    };
    mock_check(zmk_behavior_validate_binding(&binding) < 0, "unknown speed command rejected");
    printk("PASS: all %u unknown command validation checks\n",
           (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
