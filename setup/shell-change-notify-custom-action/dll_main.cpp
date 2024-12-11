// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

#include <Windows.h>
#include <msiquery.h>
#include <ShlObj.h>

unsigned int __stdcall SHChangeNotifyCustomAction(const MSIHANDLE install)
{
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    // Create a log entry (allows checking if custom action was called correctly).
    PMSIHANDLE record{MsiCreateRecord(1)};
    MsiRecordSetString(record, 0, L"SHChangeNotify triggered");
    MsiProcessMessage(install, INSTALLMESSAGE_INFO, record);

    return ERROR_SUCCESS;
}
