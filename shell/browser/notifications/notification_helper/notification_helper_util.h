// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_NOTIFICATION_HELPER_NOTIFICATION_HELPER_UTIL_H_
#define CHROME_NOTIFICATION_HELPER_NOTIFICATION_HELPER_UTIL_H_

#include <Combaseapi.h>

#include "base/files/file_path.h"

namespace notification_helper {

// Returns the file path of the main App executable if found, or an empty file
// path if not.
base::FilePath GetAppExePath();

// Returns the CLSID under which this executable is registered as a COM server.
// This happens to be also the ToastActivatorClsid.
CLSID GetToastActivatorClsid();

HRESULT RegisterComServer(base::string16 toastActivatorClsid);

HRESULT UnregisterComServer();

}  // namespace notification_helper

#endif  // CHROME_NOTIFICATION_HELPER_NOTIFICATION_HELPER_UTIL_H_
