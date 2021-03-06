// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_PLUGIN_TEST_BROWSER_PLUGIN_EMBEDDER_H_
#define CONTENT_BROWSER_BROWSER_PLUGIN_TEST_BROWSER_PLUGIN_EMBEDDER_H_

#include "base/compiler_specific.h"
#include "content/browser/browser_plugin/browser_plugin_embedder.h"
#include "content/public/test/test_utils.h"

namespace content {

class BrowserPluginGuest;
class RenderViewHost;
class WebContentsImpl;

// Test class for BrowserPluginEmbedder.
//
// Provides utilities to wait for certain state/messages in
// BrowserPluginEmbedder to be used in tests.
class TestBrowserPluginEmbedder : public BrowserPluginEmbedder {
 public:
  TestBrowserPluginEmbedder(WebContentsImpl* web_contents,
                            RenderViewHost* render_view_host);
  virtual ~TestBrowserPluginEmbedder();

  const ContainerInstanceMap& guest_web_contents_for_testing() const {
    return guest_web_contents_by_instance_id_;
  }

  // Waits until at least one guest is added to this embedder.
  void WaitForGuestAdded();

  WebContentsImpl* web_contents() const;

 private:
  // Overridden to intercept in test.
  virtual void AddGuest(int instance_id,
                        WebContents* guest_web_contents) OVERRIDE;

  scoped_refptr<MessageLoopRunner> message_loop_runner_;

  DISALLOW_COPY_AND_ASSIGN(TestBrowserPluginEmbedder);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BROWSER_PLUGIN_TEST_BROWSER_PLUGIN_EMBEDDER_H_
