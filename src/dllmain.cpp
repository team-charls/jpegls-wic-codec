// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "jpegls_bitmap_decoder.h"
#include "jpegls_bitmap_encoder.h"
#include "util.h"
#include "trace.h"
#include "guids.h"

#include <OleCtl.h>
#include <ShlObj.h>

using std::wstring;

// ReSharper disable CppInconsistentNaming
// ReSharper disable CppParameterNamesMismatch

BOOL APIENTRY DllMain(const HMODULE module, const DWORD reason_for_call, void* /*reserved*/) noexcept
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
    const auto result = winrt::get_module_lock() ? S_FALSE : S_OK;
    TRACE("jpegls-wic-codec::DllCanUnloadNow hr = %d (0 = S_OK -> unload OK)\n", result);
    return result;
}

// Purpose: Returns a class factory to create an object of the requested type
_Check_return_
STDAPI DllGetClassObject(_In_ GUID const& class_id, _In_ GUID const& interface_id, _Outptr_ void** result)
{
    if (class_id == CLSID_JpegLSDecoder)
        return jpegls_bitmap_decoder_create_factory(interface_id, result);

    if (class_id == CLSID_JpegLSEncoder)
        return jpegls_bitmap_encoder_create_factory(interface_id, result);

    return CLASS_E_CLASSNOTAVAILABLE;
}

WARNING_SUPPRESS(28301) // No annotations for first declaration (latest Windows SDK doesn't have annotations)
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
        registry::set_value(sub_key.c_str(), L"SupportAnimation", 0U);
        registry::set_value(sub_key.c_str(), L"SupportChromaKey", 0U);
        registry::set_value(sub_key.c_str(), L"SupportLossless", 1U);
        registry::set_value(sub_key.c_str(), L"SupportMultiframe", 0U);
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
        registry::set_value(patterns_sub_key.c_str(), L"Position", 0U);

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

        registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"", L"jlsfile");
        registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"Content Type", L"image/jls");
        registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"PerceivedType", L"image"); // Can be used by Windows Vista and newer

        registry::set_value(LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\KindMap)", L".jls", L"picture");

        // Register with the Windows Thumbnail Cache
        registry::set_value(LR"(SOFTWARE\Classes\jlsfile\ShellEx\{e357fccd-a995-4576-b01f-234630154e96})", L"", L"{C7657C4A-9F68-40fa-A4DF-96BC08EB3551}");
        registry::set_value(LR"(SOFTWARE\Classes\SystemFileAssociations\.jls\ShellEx\{e357fccd-a995-4576-b01f-234630154e96})", L"", L"{C7657C4A-9F68-40fa-A4DF-96BC08EB3551}");

        // Register with the legacy Windows Photo Viewer (still installed on Windows 10), just forward to the TIFF registration.
        registry::set_value(LR"(SOFTWARE\Microsoft\Windows Photo Viewer\Capabilities\FileAssociations)", L".jls", L"PhotoViewer.FileAssoc.Tiff");

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

    // Note: keep the .jls file registration intact.

    return FAILED(result1) ? result1 : result2;
}

WARNING_UNSUPPRESS()

// ReSharper restore CppParameterNamesMismatch
// ReSharper restore CppInconsistentNaming
