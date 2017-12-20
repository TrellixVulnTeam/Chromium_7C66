// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/media/cma/pipeline/audio_decoder_software_wrapper.h"

#include <ostream>

#include "base/bind.h"
#include "base/logging.h"
#include "base/threading/thread_task_runner_handle.h"
#include "chromecast/media/cma/base/decoder_buffer_base.h"

namespace chromecast {
namespace media {

namespace {

std::ostream& operator<<(std::ostream& stream, AudioCodec codec) {
  switch (codec) {
    case kAudioCodecUnknown:
      return stream << "unknown codec";
    case kCodecAAC:
      return stream << "AAC";
    case kCodecMP3:
      return stream << "MP3";
    case kCodecPCM:
      return stream << "PCM";
    case kCodecPCM_S16BE:
      return stream << "PCM_S16BE";
    case kCodecVorbis:
      return stream << "Vorbis";
    case kCodecOpus:
      return stream << "Opus";
    case kCodecEAC3:
      return stream << "EAC3";
    case kCodecAC3:
      return stream << "AC3";
    case kCodecDTS:
      return stream << "DTS";
    case kCodecFLAC:
      return stream << "FLAC";
  }
  NOTREACHED();
  return stream << "UNKNOWN";
}

}  // namespace

AudioDecoderSoftwareWrapper::AudioDecoderSoftwareWrapper(
    MediaPipelineBackend::AudioDecoder* backend_decoder)
    : backend_decoder_(backend_decoder),
      delegate_(nullptr),
      weak_factory_(this) {
  DCHECK(backend_decoder_);
  backend_decoder_->SetDelegate(this);
}

AudioDecoderSoftwareWrapper::~AudioDecoderSoftwareWrapper() {}

void AudioDecoderSoftwareWrapper::SetDelegate(DecoderDelegate* delegate) {
  DCHECK(delegate);
  delegate_ = delegate;
}

MediaPipelineBackend::BufferStatus AudioDecoderSoftwareWrapper::PushBuffer(
    CastDecoderBuffer* buffer) {
  DCHECK(buffer);
  if (!software_decoder_)
    return backend_decoder_->PushBuffer(buffer);

  DecoderBufferBase* buffer_base = static_cast<DecoderBufferBase*>(buffer);
  if (!software_decoder_->Decode(
          make_scoped_refptr(buffer_base),
          base::Bind(&AudioDecoderSoftwareWrapper::OnDecodedBuffer,
                     weak_factory_.GetWeakPtr()))) {
    return MediaPipelineBackend::kBufferFailed;
  }
  return MediaPipelineBackend::kBufferPending;
}

void AudioDecoderSoftwareWrapper::GetStatistics(Statistics* statistics) {
  DCHECK(statistics);
  return backend_decoder_->GetStatistics(statistics);
}

bool AudioDecoderSoftwareWrapper::SetConfig(const AudioConfig& config) {
  DCHECK(delegate_);
  DCHECK(IsValidConfig(config));

  if (backend_decoder_->SetConfig(config)) {
    LOG(INFO) << "Using backend decoder for " << config.codec;
    software_decoder_.reset();
    output_config_ = config;
    return true;
  }

  if (!CreateSoftwareDecoder(config)) {
    LOG(INFO) << "Failed to create software decoder";
    return false;
  }

  LOG(INFO) << "Using software decoder for " << config.codec;

  output_config_.codec = media::kCodecPCM;
  output_config_.sample_format = media::kSampleFormatS16;
  output_config_.channel_number = 2;
  output_config_.bytes_per_channel = 2;
  output_config_.samples_per_second = config.samples_per_second;
  output_config_.encryption_scheme = Unencrypted();
  return backend_decoder_->SetConfig(output_config_);
}

bool AudioDecoderSoftwareWrapper::SetVolume(float multiplier) {
  return backend_decoder_->SetVolume(multiplier);
}

AudioDecoderSoftwareWrapper::RenderingDelay
AudioDecoderSoftwareWrapper::GetRenderingDelay() {
  return backend_decoder_->GetRenderingDelay();
}

bool AudioDecoderSoftwareWrapper::CreateSoftwareDecoder(
    const AudioConfig& config) {
  // TODO(kmackay) Consider using planar float instead.
  software_decoder_ = media::CastAudioDecoder::Create(
      base::ThreadTaskRunnerHandle::Get(), config,
      media::CastAudioDecoder::kOutputSigned16,
      base::Bind(&AudioDecoderSoftwareWrapper::OnDecoderInitialized,
                 weak_factory_.GetWeakPtr()));
  return (software_decoder_.get() != nullptr);
}

void AudioDecoderSoftwareWrapper::OnDecoderInitialized(bool success) {
  if (!success) {
    LOG(ERROR) << "Failed to initialize software decoder";
    delegate_->OnDecoderError();
  }
}

void AudioDecoderSoftwareWrapper::OnDecodedBuffer(
    CastAudioDecoder::Status status,
    const scoped_refptr<DecoderBufferBase>& decoded) {
  if (status != CastAudioDecoder::kDecodeOk) {
    delegate_->OnPushBufferComplete(MediaPipelineBackend::kBufferFailed);
    return;
  }

  pending_pushed_buffer_ = decoded;
  MediaPipelineBackend::BufferStatus buffer_status =
      backend_decoder_->PushBuffer(pending_pushed_buffer_.get());
  if (buffer_status != MediaPipelineBackend::kBufferPending)
    delegate_->OnPushBufferComplete(buffer_status);
}

void AudioDecoderSoftwareWrapper::OnPushBufferComplete(
    MediaPipelineBackend::BufferStatus status) {
  DCHECK(delegate_);
  delegate_->OnPushBufferComplete(status);
}

void AudioDecoderSoftwareWrapper::OnEndOfStream() {
  DCHECK(delegate_);
  delegate_->OnEndOfStream();
}

void AudioDecoderSoftwareWrapper::OnDecoderError() {
  DCHECK(delegate_);
  delegate_->OnDecoderError();
}

void AudioDecoderSoftwareWrapper::OnKeyStatusChanged(const std::string& key_id,
                                                     CastKeyStatus key_status,
                                                     uint32_t system_code) {
  DCHECK(delegate_);
  delegate_->OnKeyStatusChanged(key_id, key_status, system_code);
}

void AudioDecoderSoftwareWrapper::OnVideoResolutionChanged(const Size& size) {
  NOTREACHED();
}

}  // namespace media
}  // namespace chromecast
