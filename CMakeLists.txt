# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_BEHAVIOR_AUTO_REPEAT)
  zephyr_library_named(zmk_behavior_auto_repeat)
  zephyr_library_include_directories(include)
  target_include_directories(${ZEPHYR_CURRENT_LIBRARY} SYSTEM PRIVATE
      ${APPLICATION_SOURCE_DIR}/include
  )
  zephyr_library_compile_options(
      -Werror
      -Wextra
      -Wshadow
      -Wformat=2
      -Wformat-truncation=2
      -Wformat-overflow=2
      -Wformat-signedness
      -Wlogical-op
      -Wduplicated-cond
      -Wduplicated-branches
      -Wmissing-prototypes
      -Wmissing-declarations
      -Wjump-misses-init
      -Wwrite-strings
      -Winit-self
      -Wold-style-definition
      -Wswitch-enum
      -Wswitch-default
      -Wimplicit-fallthrough=5
      -Wnull-dereference
      -Wvla
      -Walloca
      -Wfloat-equal
      -Wshift-overflow=2
      -Wstringop-overflow=4
      -Warray-bounds=2
      -Wstrict-overflow=2
      -Wtrampolines
      -Wstack-usage=256
      -fanalyzer
  )
  zephyr_library_sources(
      src/auto_repeat_interval.c
      src/auto_repeat_slots.c
      src/behavior_auto_repeat.c
      src/behavior_auto_repeat_speed.c
  )
endif()
