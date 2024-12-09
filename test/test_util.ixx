// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

export module test.util;

import test.winrt;
import <win.hpp>;

using winrt::com_ptr;

export constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

export constexpr bool succeeded(winrt::hresult const result) noexcept
{
    return result >= 0;
}

export com_ptr<IStream> create_stream_on_file(const wchar_t* filename) noexcept
{
    com_ptr<IStream> stream;
    winrt::check_hresult(SHCreateStreamOnFileW(filename, STGM_READ, stream.put()));
    return stream;
}
