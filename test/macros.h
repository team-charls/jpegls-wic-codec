// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#define WARNING_SUPPRESS_NEXT_LINE(x) \
    __pragma(warning(suppress \
                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

// Include additional headers as workaround that IntelliSense in VS 2020 17.9 fails to parse #import <win.h>
#ifdef __INTELLISENSE__
#define _AMD64_
#include <winerror.h>
#include <libloaderapi.h>
#include <combaseapi.h>
#endif
