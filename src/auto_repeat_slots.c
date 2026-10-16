// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Pool of repeating keys, their timed taps and the one interval they share. */

#include "auto_repeat_slots.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <zmk/behavior.h>

#include "auto_repeat_interval.h"
#include "auto_repeat_node.h"

LOG_MODULE_DECLARE(behavior_auto_repeat, CONFIG_ZMK_BEHAVIOR_AUTO_REPEAT_LOG_LEVEL);

BUILD_ASSERT(AUTO_REPEAT_PROP(interval_min_ms) > AUTO_REPEAT_PROP(tap_ms),
             "interval-min-ms must exceed tap-ms");
BUILD_ASSERT(AUTO_REPEAT_PROP(interval_ms) >= AUTO_REPEAT_PROP(interval_min_ms),
             "interval-ms must not be below interval-min-ms");
BUILD_ASSERT(AUTO_REPEAT_PROP(interval_ms) <= AUTO_REPEAT_PROP(interval_max_ms),
             "interval-ms must not exceed interval-max-ms");
BUILD_ASSERT(AUTO_REPEAT_PROP(interval_step_ms) > 0, "interval-step-ms must be at least 1");
BUILD_ASSERT(AUTO_REPEAT_PROP(max_active) > 0, "max-active must be at least 1");

static const uint32_t tap_ms = AUTO_REPEAT_PROP(tap_ms);
static const struct auto_repeat_interval_limits interval_limits = {
    .step_ms = AUTO_REPEAT_PROP(interval_step_ms),
    .minimum_ms = AUTO_REPEAT_PROP(interval_min_ms),
    .maximum_ms = AUTO_REPEAT_PROP(interval_max_ms),
};

struct auto_repeat_slot {
    struct k_work_delayable press_work;
    struct k_work_delayable release_work;
    struct zmk_behavior_binding binding;
    struct zmk_behavior_binding_event event;
    int64_t last_press_ms;
    bool active;
    bool binding_pressed;
};

static struct auto_repeat_slot slots[AUTO_REPEAT_PROP(max_active)];
static uint32_t current_interval_ms = AUTO_REPEAT_PROP(interval_ms);

static void invoke_binding(const struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event,
                           bool pressed) {
    event.timestamp = k_uptime_get();
    int error = zmk_behavior_invoke_binding(binding, event, pressed);
    if (error < 0) {
        LOG_ERR("Auto-repeat binding %s returned %d", binding->behavior_dev, error);
    }
}

static void schedule_slot_press(struct auto_repeat_slot *slot) {
    int64_t next_press_ms = slot->last_press_ms + current_interval_ms;
    k_work_reschedule(&slot->press_work, K_TIMEOUT_ABS_MS(next_press_ms));
}

static void release_slot_tap(struct auto_repeat_slot *slot) {
    if (!slot->binding_pressed) {
        return;
    }
    slot->binding_pressed = false;
    invoke_binding(&slot->binding, slot->event, false);
}

static void press_slot_tap(struct auto_repeat_slot *slot) {
    slot->last_press_ms = k_uptime_get();
    slot->binding_pressed = true;
    invoke_binding(&slot->binding, slot->event, true);
    int64_t release_ms = slot->last_press_ms + tap_ms;
    k_work_reschedule(&slot->release_work, K_TIMEOUT_ABS_MS(release_ms));
    schedule_slot_press(slot);
}

static void press_work_handler(struct k_work *work) {
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    press_slot_tap(CONTAINER_OF(delayable, struct auto_repeat_slot, press_work));
}

static void release_work_handler(struct k_work *work) {
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    release_slot_tap(CONTAINER_OF(delayable, struct auto_repeat_slot, release_work));
}

static void stop_slot(struct auto_repeat_slot *slot) {
    slot->active = false;
    (void)k_work_cancel_delayable(&slot->press_work);
    (void)k_work_cancel_delayable(&slot->release_work);
    release_slot_tap(slot);
}

static struct auto_repeat_slot *slot_at(uint32_t position) {
    for (size_t index = 0; index < ARRAY_SIZE(slots); index++) {
        if (slots[index].active && slots[index].event.position == position) {
            return &slots[index];
        }
    }
    return NULL;
}

static struct auto_repeat_slot *free_slot(void) {
    for (size_t index = 0; index < ARRAY_SIZE(slots); index++) {
        if (!slots[index].active) {
            return &slots[index];
        }
    }
    return NULL;
}

static void set_interval(uint32_t interval_ms) {
    current_interval_ms = interval_ms;
    for (size_t index = 0; index < ARRAY_SIZE(slots); index++) {
        if (slots[index].active) {
            schedule_slot_press(&slots[index]);
        }
    }
}

void auto_repeat_slots_init(void) {
    for (size_t index = 0; index < ARRAY_SIZE(slots); index++) {
        k_work_init_delayable(&slots[index].press_work, press_work_handler);
        k_work_init_delayable(&slots[index].release_work, release_work_handler);
    }
}

bool auto_repeat_slots_repeating_at(uint32_t position) {
    return slot_at(position) != NULL;
}

int auto_repeat_slots_start(const struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    __ASSERT_NO_MSG(binding != NULL);
    struct auto_repeat_slot *slot = free_slot();
    if (slot == NULL) {
        return -ENOSPC;
    }
    slot->binding = *binding;
    slot->event = event;
    slot->last_press_ms = k_uptime_get();
    slot->binding_pressed = false;
    slot->active = true;
    schedule_slot_press(slot);
    return 0;
}

void auto_repeat_slots_stop_at(uint32_t position) {
    struct auto_repeat_slot *slot = slot_at(position);
    if (slot != NULL) {
        stop_slot(slot);
    }
}

void auto_repeat_slots_stop_all(void) {
    for (size_t index = 0; index < ARRAY_SIZE(slots); index++) {
        if (slots[index].active) {
            stop_slot(&slots[index]);
        }
    }
}

void auto_repeat_slots_faster(void) {
    uint32_t interval_ms = auto_repeat_interval_faster(current_interval_ms, &interval_limits);
    set_interval(interval_ms);
}

void auto_repeat_slots_slower(void) {
    uint32_t interval_ms = auto_repeat_interval_slower(current_interval_ms, &interval_limits);
    set_interval(interval_ms);
}
