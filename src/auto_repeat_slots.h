// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Pool of repeating keys, their timed taps and the one interval they share. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/toolchain.h>

#include <zmk/behavior.h>

void auto_repeat_slots_init(void);

bool auto_repeat_slots_repeating_at(uint32_t position);

/* First timed tap lands one interval after call.
 * Returns -ENOSPC once every slot repeats. */
__must_check int auto_repeat_slots_start(const struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event);

void auto_repeat_slots_stop_at(uint32_t position);

void auto_repeat_slots_stop_all(void);

void auto_repeat_slots_faster(void);

void auto_repeat_slots_slower(void);
