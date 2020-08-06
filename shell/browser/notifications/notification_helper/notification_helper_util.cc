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

// Searches the registry for a COM server registration
// of this executable. The registraton can be under HKLM or HKCU.
CLSID GetToastActivatorClsidInternal(HKEY hive) {
  base::FilePath currentExePath;
  if (base::PathService::Get(base::FILE_EXE, &currentExePath)) {
    base::string16 currentExePathStr = L"\"" + currentExePath.value() + L"\"";
    base::string16 classesKeyPath = L"SOFTWARE\\Classes\\CLSID";
    base::win::RegistryKeyIterator class_iterator(hive, classesKeyPath.c_str());
    while (class_iterator.Valid()) {
      std::wstring clsidKeyName(class_iterator.Name());
      std::wstring subKeyPath = classesKeyPath + L"\\" + clsidKeyName;
      Trace(subKeyPath.c_str());
      Trace(L"\n");
      base::win::RegistryKeyIterator localServer_iterator(hive,
                                                          subKeyPath.c_str());
      while (localServer_iterator.Valid()) {
        std::wstring subKeyName(localServer_iterator.Name());
        if (subKeyName == L"LocalServer32") {
          base::win::RegKey key;
          base::string16 localServerKeyPath = subKeyPath + L"\\LocalServer32";
          if (key.Open(hive, localServerKeyPath.c_str(), KEY_READ) ==
              ERROR_SUCCESS) {
            std::wstring pathToComServer;
            if (key.ReadValue(L"", &pathToComServer) == ERROR_SUCCESS) {
              if (pathToComServer == currentExePathStr) {
                CLSID clsid;
                HRESULT res = CLSIDFromString(clsidKeyName.c_str(), &clsid);
                if (res != NOERROR) {
                  Trace(L"Invalid CLSID encountered. %s", clsidKeyName.c_str());
                  return GUID_NULL;
                } else {
                  return clsid;
                }
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
    return GUID_NULL;
  } else {
    Trace(L"Could not find current executable path.");
    return GUID_NULL;
  }
}

base::FilePath GetAppExePath() {
  // Extract the App name on the convention that this notifcation helper is
  // named <appName>_notification_helper.exe
  base::FilePath currentExePath;
  if (!base::PathService::Get(base::FILE_EXE, &currentExePath))
    return base::FilePath();
  base::FilePath currentExe = currentExePath.BaseName();
  std::vector<std::wstring> arr = base::SplitString(
      currentExe.value(), L"_", base::WhitespaceHandling::TRIM_WHITESPACE,
      base::SplitResult::SPLIT_WANT_NONEMPTY);
  std::wstring appName = arr[0].append(L".exe");

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
  CLSID toastActivatorClsid = GetToastActivatorClsidInternal(HKEY_CURRENT_USER);
  if (toastActivatorClsid == GUID_NULL) {
    toastActivatorClsid = GetToastActivatorClsidInternal(HKEY_LOCAL_MACHINE);
  }
  return toastActivatorClsid;
}

}  // namespace notification_helper
