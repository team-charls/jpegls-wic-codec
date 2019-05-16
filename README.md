# JPEG-LS Windows Imaging Component Codec

This Windows Imaging Component (WIC) codec makes it possible to decode and encode JPEG-LS (.jls) files in Windows applications that can leverage WIC codecs. This makes it possible to see JPEG-LS files in Windows PhotoViewer, Windows Explorer and import .jls in Microsoft Office documents.

[![Build Status](https://dev.azure.com/team-charls/jpegls-wic-codec/_apis/build/status/team-charls.jpegls-wic-codec?branchName=master)](https://dev.azure.com/team-charls/jpegls-wic-codec/_build/latest?definitionId=1&branchName=master)

## Introduction

The Windows Imaging Component (WIC) is a build-in codec framework of Windows that makes it possible to create independent native image codecs that can be used by a large set of applications. WIC is implemented using COM technology. Image codecs for many popular formats are already pre-installed on Windows.

### Status

This project is the development phase:

- Only a x64 version of the codec is provided
- Only the decoder path is currently functional
- No installer with pre-build binaries is available, manually building and registering is required.

### Supported Operation Systems

The JPEG-LS WIC codec supports the following Windows operating systems:

- Windows 10 x86-64
- Windows Server 2016

### Applications that can use the JPEG-LS WIC codec

The following application have been validated to work with the JPEG-LS WIC codec:

- Windows Explorer Thumbnail cache. This functionality allows Windows Explorer to show previews of images.
- Windows Photo Viewer.  
 Note: On clean installations of Windows 10 this component is installed but not registered. Instructions how to enable this Photo viewer can be found on the Internet.
- WIC Explorer (sample application from Microsoft). An updated version of this application can be found at <https://github.com/vbaderks/WICExplorer>
- ZackViewer <https://github.com/peirick/ZackViewer>. This viewer can also be used to convert from one image encoding format to another.

#### Windows 10 Microsoft Photos Application not supported

The standard Windows 10 Microsoft Photos application cannot be used at this moment as it is limited to the WIC codecs that are pre-installed on Windows or are provided by Microsoft in the Microsoft Store.
Microsoft currently does not make it possible to create WIC codecs that can be uploaded to the Microsoft Store.

## WIC Codec Identity

The following table provides the codec identification information:

|Property||
|---|---|
|Formal Name|JPEG-LS Format|
|File Name Extension|.jls|
|MIME type| image/jls|
|Specification Support| JPEG-LS (ISO/IEC 14495-1), SPIFF header (ISO/IEC 10918-3)|

The following table lists the GUIDs used to identify the native JPEG-LS codec components:

|Component|Friendly Name|GUID
|---|---|---|
|Container Format|GUID_ContainerFormatJpegLS|52c25458-282d-4ef4-a69f-021bb2984543
|Decoder|CLSID_JpegLSDecoder|e57dc18b-019c-47f2-8ed0-bf587be4ff1b|
|Encoder|CLSID_JpegLSEncoder|70a823ea-009f-402f-9bda-e9b8f6332d61|

The following table lists the pixel formats that can be decoded:

|GUID|Component Count|Bits per Sample
|---|---|---|
|GUID_WICPixelFormat2bppGray|1|2|
|GUID_WICPixelFormat4bppGray|1|4|
|GUID_WICPixelFormat8bppGray|1|8|
|GUID_WICPixelFormat16bppGray|1|16|
|GUID_WICPixelFormat24bppRGB|3|8|
|GUID_WICPixelFormat48bppRGB|3|16|

## Build Instructions

1. Clone this repro, use clone --recurse-submodules to ensure the CharLS git submodule is also cloned correctly in your local git repository.
1. Open Visual Studio 2017 or 2019 16.1 or newer and open the jpeg-wic-codec.sln. Batch build all projects.  
Note: Visual Studio 2019 16.0 fails to copy the output of the CharLS project to the final output folder. Workaround to use VS 2019 16.0 is to copy these DLLs manually.
1. Or use a Developer Command Prompt and run use MSbuild in the root of the cloned repository.

## Installation

1. Open a command prompt with elevated rights
1. Navigate to folder with the jpegls-wic-codec.dll
1. Execute:

```shell
regsvr32 jpegls-wic-codec.dll
```

## Uninstall

1. Open a command prompt with elevated rights
1. Navigate to folder with the jpegls-wic-codec.dll
1. Execute:

```shell
regsvr32 -u jpegls-wic-codec.dll
```