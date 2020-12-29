
REM a WixProj cannot access Visual Studio C++ environment variables: copy merge modules:
copy "%VCINSTALLDIR%Redist\MSVC\v142\MergeModules\Microsoft_VC142_CRT_x86.msm" bin\x64\Release\
copy "%VCINSTALLDIR%Redist\MSVC\v142\MergeModules\Microsoft_VC142_CRT_x64.msm" bin\x64\Release\

msbuild jpegls-wic-codec.sln -p:Configuration=Release -p:Platform=Win32
msbuild jpegls-wic-codec.sln -p:Configuration=Release -p:Platform=x64
msbuild setup\setup.sln
