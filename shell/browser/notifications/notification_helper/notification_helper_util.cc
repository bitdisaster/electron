// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "electron/shell/browser/notifications/notification_helper/notification_helper_util.h"

#include <Combaseapi.h>

#include "base/base_paths.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_split.h"
#include "base/win/registry.h"
#include "electron/shell/browser/notifications/notification_helper/trace_util.h"

namespace notification_helper {

const base::string16 classesKeyPath = L"SOFTWARE\\Classes\\CLSID\\";
const base::string16 localServer32Key = L"LocalServer32";

bool clsidToString(CLSID clsid, base::string16* clsidStr) {
  wchar_t* clsid_t = nullptr;
  if (StringFromCLSID(clsid, &clsid_t) == S_OK) {
    *clsidStr = base::string16(clsid_t);
    CoTaskMemFree(clsid_t);
    return true;
  } else {
    return false;
  }
}

// Searches the registry for a COM server registration
// of this executable. The registration can be under HKLM or HKCU.
base::string16 GetToastActivatorClsidInternal(HKEY hive,
                                              base::FilePath helperExePath) {
  if (!helperExePath.empty()) {
    base::string16 helperExePathStr = L"\"" + helperExePath.value() + L"\"";
    base::win::RegistryKeyIterator class_iterator(hive, classesKeyPath.c_str());
    while (class_iterator.Valid()) {
      base::string16 clsidKeyName(class_iterator.Name());
      base::string16 subKeyPath = classesKeyPath + L"\\" + clsidKeyName;
      base::win::RegistryKeyIterator localServer_iterator(hive,
                                                          subKeyPath.c_str());
      while (localServer_iterator.Valid()) {
        base::string16 subKeyName(localServer_iterator.Name());
        if (subKeyName == localServer32Key) {
          base::win::RegKey key;
          base::string16 localServerKeyPath =
              subKeyPath + L"\\" + localServer32Key;
          if (key.Open(hive, localServerKeyPath.c_str(), KEY_READ) ==
              ERROR_SUCCESS) {
            base::string16 pathToComServer;
            if (key.ReadValue(L"", &pathToComServer) == ERROR_SUCCESS) {
              key.Close();
              if (pathToComServer == helperExePathStr) {
                return clsidKeyName;
              }
            }
          }
        }
        localServer_iterator.operator++();
      }
      class_iterator.operator++();
    }
    // If we end here then we didn't find ourselves in the COM server
    // registration.
    Trace(
        L"Could not find a CLSID under which we should be registered as a COM "
        L"server.");
    return L"";
  } else {
    Trace(L"No valid executable path provided.");
    return L"";
  }
}

base::FilePath GetNotificationHelperExePath() {
  // Extract the path to the notifcation helper executable from
  // the view point of the Electron App. By convention the
  // notifcation helper is named <appName>_notification_helper.exe
  base::FilePath currentExePath;
  if (!base::PathService::Get(base::FILE_EXE, &currentExePath)) {
    Trace(L"Failed ot find current executable path.");
    return base::FilePath();
  }
  base::FilePath appName = currentExePath.BaseName().RemoveExtension();
  base::string16 helperExe = appName.value() + L"_notification_helper.exe";

  // Look for <appName>_notification_helper.exe alongside
  // <appName>.exe
  base::FilePath dir_exe;
  if (!base::PathService::Get(base::DIR_EXE, &dir_exe)) {
    Trace(L"Failed ot find current executable directory.");
    return base::FilePath();
  }

  base::FilePath helperExePath = dir_exe.Append(helperExe);
  if (base::PathExists(helperExePath)) {
    return helperExePath;
  } else {
    Trace(L"Failed to find notification helper where it should be.");
    return base::FilePath();
  }
}

base::FilePath GetElectronAppExePath() {
  // Resolves the path to the Electron executable from the view point
  // of the notification helper. By convention t the notifcation helper
  // is named <appName>_notification_helper.exe
  base::FilePath currentExePath;
  if (!base::PathService::Get(base::FILE_EXE, &currentExePath))
    return base::FilePath();
  base::FilePath currentExe = currentExePath.BaseName();
  std::vector<base::string16> arr = base::SplitString(
      currentExe.value(), L"_", base::WhitespaceHandling::TRIM_WHITESPACE,
      base::SplitResult::SPLIT_WANT_NONEMPTY);
  base::string16 appName = arr[0].append(L".exe");

  // Look for <appName>.exe alongside <appName>_notification_helper.exe
  base::FilePath dir_exe;
  if (!base::PathService::Get(base::DIR_EXE, &dir_exe)) {
    Trace(L"Failed ot find current executable directory.");
    return base::FilePath();
  }

  base::FilePath appExePath = dir_exe.Append(appName);

  if (!base::PathExists(appExePath)) {
    Trace(L"App exe not found in current directory.");
    return base::FilePath();
  }

  return appExePath;
}

CLSID GetToastActivatorClsid() {
  base::FilePath currentExePath;
  if (base::PathService::Get(base::FILE_EXE, &currentExePath)) {
    base::string16 clsidStr =
        GetToastActivatorClsidInternal(HKEY_CURRENT_USER, currentExePath);
    if (clsidStr.empty()) {
      clsidStr =
          GetToastActivatorClsidInternal(HKEY_LOCAL_MACHINE, currentExePath);
    }

    CLSID toastActivatorClsid;
    HRESULT res = CLSIDFromString(clsidStr.c_str(), &toastActivatorClsid);
    if (res != NOERROR) {
      Trace(L"Invalid CLSID encountered. %s", clsidStr.c_str());
      return GUID_NULL;
    } else {
      return toastActivatorClsid;
    }
  } else {
    return GUID_NULL;
  }
}

// This method is meant to be called from Electron directly and
// not from the helper executable itself.
HRESULT RegisterComServer(CLSID clsid) {
  base::string16 toastActivatorClsid;
  if (clsidToString(clsid, &toastActivatorClsid)) {
    base::FilePath helperExePath = GetNotificationHelperExePath();
    if (!helperExePath.empty()) {
      Trace(toastActivatorClsid.c_str());
      base::win::RegKey key;
      base::string16 localServer32KeyPath =
          classesKeyPath + toastActivatorClsid + L"\\" + localServer32Key;
      Trace(localServer32KeyPath.c_str());
      HRESULT result = key.Create(HKEY_CURRENT_USER,
                                  localServer32KeyPath.c_str(), KEY_WRITE);
      if (result == ERROR_SUCCESS) {
        base::string16 helperExePathStr = L"\"" + helperExePath.value() + L"\"";
        result = key.WriteValue(nullptr, helperExePathStr.c_str());
      }
      return result;
    } else {
      Trace(L"Could not find path to helper executable.");
      return E_FAIL;
    }
  } else {
    Trace(L"Could not format CLSID as a string. Likely invalid CLSID.");
    return E_FAIL;
  }
}

HRESULT UnregisterComServer() {
  base::FilePath helperExePath = GetNotificationHelperExePath();
  if (!helperExePath.empty()) {
    base::string16 toastActivatorClsid =
        GetToastActivatorClsidInternal(HKEY_CURRENT_USER, helperExePath);
    if (!toastActivatorClsid.empty()) {
      base::win::RegKey key;
      HRESULT result =
          key.Open(HKEY_CURRENT_USER, classesKeyPath.c_str(), KEY_WRITE);
      if (result == ERROR_SUCCESS) {
        result = key.DeleteKey(toastActivatorClsid.c_str());
      }
      return result;
    }
    return ERROR_SUCCESS;
  } else {
    Trace(L"Unable to find notification helper executable.");
    return E_FAIL;
  }
}
}  // namespace notification_helper
