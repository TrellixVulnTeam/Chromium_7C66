// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_AVDA_SHARED_STATE_H_
#define MEDIA_GPU_AVDA_SHARED_STATE_H_

#include "base/synchronization/waitable_event.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "media/base/android/media_codec_bridge.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_image.h"
#include "ui/gl/gl_surface.h"

namespace gl {
class SurfaceTexture;
}

namespace media {

class AVDACodecImage;

// Shared state to allow communication between the AVDA and the
// GLImages that configure GL for drawing the frames.
class AVDASharedState : public base::RefCounted<AVDASharedState> {
 public:
  AVDASharedState();

  GLuint surface_texture_service_id() const {
    return surface_texture_service_id_;
  }

  // Signal the "frame available" event.  This may be called from any thread.
  void SignalFrameAvailable();

  void WaitForFrameAvailable();

  void SetSurfaceTexture(scoped_refptr<gl::SurfaceTexture> surface_texture,
                         GLuint attached_service_id);

  // Context and surface that |surface_texture_| is bound to, if
  // |surface_texture_| is not null.
  gl::GLContext* context() const { return context_.get(); }

  gl::GLSurface* surface() const { return surface_.get(); }

  // Helper method for coordinating the interactions between
  // MediaCodec::ReleaseOutputBuffer() and WaitForFrameAvailable() when
  // rendering to a SurfaceTexture; this method should never be called when
  // rendering to a SurfaceView.
  //
  // The release of the codec buffer to the surface texture is asynchronous, by
  // using this helper we can attempt to let this process complete in a non
  // blocking fashion before the SurfaceTexture is used.
  //
  // Clients should call this method to release the codec buffer for rendering
  // and then call WaitForFrameAvailable() before using the SurfaceTexture. In
  // the ideal case the SurfaceTexture has already been updated, otherwise the
  // method will wait for a pro-rated amount of time based on elapsed time up
  // to a short deadline.
  //
  // Some devices do not reliably notify frame availability, so we use a very
  // short deadline of only a few milliseconds to avoid indefinite stalls.
  void RenderCodecBufferToSurfaceTexture(MediaCodecBridge* codec,
                                         int codec_buffer_index);

  // Helper methods for interacting with |surface_texture_|. See
  // gfx::SurfaceTexture for method details.
  void UpdateTexImage();
  // Returns a matrix that needs to be y flipped in order to match the
  // StreamTextureMatrix contract. See GLStreamTextureImage::YInvertMatrix().
  void GetTransformMatrix(float matrix[16]) const;

  // Resets the last time for RenderCodecBufferToSurfaceTexture(). Should be
  // called during codec changes.
  void clear_release_time() { release_time_ = base::TimeTicks(); }

 protected:
  virtual ~AVDASharedState();

 private:
  friend class base::RefCounted<AVDASharedState>;

  scoped_refptr<gl::SurfaceTexture> surface_texture_;

  // Platform gl texture id for |surface_texture_|.
  GLuint surface_texture_service_id_;

  // For signalling OnFrameAvailable().
  base::WaitableEvent frame_available_event_;

  // Context and surface that |surface_texture_| is bound to, if
  // |surface_texture_| is not null.
  scoped_refptr<gl::GLContext> context_;
  scoped_refptr<gl::GLSurface> surface_;

  // The time of the last call to RenderCodecBufferToSurfaceTexture(), null if
  // if there has been no last call or WaitForFrameAvailable() has been called
  // since the last call.
  base::TimeTicks release_time_;

  // Texture matrix of the front buffer of the surface texture.
  float gl_matrix_[16];

  class OnFrameAvailableHandler;
  scoped_refptr<OnFrameAvailableHandler> on_frame_available_handler_;

  DISALLOW_COPY_AND_ASSIGN(AVDASharedState);
};

}  // namespace media

#endif  // MEDIA_GPU_AVDA_SHARED_STATE_H_
