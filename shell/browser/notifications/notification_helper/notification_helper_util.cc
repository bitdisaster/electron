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

// Searches the registry for a COM server registration
// of this executable. The registraton can be under HKLM or HKCU.
base::string16 GetToastActivatorClsidInternal(HKEY hive) {
  base::FilePath currentExePath;
  if (base::PathService::Get(base::FILE_EXE, &currentExePath)) {
    base::string16 currentExePathStr = L"\"" + currentExePath.value() + L"\"";
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
              if (pathToComServer == currentExePathStr) {
                return clsidKeyName;
              }
            }
          }
        }
        localServer_iterator.operator++();
      }
      class_iterator.operator++();
    }
    // if we end here then we didn't find ourselves in the COM server
    // registration
    Trace(
        L"Could not find a CLSID under which we should be registered as a COM "
        L"server.");
    return L"";
  } else {
    Trace(L"Could not find current executable path.");
    return L"";
  }
}

base::FilePath GetAppExePath() {
  // Extract the App name on the convention that this notifcation helper is
  // named <appName>_notification_helper.exe
  base::FilePath currentExePath;
  if (!base::PathService::Get(base::FILE_EXE, &currentExePath))
    return base::FilePath();
  base::FilePath currentExe = currentExePath.BaseName();
  std::vector<base::string16> arr = base::SplitString(
      currentExe.value(), L"_", base::WhitespaceHandling::TRIM_WHITESPACE,
      base::SplitResult::SPLIT_WANT_NONEMPTY);
  base::string16 appName = arr[0].append(L".exe");

  // Look for <appName>.exe one folder above <appName>_notification_helper.exe
  // (as expected in Electron installs). Failing that, look for it alongside
  // <appName>_notification_helper.exe
  base::FilePath dir_exe;
  if (!base::PathService::Get(base::DIR_EXE, &dir_exe)) {
    Trace(L"Failed ot find current executable directory.");
    return base::FilePath();
  }

  base::FilePath app_exe = dir_exe.DirName().Append(appName);

  if (!base::PathExists(app_exe)) {
    app_exe = dir_exe.Append(appName);
    if (!base::PathExists(app_exe)) {
      Trace(L"App exe not found in parent and neither in current directory.");
      return base::FilePath();
    }
  }
  return app_exe;
}

CLSID GetToastActivatorClsid() {
  base::string16 clsidStr = GetToastActivatorClsidInternal(HKEY_CURRENT_USER);
  if (clsidStr.empty()) {
    clsidStr = GetToastActivatorClsidInternal(HKEY_LOCAL_MACHINE);
  }

  CLSID toastActivatorClsid;
  HRESULT res = CLSIDFromString(clsidStr.c_str(), &toastActivatorClsid);
  if (res != NOERROR) {
    Trace(L"Invalid CLSID encountered. %s", clsidStr.c_str());
    return GUID_NULL;
  } else {
    return toastActivatorClsid;
  }
}

HRESULT RegisterComServer(base::string16 toastActivatorClsid) {
  CLSID clsid;
  HRESULT res = CLSIDFromString(toastActivatorClsid.c_str(), &clsid);
  if (res != NOERROR) {
    Trace(L"Invalid CLSID provided. %s", toastActivatorClsid.c_str());
    return E_FAIL;
  }

  base::FilePath currentExePath;
  if (base::PathService::Get(base::FILE_EXE, &currentExePath)) {
    Trace(toastActivatorClsid.c_str());
    base::win::RegKey key;
    base::string16 localServer32KeyPath =
        classesKeyPath + toastActivatorClsid + L"\\" + localServer32Key;
    Trace(classesKeyPath.c_str());
    HRESULT result =
        key.Create(HKEY_CURRENT_USER, localServer32KeyPath.c_str(), KEY_WRITE);
    if (result == ERROR_SUCCESS) {
      base::string16 currentExePathValue =
          L"\"" + currentExePath.value() + L"\"";
      result = key.WriteValue(nullptr, currentExePathValue.c_str());
    }
    return result;
  } else {
    Trace(L"Could not find current executable path.");
    return E_FAIL;
  }
}

HRESULT UnregisterComServer() {
  base::string16 toastActivatorClsid =
      GetToastActivatorClsidInternal(HKEY_CURRENT_USER);
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
}
}  // namespace notification_helper
