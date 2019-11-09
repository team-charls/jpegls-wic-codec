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

#include <charls/charls.h>

#include <string>
