// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

/**
 * Simple interface allowing customized response to an overscrolling pull input.
 */
public interface OverscrollRefreshHandler {
    /**
     * Signals the start of an overscrolling pull.
     * @return Whether the handler will consume the overscroll sequence.
     */
    public boolean start();

    /**
     * Signals a pull update.
     * @param delta The change in pull distance (positive if pulling down, negative if up).
     */
    public void pull(float delta);

    /**
     * Signals the release of the pull.
     * @param allowRefresh Whether the release signal should be allowed to trigger a refresh.
     */
    public void release(boolean allowRefresh);

    /**
     * Reset the active pull state.
     */
    public void reset();

    /**
     * Toggle whether the effect is active.
     * @param enabled Whether to enable the effect.
     *                If disabled, the effect should deactive itself apropriately.
     */
    public void setEnabled(boolean enabled);
}
