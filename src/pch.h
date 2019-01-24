// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <SDKDDKVer.h> // Defines the highest available Windows platform.

#include <olectl.h>
#include <Shlobj.h>
#include <unknwn.h>

#pragma warning(push)
#pragma warning(disable: 4777) // format string '%08x' requires an argument of type 'unsigned int', but variadic argument (solved in SDK 10.0.17763.0)
#pragma warning(disable: 4946) // reinterpret_cast used between related classes
#include <winrt/base.h>
#pragma warning(pop)

#include <wincodec.h>

#pragma warning(push)
#pragma warning(disable: 26492) // Don't use const cast
#include <charls/jpegls_decoder.h>
#pragma warning(pop)
