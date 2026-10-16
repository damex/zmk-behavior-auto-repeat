// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Step shared auto-repeat interval faster or slower. */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/auto_repeat.h>
#include <zmk/behavior.h>

#include "auto_repeat_slots.h"

LOG_MODULE_DECLARE(behavior_auto_repeat, CONFIG_ZMK_BEHAVIOR_AUTO_REPEAT_LOG_LEVEL);

#define DT_DRV_COMPAT zmk_behavior_auto_repeat_speed

IF_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA, (
/* display_name lacks const, so names live in writable arrays. */
static char faster_name[] = "Faster";
static char slower_name[] = "Slower";

static const struct behavior_parameter_value_metadata speed_command_values[] = {
    {
        .display_name = faster_name,
        .value = AUTO_REPEAT_FASTER,
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
    },
    {
        .display_name = slower_name,
        .value = AUTO_REPEAT_SLOWER,
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
    },
};

static const struct behavior_parameter_metadata_set speed_command_set = {
    .param1_values = speed_command_values,
    .param1_values_len = ARRAY_SIZE(speed_command_values),
};

static const struct behavior_parameter_metadata speed_metadata = {
    .sets_len = 1,
    .sets = &speed_command_set,
};
))

static int on_auto_repeat_speed_pressed(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    switch (binding->param1) {
    case AUTO_REPEAT_FASTER:
        auto_repeat_slots_faster();
        return ZMK_BEHAVIOR_OPAQUE;
    case AUTO_REPEAT_SLOWER:
        auto_repeat_slots_slower();
        return ZMK_BEHAVIOR_OPAQUE;
    default:
        LOG_ERR("Unknown auto-repeat speed command %u", (unsigned int)binding->param1);
        return -ENOTSUP;
    }
}

static int on_auto_repeat_speed_released(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api auto_repeat_speed_driver_api = {
    .binding_pressed = on_auto_repeat_speed_pressed,
    .binding_released = on_auto_repeat_speed_released,
    IF_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA, (.parameter_metadata = &speed_metadata, ))
};

#define AUTO_REPEAT_SPEED_INST(instance)                                                           \
    BEHAVIOR_DT_INST_DEFINE(instance, NULL, NULL, NULL, NULL, POST_KERNEL,                         \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &auto_repeat_speed_driver_api);

DT_INST_FOREACH_STATUS_OKAY(AUTO_REPEAT_SPEED_INST)
