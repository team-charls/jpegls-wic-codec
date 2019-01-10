// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <Windows.h>

#ifdef _DEBUG

template <typename... Args>
void TRACE(char const* const message, Args... args) noexcept
{
    char buffer[1024];
    static_cast<void>(snprintf(buffer, sizeof(buffer), message, args...));
    OutputDebugStringA(buffer);
}

#else
#define TRACE __noop
#endif
