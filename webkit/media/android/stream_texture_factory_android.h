// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_MEDIA_ANDROID_STREAM_TEXTURE_FACTORY_ANDROID_H_
#define WEBKIT_MEDIA_ANDROID_STREAM_TEXTURE_FACTORY_ANDROID_H_

namespace WebKit {
class WebStreamTextureClient;
}

namespace webkit_media {

// The proxy class for the gpu thread to notify the compositor thread
// when a new video frame is available.
class StreamTextureProxy {
 public:
  virtual ~StreamTextureProxy() {}

  // Initialize the the stream_id, texture width and height. This should
  // be called on the compositor thread.
  virtual bool Initialize(int stream_id, int width, int height) = 0;

  virtual bool IsInitialized() = 0;

  // Setting the target for callback when a frame is available. This function
  // could be called on both the main thread and the compositor thread.
  virtual void SetClient(WebKit::WebStreamTextureClient* client) = 0;
};


// Factory class for managing the stream texture.
class StreamTextureFactory {
 public:
  virtual ~StreamTextureFactory() {}

  // Create the StreamTextureProxy object.
  virtual StreamTextureProxy* CreateProxy() = 0;

  // Send an IPC message to the browser process to request a java surface
  // object for the given stream_id. After the the surface is created,
  // it will be passed back to the WebMediaPlayerAndroid object identified by
  // the player_id.
  virtual void EstablishPeer(int stream_id, int player_id) = 0;

  // Create the streamTexture and return the stream Id and set the texture id.
  virtual unsigned CreateStreamTexture(unsigned* texture_id) = 0;

  // Destroy the streamTexture for the given texture Id.
  virtual void DestroyStreamTexture(unsigned texture_id) = 0;
};

} // namespace webkit_media

#endif  // WEBKIT_MEDIA_ANDROID_STREAM_TEXTURE_FACTORY_ANDROID_H_
