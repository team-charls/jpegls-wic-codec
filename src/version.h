// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

// Turn A into a string literal without expanding macro definitions
// (however, if invoked from a macro, macro arguments are expanded).
#define TO_STRING_NX(A) L#A

// Turn A into a string literal after macro-expanding it.
#define TO_STRING(A) TO_STRING_NX(A)

#define VERSION TO_STRING(VERSION_MAJOR) L"." TO_STRING(VERSION_MINOR) L"." TO_STRING(VERSION_PATCH) L".0"
