// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/audio_scheduler.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "remoting/codec/audio_encoder.h"
#include "remoting/host/audio_capturer.h"
#include "remoting/proto/audio.pb.h"
#include "remoting/protocol/audio_stub.h"

namespace remoting {

AudioScheduler::AudioScheduler(
    scoped_refptr<base::SingleThreadTaskRunner> audio_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> network_task_runner,
    AudioCapturer* audio_capturer,
    scoped_ptr<AudioEncoder> audio_encoder,
    protocol::AudioStub* audio_stub)
    : audio_task_runner_(audio_task_runner),
      network_task_runner_(network_task_runner),
      audio_capturer_(audio_capturer),
      audio_encoder_(audio_encoder.Pass()),
      audio_stub_(audio_stub),
      network_stopped_(false),
      enabled_(true) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());
  DCHECK(audio_capturer_);
  DCHECK(audio_stub_);

  audio_task_runner_->PostTask(
      FROM_HERE, base::Bind(&AudioScheduler::StartOnAudioThread, this));
}

void AudioScheduler::Stop(const base::Closure& done_task) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());
  DCHECK(!done_task.is_null());
  DCHECK(audio_stub_);

  // Clear |audio_stub_| to prevent audio packets being delivered to the client.
  audio_stub_ = NULL;

  audio_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AudioScheduler::StopOnAudioThread, this, done_task));
}

AudioScheduler::~AudioScheduler() {
}

void AudioScheduler::StartOnAudioThread() {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());

  // TODO(kxing): Do something with the return value.
  audio_capturer_->Start(
      base::Bind(&AudioScheduler::EncodeAudioPacket, this));
}

void AudioScheduler::StopOnAudioThread(const base::Closure& done_task) {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());
  audio_capturer_->Stop();

  network_task_runner_->PostTask(FROM_HERE, done_task);
}

void AudioScheduler::SetEnabled(bool enabled) {
  if (!audio_task_runner_->BelongsToCurrentThread()) {
    audio_task_runner_->PostTask(
        FROM_HERE, base::Bind(&AudioScheduler::SetEnabled, this, enabled));
    return;
  }

  enabled_ = enabled;
}

void AudioScheduler::EncodeAudioPacket(scoped_ptr<AudioPacket> packet) {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());
  DCHECK(packet.get());

  if (!enabled_)
    return;

  scoped_ptr<AudioPacket> encoded_packet =
      audio_encoder_->Encode(packet.Pass());

  // The audio encoder returns a NULL audio packet if there's no audio to send.
  if (encoded_packet.get()) {
    network_task_runner_->PostTask(
        FROM_HERE, base::Bind(&AudioScheduler::SendAudioPacket,
                              this, base::Passed(encoded_packet.Pass())));
  }
}

void AudioScheduler::SendAudioPacket(scoped_ptr<AudioPacket> packet) {
  DCHECK(network_task_runner_->BelongsToCurrentThread());
  DCHECK(packet.get());

  if (!audio_stub_)
    return;

  audio_stub_->ProcessAudioPacket(packet.Pass(), base::Closure());
}

}  // namespace remoting
