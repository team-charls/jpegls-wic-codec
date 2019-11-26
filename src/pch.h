// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <SDKDDKVer.h> // Defines the highest available Windows platform.

#include <Shlobj.h>
#include <olectl.h>
#include <unknwn.h>
#include <wincodec.h>
#include <mfapi.h>

#pragma warning(push)
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#include <winrt/base.h>
#pragma warning(pop)

#include <charls/charls.h>

#include <string>
