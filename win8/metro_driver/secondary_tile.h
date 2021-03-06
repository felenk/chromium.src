// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CHROME_BROWSER_UI_METRO_DRIVER_SECONDARY_TILE_H_
#define CHROME_BROWSER_UI_METRO_DRIVER_SECONDARY_TILE_H_

#include "base/file_path.h"
#include "base/string16.h"

extern "C" __declspec(dllexport)
BOOL MetroIsPinnedToStartScreen(const string16& tile_id);

extern "C" __declspec(dllexport)
void MetroUnPinFromStartScreen(const string16& tile_id);

extern "C" __declspec(dllexport)
void MetroPinToStartScreen(const string16& tile_id,
                           const string16& title,
                           const string16& url,
                           const FilePath& logo_path);

#endif  // CHROME_BROWSER_UI_METRO_DRIVER_SECONDARY_TILE_H_

