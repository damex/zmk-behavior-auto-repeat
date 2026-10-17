# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20.0)

project(auto_repeat_unit_test)
find_package(Zephyr COMPONENTS unittest HINTS $ENV{ZEPHYR_BASE})

# Zephyr unittest forces -m32, which needs 32-bit glibc on host.
string(REPLACE "-m32" "-m64" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-m32" "-m64" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(auto_repeat_root ${CMAKE_CURRENT_SOURCE_DIR}/../..)

target_include_directories(testbinary PRIVATE
  ${auto_repeat_root}/src
)

target_sources(testbinary PRIVATE
  src/test_auto_repeat_interval.c
  ${auto_repeat_root}/src/auto_repeat_interval.c
)

set(auto_repeat_strict_flags
  -Wall
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

set_source_files_properties(${auto_repeat_root}/src/auto_repeat_interval.c
  PROPERTIES COMPILE_OPTIONS "${auto_repeat_strict_flags}"
)

target_compile_options(testbinary PRIVATE
  -fsanitize=address,undefined
  -fno-sanitize-recover=all
)

target_link_options(testbinary PRIVATE
  -fsanitize=address,undefined
)
