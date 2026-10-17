// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/* Verdict helpers shared by mock cases. */

#include "mock.h"

#include <stdlib.h>

#include <zephyr/sys/printk.h>

static size_t checks_passed;

void mock_check(bool passed, const char *what) {
    if (!passed) {
        printk("FAIL: %s\n", what);
        exit(1);
    }
    checks_passed++;
}

size_t mock_checks_passed(void) {
    return checks_passed;
}
