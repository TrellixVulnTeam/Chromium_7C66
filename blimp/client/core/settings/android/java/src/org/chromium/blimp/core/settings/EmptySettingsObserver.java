// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.blimp.core.settings;

/**
 * EmptySettingsObserver is a helper class that has an empty implementation of all the
 * methods in the {@link SettingsObserver}. It is intented to be used by every class that
 * does not need to implement all the observer-methods.
 */
public class EmptySettingsObserver implements SettingsObserver {
    @Override
    public void onBlimpModeEnabled(boolean enable) {}

    @Override
    public void onShowNetworkStatsChanged(boolean enable) {}

    @Override
    public void onRecordWholeDocumentChanged(boolean enable) {}

    @Override
    public void onRestartRequired() {}
}
