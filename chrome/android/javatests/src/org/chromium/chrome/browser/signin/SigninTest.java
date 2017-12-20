// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.signin;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.DialogInterface;
import android.preference.Preference;
import android.support.v7.app.AlertDialog;
import android.test.suitebuilder.annotation.MediumTest;
import android.widget.Button;

import org.chromium.base.ThreadUtils;
import org.chromium.base.test.util.Restriction;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.bookmarks.BookmarkBridge;
import org.chromium.chrome.browser.preferences.MainPreferences;
import org.chromium.chrome.browser.preferences.PrefServiceBridge;
import org.chromium.chrome.browser.preferences.Preferences;
import org.chromium.chrome.browser.preferences.SignInPreference;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.sync.ProfileSyncService;
import org.chromium.chrome.test.ChromeTabbedActivityTestBase;
import org.chromium.chrome.test.util.ActivityUtils;
import org.chromium.chrome.test.util.ChromeRestriction;
import org.chromium.chrome.test.util.browser.signin.SigninTestUtil;
import org.chromium.components.bookmarks.BookmarkId;
import org.chromium.components.signin.ChromeSigninController;
import org.chromium.components.signin.test.util.MockAccountManager;
import org.chromium.content.browser.test.util.TestTouchUtils;

/**
 * Test suite for sign in tests.
 *
 * These tests cover the sign in flow for both consumer and managed accounts. They also verify
 * the state of the browser while signed in, and any changes when signing out.
 *
 * The accounts used to sign in are mocked by a MockAccountManager.
 */
public class SigninTest extends ChromeTabbedActivityTestBase {

    /**
     * Helper class that observes when signing in becomes allowed.
     */
    private static class TestSignInAllowedObserver implements SigninManager.SignInAllowedObserver {
        private final Object mLock = new Object();
        private boolean mIsSignInAllowed;
        private SigninManager mSigninManager;

        public void startObserving(SigninManager signinManager) {
            ThreadUtils.assertOnUiThread();
            mIsSignInAllowed = signinManager.isSignInAllowed();
            if (!mIsSignInAllowed) {
                mSigninManager = signinManager;
                mSigninManager.addSignInAllowedObserver(this);
            }
        }

        @Override
        public void onSignInAllowedChanged() {
            ThreadUtils.assertOnUiThread();
            synchronized (mLock) {
                if (mSigninManager.isSignInAllowed()) {
                    mIsSignInAllowed = true;
                    mSigninManager.removeSignInAllowedObserver(this);
                    mLock.notifyAll();
                }
            }
        }

        public void waitForSignInAllowed() {
            assert !ThreadUtils.runningOnUiThread();
            synchronized (mLock) {
                while (!mIsSignInAllowed) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException exception) {
                        // Ignore.
                    }
                }
            }
        }
    }

    /**
     * Helper class that observes signin state changes.
     */
    private static class TestSignInObserver implements SigninManager.SignInStateObserver {
        private final Object mLock = new Object();
        public int mSignInCount;
        public int mSignOutCount;

        public void waitForSignInEvents(int total) {
            synchronized (mLock) {
                while (mSignInCount + mSignOutCount < total) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException exception) {
                        // Ignore.
                    }
                }
            }
        }

        @Override
        public void onSignedIn() {
            synchronized (mLock) {
                mSignInCount++;
                mLock.notifyAll();
            }
        }

        @Override
        public void onSignedOut() {
            synchronized (mLock) {
                mSignOutCount++;
                mLock.notifyAll();
            }
        }
    }

    /**
     * Helper class that waits for the BookmarkModel to be ready.
     */
    private static class TestBookmarkModelObserver extends BookmarkBridge.BookmarkModelObserver {
        private final Object mLock = new Object();
        private boolean mIsLoaded;
        private boolean mAdded;
        private boolean mAllUserNodesRemoved;

        public TestBookmarkModelObserver(BookmarkBridge bookmarks) {
            mIsLoaded = bookmarks.isBookmarkModelLoaded();
            mAdded = false;
            mAllUserNodesRemoved = false;
        }

        public void waitForBookmarkModelToLoad() {
            synchronized (mLock) {
                while (!mIsLoaded) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException exception) {
                        // Ignore.
                    }
                }
            }
        }

        public void waitForBookmarkAdded() {
            synchronized (mLock) {
                while (!mAdded) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException exception) {
                        // Ignore.
                    }
                }
            }
        }

        public void waitForAllUserBookmarksRemoved() {
            synchronized (mLock) {
                while (!mAllUserNodesRemoved) {
                    try {
                        mLock.wait();
                    } catch (InterruptedException exception) {
                        // Ignore.
                    }
                }
            }
        }

        @Override
        public void bookmarkModelLoaded() {
            synchronized (mLock) {
                mIsLoaded = true;
                mLock.notify();
            }
        }

        @Override
        public void bookmarkNodeAdded(BookmarkBridge.BookmarkItem parent, int index) {
            synchronized (mLock) {
                mAdded = true;
                mLock.notify();
            }
        }

        @Override
        public void bookmarkAllUserNodesRemoved() {
            synchronized (mLock) {
                mAllUserNodesRemoved = true;
                mLock.notify();
            }
        }

        @Override
        public void bookmarkModelChanged() {
            // Ignore.
        }
    }

    private Context mContext;
    private MockAccountManager mAccountManager;
    private SigninManager mSigninManager;
    private PrefServiceBridge mPrefService;
    private BookmarkBridge mBookmarks;
    private TestBookmarkModelObserver mTestBookmarkModelObserver;
    private TestSignInObserver mTestSignInObserver;

    @Override
    protected void setUp() throws Exception {
        // Mock out the account manager on the device.
        SigninTestUtil.setUpAuthForTest(getInstrumentation());

        super.setUp();

        mContext = getInstrumentation().getTargetContext();
        final TestSignInAllowedObserver signinAllowedObserver = new TestSignInAllowedObserver();

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                // This call initializes the ChromeSigninController to use our test context.
                ChromeSigninController.get(mContext);

                // Start observing the SigninManager.
                mTestSignInObserver = new TestSignInObserver();
                mSigninManager = SigninManager.get(mContext);
                mSigninManager.addSignInStateObserver(mTestSignInObserver);

                // Get these handles in the UI thread.
                mPrefService = PrefServiceBridge.getInstance();
                Profile profile = getActivity().getActivityTab().getProfile();
                mBookmarks = new BookmarkBridge(profile);

                // Add a test bookmark, to verify later if sign out cleared the bookmarks.
                mTestBookmarkModelObserver = new TestBookmarkModelObserver(mBookmarks);
                mBookmarks.addObserver(mTestBookmarkModelObserver);
                mTestBookmarkModelObserver.waitForBookmarkModelToLoad();
                assertEquals(0, mBookmarks.getChildCount(mBookmarks.getMobileFolderId()));
                BookmarkId mTestBookmark = mBookmarks.addBookmark(
                        mBookmarks.getMobileFolderId(), 0, "Test Bookmark", "http://google.com");
                mTestBookmarkModelObserver.waitForBookmarkAdded();
                assertNotNull(mTestBookmark);
                assertEquals(1, mBookmarks.getChildCount(mBookmarks.getMobileFolderId()));

                // Start observing if signing in is allowed. This observer must be installed on
                // the UI thread, but waiting must be done outside the UI thread (otherwise it
                // won't ever unblock).
                signinAllowedObserver.startObserving(mSigninManager);
            }
        });

        signinAllowedObserver.waitForSignInAllowed();
        assertTrue(mSigninManager.isSignInAllowed());
    }

    @Override
    protected void tearDown() throws Exception {
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                mBookmarks.removeObserver(mTestBookmarkModelObserver);

                mSigninManager.removeSignInStateObserver(mTestSignInObserver);

                if (ChromeSigninController.get(mContext).isSignedIn()) {
                    mSigninManager.signOut(null, null);
                }

                mBookmarks.destroy();
            }
        });
        super.tearDown();
    }

    @Override
    public void startMainActivity() throws InterruptedException {
        startMainActivityOnBlankPage();
    }

    @MediumTest
    @Restriction(ChromeRestriction.RESTRICTION_TYPE_GOOGLE_PLAY_SERVICES)
    public void testConsumerSignin() throws InterruptedException {
        SigninTestUtil.addTestAccount();
        signInToSingleAccount();

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                // Verify that the account isn't managed.
                assertNull(mSigninManager.getManagementDomain());

                // Verify that the password manager is enabled by default.
                assertTrue(mPrefService.isRememberPasswordsEnabled());
                assertFalse(mPrefService.isRememberPasswordsManaged());
            }
        });

        // Verify that its preference UI is enabled.
        Preferences prefActivity = startPreferences(null);
        MainPreferences mainPrefs = getMainPreferences(prefActivity);
        Preference passwordPref = mainPrefs.findPreference(MainPreferences.PREF_SAVED_PASSWORDS);
        assertNotNull(passwordPref);
        // This preference opens a new fragment when clicked.
        assertNotNull(passwordPref.getFragment());
        // There is no icon for this preference by default.
        assertNull(passwordPref.getIcon());
        prefActivity.finish();

        // Sign out now.
        signOut();

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                // Verify that the profile data hasn't been wiped when signing out of a normal
                // account. We check that by looking for the test bookmark from setUp().
                assertEquals(1, mBookmarks.getChildCount(mBookmarks.getMobileFolderId()));
            }
        });
    }

    private void signInToSingleAccount() {
        // Verify that we aren't signed in yet.
        assertFalse(ChromeSigninController.get(mContext).isSignedIn());

        // Open the preferences UI.
        final Preferences prefActivity = startPreferences(null);
        getInstrumentation().waitForIdleSync();

        // Create a monitor to catch the AccountSigninActivity when it is created.
        ActivityMonitor monitor = getInstrumentation().addMonitor(
                AccountSigninActivity.class.getName(), null, false);

        // Click sign in.
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                clickSigninPreference(prefActivity);
            }
        });

        // Pick the mock account.
        AccountSigninActivity signinActivity = (AccountSigninActivity)
                getInstrumentation().waitForMonitor(monitor);
        Button positiveButton = (Button) signinActivity.findViewById(R.id.positive_button);
        // Press 'sign in'.
        TestTouchUtils.performClickOnMainSync(getInstrumentation(), positiveButton);
        getInstrumentation().waitForIdleSync();
        // Press 'ok, got it' (the same button is reused).
        TestTouchUtils.performClickOnMainSync(getInstrumentation(), positiveButton);

        // Sync doesn't actually start up until we finish the sync setup. This usually happens
        // in the resume of the Main activity, but we forcefully do this here.
        getInstrumentation().waitForIdleSync();
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                ProfileSyncService syncService = ProfileSyncService.get();
                syncService.setFirstSetupComplete();
                syncService.setSetupInProgress(false);
            }
        });
        prefActivity.finish();

        // Verify that signin succeeded.
        mTestSignInObserver.waitForSignInEvents(1);
        assertEquals(1, mTestSignInObserver.mSignInCount);
        assertEquals(0, mTestSignInObserver.mSignOutCount);
        assertTrue(ChromeSigninController.get(mContext).isSignedIn());
    }

    private void signOut() throws InterruptedException {
        // Verify that we are currently signed in.
        assertTrue(ChromeSigninController.get(mContext).isSignedIn());

        // Open the account preferences.
        final Preferences prefActivity =
                startPreferences(AccountManagementFragment.class.getName());
        getInstrumentation().waitForIdleSync();

        // Click on the signout button.
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                clickSignOut(prefActivity);
            }
        });

        // Accept the warning dialog.
        acceptAlertDialogWithTag(prefActivity, AccountManagementFragment.SIGN_OUT_DIALOG_TAG);

        // Verify that signout succeeded.
        mTestSignInObserver.waitForSignInEvents(2);
        assertEquals(1, mTestSignInObserver.mSignInCount);
        assertEquals(1, mTestSignInObserver.mSignOutCount);
        assertFalse(ChromeSigninController.get(mContext).isSignedIn());

        if (!prefActivity.isFinishing()) prefActivity.finish();
        getInstrumentation().waitForIdleSync();
    }

    private static MainPreferences getMainPreferences(Preferences prefActivity) {
        Fragment fragment = prefActivity.getFragmentForTest();
        assertNotNull(fragment);
        assertTrue(fragment instanceof MainPreferences);
        return (MainPreferences) fragment;
    }

    private static void clickSigninPreference(Preferences prefActivity) {
        MainPreferences mainPrefs = getMainPreferences(prefActivity);
        Preference signinPref = mainPrefs.findPreference(MainPreferences.PREF_SIGN_IN);
        assertNotNull(signinPref);
        assertTrue(signinPref instanceof SignInPreference);
        assertNotNull(signinPref.getOnPreferenceClickListener());
        signinPref.getOnPreferenceClickListener().onPreferenceClick(signinPref);
    }

    private static void clickSignOut(Preferences prefActivity) {
        Fragment fragment = prefActivity.getFragmentForTest();
        assertNotNull(fragment);
        assertTrue(fragment instanceof AccountManagementFragment);
        AccountManagementFragment managementFragment = (AccountManagementFragment) fragment;
        Preference signOutPref = managementFragment.findPreference(
                AccountManagementFragment.PREF_SIGN_OUT);
        assertNotNull(signOutPref);
        assertNotNull(signOutPref.getOnPreferenceClickListener());
        signOutPref.getOnPreferenceClickListener().onPreferenceClick(signOutPref);
    }

    private void acceptAlertDialogWithTag(Activity activity, String tag)
            throws InterruptedException {
        getInstrumentation().waitForIdleSync();
        DialogFragment fragment = ActivityUtils.waitForFragment(activity, tag);
        AlertDialog dialog = (AlertDialog) fragment.getDialog();
        assertTrue(dialog != null);
        assertTrue(dialog.isShowing());
        Button button = dialog.getButton(DialogInterface.BUTTON_POSITIVE);
        assertNotNull("Could not find the accept button.", button);
        TestTouchUtils.performClickOnMainSync(getInstrumentation(), button);
    }
}
