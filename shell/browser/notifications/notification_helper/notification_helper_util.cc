// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "electron/shell/browser/notifications/notification_helper/notification_helper_util.h"

#include "base/base_paths.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_split.h"
#include "electron/shell/browser/notifications/notification_helper/trace_util.h"

namespace notification_helper {

void TracePath(base::FilePath path) {
  Trace(L"path %s \n", path.value().c_str());
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

  TracePath(app_exe);

  if (!base::PathExists(app_exe)) {
    app_exe = dir_exe.Append(appName);
    if (!base::PathExists(app_exe)) {
      Trace(L"App exe not found in parent and neither in current directory.");
      return base::FilePath();
    }
  }
  TracePath(app_exe);
  return app_exe;
}
}  // namespace notification_helper
