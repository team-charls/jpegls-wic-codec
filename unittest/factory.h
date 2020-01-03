#pragma once

#include "../src/guids.h"

#include <Windows.h>
#include <wincodec.h>
#include <winrt/base.h>

class factory final
{
public:
    factory() noexcept(false) :
        library_{LoadLibrary(L"jpegls-wic-codec.dll")}
    {
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
        winrt::com_ptr<IWICBitmapDecoder> decoder;
        winrt::check_hresult(get_class_factory(CLSID_JpegLSDecoder)->CreateInstance(nullptr, IID_PPV_ARGS(decoder.put())));

        return decoder;
    }

    [[nodiscard]] winrt::com_ptr<IWICBitmapEncoder> create_encoder() const
    {
        winrt::com_ptr<IWICBitmapEncoder> encoder;
        winrt::check_hresult(get_class_factory(CLSID_JpegLSEncoder)->CreateInstance(nullptr, IID_PPV_ARGS(encoder.put())));

        return encoder;
    }

    [[nodiscard]] winrt::com_ptr<IClassFactory> get_class_factory(GUID const& class_id) const
    {
        winrt::com_ptr<IClassFactory> class_factory;
        winrt::check_hresult(get_class_factory(class_id, class_factory));

        return class_factory;
    }

    [[nodiscard]] HRESULT get_class_factory(GUID const& class_id, winrt::com_ptr<IClassFactory>& class_factory) const
    {
        const auto get_class_object =
            static_cast<dll_get_class_object_ptr>(
                static_cast<void*>(GetProcAddress(library_, "DllGetClassObject")));
        if (!get_class_object)
            winrt::throw_last_error();

        return get_class_object(class_id, IID_PPV_ARGS(class_factory.put()));
    }

private:
    using dll_get_class_object_ptr = HRESULT(_stdcall*)(GUID const&, GUID const&, void** result);

    HINSTANCE library_{};
};
