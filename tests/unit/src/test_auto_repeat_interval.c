// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Interval step and clamp tests, real auto_repeat_interval.c. */

#include <stdint.h>

#include <zephyr/ztest.h>

#include "auto_repeat_interval.h"

#define INTERVAL_MS 50
#define STEP_MS 10
#define PARTIAL_STEP_MS 5
#define MINIMUM_MS 20
#define MAXIMUM_MS 500
#define WIDE_STEP_MS 30
#define BELOW_WIDE_STEP_MS 10

static const struct auto_repeat_interval_limits limits = {
    .step_ms = STEP_MS,
    .minimum_ms = MINIMUM_MS,
    .maximum_ms = MAXIMUM_MS,
};

ZTEST_SUITE(auto_repeat_interval, NULL, NULL, NULL, NULL, NULL);

ZTEST(auto_repeat_interval, test_faster_takes_one_step) {
    zassert_equal(auto_repeat_interval_faster(INTERVAL_MS, &limits), INTERVAL_MS - STEP_MS);
}

ZTEST(auto_repeat_interval, test_faster_stops_at_minimum) {
    zassert_equal(auto_repeat_interval_faster(MINIMUM_MS + PARTIAL_STEP_MS, &limits), MINIMUM_MS);
    zassert_equal(auto_repeat_interval_faster(MINIMUM_MS, &limits), MINIMUM_MS);
}

ZTEST(auto_repeat_interval, test_faster_below_one_step_lands_on_minimum) {
    const struct auto_repeat_interval_limits wide_step = {
        .step_ms = WIDE_STEP_MS,
        .minimum_ms = 1,
        .maximum_ms = MAXIMUM_MS,
    };
    uint32_t interval_ms = auto_repeat_interval_faster(BELOW_WIDE_STEP_MS, &wide_step);
    zassert_equal(interval_ms, wide_step.minimum_ms);
}

ZTEST(auto_repeat_interval, test_slower_takes_one_step) {
    zassert_equal(auto_repeat_interval_slower(INTERVAL_MS, &limits), INTERVAL_MS + STEP_MS);
}

ZTEST(auto_repeat_interval, test_slower_stops_at_maximum) {
    zassert_equal(auto_repeat_interval_slower(MAXIMUM_MS - PARTIAL_STEP_MS, &limits), MAXIMUM_MS);
    zassert_equal(auto_repeat_interval_slower(MAXIMUM_MS, &limits), MAXIMUM_MS);
}

ZTEST(auto_repeat_interval, test_slower_near_type_maximum_stays_at_maximum) {
    const struct auto_repeat_interval_limits unbounded = {
        .step_ms = STEP_MS,
        .minimum_ms = MINIMUM_MS,
        .maximum_ms = UINT32_MAX,
    };
    uint32_t interval_ms = auto_repeat_interval_slower(UINT32_MAX - PARTIAL_STEP_MS, &unbounded);
    zassert_equal(interval_ms, UINT32_MAX);
}
