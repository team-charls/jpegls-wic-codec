#pragma once

// The purpose of this interface definition is to add noexcept.
// COM methods cannot throw, but lack the noexcept specifier.
// Note: IDL compiled with MIDLRT will have noexcept.

#include <Unknwn.h>

#ifdef __IWICBitmapSource_INTERFACE_DEFINED__
#error Include this header file before wincodec.h
#endif

#define __IWICBitmapSource_INTERFACE_DEFINED__  // NOLINT(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)

using WICPixelFormatGUID = GUID;
struct IWICPalette;
struct WICRect;

MIDL_INTERFACE("00000120-a8f2-4877-ba0a-fd2b6645fb94")
IWICBitmapSource : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetSize(
        /* [out] */ __RPC__out UINT * puiWidth,
        /* [out] */ __RPC__out UINT * puiHeight) noexcept = 0;

    virtual HRESULT STDMETHODCALLTYPE GetPixelFormat(
        /* [out] */ __RPC__out WICPixelFormatGUID * pPixelFormat) noexcept = 0;

    virtual HRESULT STDMETHODCALLTYPE GetResolution(
        /* [out] */ __RPC__out double* pDpiX,
        /* [out] */ __RPC__out double* pDpiY) noexcept = 0;

    virtual HRESULT STDMETHODCALLTYPE CopyPalette(
        /* [in] */ __RPC__in_opt IWICPalette * pIPalette) noexcept = 0;

    virtual HRESULT STDMETHODCALLTYPE CopyPixels(
        /* [unique][in] */ __RPC__in_opt const WICRect* prc,
        /* [in] */ UINT cbStride,
        /* [in] */ UINT cbBufferSize,
        /* [size_is][out] */ __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer) noexcept = 0;
};
