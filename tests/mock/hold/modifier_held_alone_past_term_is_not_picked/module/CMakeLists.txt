# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

include(${CMAKE_CURRENT_LIST_DIR}/../../../common/mock.cmake)

zephyr_library_sources(
  src/test_auto_repeat.c
  ${mock_common}/src/key_record.c
  ${mock_common}/src/mock.c
  ${mock_common}/src/tap_check.c
)
