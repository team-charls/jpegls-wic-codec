// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <SDKDDKVer.h> // Defines the highest available Windows platform.

#pragma warning(push)
#pragma warning(disable : 5105) // macro expansion producing 'defined' has undefined behavior
#include <Shlwapi.h>
#include <Windows.h>
#include <wincodec.h>
#pragma warning(pop)

#include <unknwn.h>

#pragma warning(push)
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#pragma warning(disable : 5246) // The initialization of a sub-object should be wrapped in braces
#pragma warning(disable : 5262) // implicit fall-through occurs here;
#include <winrt/base.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 26432) // If you define or delete any default operation in the type 'struct
                                 // Microsoft::VisualStudio::CppUnitTestFramework::TestClassImpl::CrtHandlersSetter', define
                                 // or delete them all (c.21).
#pragma warning(disable : 26433) // Function should be marked with 'override'
#pragma warning(disable : 26440) // Function can be declared 'noexcept' (f.6).
#pragma warning(disable : 26443) // Overriding destructor should not use explicit 'override' or 'virtual' specifiers (c.128).
#pragma warning(disable : 26455) // Default constructor may not throw. [Problem in VS 2017 15.8.0]
#pragma warning(disable : 26461) // the pointer argument can be marked as a pointer to const
#pragma warning(disable : 26477) // Use 'nullptr' rather than 0 or NULL (es.47) [Problem in VS 2017 15.8.0]
#pragma warning(disable : 26493) // Don't use C-style casts
#pragma warning(disable : 26495) // Variable is uninitialized. Always initialize a member variable (type.6).
#pragma warning(disable : 26496) // The variable 'diff' is assigned only once, mark it as const (con.4).
#include <CppUnitTest.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 5262) // implicit fall-through occurs here;
#include <fstream>
#include <ios>
#include <sstream>
#include <array>
#include <cstddef>
#include <vector>
#include <array>
#pragma warning(pop)
