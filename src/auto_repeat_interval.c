// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Auto-repeat interval steps, clamped to the configured limits. */

#include "auto_repeat_interval.h"

#include <stddef.h>

#include <zephyr/sys/__assert.h>
#include <zephyr/sys/util.h>

uint32_t auto_repeat_interval_faster(uint32_t interval_ms,
                                     const struct auto_repeat_interval_limits *limits) {
    __ASSERT_NO_MSG(limits != NULL);
    uint32_t shorter_ms = 0;
    if (interval_ms > limits->step_ms) {
        shorter_ms = interval_ms - limits->step_ms;
    }
    return CLAMP(shorter_ms, limits->minimum_ms, limits->maximum_ms);
}

uint32_t auto_repeat_interval_slower(uint32_t interval_ms,
                                     const struct auto_repeat_interval_limits *limits) {
    __ASSERT_NO_MSG(limits != NULL);
    uint32_t longer_ms = UINT32_MAX;
    if (interval_ms <= UINT32_MAX - limits->step_ms) {
        longer_ms = interval_ms + limits->step_ms;
    }
    return CLAMP(longer_ms, limits->minimum_ms, limits->maximum_ms);
}
