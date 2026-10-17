// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Verdict helpers shared by mock cases. */
#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Exits 1 printing what unless passed. */
void mock_check(bool passed, const char *what);

size_t mock_checks_passed(void);
