// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "guids.h"
#include "jpegls_bitmap_decoder.h"
#include "jpegls_bitmap_encoder.h"
#include "trace.h"
#include "util.h"

#include <OleCtl.h>
#include <ShlObj.h>

using std::wstring;
using namespace std::string_literals;

namespace {

constexpr GUID wic_bitmap_decoders_category{0x7ED96837, 0x96F0, 0x4812, {0xB2, 0x11, 0xF1, 0x3C, 0x24, 0x11, 0x7E, 0xD3}};
constexpr GUID wic_bitmap_encoders_category{0xAC757296, 0x3522, 0x4E11, {0x98, 0x62, 0xC1, 0x7B, 0xE5, 0xA1, 0x76, 0x7E}};

void register_general_decoder_encoder_settings(const GUID& class_id, const GUID& wic_category_id, const wchar_t* friendly_name)
{
    const wstring sub_key = LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(class_id);
    registry::set_value(sub_key, L"ArbitrationPriority", 10);
    registry::set_value(sub_key, L"Author", L"Team CharLS");
    registry::set_value(sub_key, L"ColorManagementVersion", L"1.0.0.0");
    registry::set_value(sub_key, L"ContainerFormat", guid_to_string(GUID_ContainerFormatJpegLS).c_str());
    registry::set_value(sub_key, L"Description", L"JPEG-LS Codec");
    registry::set_value(sub_key, L"FileExtensions", L".jls");
    registry::set_value(sub_key, L"FriendlyName", friendly_name);
    registry::set_value(sub_key, L"MimeTypes", L"image/jls");
    registry::set_value(sub_key, L"SpecVersion", L"1.0.0.0"); // TODO: Read from .rc
    registry::set_value(sub_key, L"SupportAnimation", 0U);
    registry::set_value(sub_key, L"SupportChromaKey", 0U);
    registry::set_value(sub_key, L"SupportLossless", 1U);
    registry::set_value(sub_key, L"SupportMultiframe", 0U);
    registry::set_value(sub_key, L"Vendor", guid_to_string(GUID_VendorTeamCharLS).c_str());
    registry::set_value(sub_key, L"Version", L"1.0.0.0"); // TODO: Read from .rc

    const wstring formats_sub_key = sub_key + LR"(\Formats\)";
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat2bppGray), L"", L"");
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat4bppGray), L"", L"");
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat8bppGray), L"", L"");
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat16bppGray), L"", L"");
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat24bppRGB), L"", L"");
    registry::set_value(formats_sub_key + guid_to_string(GUID_WICPixelFormat48bppRGB), L"", L"");

    // COM co-create registration.
    const wstring inproc_server_sub_key = sub_key + LR"(\InprocServer32\)";
    registry::set_value(inproc_server_sub_key, L"", get_module_path().c_str());
    registry::set_value(inproc_server_sub_key, L"ThreadingModel", L"Both");

    const wstring category_id_key = LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(wic_category_id) + LR"(\Instance\)" + guid_to_string(CLSID_JpegLSDecoder);
    registry::set_value(category_id_key, L"FriendlyName", L"Team CharLS JPEG-LS Decoder");
    registry::set_value(category_id_key, L"CLSID", guid_to_string(class_id).c_str());
}

void register_decoder()
{
    register_general_decoder_encoder_settings(CLSID_JpegLSDecoder, wic_bitmap_decoders_category, L"JPEG-LS Decoder");

    const wstring sub_key = LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(CLSID_JpegLSDecoder);

    const wstring patterns_sub_key = sub_key + LR"(\Patterns\0)";
    registry::set_value(patterns_sub_key, L"Length", 3);
    registry::set_value(patterns_sub_key, L"Position", 0U);

    const std::byte mask[]{std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF}};
    registry::set_value(patterns_sub_key, L"Mask", mask, sizeof mask);
    const std::byte pattern[]{std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF}};
    registry::set_value(patterns_sub_key, L"Pattern", pattern, sizeof pattern);

    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"", L"jlsfile");
    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"Content Type", L"image/jls");
    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"PerceivedType", L"image"); // Can be used by Windows Vista and newer

    registry::set_value(LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\KindMap)", L".jls", L"picture");

    // Register with the Windows Thumbnail Cache
    registry::set_value(LR"(SOFTWARE\Classes\jlsfile\ShellEx\{e357fccd-a995-4576-b01f-234630154e96})", L"", L"{C7657C4A-9F68-40fa-A4DF-96BC08EB3551}");
    registry::set_value(LR"(SOFTWARE\Classes\SystemFileAssociations\.jls\ShellEx\{e357fccd-a995-4576-b01f-234630154e96})", L"", L"{C7657C4A-9F68-40fa-A4DF-96BC08EB3551}");

    // Register with the legacy Windows Photo Viewer (still installed on Windows 10), just forward to the TIFF registration.
    registry::set_value(LR"(SOFTWARE\Microsoft\Windows Photo Viewer\Capabilities\FileAssociations)", L".jls", L"PhotoViewer.FileAssoc.Tiff");
}

void register_encoder()
{
    register_general_decoder_encoder_settings(CLSID_JpegLSEncoder, wic_bitmap_encoders_category, L"JPEG-LS Encoder");
}

HRESULT unregister(const GUID& class_id, const GUID& wic_category_id)
{
    const wstring sub_key = LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(class_id);
    const HRESULT result1 = registry::delete_tree(sub_key.c_str());

    const wstring category_id_key = LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(wic_category_id) + LR"(\Instance\)" + guid_to_string(class_id);
    const HRESULT result2 = registry::delete_tree(category_id_key.c_str());

    return FAILED(result1) ? result1 : result2;
}

} // namespace

// ReSharper disable CppInconsistentNaming
// ReSharper disable CppParameterNamesMismatch

BOOL APIENTRY DllMain(const HMODULE module, const DWORD reason_for_call, void* /*reserved*/) noexcept
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        VERIFY(DisableThreadLibraryCalls(module));
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;

    default:
        ASSERT(false);
        break;
    }

    return true;
}

// Purpose: Used to determine whether the COM sub-system can unload the DLL from memory.
__control_entrypoint(DllExport)
HRESULT __stdcall DllCanUnloadNow()
{
    const auto result = winrt::get_module_lock() ? S_FALSE : S_OK;
    TRACE("jpegls-wic-codec::DllCanUnloadNow hr = %d (0 = S_OK -> unload OK)\n", result);
    return result;
}

// Purpose: Returns a class factory to create an object of the requested type
_Check_return_
HRESULT __stdcall DllGetClassObject(_In_ GUID const& class_id, _In_ GUID const& interface_id, _Outptr_ void** result)
{
    if (class_id == CLSID_JpegLSDecoder)
        return jpegls_bitmap_decoder_create_factory(interface_id, result);

    if (class_id == CLSID_JpegLSEncoder)
        return jpegls_bitmap_encoder_create_factory(interface_id, result);

    return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT __stdcall DllRegisterServer()
{
    try
    {
        register_decoder();
        register_encoder();

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

        return S_OK;
    }
    catch (...)
    {
        return SELFREG_E_CLASS;
    }
}

HRESULT __stdcall DllUnregisterServer()
{
    const HRESULT result_decoder = unregister(CLSID_JpegLSDecoder, wic_bitmap_decoders_category);
    const HRESULT result_encoder = unregister(CLSID_JpegLSEncoder, wic_bitmap_encoders_category);

    // Note: keep the .jls file registration intact.

    return FAILED(result_decoder) ? result_decoder : result_encoder;
}

// ReSharper restore CppParameterNamesMismatch
// ReSharper restore CppInconsistentNaming
