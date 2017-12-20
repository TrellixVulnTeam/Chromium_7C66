// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.input;

import org.chromium.base.VisibleForTesting;

/**
 * A simple class to set start and end in int type.
 * TODO(changwan): replace this with android.util.Range when the default SDK
 * version becomes 21 or higher.
 */
public class Range {
    private int mStart;
    private int mEnd;

    public Range(int start, int end) {
        set(start, end);
    }

    public int start() {
        return mStart;
    }

    public int end() {
        return mEnd;
    }

    @VisibleForTesting
    public void set(int start, int end) {
        mStart = Math.min(start, end);
        mEnd = Math.max(start, end);
    }

    public void clamp(int start, int end) {
        mStart = Math.min(Math.max(mStart, start), end);
        mEnd = Math.max(Math.min(mEnd, end), start);
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Range)) return false;
        if (o == this) return true;
        Range r = (Range) o;
        return mStart == r.mStart && mEnd == r.mEnd;
    }

    @Override
    public int hashCode() {
        return 11 * mStart + 31 * mEnd;
    }

    @Override
    public String toString() {
        return "[ " + mStart + ", " + mEnd + " ]";
    }
}
