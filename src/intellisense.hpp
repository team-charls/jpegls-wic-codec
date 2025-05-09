// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// Include explicit headers as workaround that IntelliSense (VS 2022 17.13)
// and ReSharper (2025.1) fail to parse #import <win.hpp>
#if defined(__INTELLISENSE__) || defined(__RESHARPER__)
// ReSharper disable once CppInconsistentNaming
#define _AMD64_
#include <combaseapi.h>
#include <libloaderapi.h>
#include <sal.h>
#include <wincodec.h>
#include <winreg.h>
#include <propkey.h>
#include "macros.hpp"
#endif
