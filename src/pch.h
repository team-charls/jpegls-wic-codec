// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once


#include <sdkddkver.h> // Defines the highest available Windows platform.

#pragma warning(push)
#pragma warning(disable : 5105) // macro expansion producing 'defined' has undefined behavior
#include <olectl.h>
#include <ShlObj.h>
#include <Unknwn.h>
#pragma warning(pop)

#include <wincodec.h>

#pragma warning(push)
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#include <mfapi.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#pragma warning(disable : 5246) // The initialization of a sub-object should be wrapped in braces
#pragma warning(disable : 5262) // implicit fall-through occurs here;
#include <winrt/base.h>
#pragma warning(pop)

#include <charls/charls.h>

#pragma warning(push)
#pragma warning(disable : 5262) // implicit fall-through occurs here;
#include <string>
#include <span>
#pragma warning(pop)
