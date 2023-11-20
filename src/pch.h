// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

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

#include <charls/charls.h>

#pragma warning(push)
#pragma warning(disable : 5262) // implicit fall-through occurs here;
#include <string>
#include <span>
#pragma warning(pop)
