# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

set(mock_common ${CMAKE_CURRENT_LIST_DIR})

zephyr_library_named(zmk_behavior_auto_repeat_test)
zephyr_library_include_directories(
  ${mock_common}/include
  ${mock_common}/../../../include
  ${APPLICATION_SOURCE_DIR}/include
)
