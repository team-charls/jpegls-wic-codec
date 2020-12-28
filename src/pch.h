// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <sdkddkver.h> // Defines the highest available Windows platform.

#include <OleCtl.h>
#include <ShlObj.h>
#include <Unknwn.h>

#include "wic_bitmap_source.h"
#include <wincodec.h>

#pragma warning(push)
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#include <mfapi.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#include <winrt/base.h>
#pragma warning(pop)

#include <charls/charls.h>

#include <string>
