// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Single auto_repeat devicetree node and its properties. */
#pragma once

#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#include <zephyr/toolchain.h>

#define AUTO_REPEAT_HAS_NODE DT_HAS_COMPAT_STATUS_OKAY(zmk_behavior_auto_repeat)
#define AUTO_REPEAT_NODE DT_INST(0, zmk_behavior_auto_repeat)
#define AUTO_REPEAT_PROP(prop)                                                                     \
    COND_CODE_1(AUTO_REPEAT_HAS_NODE, (DT_PROP(AUTO_REPEAT_NODE, prop)), (1))

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(zmk_behavior_auto_repeat) == 1,
             "auto-repeat needs one auto_repeat node, include behaviors/auto_repeat.dtsi");
