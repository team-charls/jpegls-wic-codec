// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#include "macros.h"
#include <CppUnitTest.h>

import test_stream;
import errors;
import factory;
import guids;
import util;
import charls;
import winrt;
import "std.h";
import "win.h";

template<>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<winrt::hresult>(const winrt::hresult& q)
{
    RETURN_WIDE_STRING(static_cast<int>(q));
}

using std::vector;
using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_decoder_test)
{
public:
    TEST_METHOD(GetContainerFormat) // NOLINT
    {
        GUID container_format;
        const HRESULT result{factory_.create_decoder()->GetContainerFormat(&container_format)};

        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(id::container_format_jpegls == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr) // NOLINT
    {
        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{factory_.create_decoder()->GetContainerFormat(nullptr)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(GetDecoderInfo) // NOLINT
    {
        const com_ptr decoder{factory_.create_decoder()};

        com_ptr<IWICBitmapDecoderInfo> decoder_info;
        const HRESULT result{decoder->GetDecoderInfo(decoder_info.put())};

        Assert::IsTrue(result == error_ok || result == wincodec::error_component_not_found);
        if (succeeded(result))
        {
            Assert::IsNotNull(decoder_info.get());
        }
        else
        {
            Assert::IsNull(decoder_info.get());
        }
    }

    TEST_METHOD(GetDecoderInfo_with_nullptr) // NOLINT
    {
        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{factory_.create_decoder()->GetDecoderInfo(nullptr)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(CopyPalette) // NOLINT
    {
        const com_ptr<IWICPalette> palette;
        const HRESULT result{factory_.create_decoder()->CopyPalette(palette.get())};

        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(GetMetadataQueryReader) // NOLINT
    {
        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const HRESULT result{factory_.create_decoder()->GetMetadataQueryReader(metadata_query_reader.put())};

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetPreview) // NOLINT
    {
        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result{factory_.create_decoder()->GetPreview(bitmap_source.put())};

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetColorContexts) // NOLINT
    {
        com_ptr<IWICColorContext> color_contexts;
        uint32_t actual_count;
        const HRESULT result{factory_.create_decoder()->GetColorContexts(1, color_contexts.put(), &actual_count)};

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetThumbnail) // NOLINT
    {
        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result{factory_.create_decoder()->GetThumbnail(bitmap_source.put())};

        Assert::AreEqual(wincodec::error_codec_no_thumbnail, result);
    }

    TEST_METHOD(GetFrameCount) // NOLINT
    {
        uint32_t frame_count;
        const HRESULT result{factory_.create_decoder()->GetFrameCount(&frame_count)};

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(1U, frame_count);
    }

    TEST_METHOD(GetFrameCount_count_parameter_is_null) // NOLINT
    {
        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{factory_.create_decoder()->GetFrameCount(nullptr)};

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(QueryCapability_cannot_decode_empty) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        DWORD capability;
        const HRESULT result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0UL, capability);
    }

    TEST_METHOD(QueryCapability_can_decode_8_bit_monochrome) // NOLINT
    {
        com_ptr<IStream> stream;
        check_hresult(
            SHCreateStreamOnFileEx(L"tulips-gray-8bit-512-512.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));
        DWORD capability;
        const HRESULT result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(static_cast<DWORD>(WICBitmapDecoderCapabilityCanDecodeAllImages), capability);
    }

    TEST_METHOD(QueryCapability_can_not_decode_5_bit_monochrome) // NOLINT
    {
        // TODO: create test file and .jls file
        Assert::IsTrue(true);
    }

    TEST_METHOD(QueryCapability_stream_argument_null) // NOLINT
    {
        DWORD capability;
        const HRESULT result{factory_.create_decoder()->QueryCapability(nullptr, &capability)};

        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(QueryCapability_capability_argument_null) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{factory_.create_decoder()->QueryCapability(stream.get(), nullptr)};

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(QueryCapability_read_error_on_stream) // NOLINT
    {
        const com_ptr stream{make<test_stream>(true, 2)};

        DWORD capability;
        const hresult result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(QueryCapability_seek_error_on_stream) // NOLINT
    {
        const com_ptr stream{winrt::make<test_stream>(false, 1)};

        DWORD capability;
        const hresult result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(QueryCapability_seek_error_on_stream_reset) // NOLINT
    {
        const com_ptr stream{winrt::make<test_stream>(false, 1)};

        DWORD capability;
        const hresult result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(Initialize_cache_on_demand) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const HRESULT result{factory_.create_decoder()->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_cache_on_load) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const HRESULT result{factory_.create_decoder()->Initialize(stream.get(), WICDecodeMetadataCacheOnLoad)};
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr decoder{factory_.create_decoder()};
        HRESULT result{decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);

        result = decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnLoad);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_bad_cache_option) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const HRESULT result{factory_.create_decoder()->Initialize(stream.get(), static_cast<WICDecodeOptions>(4))};

        // Cache options is not used by decoder and by design not validated.
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_null_stream) // NOLINT
    {
        const HRESULT result{factory_.create_decoder()->Initialize(nullptr, WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetFrame) // NOLINT
    {
        com_ptr<IStream> stream;
        check_hresult(
            SHCreateStreamOnFileEx(L"tulips-gray-8bit-512-512.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        const com_ptr decoder{factory_.create_decoder()};
        HRESULT result{decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);

        uint32_t frame_count;
        result = decoder->GetFrameCount(&frame_count);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(1U, frame_count);

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        result = decoder->GetFrame(0, bitmap_frame_decode.put());
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(bitmap_frame_decode.get() != nullptr);
    }

    TEST_METHOD(GetFrame_with_frame_argument_null) // NOLINT
    {
        com_ptr<IStream> stream;
        check_hresult(
            SHCreateStreamOnFileEx(L"tulips-gray-8bit-512-512.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        const com_ptr decoder{factory_.create_decoder()};
        HRESULT result{decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        result = decoder->GetFrame(0, nullptr);

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(GetFrame_with_bad_index) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{factory_.create_decoder()->GetFrame(1, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_frame_missing, result);
    }

    TEST_METHOD(GetFrame_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{factory_.create_decoder()->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

    TEST_METHOD(GetFrame_3_bit_monochrome_is_not_supported) // NOLINT
    {
        const vector pixel_data{std::byte{}};
        auto encoded_data = charls::jpegls_encoder::encode(
            pixel_data, {.width = 1, .height = 1, .bits_per_sample = 3, .component_count = 1});
        const auto bitmap_decoder{create_decoder(encoded_data)};

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{bitmap_decoder->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_unsupported_pixel_format, result);
    }

    TEST_METHOD(GetFrame_7_bit_rgb_is_not_supported) // NOLINT
    {
        const vector<std::byte> pixel_data(3);
        auto encoded_data = charls::jpegls_encoder::encode(
            pixel_data, {.width = 1, .height = 1, .bits_per_sample = 7, .component_count = 3});
        const auto bitmap_decoder{create_decoder(encoded_data)};

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{bitmap_decoder->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_unsupported_pixel_format, result);
    }

    TEST_METHOD(GetFrame_8_bit_2_components_is_not_supported) // NOLINT
    {
        const vector<std::byte> pixel_data(2);
        auto encoded_data = charls::jpegls_encoder::encode(
            pixel_data, {.width = 1, .height = 1, .bits_per_sample = 8, .component_count = 2});
        const auto bitmap_decoder{create_decoder(encoded_data)};

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{bitmap_decoder->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_unsupported_pixel_format, result);
    }

    TEST_METHOD(GetFrame_empty_buffer_causes_bad_header) // NOLINT
    {
        const vector<std::byte> empty_buffer(2);
        const auto bitmap_decoder{create_decoder(empty_buffer)};

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const HRESULT result{bitmap_decoder->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_bad_header, result);
    }

private:
    [[nodiscard]]
    static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                                       imaging_factory.put_void()));

        return imaging_factory;
    }

    [[nodiscard]]
    com_ptr<IWICBitmapDecoder> create_decoder(const std::span<const std::byte> buffer) const
    {
        const com_ptr<IStream> stream{
            SHCreateMemStream(reinterpret_cast<const BYTE*>(buffer.data()), static_cast<UINT>(buffer.size())),
            take_ownership_from_abi};

        const com_ptr bitmap_decoder{factory_.create_decoder()};
        check_hresult(bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand));

        return bitmap_decoder;
    }

    factory factory_;
};
