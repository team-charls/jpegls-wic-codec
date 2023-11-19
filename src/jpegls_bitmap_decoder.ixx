// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include <Windows.h>

export module jpegls_bitmap_decoder;

export HRESULT create_jpegls_bitmap_decoder_factory(_In_ GUID const& interface_id, _Outptr_ void** result);
