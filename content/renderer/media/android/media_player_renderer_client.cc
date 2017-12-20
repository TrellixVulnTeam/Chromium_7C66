// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/android/media_player_renderer_client.h"

#include "base/callback_helpers.h"

namespace content {

MediaPlayerRendererClient::MediaPlayerRendererClient(
    scoped_refptr<base::SingleThreadTaskRunner> media_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> compositor_task_runner,
    media::MojoRenderer* mojo_renderer,
    media::ScopedStreamTextureWrapper stream_texture_wrapper,
    media::VideoRendererSink* sink)
    : mojo_renderer_(mojo_renderer),
      stream_texture_wrapper_(std::move(stream_texture_wrapper)),
      client_(nullptr),
      sink_(sink),
      media_task_runner_(std::move(media_task_runner)),
      compositor_task_runner_(std::move(compositor_task_runner)),
      weak_factory_(this) {}

MediaPlayerRendererClient::~MediaPlayerRendererClient() {}

void MediaPlayerRendererClient::Initialize(
    media::DemuxerStreamProvider* demuxer_stream_provider,
    media::RendererClient* client,
    const media::PipelineStatusCB& init_cb) {
  DCHECK(media_task_runner_->BelongsToCurrentThread());
  DCHECK(!init_cb_);

  client_ = client;
  init_cb_ = init_cb;

  // Initialize the StreamTexture using a 1x1 texture because we do not have
  // any size information from the MediaPlayer yet.
  // The size will be automatically updated in OnVideoNaturalSizeChange() once
  // we parse the media's metadata.
  // Unretained is safe here because |stream_texture_wrapper_| resets the
  // Closure it has before destroying itself on |compositor_task_runner_|,
  // and |this| is garanteed to live until the Closure has been reset.
  stream_texture_wrapper_->Initialize(
      base::Bind(&MediaPlayerRendererClient::OnFrameAvailable,
                 base::Unretained(this)),
      gfx::Size(1, 1), compositor_task_runner_,
      base::Bind(&MediaPlayerRendererClient::InitializeRemoteRenderer,
                 weak_factory_.GetWeakPtr(), demuxer_stream_provider));
}

void MediaPlayerRendererClient::InitializeRemoteRenderer(
    media::DemuxerStreamProvider* demuxer_stream_provider) {
  DCHECK(media_task_runner_->BelongsToCurrentThread());
  mojo_renderer_->Initialize(
      demuxer_stream_provider, this,
      base::Bind(&MediaPlayerRendererClient::CompleteInitialization,
                 weak_factory_.GetWeakPtr()));
}

void MediaPlayerRendererClient::OnScopedSurfaceRequested(
    const base::UnguessableToken& request_token) {
  DCHECK(request_token);
  stream_texture_wrapper_->ForwardStreamTextureForSurfaceRequest(request_token);
}

void MediaPlayerRendererClient::CompleteInitialization(
    media::PipelineStatus status) {
  DCHECK(media_task_runner_->BelongsToCurrentThread());
  DCHECK(!init_cb_.is_null());

  // TODO(tguilbert): Measure and smooth out the initialization's ordering to
  // have the lowest total initialization time.
  mojo_renderer_->InitiateScopedSurfaceRequest(
      base::Bind(&MediaPlayerRendererClient::OnScopedSurfaceRequested,
                 weak_factory_.GetWeakPtr()));

  base::ResetAndReturn(&init_cb_).Run(status);
}

void MediaPlayerRendererClient::SetCdm(
    media::CdmContext* cdm_context,
    const media::CdmAttachedCB& cdm_attached_cb) {
  NOTREACHED();
}

void MediaPlayerRendererClient::Flush(const base::Closure& flush_cb) {
  mojo_renderer_->Flush(flush_cb);
}

void MediaPlayerRendererClient::StartPlayingFrom(base::TimeDelta time) {
  mojo_renderer_->StartPlayingFrom(time);
}

void MediaPlayerRendererClient::SetPlaybackRate(double playback_rate) {
  mojo_renderer_->SetPlaybackRate(playback_rate);
}

void MediaPlayerRendererClient::SetVolume(float volume) {
  mojo_renderer_->SetVolume(volume);
}

base::TimeDelta MediaPlayerRendererClient::GetMediaTime() {
  return mojo_renderer_->GetMediaTime();
}

bool MediaPlayerRendererClient::HasAudio() {
  // We do not know whether or not the media has Audio before starting playback.
  // Conservatively assume we do.
  // TODO(tguilbert): Consider using MIME types to determine presence of audio.
  // Alternatively, consider piping the HasAudio() from the MediaPlayerRenderer
  // through the mojo::Renderer interface.
  return true;
}

bool MediaPlayerRendererClient::HasVideo() {
  // We do not know whether or not the media has Video before starting playback.
  // Conservatively assume we do.
  // TODO(tguilbert): Consider using MIME types to determine presence of video.
  // Alternatively, consider piping the HasVideo() from the MediaPlayerRenderer
  // through the mojo::Renderer interface.
  return true;
}

void MediaPlayerRendererClient::OnFrameAvailable() {
  DCHECK(compositor_task_runner_->BelongsToCurrentThread());
  sink_->PaintSingleFrame(stream_texture_wrapper_->GetCurrentFrame(), true);
}

void MediaPlayerRendererClient::OnError(media::PipelineStatus status) {
  client_->OnError(status);
}

void MediaPlayerRendererClient::OnEnded() {
  client_->OnEnded();
}

void MediaPlayerRendererClient::OnStatisticsUpdate(
    const media::PipelineStatistics& stats) {
  client_->OnStatisticsUpdate(stats);
}

void MediaPlayerRendererClient::OnBufferingStateChange(
    media::BufferingState state) {
  client_->OnBufferingStateChange(state);
}

void MediaPlayerRendererClient::OnWaitingForDecryptionKey() {
  client_->OnWaitingForDecryptionKey();
}

void MediaPlayerRendererClient::OnVideoNaturalSizeChange(
    const gfx::Size& size) {
  stream_texture_wrapper_->UpdateTextureSize(size);
  client_->OnVideoNaturalSizeChange(size);
}

void MediaPlayerRendererClient::OnVideoOpacityChange(bool opaque) {
  client_->OnVideoOpacityChange(opaque);
}

void MediaPlayerRendererClient::OnDurationChange(base::TimeDelta duration) {
  client_->OnDurationChange(duration);
}

}  // namespace content
