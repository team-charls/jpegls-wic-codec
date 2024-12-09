// SPDX-FileCopyrightText: © 2023 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

module;

#define _CRT_MEMCPY_S_INLINE inline

// ReSharper disable once CppUnusedIncludeDirective
#include <Unknwn.h> // Required to enable classic COM support in WinRT.

#pragma warning(push)
#pragma warning(disable : 4265) // : '' : class has virtual functions, but its non-trivial destructor is not virtual
#pragma warning(disable : 4946) // reinterpret_cast used between related classes
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual
#pragma warning(disable : 5260) // the constant variable has external\internal linkage
#pragma warning(disable : 26432) // If you define or delete any default operation in the type define or delete them.
#include <winrt/base.h>
#pragma warning(pop)

export module test.winrt;

export namespace winrt {

using winrt::throw_hresult;
using winrt::hresult;
using winrt::implements;
using winrt::make;
using winrt::to_hresult;
using winrt::throw_hresult;
using winrt::com_ptr;
using winrt::check_hresult;
using winrt::make_self;
using winrt::check_win32;
using winrt::get_module_lock;
using winrt::throw_last_error;
using winrt::take_ownership_from_abi;

}