// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <wincodec.h>
#include <winrt/base.h>

class jpegls_bitmap_frame_decode final
    : public winrt::implements<jpegls_bitmap_frame_decode, IWICBitmapFrameDecode, IWICBitmapSource>
{
public:
    jpegls_bitmap_frame_decode(_In_ IStream* stream, _In_ IWICImagingFactory* factory);

    // IWICBitmapSource
    HRESULT __stdcall GetSize(uint32_t* width, uint32_t* height) override;
    HRESULT __stdcall GetPixelFormat(GUID* pixel_format) override;
    HRESULT __stdcall GetResolution(double* dpi_x, double* dpi_y) override;
    HRESULT __stdcall CopyPixels(const WICRect* rectangle, uint32_t stride, uint32_t buffer_size, BYTE* buffer) override;
    HRESULT __stdcall CopyPalette(IWICPalette* /*palette*/) noexcept override;

    // IWICBitmapFrameDecode : IWICBitmapSource
    HRESULT __stdcall GetThumbnail(IWICBitmapSource** /*source*/) noexcept override;
    HRESULT __stdcall GetColorContexts(uint32_t count, IWICColorContext** color_contexts,
                                       uint32_t* actual_count) noexcept override;
    HRESULT __stdcall GetMetadataQueryReader(IWICMetadataQueryReader** metadata_query_reader) noexcept override;

    [[nodiscard]]
    static bool can_decode_to_wic_pixel_format(int32_t bits_per_sample, int32_t component_count) noexcept;

private:
    winrt::com_ptr<IWICBitmapSource> bitmap_source_;
};
