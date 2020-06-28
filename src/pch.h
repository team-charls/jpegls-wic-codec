// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <sdkddkver.h> // Defines the highest available Windows platform.

#include <ShlObj.h>
#include <OleCtl.h>
#include <Unknwn.h>
#include <wincodec.h>

#pragma warning(push)
#pragma warning(disable : 4619) // #pragma warning : there is no warning number '5204' (older MSVC compiler versions don't know 5204)
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#include <mfapi.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4619) // #pragma warning : there is no warning number '5204'  (older MSVC compiler versions don't know 5204)
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#include <winrt/base.h>
#pragma warning(pop)

#include <charls/charls.h>

#include <string>
