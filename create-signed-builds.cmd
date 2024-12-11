:: SPDX-FileCopyrightText: © 2024 Team CharLS
:: SPDX-License-Identifier: BSD-3-Clause

:: Build signed x86
msbuild -t:clean -p:Configuration=Release -p:Platform=Win32
msbuild -t:restore -p:Configuration=Release -p:Platform=Win32
msbuild -t:build -p:ContinuousIntegrationBuild=true -p:Configuration=Release -p:Platform=Win32 -p:CHARLS_DETERMINISTIC_BUILD=true -p:SignOutput=true -p:CertificateThumbprint=%1 -p:TimestampUrl=%2

:: Build signed x64
msbuild -t:clean -p:Configuration=Release -p:Platform=x64
msbuild -t:restore -p:Configuration=Release -p:Platform=x64
msbuild -t:build -p:ContinuousIntegrationBuild=true -p:Configuration=Release -p:Platform=x64 -p:CHARLS_DETERMINISTIC_BUILD=true -p:SignOutput=true -p:CertificateThumbprint=%1 -p:TimestampUrl=%2

:: Build signed ARM64
msbuild -t:clean -p:Configuration=Release -p:Platform=ARM64
msbuild -t:restore -p:Configuration=Release -p:Platform=ARM64
msbuild -t:build -p:ContinuousIntegrationBuild=true -p:Configuration=Release -p:Platform=ARM64 -p:CHARLS_DETERMINISTIC_BUILD=true -p:SignOutput=true -p:CertificateThumbprint=%1 -p:TimestampUrl=%2

