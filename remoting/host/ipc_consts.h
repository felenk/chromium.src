// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_IPC_CONSTS_H_
#define REMOTING_HOST_IPC_CONSTS_H_

#include "base/file_path.h"

namespace remoting {

// The command line switch specifying the daemon IPC endpoint.
extern const char kDaemonPipeSwitchName[];

// Name of the daemon process binary.
extern const FilePath::CharType kDaemonBinaryName[];

// Name of the host process binary.
extern const FilePath::CharType kHostBinaryName[];

// Returns the full path to an installed |binary| in |full_path|.
bool GetInstalledBinaryPath(const FilePath::StringType& binary,
                            FilePath* full_path);

}  // namespace remoting

#endif  // REMOTING_HOST_IPC_CONSTS_H_
