﻿// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include <Windows.h>

export module jpegls_bitmap_decoder;

export HRESULT create_jpegls_bitmap_decoder_factory(GUID const& interface_id, void** result);