// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Combaseapi.h>
#include <rpc.h>
#include <windows.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/process/memory.h"
#include "base/win/process_startup_helper.h"
#include "base/win/scoped_winrt_initializer.h"
#include "electron/shell/browser/notifications/notification_helper/com_server_module.h"
// #include "chrome/notification_helper/notification_helper_constants.h"
// #include
// "chrome/notification_helper/notification_helper_crash_reporter_client.h"
#include "electron/shell/browser/notifications/notification_helper/notification_helper_util.h"
#include "electron/shell/browser/notifications/notification_helper/trace_util.h"

extern "C" int WINAPI wWinMain(HINSTANCE instance,
                               HINSTANCE prev_instance,
                               wchar_t* command_line,
                               int show_command) {
  // Initialize the CommandLine singleton from the environment.
  base::CommandLine::Init(0, nullptr);

  // This process is designed to be launched by COM only, which appends the
  // "-Embedding" flag to the command line. If this flag is not found, the
  // process should exit immediately.
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms683844.aspx
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch("embedding")) {
    Trace(L"Not COM activated. Exiting");
    return 0;
  }

  // The exit manager is in charge of calling the dtors of singletons.
  base::AtExitManager exit_manager;

  // TODO: Crash Reporter
  // Use crashpad embedded in chrome.exe as the crash handler.
  // base::FilePath chrome_exe_path = notification_helper::GetChromeExePath();
  // if (!chrome_exe_path.empty()) {
  //   NotificationHelperCrashReporterClient::
  //       InitializeCrashReportingForProcessWithHandler(chrome_exe_path);
  // }

  // Make sure the process exits cleanly on unexpected errors.
  base::EnableTerminationOnHeapCorruption();
  base::EnableTerminationOnOutOfMemory();
  base::win::RegisterInvalidParamHandler();
  base::win::SetupCRT(*base::CommandLine::ForCurrentProcess());

  base::win::ScopedWinrtInitializer winrt_initializer;
  if (!winrt_initializer.Succeeded()) {
    Trace(L"Failed initializing WinRT\n");
    return -1;
  }

  notification_helper::ComServerModule com_server_module;
  com_server_module.Run();

  return 0;
}
