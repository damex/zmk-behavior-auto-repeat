// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Fake split transport feeding central the auto_repeat hold and key press of peripheral half,
 * real auto-repeat module.
 * Exits 0 once the peripheral key types on its press, then taps on the central every
 * interval-ms, each tap held tap-ms.
 * Exits 1 on a refused peripheral event, a missing first press, a wrong gap or hold, or a
 * missing tap.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zmk/split/transport/central.h>
#include <zmk/split/transport/types.h>

#include "auto_repeat_keymap.h"
#include "key_record.h"
#include "mock.h"
#include "tap_check.h"

#define PERIPHERAL_SOURCE 0
#define FAKE_TRANSPORT_PRIORITY 0
#define FIRST_EVENT_MS 20
#define EVENT_GAP_MS 6
#define TIMED_TAPS 4
#define VERDICT_MS 400

struct peripheral_step {
    uint8_t position;
    bool pressed;
};

static const struct peripheral_step peripheral_steps[] = {
    {.position = AUTO_REPEAT_POSITION, .pressed = true},
    {.position = A_POSITION, .pressed = true},
    {.position = A_POSITION, .pressed = false},
    {.position = AUTO_REPEAT_POSITION, .pressed = false},
};
static size_t steps_sent;

static int fake_send_command(uint8_t source, struct zmk_split_transport_central_command command) {
    ARG_UNUSED(source);
    ARG_UNUSED(command);
    return 0;
}

static int fake_get_available_source_ids(uint8_t *sources) {
    ARG_UNUSED(sources);
    return 0;
}

static int fake_set_enabled(bool enabled) {
    ARG_UNUSED(enabled);
    return 0;
}

static struct zmk_split_transport_status fake_get_status(void) {
    return (struct zmk_split_transport_status){
        .available = true,
        .enabled = true,
        .connections = ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_ALL_CONNECTED,
    };
}

static int fake_set_status_callback(zmk_split_transport_central_status_changed_cb_t callback) {
    ARG_UNUSED(callback);
    return 0;
}

static const struct zmk_split_transport_central_api fake_central_api = {
    .send_command = fake_send_command,
    .get_available_source_ids = fake_get_available_source_ids,
    .set_enabled = fake_set_enabled,
    .get_status = fake_get_status,
    .set_status_callback = fake_set_status_callback,
};

ZMK_SPLIT_TRANSPORT_CENTRAL_REGISTER(fake_central, &fake_central_api, FAKE_TRANSPORT_PRIORITY);

static void send_step(const struct peripheral_step *step) {
    struct zmk_split_transport_peripheral_event event = {
        .type = ZMK_SPLIT_TRANSPORT_PERIPHERAL_EVENT_TYPE_KEY_POSITION_EVENT,
        .data.key_position_event =
            {
                .position = step->position,
                .pressed = step->pressed,
            },
    };
    int error = zmk_split_transport_central_peripheral_event_handler(&fake_central,
                                                                     PERIPHERAL_SOURCE,
                                                                     event);
    mock_check(error == 0, "central takes the peripheral key event");
}

static void step_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(step_work, step_handler);

static void step_handler(struct k_work *work) {
    ARG_UNUSED(work);
    send_step(&peripheral_steps[steps_sent]);
    steps_sent++;
    if (steps_sent < ARRAY_SIZE(peripheral_steps)) {
        k_work_reschedule(&step_work, K_MSEC(EVENT_GAP_MS));
    }
}

static void verdict_handler(struct k_work *work) {
    ARG_UNUSED(work);
    int64_t pick_ms = key_record_position_press_ms(A_POSITION, 0);
    mock_check(pick_ms != KEY_RECORD_ABSENT_MS, "peripheral A press reached the keymap");
    mock_check(key_record_keycode_ms(KEYCODE_A, true, 0) == pick_ms, "picking press types A");
    tap_check_gaps(KEYCODE_A, 1, TIMED_TAPS, INTERVAL_MS);
    tap_check_holds(KEYCODE_A, 1, TIMED_TAPS, TAP_MS);
    printk("PASS: all %u peripheral pick checks\n", (unsigned int)mock_checks_passed());
    exit(0);
}
static K_WORK_DELAYABLE_DEFINE(verdict_work, verdict_handler);

static int test_auto_repeat_init(void) {
    k_work_reschedule(&step_work, K_MSEC(FIRST_EVENT_MS));
    k_work_reschedule(&verdict_work, K_MSEC(VERDICT_MS));
    return 0;
}
SYS_INIT(test_auto_repeat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
