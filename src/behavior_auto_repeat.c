// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Hold auto_repeat or turn on a listed layer, then press keys to toggle their timed repeat. */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/hid_usage_pages.h>
#include <dt-bindings/zmk/modifiers.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>
#include <zmk/keys.h>

#include "auto_repeat_node.h"
#include "auto_repeat_slots.h"

LOG_MODULE_REGISTER(behavior_auto_repeat, CONFIG_ZMK_BEHAVIOR_AUTO_REPEAT_LOG_LEVEL);

#define AUTO_REPEAT_DEVICE_NAME(node_id) DEVICE_DT_NAME(node_id),
#define AUTO_REPEAT_LAYER(node_id, prop, index) DT_PROP_BY_IDX(node_id, prop, index),
#define AUTO_REPEAT_LAYER_IN_KEYMAP(node_id, prop, index)                                          \
    BUILD_ASSERT(DT_PROP_BY_IDX(node_id, prop, index) < ZMK_KEYMAP_LAYERS_LEN,                     \
                 "auto-repeat layers entry must name a keymap layer");

static const char *const key_press_names[] = {
    DT_FOREACH_STATUS_OKAY(zmk_behavior_key_press, AUTO_REPEAT_DEVICE_NAME) NULL};
static const char *const mouse_key_press_names[] = {
    DT_FOREACH_STATUS_OKAY(zmk_behavior_mouse_key_press, AUTO_REPEAT_DEVICE_NAME) NULL};
static const char *const transparent_names[] = {
    DT_FOREACH_STATUS_OKAY(zmk_behavior_transparent, AUTO_REPEAT_DEVICE_NAME) NULL};
static const zmk_keymap_layer_id_t picking_layers[] = {
    COND_CODE_1(DT_NODE_HAS_PROP(AUTO_REPEAT_NODE, layers),
                (DT_FOREACH_PROP_ELEM(AUTO_REPEAT_NODE, layers, AUTO_REPEAT_LAYER)), ())
        ZMK_KEYMAP_LAYER_ID_INVAL};
COND_CODE_1(DT_NODE_HAS_PROP(AUTO_REPEAT_NODE, layers),
            (DT_FOREACH_PROP_ELEM(AUTO_REPEAT_NODE, layers, AUTO_REPEAT_LAYER_IN_KEYMAP)), ())

struct pending_modifier {
    struct zmk_behavior_binding binding;
    struct zmk_behavior_binding_event event;
    bool active;
};

static const int64_t tapping_term_ms = AUTO_REPEAT_PROP(tapping_term_ms);
static bool held;
static bool pressed_while_held;
static uint32_t held_position;
static struct pending_modifier pending_modifier;

static bool names_contain(const char *const *names, const char *name) {
    for (size_t index = 0; names[index] != NULL; index++) {
        if (strcmp(names[index], name) == 0) {
            return true;
        }
    }
    return false;
}

static bool binding_uses(const struct zmk_behavior_binding *binding, const char *const *names) {
    return binding->behavior_dev != NULL && names_contain(names, binding->behavior_dev);
}

static bool binding_is_modifier(const struct zmk_behavior_binding *binding) {
    uint8_t usage_page = (uint8_t)ZMK_HID_USAGE_PAGE(binding->param1);
    uint32_t keycode = ZMK_HID_USAGE_ID(binding->param1);
    return binding_uses(binding, key_press_names) && is_mod(usage_page, keycode);
}

static bool binding_is_pickable(const struct zmk_behavior_binding *binding) {
    return binding_uses(binding, key_press_names) || binding_uses(binding, mouse_key_press_names);
}

static const struct zmk_behavior_binding *active_binding_at(uint32_t position,
                                                            zmk_keymap_layer_id_t *layer) {
    for (int index = zmk_keymap_highest_layer_active(); index >= 0; index--) {
        zmk_keymap_layer_index_t layer_index = (zmk_keymap_layer_index_t)index;
        zmk_keymap_layer_id_t layer_id = zmk_keymap_layer_index_to_id(layer_index);
        if (layer_id == ZMK_KEYMAP_LAYER_ID_INVAL || !zmk_keymap_layer_active(layer_id)) {
            continue;
        }
        const struct zmk_behavior_binding *binding =
            zmk_keymap_get_layer_binding_at_idx(layer_id, (uint16_t)position);
        if (binding == NULL || binding_uses(binding, transparent_names)) {
            continue;
        }
        *layer = layer_id;
        return binding;
    }
    return NULL;
}

static uint32_t keycode_with_mods(uint32_t keycode, zmk_mod_flags_t mods) {
    uint32_t modifiers = mods;
    return APPLY_MODS(modifiers, keycode);
}

static struct zmk_behavior_binding
binding_with_held_mods(const struct zmk_behavior_binding *binding) {
    struct zmk_behavior_binding repeat_binding = *binding;
    if (binding_uses(binding, key_press_names)) {
        zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
        repeat_binding.param1 = keycode_with_mods(binding->param1, mods);
    }
    return repeat_binding;
}

static bool picking_layer_active(void) {
    for (size_t index = 0; picking_layers[index] != ZMK_KEYMAP_LAYER_ID_INVAL; index++) {
        if (zmk_keymap_layer_active(picking_layers[index])) {
            return true;
        }
    }
    return false;
}

static void start_repeat(const struct zmk_behavior_binding *binding,
                         struct zmk_behavior_binding_event event) {
    const struct zmk_behavior_binding repeat_binding = binding_with_held_mods(binding);
    int error = auto_repeat_slots_start(&repeat_binding, event);
    if (error == -ENOSPC) {
        LOG_WRN("All %u auto-repeat slots busy", (unsigned int)AUTO_REPEAT_PROP(max_active));
    }
}

static void toggle_pick(const struct zmk_position_state_changed *press) {
    if (auto_repeat_slots_repeating_at(press->position)) {
        auto_repeat_slots_stop_at(press->position);
        return;
    }
    zmk_keymap_layer_id_t layer = 0;
    const struct zmk_behavior_binding *binding = active_binding_at(press->position, &layer);
    if (binding == NULL || !binding_is_pickable(binding)) {
        return;
    }
    /* kp and mkp run on the central whatever source the event names. */
    const struct zmk_behavior_binding_event event = {
        .layer = layer,
        .position = press->position,
        .timestamp = press->timestamp,
    };
    if (binding_is_modifier(binding)) {
        pending_modifier = (struct pending_modifier){
            .binding = *binding,
            .event = event,
            .active = true,
        };
        return;
    }
    start_repeat(binding, event);
}

static void pick_pending_modifier(const struct zmk_position_state_changed *release) {
    if (!pending_modifier.active || pending_modifier.event.position != release->position) {
        return;
    }
    pending_modifier.active = false;
    int64_t held_ms = release->timestamp - pending_modifier.event.timestamp;
    if (held_ms >= tapping_term_ms) {
        return;
    }
    start_repeat(&pending_modifier.binding, pending_modifier.event);
}

static int auto_repeat_listener(const zmk_event_t *event) {
    const struct zmk_position_state_changed *position_event = as_zmk_position_state_changed(event);
    if (position_event == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    if (!position_event->state) {
        pick_pending_modifier(position_event);
        return ZMK_EV_EVENT_BUBBLE;
    }
    if (held && position_event->position == held_position) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    pending_modifier.active = false;
    if (held) {
        pressed_while_held = true;
    }
    if (held || picking_layer_active()) {
        toggle_pick(position_event);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_auto_repeat, auto_repeat_listener);
ZMK_SUBSCRIPTION(behavior_auto_repeat, zmk_position_state_changed);

#define DT_DRV_COMPAT zmk_behavior_auto_repeat

static int on_auto_repeat_pressed(struct zmk_behavior_binding *binding,
                                  struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    held = true;
    pressed_while_held = false;
    held_position = event.position;
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_auto_repeat_released(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    held = false;
    if (!pressed_while_held) {
        auto_repeat_slots_stop_all();
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api auto_repeat_driver_api = {
    .binding_pressed = on_auto_repeat_pressed,
    .binding_released = on_auto_repeat_released,
    IF_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA,
               (.get_parameter_metadata = zmk_behavior_get_empty_param_metadata, ))
};

static int auto_repeat_init(const struct device *device) {
    ARG_UNUSED(device);
    auto_repeat_slots_init();
    return 0;
}

#define AUTO_REPEAT_INST(instance)                                                                 \
    BEHAVIOR_DT_INST_DEFINE(instance, auto_repeat_init, NULL, NULL, NULL, POST_KERNEL,             \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &auto_repeat_driver_api);

DT_INST_FOREACH_STATUS_OKAY(AUTO_REPEAT_INST)
