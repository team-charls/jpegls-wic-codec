// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT
#pragma once

#include "../src/errors.h"

#include <Windows.h>
#include <wincodec.h>

class test_stream final : public winrt::implements<test_stream, IStream>
{
public:
    test_stream(bool fail_on_read, int fail_on_seek_counter) noexcept :
        fail_on_read_{fail_on_read}, fail_on_seek_counter_{fail_on_seek_counter}
    {
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall Read(_Out_writes_bytes_to_(cb, *pcbRead) void* /*pv*/, _In_ [[maybe_unused]] ULONG cb,
                           _Out_opt_ [[maybe_unused]] ULONG* pcbRead) noexcept override
    {
        return fail_on_read_ ? error_fail : error_ok;
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall Write(_In_reads_bytes_(cb) const void* /*pv*/, _In_ [[maybe_unused]] ULONG cb,
                            _Out_opt_ ULONG* /*pcbWritten*/) noexcept override
    {
        return error_fail;
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall Seek(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/,
                           _Out_opt_ ULARGE_INTEGER* /*plibNewPosition*/) noexcept override
    {
        --fail_on_seek_counter_;

        return fail_on_seek_counter_ <= 0 ? error_fail : error_ok;
    }

    HRESULT __stdcall SetSize(ULARGE_INTEGER /*libNewSize*/) noexcept override
    {
        return error_fail;
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall CopyTo(_In_ IStream* /*pstm*/, ULARGE_INTEGER /*cb*/, _Out_opt_ ULARGE_INTEGER* /*pcbRead*/,
                             _Out_opt_ ULARGE_INTEGER* /*pcbWritten*/) noexcept override
    {
        return error_fail;
    }

    HRESULT __stdcall Commit(DWORD /*grfCommitFlags*/) noexcept override
    {
        return error_fail;
    }

    HRESULT __stdcall Revert() noexcept override
    {
        return error_fail;
    }

    HRESULT __stdcall LockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/, DWORD /*dwLockType*/) noexcept override
    {
        return error_fail;
    }

    HRESULT __stdcall UnlockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/,
                                   DWORD /*dwLockType*/) noexcept override
    {
        return error_fail;
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall Stat(__RPC__out STATSTG* /*pstatstg*/, DWORD /*grfStatFlag*/) noexcept override
    {
        return error_fail;
    }

    SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE
    HRESULT __stdcall Clone(__RPC__deref_out_opt IStream** /*ppstm*/) noexcept override
    {
        return error_fail;
    }

private:
    bool fail_on_read_;
    int fail_on_seek_counter_;
};
