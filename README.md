<!--
  SPDX-FileCopyrightText: © 2019 Team CharLS
  SPDX-License-Identifier: BSD-3-Clause
-->

# JPEG-LS Windows Imaging Component Codec

[![Build Status](https://dev.azure.com/team-charls/jpegls-wic-codec/_apis/build/status/team-charls.jpegls-wic-codec?branchName=main)](https://dev.azure.com/team-charls/jpegls-wic-codec/_build/latest?definitionId=1&branchName=main)
[![REUSE status](https://api.reuse.software/badge/github.com/team-charls/jpegls-wic-codec)](https://api.reuse.software/info/github.com/team-charls/jpegls-wic-codec)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=team-charls_jpegls-wic-codec&metric=sqale_rating)](https://sonarcloud.io/summary/overall?id=team-charls_jpegls-wic-codec)

## Introduction

This WIC (Windows Imaging Component) codec makes it possible to decode and encode JPEG-LS (.jls) files with Windows applications that can use WIC codecs. It allows to view JPEG-LS encoded images in Windows PhotoViewer, Windows Explorer and import JPEG-LS images in Microsoft Office documents.

It is a C++ 23 implementation that uses the CharLS JPEG-LS codec.
The C++/WinRT framework is used to implement the classic COM components required for a WIC codec and it uses Wix v5 for the installer.

### Windows Imaging Component Background

Windows Imaging Component (WIC) is a built-in codec framework of Windows that makes it possible to create independent native image codecs that can be used by a large set of applications. WIC is implemented using COM technology. Many image codecs for popular formats are standard pre-installed on Windows.

## Installing

### Requirements

- Windows 11 or Windows 10 version 22H2.
- x86, x64 or ARM64 processor.

### Via GitHub with EXE

Go to the [releases](https://github.com/team-charls/jpegls-wic-codec/releases) page and click on
Assets at the bottom to show the files available in the release.
Please use the appropriate installer that matches your machine's architecture.

> [!NOTE]
> Microsoft Defender SmartScreen may show a warning about an unrecognized app when running the installer. Click on "More Info" + "Run anyway" to continue the installation.  
The installer and the DLL are signed, but Defender SmartScreen requires an EV code signing certificate, which is only available to commercial organizations.

## Applications that can use the JPEG-LS WIC codec

The following application have been validated to work with the JPEG-LS WIC codec:

- Windows Explorer Thumbnail cache. This functionality allows Windows Explorer to show previews of images.
- Windows Photo Viewer.  
 Note: On clean installations of Windows 10 this component is installed but not registered. An Registry file called restore-windows-photo-viewer.reg is provided to restore this functionality. The Registry Editor can be used to add import this .reg file.
- WIC Explorer (sample application from Microsoft). An updated version of this application can be found at <https://github.com/vbaderks/WICExplorer>
- ZackViewer <https://github.com/peirick/ZackViewer>. This viewer can also be used to convert from one image encoding format to another.
- Microsoft Office applications like Word, Excel, PowerPoint. These applications can, when the JPEG-LS codec is installed, import JPEG-LS images in their documents.

### Windows 10 Microsoft Photos Application not supported

The standard Windows 10 Microsoft Photos application cannot be used at this moment as it is limited to the WIC codecs that are pre-installed on Windows or are provided by Microsoft in the Microsoft Store.
Microsoft currently does not make it possible to create WIC codecs that can be uploaded to the Microsoft Store.

#### 16 bit grayscale images may display as only containg black pixels

Windows has full support for the 16 bit grayscale pixel format (GUID_WICPixelFormat16bppGray).
It expect however that the full dynamic range (0 .. 65535) is used. To be able to display the image
on the screen Windows needs to convert the image internally to 24 bits BRGA. In essence this means
that only the high 8 bits are used.  
Medical (DICOM) grayscale images are often using more then 256 different values of gray and will therefore
need to use 10, 12 or even 16 bits. If the image format is 16 bit but the dynamic range is much smaller, standard
Windows viewing software will show these images as a black rectangle.  
Medical viewing software should be used to view these kinds of images. Often additional metadata, not stored in the .jls file,
is needed for a good display.  
The WIC Explorer, see the section above, has also special support to transform the pixels into the correct dynamic range and can be used to view these images.

## WIC Codec Identity

The following table provides the codec identification information:

| Property              |                                                           |
|-----------------------|-----------------------------------------------------------|
| Formal Name           | JPEG-LS Format                                            |
| File Name Extension   | .jls                                                      |
| MIME type             | image/jls                                                 |
| Specification Support | JPEG-LS (ISO/IEC 14495-1), SPIFF header (ISO/IEC 10918-3) |

The following table lists the GUIDs used to identify the Team CharLS JPEG-LS codec component:

| Component        | GUID                                 |
|------------------|--------------------------------------|
| Container Format | 52c25458-282d-4ef4-a69f-021bb2984543 |
| Decoder          | e57dc18b-019c-47f2-8ed0-bf587be4ff1b |
| Encoder          | 70a823ea-009f-402f-9bda-e9b8f6332d61 |

The following table lists the pixel formats that can be decoded:

| WIC Format GUID              | Component Count | Bits per Sample |
|------------------------------|-----------------|-----------------|
| GUID_WICPixelFormat2bppGray  | 1               | 2               |
| GUID_WICPixelFormat4bppGray  | 1               | 4               |
| GUID_WICPixelFormat8bppGray  | 1               | 8               |
| GUID_WICPixelFormat16bppGray | 1               | 16,12,10*       |
| GUID_WICPixelFormat24bppRGB  | 3               | 8               |
| GUID_WICPixelFormat48bppRGB  | 3               | 16              |

Note \*: monochrome JPEG-LS images with 10 or 12 pixels will be upscaled to 16 to match a defined WIC pixel format.

The following table lists the pixel formats that can be encoded:

| WIC Format GUID                 | Component Count | Bits per Sample |
|---------------------------------|-----------------|-----------------|
| GUID_WICPixelFormat2bppGray     | 1               | 2               |
| GUID_WICPixelFormat4bppGray     | 1               | 4               |
| GUID_WICPixelFormat8bppGray     | 1               | 8               |
| GUID_WICPixelFormat16bppGray    | 1               | 16              |
| GUID_WICPixelFormat24bppBGR\*\* | 3               | 8               |
| GUID_WICPixelFormat24bppRGB     | 3               | 8               |
| GUID_WICPixelFormat48bppRGB     | 3               | 16              |

Note \*\*: BGR images will be converted and saved as RGB. JPEG-LS provides no support to set a BGR color space in the SPIFF header.

## Manual Build Instructions

Remark: to build this repository Visual Studio 2022 17.13 or newer with the extension HeatWave for VS2022 installed is needed.

1. Clone this repo, use clone --recurse-submodules to ensure the CharLS git submodule is also cloned correctly in your local git repository.
2. Use Visual Studio to open the jpegls-wic-codec.sln. Batch build all projects.
3. Or use a Developer Command Prompt and run MSBuild in the root of the cloned repository.

### Installation

1. Open a command prompt with elevated rights
2. Navigate to folder with the jpegls-wic-codec.dll
3. Execute:

```shell
regsvr32 jpegls-wic-codec.dll
```

### Uninstall

1. Open a command prompt with elevated rights
2. Navigate to folder with the jpegls-wic-codec.dll
3. Execute:

```shell
regsvr32 -u jpegls-wic-codec.dll
```

### Building and code signing

A command file is available to build and sign the WIC DLL, CharLS coded DLL and the setup application.  
Instructions:

- Open a Visual Studio Developer Command Prompt
- Go the root of the cloned repository
- Ensure a code signing certificate is available
- Execute the command `create-signed-builds.cmd certificate-thumb-print time-stamp-url`  
 Note: the certificate thumbprint and time stamp URL arguments are depending on the used code signing certificate.

 The WIC DLL, CharLS codec DLL and the installer will be signed for the release builds of x86, x64 and ARM64.
