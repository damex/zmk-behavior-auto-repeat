# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

PYTHON ?= python3
ZEPHYR_BASE ?= $(error ZEPHYR_BASE is not set)
ZEPHYR_TOOLCHAIN_VARIANT ?= host
ZMK_APP ?= $(error ZMK_APP is not set)

MOCK_ROOT := $(CURDIR)/tests/mock
MOCK_CASES += \
	hold/full_slots_refuse_another_key \
	hold/key_typed_during_tap_gets_no_modifier \
	hold/keys_after_release_type_normally \
	hold/modifier_at_pick_repeats_with_key \
	hold/modifier_held_alone_past_term_is_not_picked \
	hold/modifier_held_for_key_is_not_picked \
	hold/modifier_tapped_alone_repeats \
	hold/mouse_button_repeats_at_interval \
	hold/picked_key_again_stops_it \
	hold/picked_key_repeats_at_interval \
	hold/repeat_keeps_its_layer_binding \
	hold/tap_alone_stops_all \
	layer/key_on_picking_layer_repeats \
	layer/second_press_on_picking_layer_stops_key \
	metadata/bindings_pass_validation \
	metadata/unknown_speed_command_fails_validation \
	speed/encoder_turn_changes_speed \
	speed/faster_past_due_taps_at_once \
	speed/faster_shortens_interval \
	speed/faster_stops_at_min_interval \
	speed/slower_lengthens_interval \
	speed/slower_stops_at_max_interval \
	speed/speed_change_reaches_every_key \
	speed/unknown_command_keeps_interval \
	split_central/peripheral_key_picked_repeats
MOCK_TARGETS := $(addprefix mock-test-,$(MOCK_CASES))

.PHONY: unit-test mock-test $(MOCK_TARGETS)
unit-test:
	ZEPHYR_TOOLCHAIN_VARIANT=$(ZEPHYR_TOOLCHAIN_VARIANT) $(PYTHON) $(ZEPHYR_BASE)/scripts/twister --testsuite-root tests/unit --platform unit_testing --inline-logs --clobber-output

mock-test: $(MOCK_TARGETS)

define mock_case_rule
mock-test-$(1):
	cd $$(ZMK_APP) && ZEPHYR_TOOLCHAIN_VARIANT=$$(ZEPHYR_TOOLCHAIN_VARIANT) west build -p -d $$(CURDIR)/mock-test-out/$(1) -b native_sim//zmk_test_mock -- -DZMK_CONFIG=$$(MOCK_ROOT)/$(1) -DZMK_EXTRA_MODULES="$$(CURDIR);$$(MOCK_ROOT)/$(1)/module"
	mock-test-out/$(1)/zephyr/zmk.exe
endef

$(foreach case,$(MOCK_CASES),$(eval $(call mock_case_rule,$(case))))
