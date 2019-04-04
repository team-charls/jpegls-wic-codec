// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <SDKDDKVer.h> // Defines the highest available Windows platform.

#include <olectl.h>
#include <Shlobj.h>
#include <unknwn.h>
#include <wincodec.h>

#pragma warning(push)
#pragma warning(disable: 4946) // reinterpret_cast used between related classes
#include <winrt/base.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 26492) // Don't use const cast
#include <charls/jpegls_decoder.h>
#include <charls/jpegls_encoder.h>
#pragma warning(pop)

#include <string>
