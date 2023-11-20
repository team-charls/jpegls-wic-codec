// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "macros.h"

#include <cassert>
#include <cstddef>
#include <string>

export module util;

import "win.h";

import errors;

import winrt;

extern "C" IMAGE_DOS_HEADER __ImageBase; // NOLINT(bugprone-reserved-identifier)

export {

constexpr std::byte operator"" _byte(const unsigned long long int n)
{
    return static_cast<std::byte>(n);
}

[[nodiscard]] inline HMODULE get_current_module() noexcept
{
    return reinterpret_cast<HINSTANCE>(&__ImageBase);
}

[[nodiscard]] inline std::wstring get_module_path()
{
    std::wstring path(100, L'?');
    size_t path_size;
    size_t actual_size;

    // GetModuleFileNameW truncates if the buffer is too small. There is no option to get the required size.
    do
    {
        path_size = path.size();
        actual_size = GetModuleFileNameW(get_current_module(), path.data(), static_cast<DWORD>(path_size));

        if (actual_size + 1 > path_size)
        {
            path.resize(path_size * 2, L'?');
        }
    } while (actual_size + 1 > path_size);

    path.resize(actual_size);
    return path;
}

[[nodiscard]] inline std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text;

    guid_text.resize(39);
    VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

    // Remove the double null terminator.
    guid_text.resize(guid_text.size() - 1);

    return guid_text;
}

[[nodiscard]] inline const char* pixel_format_to_string(const GUID& guid) noexcept
{
    if (guid == GUID_WICPixelFormat2bppGray)
        return "GUID_WICPixelFormat2bppGray";

    if (guid == GUID_WICPixelFormat4bppGray)
        return "GUID_WICPixelFormat4bppGray";

    if (guid == GUID_WICPixelFormat8bppGray)
        return "GUID_WICPixelFormat8bppGray";

    if (guid == GUID_WICPixelFormat16bppGray)
        return "GUID_WICPixelFormat16bppGray";

    if (guid == GUID_WICPixelFormat24bppRGB)
        return "GUID_WICPixelFormat24bppRGB";

    if (guid == GUID_WICPixelFormat48bppRGB)
        return "GUID_WICPixelFormat48bppRGB";

    return "Unknown";
}

constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

namespace registry {

SUPPRESS_WARNING_NEXT_LINE(26493)                  // Don't use C-style casts (used by HKEY_LOCAL_MACHINE macro)
const HKEY hkey_local_machine{HKEY_LOCAL_MACHINE}; // NOLINT

inline void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      _Null_terminated_ const wchar_t* value)
{
    const auto length{wcslen(value) + 1};
    winrt::check_win32(
        RegSetKeyValue(hkey_local_machine, sub_key, value_name, REG_SZ, value,
                       static_cast<DWORD>(length * sizeof(wchar_t)))); // NOLINT(bugprone-misplaced-widening-cast)
}

inline void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name,
                      _Null_terminated_ const wchar_t* value)
{
    set_value(sub_key.c_str(), value_name, value);
}

inline void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      const uint32_t value)
{
    winrt::check_win32(RegSetKeyValueW(hkey_local_machine, sub_key, value_name, REG_DWORD, &value, sizeof value));
}

inline void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name, const uint32_t value)
{
    set_value(sub_key.c_str(), value_name, value);
}

inline void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      const void* value, const DWORD value_size_in_bytes)
{
    winrt::check_win32(RegSetKeyValueW(hkey_local_machine, sub_key, value_name, REG_BINARY, value, value_size_in_bytes));
}

inline void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name, const void* value,
                      const DWORD value_size_in_bytes)
{
    set_value(sub_key.c_str(), value_name, value, value_size_in_bytes);
}

inline HRESULT delete_tree(_Null_terminated_ const wchar_t* sub_key) noexcept
{
    if (const LSTATUS result{RegDeleteTreeW(hkey_local_machine, sub_key)}; result != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(result);
    }

    return error_ok;
}

} // namespace registry

}
