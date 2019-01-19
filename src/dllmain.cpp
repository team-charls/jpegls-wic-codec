// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "jpegls_bitmap_decoder_factory.h"
#include "util.h"

#include <olectl.h>
#include <Shlobj.h>

using std::wstring;

BOOL APIENTRY DllMain(HMODULE module, const DWORD reason_for_call, void* /*reserved*/) noexcept
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WINRT_VERIFY(DisableThreadLibraryCalls(module));
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;

    default:
        WINRT_ASSERT(false);
        break;
    }

    return true;
}

// Purpose: Used to determine whether the COM sub-system can unload the DLL from memory.
__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow()
{
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}

// Purpose: Returns a class factory to create an object of the requested type
_Check_return_
STDAPI DllGetClassObject(_In_ GUID const& class_id, _In_ GUID const& interface_id, _Outptr_ void** result)
{
    if (class_id == CLSID_JpegLSDecoder)
        return winrt::make<jpegls_bitmap_decoder_factory>()->QueryInterface(interface_id, result);

    return CLASS_E_CLASSNOTAVAILABLE;
}

__control_entrypoint(DllExport)
STDAPI DllRegisterServer()
{
    try
    {
        const wstring sub_key = wstring{LR"(SOFTWARE\Classes\CLSID\)"} + guid_to_string(CLSID_JpegLSDecoder);
        registry::set_value(sub_key.c_str(), L"ArbitrationPriority", 10);
        registry::set_value(sub_key.c_str(), L"Author", L"Team CharLS");
        registry::set_value(sub_key.c_str(), L"ColorManagementVersion", L"1.0.0.0");
        registry::set_value(sub_key.c_str(), L"ContainerFormat", guid_to_string(GUID_ContainerFormatJpegLS).c_str());
        registry::set_value(sub_key.c_str(), L"Description", L"JPEG-LS Codec");
        registry::set_value(sub_key.c_str(), L"FileExtensions", L".jls");
        registry::set_value(sub_key.c_str(), L"FriendlyName", L"Team CharLS JPEG-LS Decoder");
        registry::set_value(sub_key.c_str(), L"MimeTypes", L"image/jls");
        registry::set_value(sub_key.c_str(), L"SpecVersion", L"1.0.0.0"); // TODO: Read from .rc
        registry::set_value(sub_key.c_str(), L"SupportAnimation", 0u);
        registry::set_value(sub_key.c_str(), L"SupportChromaKey", 0u);
        registry::set_value(sub_key.c_str(), L"SupportLossless", 1u);
        registry::set_value(sub_key.c_str(), L"SupportMultiframe", 0u);
        registry::set_value(sub_key.c_str(), L"Vendor", guid_to_string(GUID_VendorTeamCharLS).c_str());
        registry::set_value(sub_key.c_str(), L"Version", L"1.0.0.0");  // TODO: Read from .rc

        const wstring formats_sub_key = sub_key + LR"(\Formats\)";
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat2bppGray)).c_str(), L"", L"");
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat4bppGray)).c_str(), L"", L"");
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat8bppGray)).c_str(), L"", L"");
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat16bppGray)).c_str(), L"", L"");
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat24bppRGB)).c_str(), L"", L"");
        registry::set_value((formats_sub_key + guid_to_string(GUID_WICPixelFormat48bppRGB)).c_str(), L"", L"");

        const wstring patterns_sub_key = sub_key + LR"(\Patterns\0)";
        registry::set_value(patterns_sub_key.c_str(), L"Length", 3);
        registry::set_value(patterns_sub_key.c_str(), L"Position", 0u);

        const std::byte mask[]{std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}};
        registry::set_value(patterns_sub_key.c_str(), L"Mask", mask, sizeof mask);
        const std::byte pattern[]{std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF}};
        registry::set_value(patterns_sub_key.c_str(), L"Pattern", pattern, sizeof pattern);

        const wstring inproc_server_sub_key = sub_key + LR"(\InprocServer32\)";
        registry::set_value(inproc_server_sub_key.c_str(), L"", get_module_path().c_str());
        registry::set_value(inproc_server_sub_key.c_str(), L"ThreadingModel", L"Both");

        const wstring category_id_key = wstring{LR"(SOFTWARE\Classes\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\)"}
            + guid_to_string(CLSID_JpegLSDecoder);
        registry::set_value(category_id_key.c_str(), L"FriendlyName", L"Team CharLS JPEG-LS Decoder");
        registry::set_value(category_id_key.c_str(), L"CLSID", guid_to_string(CLSID_JpegLSDecoder).c_str());

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

        return S_OK;
    }
    catch (...)
    {
        return SELFREG_E_CLASS;
    }
}

__control_entrypoint(DllExport)
STDAPI DllUnregisterServer()
{
    const wstring sub_key = wstring{LR"(SOFTWARE\Classes\CLSID\)"} + guid_to_string(CLSID_JpegLSDecoder);
    const HRESULT result1 = registry::delete_tree(sub_key.c_str());

    const wstring category_id_key = wstring{LR"(SOFTWARE\Classes\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\)"}
        + guid_to_string(CLSID_JpegLSDecoder);
    const HRESULT result2 = registry::delete_tree(category_id_key.c_str());

    return FAILED(result1) ? result1 : result2;
}
