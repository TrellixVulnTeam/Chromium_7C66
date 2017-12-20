// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
#define CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "build/build_config.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/page_transition_types.h"

namespace base {

class CommandLine;

#if defined(OS_MACOSX)
namespace mac {
class ScopedNSAutoreleasePool;
}  // namespace mac
#endif  // defined(OS_MACOSX)

#if defined(OS_WIN)
namespace win {
class ScopedCOMInitializer;
}
#endif  // defined(OS_WIN)
}  // namespace base

class Browser;
class Profile;
#if defined(OS_MACOSX)
class ScopedBundleSwizzlerMac;
#endif  // defined(OS_MACOSX)

namespace content {
class ContentRendererClient;
}

// Base class for tests wanting to bring up a browser in the unit test process.
// Writing tests with InProcessBrowserTest is slightly different than that of
// other tests. This is necessitated by InProcessBrowserTest running a message
// loop. To use InProcessBrowserTest do the following:
// . Use the macro IN_PROC_BROWSER_TEST_F to define your test.
// . Your test method is invoked on the ui thread. If you need to block until
//   state changes you'll need to run the message loop from your test method.
//   For example, if you need to wait till a find bar has completely been shown
//   you'll need to invoke content::RunMessageLoop. When the message bar is
//   shown, invoke MessageLoop::current()->QuitWhenIdle() to return control back
//   to your test method.
// . If you subclass and override SetUp, be sure and invoke
//   InProcessBrowserTest::SetUp. (But see also SetUpOnMainThread,
//   SetUpInProcessBrowserTestFixture and other related hook methods for a
//   cleaner alternative).
//
// The following four hook methods are called in sequence before calling
// BrowserMain(), thus no browser has been created yet. They are mainly for
// setting up the environment for running the browser.
// . SetUpUserDataDirectory()
// . SetUpCommandLine()
// . SetUpDefaultCommandLine()
// . SetUpInProcessBrowserTestFixture()
//
// Default command line switches are added in the default implementation of
// SetUpDefaultCommandLine(). Addtional command line switches can be simply
// appended in SetUpCommandLine() without the need to invoke
// InProcessBrowserTest::SetUpCommandLine(). If a test needs to change the
// default command line, it can override SetUpDefaultCommandLine(), where it
// should invoke InProcessBrowserTest::SetUpDefaultCommandLine() to get the
// default swtiches, and modify them as needed.
//
// SetUpOnMainThread() is called just after creating the default browser object
// and before executing the real test code. It's mainly for setting up things
// related to the browser object and associated window, like opening a new Tab
// with a testing page loaded.
//
// TearDownOnMainThread() is called just after executing the real test code to
// do necessary cleanup before the browser is torn down.
//
// TearDownInProcessBrowserTestFixture() is called after BrowserMain() exits to
// cleanup things setup for running the browser.
//
// By default InProcessBrowserTest creates a single Browser (as returned from
// the CreateBrowser method). You can obviously create more as needed.

// InProcessBrowserTest disables the sandbox when running.
//
// See ui_test_utils for a handful of methods designed for use with this class.
//
// It's possible to write browser tests that span a restart by splitting each
// run of the browser process into a separate test. Example:
//
// IN_PROC_BROWSER_TEST_F(Foo, PRE_Bar) {
//   do something
// }
//
// IN_PROC_BROWSER_TEST_F(Foo, Bar) {
//   verify something persisted from before
// }
//
//  This is recursive, so PRE_PRE_Bar would run before PRE_BAR.
class InProcessBrowserTest : public content::BrowserTestBase {
 public:
  InProcessBrowserTest();
  ~InProcessBrowserTest() override;

  // Configures everything for an in process browser test, then invokes
  // BrowserMain. BrowserMain ends up invoking RunTestOnMainThreadLoop.
  void SetUp() override;

  // Restores state configured in SetUp.
  void TearDown() override;

  using SetUpBrowserFunction = bool(const Browser*);

  // Sets a function that is called from InProcessBrowserTest::SetUp() with the
  // first browser. This is intended to set up state applicable to all tests
  // in the suite. For example, interactive_ui_tests installs a function that
  // ensures the first browser is in the foreground, active and has focus.
  static void set_global_browser_set_up_function(
      SetUpBrowserFunction* set_up_function) {
    global_browser_set_up_function_ = set_up_function;
  }

 protected:
  // Returns the browser created by CreateBrowser.
  Browser* browser() const { return browser_; }

  // Closes the given browser and waits for it to release all its resources.
  void CloseBrowserSynchronously(Browser* browser);

  // Closes the browser without waiting for it to release all its resources.
  // WARNING: This may leave tasks posted, but not yet run, in the message
  // loops. Prefer CloseBrowserSynchronously over this method.
  void CloseBrowserAsynchronously(Browser* browser);

  // Closes all browsers. No guarantees are made about the destruction of
  // outstanding resources.
  void CloseAllBrowsers();

  // Convenience methods for adding tabs to a Browser.
  void AddTabAtIndexToBrowser(Browser* browser,
                              int index,
                              const GURL& url,
                              ui::PageTransition transition,
                              bool check_navigation_success);
  void AddTabAtIndex(int index,
                     const GURL& url,
                     ui::PageTransition transition);

  // Setups default command line that will be used to launch the child browser
  // process with an in-process test. Called by SetUp() after SetupCommandLine()
  // to add default commandline switches. A default implementation is provided
  // in this class. If a test does not want to use the default implementation,
  // it should override this method.
  virtual void SetUpDefaultCommandLine(base::CommandLine* command_line);

  // Initializes the contents of the user data directory. Called by SetUp()
  // after creating the user data directory, but before any browser is launched.
  // If a test wishes to set up some initial non-empty state in the user data
  // directory before the browser starts up, it can do so here. Returns true if
  // successful.
  virtual bool SetUpUserDataDirectory() WARN_UNUSED_RESULT;

  // BrowserTestBase:
  void RunTestOnMainThreadLoop() override;

  // Ensures that no devtools are open, and then opens the devtools.
  void OpenDevToolsWindow(content::WebContents* web_contents);

  // Opens |url| in an incognito browser window with the incognito profile of
  // |profile|, blocking until the navigation finishes. This will create a new
  // browser if a browser with the incognito profile does not exist. Returns the
  // incognito window Browser.
  Browser* OpenURLOffTheRecord(Profile* profile, const GURL& url);

  // Creates a browser with a single tab (about:blank), waits for the tab to
  // finish loading and shows the browser.
  //
  // This is invoked from Setup.
  Browser* CreateBrowser(Profile* profile);

  // Similar to |CreateBrowser|, but creates an incognito browser.
  Browser* CreateIncognitoBrowser();

  // Creates a browser for a popup window with a single tab (about:blank), waits
  // for the tab to finish loading, and shows the browser.
  Browser* CreateBrowserForPopup(Profile* profile);

  // Creates a browser for an application and waits for it to load and shows
  // the browser.
  Browser* CreateBrowserForApp(const std::string& app_name, Profile* profile);

  // Called from the various CreateBrowser methods to add a blank tab, wait for
  // the navigation to complete, and show the browser's window.
  void AddBlankTabAndShow(Browser* browser);

  // Enables running of accessibility audit for a particular test case.
  //  - Call in test body to enable/disable for one test case.
  //  - Call in SetUpOnMainThread to enable for all test cases.
  void EnableAccessibilityChecksForTestCase(bool enabled) {
    run_accessibility_checks_for_test_case_ = enabled;
  }

#if !defined OS_MACOSX
  // Return a CommandLine object that is used to relaunch the browser_test
  // binary as a browser process. This function is deliberately not defined on
  // the Mac because re-using an existing browser process when launching from
  // the command line isn't a concept that we support on the Mac; AppleEvents
  // are the Mac solution for the same need. Any test based on these functions
  // doesn't apply to the Mac.
  base::CommandLine GetCommandLineForRelaunch();
#endif

#if defined(OS_MACOSX)
  // Returns the autorelease pool in use inside RunTestOnMainThreadLoop().
  base::mac::ScopedNSAutoreleasePool* AutoreleasePool() const {
    return autorelease_pool_;
  }
#endif  // OS_MACOSX

  void set_exit_when_last_browser_closes(bool value) {
    exit_when_last_browser_closes_ = value;
  }

  void set_open_about_blank_on_browser_launch(bool value) {
    open_about_blank_on_browser_launch_ = value;
  }

  // Runs accessibility checks and sets |error_message| if it fails.
  bool RunAccessibilityChecks(std::string* error_message);

 private:
  // Creates a user data directory for the test if one is needed. Returns true
  // if successful.
  virtual bool CreateUserDataDirectory() WARN_UNUSED_RESULT;

  // Quits all open browsers and waits until there are no more browsers.
  void QuitBrowsers();

  static SetUpBrowserFunction* global_browser_set_up_function_;

  // Browser created from CreateBrowser.
  Browser* browser_;

  // Temporary user data directory. Used only when a user data directory is not
  // specified in the command line.
  base::ScopedTempDir temp_user_data_dir_;

  // True if we should exit the tests after the last browser instance closes.
  bool exit_when_last_browser_closes_;

  // True if the about:blank tab should be opened when the browser is launched.
  bool open_about_blank_on_browser_launch_;

  // True if the accessibility test should run for a particular test case.
  // This is reset for every test case.
  bool run_accessibility_checks_for_test_case_;

#if defined(OS_MACOSX)
  base::mac::ScopedNSAutoreleasePool* autorelease_pool_;
  std::unique_ptr<ScopedBundleSwizzlerMac> bundle_swizzler_;
#endif  // OS_MACOSX

#if defined(OS_WIN)
  std::unique_ptr<base::win::ScopedCOMInitializer> com_initializer_;
#endif
};

#endif  // CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
