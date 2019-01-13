// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#define WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable : x))  // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses)
#define WARNING_UNSUPPRESS() __pragma(warning(pop))
