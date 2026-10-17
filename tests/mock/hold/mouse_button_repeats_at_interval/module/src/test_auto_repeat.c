// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Mock case on shared keymap, real auto-repeat module.
 * Exits 0 once a mouse button picked while auto_repeat is held clicks on that press, then
 * every interval-ms, each click held tap-ms.
 * Exits 1 on a missing first click, a wrong gap or hold, or a missing click.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "auto_repeat_keymap.h"
#include "key_record.h"
#include "mock.h"

#define TIMED_CLICKS 3
#define BUTTON_RECORDS_MAX 32
#define BUTTON_ABSENT_MS (-1)
#define VERDICT_MS 400

struct button_record {
    bool pressed;
    int64_t uptime_ms;
};

static struct button_record button_records[BUTTON_RECORDS_MAX];
static size_t button_record_count;

static void on_mouse_button_input(struct input_event *event, void *user_data) {
    ARG_UNUSED(user_data);
    if (event->type != INPUT_EV_KEY || event->code != INPUT_BTN_0) {
        return;
    }
    if (button_record_count == BUTTON_RECORDS_MAX) {
        printk("FAIL: button record overflowed past %u events\n", (unsigned int)BUTTON_RECORDS_MAX);
        exit(1);
    }
    button_records[button_record_count] = (struct button_record){
        .pressed = event->value != 0,
        .uptime_ms = k_uptime_get(),
    };
    button_record_count++;
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(mkp)), on_mouse_button_input, NULL);

static int64_t button_ms(bool pressed, size_t nth) {
    size_t seen = 0;
    for (size_t index = 0; index < button_record_count; index++) {
        if (button_records[index].pressed != pressed) {
            continue;
        }
        if (seen == nth) {
            return button_records[index].uptime_ms;
        }
        seen++;
    }
    return BUTTON_ABSENT_MS;
}

static void check_timed_clicks(void) {
    for (size_t click = 1; click <= TIMED_CLICKS; click++) {
        int64_t press_ms = button_ms(true, click);
        mock_check(press_ms != BUTTON_ABSENT_MS, "button clicked");
        mock_check(press_ms - button_ms(true, click - 1) == INTERVAL_MS,
                   "clicks land interval-ms apart");
        mock_check(button_ms(false, click) - press_ms == TAP_MS, "each click holds tap-ms");
    }
}

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t pick_ms = key_record_position_press_ms(CLICK_POSITION, 0);
    mock_check(pick_ms != KEY_RECORD_ABSENT_MS, "button picked while auto_repeat is held");
    mock_check(button_ms(true, 0) == pick_ms, "picking press clicks");
    check_timed_clicks();
    printk("PASS: all %u mouse button checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
