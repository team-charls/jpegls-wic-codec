// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <Windows.h>

#ifdef NDEBUG

#define TRACE __noop

#else

template <typename... Args>
void trace(char const* const message, Args... args) noexcept
{
    char buffer[1024];
    static_cast<void>(snprintf(buffer, sizeof buffer, message, args...));
    OutputDebugStringA(buffer);
}

#define TRACE trace

#endif
