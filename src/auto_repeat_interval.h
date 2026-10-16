// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Auto-repeat interval steps, clamped to the configured limits. */
#pragma once

#include <stdint.h>

struct auto_repeat_interval_limits {
    uint32_t step_ms;
    uint32_t minimum_ms;
    uint32_t maximum_ms;
};

uint32_t auto_repeat_interval_faster(uint32_t interval_ms,
                                     const struct auto_repeat_interval_limits *limits);

uint32_t auto_repeat_interval_slower(uint32_t interval_ms,
                                     const struct auto_repeat_interval_limits *limits);
