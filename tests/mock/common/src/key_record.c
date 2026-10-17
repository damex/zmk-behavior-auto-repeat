// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Key position and keycode observer shared by mock cases. */

#include "key_record.h"

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/hid.h>

#define RECORDS_MAX 128

enum record_source {
    RECORD_SOURCE_POSITION,
    RECORD_SOURCE_KEYCODE,
};

struct record {
    enum record_source source;
    uint32_t code;
    bool pressed;
    int64_t uptime_ms;
    zmk_mod_flags_t modifiers;
};

static struct record records[RECORDS_MAX];
static size_t record_count;

static void record_add(enum record_source source, uint32_t code, bool pressed) {
    if (record_count == RECORDS_MAX) {
        printk("FAIL: key record overflowed past %u events\n", (unsigned int)RECORDS_MAX);
        exit(1);
    }
    records[record_count] = (struct record){
        .source = source,
        .code = code,
        .pressed = pressed,
        .uptime_ms = k_uptime_get(),
        .modifiers = zmk_hid_get_keyboard_report()->body.modifiers,
    };
    record_count++;
}

static bool record_matches(const struct record *entry,
                           enum record_source source,
                           uint32_t code,
                           bool pressed) {
    return entry->source == source && entry->code == code && entry->pressed == pressed;
}

static const struct record *record_nth(enum record_source source,
                                       uint32_t code,
                                       bool pressed,
                                       size_t nth) {
    size_t seen = 0;
    for (size_t index = 0; index < record_count; index++) {
        if (!record_matches(&records[index], source, code, pressed)) {
            continue;
        }
        if (seen == nth) {
            return &records[index];
        }
        seen++;
    }
    return NULL;
}

static int64_t record_nth_ms(enum record_source source, uint32_t code, bool pressed, size_t nth) {
    const struct record *entry = record_nth(source, code, pressed, nth);
    if (entry == NULL) {
        return KEY_RECORD_ABSENT_MS;
    }
    return entry->uptime_ms;
}

static int64_t span_ms(int64_t start_ms, int64_t end_ms) {
    if (start_ms == KEY_RECORD_ABSENT_MS || end_ms == KEY_RECORD_ABSENT_MS) {
        return KEY_RECORD_ABSENT_MS;
    }
    return end_ms - start_ms;
}

int64_t key_record_position_press_ms(uint32_t position, size_t nth) {
    return record_nth_ms(RECORD_SOURCE_POSITION, position, true, nth);
}

int64_t key_record_keycode_ms(uint32_t keycode, bool pressed, size_t nth) {
    return record_nth_ms(RECORD_SOURCE_KEYCODE, keycode, pressed, nth);
}

size_t key_record_keycode_count(uint32_t keycode, bool pressed) {
    size_t matches = 0;
    for (size_t index = 0; index < record_count; index++) {
        if (record_matches(&records[index], RECORD_SOURCE_KEYCODE, keycode, pressed)) {
            matches++;
        }
    }
    return matches;
}

int64_t key_record_keycode_gap_ms(uint32_t keycode, size_t nth) {
    if (nth == 0) {
        return KEY_RECORD_ABSENT_MS;
    }
    int64_t previous_ms = key_record_keycode_ms(keycode, true, nth - 1);
    int64_t current_ms = key_record_keycode_ms(keycode, true, nth);
    return span_ms(previous_ms, current_ms);
}

int64_t key_record_keycode_hold_ms(uint32_t keycode, size_t nth) {
    int64_t press_ms = key_record_keycode_ms(keycode, true, nth);
    int64_t release_ms = key_record_keycode_ms(keycode, false, nth);
    return span_ms(press_ms, release_ms);
}

bool key_record_keycode_has_modifier(uint32_t keycode,
                                     bool pressed,
                                     size_t nth,
                                     zmk_mod_flags_t modifier) {
    const struct record *entry = record_nth(RECORD_SOURCE_KEYCODE, keycode, pressed, nth);
    return entry != NULL && (entry->modifiers & modifier) == modifier;
}

static int key_record_listener(const zmk_event_t *event) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(event);
    if (position != NULL) {
        record_add(RECORD_SOURCE_POSITION, position->position, position->state);
        return ZMK_EV_EVENT_BUBBLE;
    }
    const struct zmk_keycode_state_changed *keycode = as_zmk_keycode_state_changed(event);
    if (keycode != NULL) {
        record_add(RECORD_SOURCE_KEYCODE, keycode->keycode, keycode->state);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(key_record, key_record_listener);
ZMK_SUBSCRIPTION(key_record, zmk_position_state_changed);
ZMK_SUBSCRIPTION(key_record, zmk_keycode_state_changed);
