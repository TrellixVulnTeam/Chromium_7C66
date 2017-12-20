// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content_public.browser;

import android.support.annotation.Nullable;

import org.chromium.content.browser.MediaSessionImpl;
import org.chromium.content_public.common.MediaMetadata;

/**
 * This class is Java implementation of native MediaSessionObserver. The class receives media
 * session messages from Java {@link MediaSession}, which acts acts as a proxy forwarding messages
 * comming from the native MediaSession.
 */
public abstract class MediaSessionObserver {
    private MediaSessionImpl mMediaSession;

    /**
     * Construct a MediaSessionObserver and start observing |mediaSession|.
     */
    protected MediaSessionObserver(MediaSession mediaSession) {
        mMediaSession = (MediaSessionImpl) mediaSession;
        mMediaSession.addObserver(this);
    }

    /**
     * @return The observed media session.
     */
    @Nullable
    public final MediaSession getMediaSession() {
        return mMediaSession;
    }

    /**
     * Called when the observed media session is destroyed.
     *
     * Gives subclass a chance to do clean up. After the whole loop over
     * {@link MediaSessionObserver}s has been finished, {@link MediaSessionObserver#getMediaSession}
     * will return null.
     */
    public void mediaSessionDestroyed() {}

    /**
     * Called when the observed {@link MediaSession} state has changed.
     * @param isControllable If the session can be resumed or suspended.
     * @param isSuspended if the session currently suspended or not.
     */
    public void mediaSessionStateChanged(boolean isControllable, boolean isSuspended) {}

    /**
     * Called when the {@link MediaSession} metadata has changed.
     * @param metadata the new MediaMetadata after change.
     */
    public void mediaSessionMetadataChanged(MediaMetadata metadata) {}

    /**
     * Called when the observed {@link MediaSession} has enabled an action.
     * @param action The enabled action.
     */
    public void mediaSessionEnabledAction(int action) {}

    /**
     * Called when the observed {@link MediaSession} has disabled an action.
     * @param action The disabled action.
     */
    public void mediaSessionDisabledAction(int action) {}

    /**
     * Stop observing the media session. Users must explicitly call this before dereferencing the
     * observer.
     */
    public final void stopObserving() {
        if (mMediaSession == null) return;
        mMediaSession.removeObserver(this);
        mMediaSession = null;
    }
}
