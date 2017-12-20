// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.download;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.chromium.base.BaseChromiumApplication;
import org.chromium.base.test.util.Feature;
import org.chromium.testing.local.LocalRobolectricTestRunner;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.multidex.ShadowMultiDex;

import java.util.UUID;

/** Unit tests for {@link DownloadSharedPreferenceEntry}. */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE, application = BaseChromiumApplication.class,
        shadows = {ShadowMultiDex.class})
public class DownloadSharedPreferenceEntryTest {
    private String newUUID() {
        return UUID.randomUUID().toString();
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringVersion1() {
        String uuid = newUUID();
        String notificationString = "1,2,1,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(2, entry.notificationId);
        assertFalse(entry.isOffTheRecord);
        assertTrue(entry.canDownloadWhileMetered);
        assertEquals(uuid, entry.downloadGuid);
        assertEquals("test,2.pdf", entry.fileName);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);

        String uuid2 = newUUID();
        notificationString = "1,3,0,0," + uuid2 + ",test,4.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(3, entry.notificationId);
        assertTrue(entry.isOffTheRecord);
        assertFalse(entry.canDownloadWhileMetered);
        assertEquals(uuid2, entry.downloadGuid);
        assertEquals("test,4.pdf", entry.fileName);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringVersion2() {
        String uuid = newUUID();
        String notificationString = "2,2,0,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(2, entry.notificationId);
        assertFalse(entry.isOffTheRecord);
        assertTrue(entry.canDownloadWhileMetered);
        assertEquals(uuid, entry.downloadGuid);
        assertEquals("test,2.pdf", entry.fileName);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);

        String uuid2 = newUUID();
        notificationString = "2,3,1,0," + uuid2 + ",test,4.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(3, entry.notificationId);
        assertTrue(entry.isOffTheRecord);
        assertFalse(entry.canDownloadWhileMetered);
        assertEquals(uuid2, entry.downloadGuid);
        assertEquals("test,4.pdf", entry.fileName);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringVersion3_Download() {
        String uuid = newUUID();
        String notificationString = "3,2,1,0,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(2, entry.notificationId);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);
        assertFalse(entry.isOffTheRecord);
        assertTrue(entry.canDownloadWhileMetered);
        assertEquals(uuid, entry.downloadGuid);
        assertEquals("test,2.pdf", entry.fileName);

        String uuid2 = newUUID();
        notificationString = "3,3,1,1,0," + uuid2 + ",test,4.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(3, entry.notificationId);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_DOWNLOAD, entry.itemType);
        assertTrue(entry.isOffTheRecord);
        assertFalse(entry.canDownloadWhileMetered);
        assertEquals(uuid2, entry.downloadGuid);
        assertEquals("test,4.pdf", entry.fileName);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringVersion3_OfflinePage() {
        String uuid = newUUID();
        String notificationString = "3,2,2,0,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(2, entry.notificationId);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_OFFLINE_PAGE, entry.itemType);
        assertFalse(entry.isOffTheRecord);
        assertTrue(entry.canDownloadWhileMetered);
        assertEquals(uuid, entry.downloadGuid);
        assertEquals("test,2.pdf", entry.fileName);

        String uuid2 = newUUID();
        notificationString = "3,3,2,1,0," + uuid2 + ",test,4.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(3, entry.notificationId);
        assertEquals(DownloadSharedPreferenceEntry.ITEM_TYPE_OFFLINE_PAGE, entry.itemType);
        assertTrue(entry.isOffTheRecord);
        assertFalse(entry.canDownloadWhileMetered);
        assertEquals(uuid2, entry.downloadGuid);
        assertEquals("test,4.pdf", entry.fileName);
    }

    @Test
    @Feature({"Download"})
    public void testGetSharedPreferencesString() {
        String uuid = newUUID();
        String notificationString = "3,2,2,0,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(notificationString, entry.getSharedPreferenceString());

        String uuid2 = newUUID();
        notificationString = "3,3,2,1,0," + uuid2 + ",test,4.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(notificationString, entry.getSharedPreferenceString());
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringInvalidVersion() {
        String uuid = newUUID();
        // Version 0 is invalid.
        String notificationString = "0,2,0,1," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Version 4 is invalid.
        notificationString = "4,2,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Version number is missing.
        notificationString = ",2,2,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not possible to parse version number.
        notificationString = "xxx,2,2,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringNotEnoughValues() {
        String uuid = newUUID();
        // Version 1 requires at least 6.
        String notificationString = "1,2,0," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Version 2 requires at least 6.
        notificationString = "2,2,0," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Version 3 requires at least 7.
        notificationString = "3,2,2,0," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringFailedToParseNotificationId() {
        String uuid = newUUID();
        // Notification ID missing in version 1.
        String notificationString = "1,,1,0," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Notification ID missing in version 2.
        notificationString = "2,,1,0," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Notification ID missing in version 3.
        notificationString = "3,,2,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse notification ID in version 1.
        notificationString = "1,xxx,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse notification ID in version 2.
        notificationString = "2,xxx,1,0," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse notification ID in version 3.
        notificationString = "3,xxx,2,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringFailedToParseItemType() {
        String uuid = newUUID();
        // Item type is only present in version 3. (Position 3)
        // Invalid item type.
        String notificationString = "3,2,0,1,0," + uuid + ",test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Invalid item type.
        notificationString = "3,2,3,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse item type.
        notificationString = "3,2,xxx,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Missing item type.
        notificationString = "3,2,,0,1," + uuid + ",test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);
    }

    @Test
    @Feature({"Download"})
    public void testParseFromStringInvalidGuid() {
        // GUID missing in version 1.
        String notificationString = "1,2,1,0,,test,2.pdf";
        DownloadSharedPreferenceEntry entry =
                DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // GUID missing in version 2.
        notificationString = "2,2,1,0,,test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // GUID missing in version 3.
        notificationString = "3,2,2,0,1,,test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse GUID in version 1.
        notificationString = "1,2,0,1,xxx,test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse GUID in version 2.
        notificationString = "2,2,1,0,xxx,test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);

        // Not able to parse GUID in version 3.
        notificationString = "3,2,2,0,1,xxx,test,2.pdf";
        entry = DownloadSharedPreferenceEntry.parseFromString(notificationString);
        assertEquals(DownloadSharedPreferenceEntry.INVALID_ENTRY, entry);
    }
}
