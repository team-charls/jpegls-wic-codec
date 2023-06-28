// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"

#include "guids.h"
#include "jpegls_bitmap_decoder.h"
#include "jpegls_bitmap_encoder.h"
#include "trace.h"
#include "util.h"
#include "version.h"

#include <olectl.h>
#include <ShlObj.h>

#include <array>

using std::array;
using std::wstring;
using namespace std::string_literals;

namespace {

void register_general_decoder_encoder_settings(const GUID& class_id, const GUID& wic_category_id,
                                               const wchar_t* friendly_name, const GUID* formats, const size_t format_count)
{
    const wstring sub_key{LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(class_id)};
    registry::set_value(sub_key, L"ArbitrationPriority", 10);
    registry::set_value(sub_key, L"Author", L"Team CharLS");
    registry::set_value(sub_key, L"ColorManagementVersion", L"1.0.0.0");
    registry::set_value(sub_key, L"ContainerFormat", guid_to_string(id::container_format_jpegls).c_str());
    registry::set_value(sub_key, L"Description", L"JPEG-LS Codec");
    registry::set_value(sub_key, L"FileExtensions", L".jls");
    registry::set_value(sub_key, L"FriendlyName", friendly_name);
    registry::set_value(sub_key, L"MimeTypes", L"image/jls");
    registry::set_value(sub_key, L"SpecVersion", L"1.0.0.0");
    registry::set_value(sub_key, L"SupportAnimation", 0U);
    registry::set_value(sub_key, L"SupportChromaKey", 0U);
    registry::set_value(sub_key, L"SupportLossless", 1U);
    registry::set_value(sub_key, L"SupportMultiframe", 0U);
    registry::set_value(sub_key, L"Vendor", guid_to_string(id::vendor_team_charls).c_str());
    registry::set_value(sub_key, L"Version", VERSION);

    const wstring formats_sub_key{sub_key + LR"(\Formats\)"};
    for (size_t i{}; i != format_count; ++i)
    {
        registry::set_value(formats_sub_key + guid_to_string(formats[i]), L"", L"");
    }

    // COM co-create registration.
    const wstring inproc_server_sub_key{sub_key + LR"(\InprocServer32\)"};
    registry::set_value(inproc_server_sub_key, L"", get_module_path().c_str());
    registry::set_value(inproc_server_sub_key, L"ThreadingModel", L"Both");

    // WIC category registration.
    const wstring category_id_key{LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(wic_category_id) + LR"(\Instance\)" +
                                  guid_to_string(class_id)};
    registry::set_value(category_id_key, L"FriendlyName", friendly_name);
    registry::set_value(category_id_key, L"CLSID", guid_to_string(class_id).c_str());
}

void register_decoder()
{
    const array formats{GUID_WICPixelFormat2bppGray,  GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray,
                        GUID_WICPixelFormat16bppGray, GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat48bppRGB};

    register_general_decoder_encoder_settings(id::jpegls_decoder, CATID_WICBitmapDecoders, L"JPEG-LS Decoder",
                                              formats.data(), formats.size());

    const wstring sub_key{LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(id::jpegls_decoder)};

    const wstring patterns_sub_key{sub_key + LR"(\Patterns\0)"};
    registry::set_value(patterns_sub_key, L"Length", 3);
    registry::set_value(patterns_sub_key, L"Position", 0U);

    constexpr array mask{0xFF_byte, 0xFF_byte, 0xFF_byte};
    registry::set_value(patterns_sub_key, L"Mask", mask.data(), static_cast<DWORD>(mask.size()));
    constexpr array pattern{0xFF_byte, 0xD8_byte, 0xFF_byte};
    registry::set_value(patterns_sub_key, L"Pattern", pattern.data(), static_cast<DWORD>(pattern.size()));

    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"", L"jlsfile");
    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"Content Type", L"image/jls");
    registry::set_value(LR"(SOFTWARE\Classes\.jls\)", L"PerceivedType", L"image"); // Can be used by Windows Vista and newer

    registry::set_value(LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\KindMap)", L".jls", L"picture");

    // Register with the Windows Thumbnail Cache
    const wstring IID_IThumbnailProvider{L"{e357fccd-a995-4576-b01f-234630154e96}"};
    constexpr wchar_t CLSID_PhotoThumbnailProvider[]{L"{c7657c4a-9f68-40fa-a4df-96bc08eb3551}"};

    registry::set_value(LR"(SOFTWARE\Classes\.jls\ShellEx\)" + IID_IThumbnailProvider, L"", CLSID_PhotoThumbnailProvider);
    registry::set_value(LR"(SOFTWARE\Classes\SystemFileAssociations\.jls\ShellEx\)" + IID_IThumbnailProvider,
                        L"", CLSID_PhotoThumbnailProvider);

    // Register with the legacy Windows Photo Viewer (still installed on Windows 10), just forward to the TIFF registration.
    registry::set_value(LR"(SOFTWARE\Microsoft\Windows Photo Viewer\Capabilities\FileAssociations)", L".jls",
                        L"PhotoViewer.FileAssoc.Tiff");
}

void register_encoder()
{
    const array formats{GUID_WICPixelFormat2bppGray,  GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray,
                        GUID_WICPixelFormat16bppGray, GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat24bppRGB,
                        GUID_WICPixelFormat48bppRGB};

    register_general_decoder_encoder_settings(id::jpegls_encoder, CATID_WICBitmapEncoders, L"JPEG-LS Encoder",
                                              formats.data(), formats.size());
}

HRESULT unregister(const GUID& class_id, const GUID& wic_category_id)
{
    const wstring sub_key{LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(class_id)};
    const HRESULT result1{registry::delete_tree(sub_key.c_str())};

    const wstring category_id_key{LR"(SOFTWARE\Classes\CLSID\)" + guid_to_string(wic_category_id) + LR"(\Instance\)" +
                                  guid_to_string(class_id)};
    const HRESULT result2{registry::delete_tree(category_id_key.c_str())};

    return failed(result1) ? result1 : result2;
}

} // namespace

// ReSharper disable CppInconsistentNaming
// ReSharper disable CppParameterNamesMismatch

BOOL __stdcall DllMain(const HMODULE module, const DWORD reason_for_call, void* /*reserved*/) noexcept
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
        return false;
    }

    return true;
}

// Purpose: Used to determine whether the COM sub-system can unload the DLL from memory.
_Use_decl_annotations_ HRESULT __stdcall DllCanUnloadNow()
{
    const auto result = winrt::get_module_lock() ? S_FALSE : S_OK;
    TRACE("jpegls-wic-codec::DllCanUnloadNow hr = {} (0 = S_OK -> unload OK)\n", result);
    return result;
}

// Purpose: Returns a class factory to create an object of the requested type
_Use_decl_annotations_ HRESULT __stdcall DllGetClassObject(GUID const& class_id, GUID const& interface_id, void** result)
try
{
    if (class_id == id::jpegls_decoder)
        return create_jpegls_bitmap_decoder_factory(interface_id, result);

    if (class_id == id::jpegls_encoder)
        return create_jpegls_bitmap_encoder_factory(interface_id, result);

    return CLASS_E_CLASSNOTAVAILABLE;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall DllRegisterServer()
try
{
    register_decoder();
    register_encoder();

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    return error_ok;
}
catch (...)
{
    return SELFREG_E_CLASS;
}

HRESULT __stdcall DllUnregisterServer()
try
{
    const HRESULT result_decoder{unregister(id::jpegls_decoder, CATID_WICBitmapDecoders)};
    const HRESULT result_encoder{unregister(id::jpegls_encoder, CATID_WICBitmapEncoders)};

    // Note: keep the .jls file registration intact.

    return failed(result_decoder) ? result_decoder : result_encoder;
}
catch (...)
{
    return winrt::to_hresult();
}

// ReSharper restore CppParameterNamesMismatch
// ReSharper restore CppInconsistentNaming
