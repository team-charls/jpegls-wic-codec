// SPDX-FileCopyrightText: © 2019 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "macros.hpp"
#include "intellisense.hpp"

export module util;

import std;
import <win.hpp>;
import winrt;

import hresults;


using std::uint32_t;

extern "C" IMAGE_DOS_HEADER __ImageBase; // NOLINT(bugprone-reserved-identifier)

export [[nodiscard]]
constexpr std::byte operator"" _byte(const unsigned long long int n)
{
    return static_cast<std::byte>(n);
}

[[nodiscard]]
static HMODULE get_current_module() noexcept
{
    return reinterpret_cast<HINSTANCE>(&__ImageBase);
}

export [[nodiscard]]
std::wstring get_module_path()
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

export [[nodiscard]]
std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text;

    guid_text.resize(39);
    VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

    // Remove the double null terminator.
    guid_text.resize(guid_text.size() - 1);

    return guid_text;
}

export [[nodiscard]]
constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

export template<typename T>
T* check_in_pointer(_In_ T* pointer)
{
    if (!pointer)
        winrt::throw_hresult(error_invalid_argument);

    return pointer;
}

export template<typename T>
T* check_out_pointer(T* pointer)
{
    if (!pointer)
        winrt::throw_hresult(error_pointer);

    return pointer;
}

export inline void check_condition(const bool condition, const winrt::hresult result_to_throw)
{
    if (!condition)
        throw_hresult(result_to_throw);
}

export namespace fmt {

// Copied from fmtlib as P2510 (Formatting Pointers) is not yet accepted.
template<typename T>
[[nodiscard]]
auto ptr(T p) noexcept -> const void*
{
    static_assert(std::is_pointer_v<T>);
    return std::bit_cast<const void*>(p);
}

// Copied from fmtlib as replacement for std::to_underlying (C++23)
template<typename Enum>
[[nodiscard]]
constexpr auto underlying(Enum e) noexcept -> std::underlying_type_t<Enum>
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

} // namespace fmt


namespace registry {

SUPPRESS_WARNING_NEXT_LINE(26493)                  // Don't use C-style casts (used by HKEY_LOCAL_MACHINE macro)
const HKEY hkey_local_machine{HKEY_LOCAL_MACHINE}; // NOLINT

export void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      _Null_terminated_ const wchar_t* value)
{
    const auto length{wcslen(value) + 1};
    winrt::check_win32(
        RegSetKeyValue(hkey_local_machine, sub_key, value_name, REG_SZ, value,
                       static_cast<DWORD>(length * sizeof(wchar_t)))); // NOLINT(bugprone-misplaced-widening-cast)
}

export void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name,
                      _Null_terminated_ const wchar_t* value)
{
    set_value(sub_key.c_str(), value_name, value);
}

export void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      const uint32_t value)
{
    winrt::check_win32(RegSetKeyValueW(hkey_local_machine, sub_key, value_name, REG_DWORD, &value, sizeof value));
}

export void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name, const uint32_t value)
{
    set_value(sub_key.c_str(), value_name, value);
}

export void set_value(_Null_terminated_ const wchar_t* sub_key, _Null_terminated_ const wchar_t* value_name,
                      std::span<const std::byte> values)
{
    winrt::check_win32(RegSetKeyValueW(hkey_local_machine, sub_key, value_name, REG_BINARY, values.data(),
                                       static_cast<DWORD>(values.size())));
}

export void set_value(const std::wstring& sub_key, _Null_terminated_ const wchar_t* value_name,
                      const std::span<const std::byte> values)
{
    set_value(sub_key.c_str(), value_name, values);
}


export [[nodiscard]]
HRESULT delete_tree(_Null_terminated_ const wchar_t* sub_key) noexcept
{
    if (const LSTATUS result{RegDeleteTreeW(hkey_local_machine, sub_key)}; result != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(result);
    }

    return success_ok;
}

} // namespace registry
