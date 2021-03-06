// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, so no include guard.

#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_param_traits.h"
#include "ipc/ipc_platform_file.h"

#define IPC_MESSAGE_START FileUtilitiesMsgStart

// File utilities messages sent from the renderer to the browser.

IPC_SYNC_MESSAGE_CONTROL1_2(FileUtilitiesMsg_GetFileInfo,
                            FilePath /* path */,
                            base::PlatformFileInfo /* result */,
                            base::PlatformFileError /* status */)

// Open the file.
IPC_SYNC_MESSAGE_CONTROL2_1(FileUtilitiesMsg_OpenFile,
                            FilePath /* path */,
                            int /* mode */,
                            IPC::PlatformFileForTransit /* result */)
