#pragma once

#include "../src/guids.h"

#include <Windows.h>
#include <wincodec.h>
#include <winrt/base.h>

class factory final
{
public:
    factory() noexcept(false)
    {
        library_ = LoadLibrary(L"jpegls-wic-codec.dll");
        if (!library_)
            winrt::throw_last_error();
    }

    ~factory()
    {
        if (library_)
        {
            FreeLibrary(library_);
        }
    }

    factory(const factory&) = delete;
    factory(factory&&) = delete;
    factory& operator=(const factory&) = delete;
    factory& operator=(factory&&) = delete;

    [[nodiscard]] winrt::com_ptr<IWICBitmapDecoder> create_decoder() const
    {
        const auto get_class_object =
            static_cast<dll_get_class_object_ptr>(
                static_cast<void*>(GetProcAddress(library_, "DllGetClassObject")));
        if (!get_class_object)
            winrt::throw_last_error();

        winrt::com_ptr<IClassFactory> class_factory;
        winrt::check_hresult(get_class_object(CLSID_JpegLSDecoder, IID_PPV_ARGS(class_factory.put())));

        winrt::com_ptr<IWICBitmapDecoder> decoder;
        winrt::check_hresult(class_factory->CreateInstance(nullptr, IID_PPV_ARGS(decoder.put())));

        return decoder;
    }

    [[nodiscard]] winrt::com_ptr<IWICBitmapEncoder> create_encoder() const
    {
        const auto get_class_object =
            static_cast<dll_get_class_object_ptr>(
                static_cast<void*>(GetProcAddress(library_, "DllGetClassObject")));
        if (!get_class_object)
            winrt::throw_last_error();

        winrt::com_ptr<IClassFactory> class_factory;
        winrt::check_hresult(get_class_object(CLSID_JpegLSEncoder, IID_PPV_ARGS(class_factory.put())));

        winrt::com_ptr<IWICBitmapEncoder> encoder;
        winrt::check_hresult(class_factory->CreateInstance(nullptr, IID_PPV_ARGS(encoder.put())));

        return encoder;
    }

private:
    using dll_get_class_object_ptr = HRESULT(_stdcall*)(GUID const&, GUID const&, void** result);

    HINSTANCE library_{};
};
