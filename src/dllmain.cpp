// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

BOOL APIENTRY DllMain(HMODULE /*module*/, const DWORD  reason_for_call,LPVOID /*reserved*/)
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
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
