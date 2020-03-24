// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "util.h"

#include "../src/errors.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_decoder_test)
{
public:
    TEST_METHOD(GetContainerFormat) // NOLINT
    {
        GUID container_format;
        const hresult result = factory_.create_decoder()->GetContainerFormat(&container_format);

        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr) // NOLINT
    {
        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result = factory_.create_decoder()->GetContainerFormat(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(GetDecoderInfo) // NOLINT
    {
        com_ptr<IWICBitmapDecoder> decoder = factory_.create_decoder();

        com_ptr<IWICBitmapDecoderInfo> decoder_info;
        const hresult result = decoder->GetDecoderInfo(decoder_info.put());

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
        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{factory_.create_decoder()->GetDecoderInfo(nullptr)};
        WARNING_UNSUPPRESS()

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(CopyPalette) // NOLINT
    {
        const com_ptr<IWICPalette> palette;
        const hresult result = factory_.create_decoder()->CopyPalette(palette.get());

        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(GetMetadataQueryReader) // NOLINT
    {
        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const hresult result = factory_.create_decoder()->GetMetadataQueryReader(metadata_query_reader.put());

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetPreview) // NOLINT
    {
        com_ptr<IWICBitmapSource> bitmap_source;
        const hresult result = factory_.create_decoder()->GetPreview(bitmap_source.put());

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetColorContexts) // NOLINT
    {
        com_ptr<IWICColorContext> color_contexts;
        uint32_t actual_count;
        const hresult result = factory_.create_decoder()->GetColorContexts(1, color_contexts.put(), &actual_count);

        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetThumbnail) // NOLINT
    {
        com_ptr<IWICBitmapSource> bitmap_source;
        const hresult result = factory_.create_decoder()->GetThumbnail(bitmap_source.put());

        Assert::AreEqual(wincodec::error_codec_no_thumbnail, result);
    }

    TEST_METHOD(GetFrameCount) // NOLINT
    {
        uint32_t frame_count;
        const hresult result = factory_.create_decoder()->GetFrameCount(&frame_count);

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(1U, frame_count);
    }

    TEST_METHOD(GetFrameCount_count_parameter_is_null) // NOLINT
    {
        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{factory_.create_decoder()->GetFrameCount(nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(QueryCapability_cannot_decode_empty) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        DWORD capability;
        const hresult result{factory_.create_decoder()->QueryCapability(stream.get(), &capability)};

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0UL, capability);
    }

    TEST_METHOD(QueryCapability_can_decode_8bit_monochrome) // NOLINT
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"lena8b.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));
        DWORD capability;
        const hresult result = factory_.create_decoder()->QueryCapability(stream.get(), &capability);

        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(static_cast<DWORD>(WICBitmapDecoderCapabilityCanDecodeAllImages), capability);
    }

    TEST_METHOD(QueryCapability_stream_argument_null) // NOLINT
    {
        DWORD capability;
        const hresult result{factory_.create_decoder()->QueryCapability(nullptr, &capability)};

        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(QueryCapability_capability_argument_null) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{factory_.create_decoder()->QueryCapability(stream.get(), nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(QueryCapability_read_error_on_stream) // NOLINT
    {
        // TODO: pass a stream that generates a read error.
        Assert::IsTrue(true);
    }

    TEST_METHOD(QueryCapability_seek_error_on_stream) // NOLINT
    {
        // TODO: pass a stream that generates a seek error.
        Assert::IsTrue(true);
    }

    TEST_METHOD(Initialize_cache_on_demand) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const hresult result{factory_.create_decoder()->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_cache_on_load) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const hresult result{factory_.create_decoder()->Initialize(stream.get(), WICDecodeMetadataCacheOnLoad)};
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapDecoder> decoder = factory_.create_decoder();
        hresult result{decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_ok, result);

        result = decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnLoad);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_bad_cache_option) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const hresult result{factory_.create_decoder()->Initialize(stream.get(), static_cast<WICDecodeOptions>(4))};

        // Cache options is not used by decoder and by design not validated.
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_null_stream) // NOLINT
    {
        const hresult result{factory_.create_decoder()->Initialize(nullptr, WICDecodeMetadataCacheOnDemand)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetFrame) // NOLINT
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"lena8b.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapDecoder> decoder = factory_.create_decoder();
        hresult result{decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand)};
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
        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{factory_.create_decoder()->GetFrame(0, nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(GetFrame_with_bad_index) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const hresult result{factory_.create_decoder()->GetFrame(1, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_frame_missing, result);
    }

    TEST_METHOD(GetFrame_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        const hresult result{factory_.create_decoder()->GetFrame(0, bitmap_frame_decode.put())};

        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

private:
    static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
                                       nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, imaging_factory.put_void()));

        return imaging_factory;
    }

    factory factory_;
};
