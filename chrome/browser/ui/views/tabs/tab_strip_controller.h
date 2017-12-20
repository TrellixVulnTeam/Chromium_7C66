// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_CONTROLLER_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_CONTROLLER_H_

#include "base/strings/string16.h"
#include "chrome/browser/ui/views/tabs/tab_strip_types.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/ui_base_types.h"

class GURL;
class Tab;
class TabStrip;

namespace gfx {
class Point;
}

namespace ui {
class ListSelectionModel;
}

// Model/Controller for the TabStrip.
// NOTE: All indices used by this class are in model coordinates.
class TabStripController {
 public:
  virtual ~TabStripController() {}

  // Returns the selection model of the tabstrip.
  virtual const ui::ListSelectionModel& GetSelectionModel() const = 0;

  // Returns the number of tabs in the model.
  virtual int GetCount() const = 0;

  // Returns true if |index| is a valid model index.
  virtual bool IsValidIndex(int index) const = 0;

  // Returns true if the tab at |index| is the active tab. The active tab is the
  // one whose content is shown.
  virtual bool IsActiveTab(int index) const = 0;

  // Returns the index of the active tab.
  virtual int GetActiveIndex() const = 0;

  // Returns true if the selected index is selected.
  virtual bool IsTabSelected(int index) const = 0;

  // Returns true if the selected index is pinned.
  virtual bool IsTabPinned(int index) const = 0;

  // Select the tab at the specified index in the model.
  virtual void SelectTab(int index) = 0;

  // Extends the selection from the anchor to the specified index in the model.
  virtual void ExtendSelectionTo(int index) = 0;

  // Toggles the selection of the specified index in the model.
  virtual void ToggleSelected(int index) = 0;

  // Adds the selection the anchor to |index|.
  virtual void AddSelectionFromAnchorTo(int index) = 0;

  // Closes the tab at the specified index in the model.
  virtual void CloseTab(int index, CloseTabSource source) = 0;

  // Toggles audio muting for the tab at the specified index in the model.
  virtual void ToggleTabAudioMute(int index) = 0;

  // Shows a context menu for the tab at the specified point in screen coords.
  virtual void ShowContextMenuForTab(Tab* tab,
                                     const gfx::Point& p,
                                     ui::MenuSourceType source_type) = 0;

  // Updates the loading animations of all the tabs.
  virtual void UpdateLoadingAnimations() = 0;

  // Returns true if the associated TabStrip's delegate supports tab moving or
  // detaching. Used by the Frame to determine if dragging on the Tab
  // itself should move the window in cases where there's only one
  // non drag-able Tab.
  virtual int HasAvailableDragActions() const = 0;

  // Notifies controller of a drop index update.
  virtual void OnDropIndexUpdate(int index, bool drop_before) = 0;

  // Performs a drop at the specified location.
  virtual void PerformDrop(bool drop_before, int index, const GURL& url) = 0;

  // Return true if this tab strip is compatible with the provided tab strip.
  // Compatible tab strips can transfer tabs during drag and drop.
  virtual bool IsCompatibleWith(TabStrip* other) const = 0;

  // Creates the new tab.
  virtual void CreateNewTab() = 0;

  // Creates a new tab, and loads |location| in the tab. If |location| is a
  // valid URL, then simply loads the URL, otherwise this can open a
  // search-result page for |location|.
  virtual void CreateNewTabWithLocation(const base::string16& location) = 0;

  // Returns true if the tab strip is in an incognito window.  This is used to
  // determining which theme may have applied to it, so this determination
  // should match the one in ThemeService::GetThemeProviderForProfile().
  virtual bool IsIncognito() = 0;

  // Invoked if the stacked layout (on or off) might have changed.
  virtual void StackedLayoutMaybeChanged() = 0;

  // Notifies controller that the user started dragging this tabstrip's tabs.
  virtual void OnStartedDraggingTabs() = 0;

  // Notifies controller that the user stopped dragging this tabstrip's tabs.
  // This is also called when the tabs that the user is dragging were detached
  // from this tabstrip but the user is still dragging the tabs.
  virtual void OnStoppedDraggingTabs() = 0;

  // Determines if the file type of the URL is supported. Should invoke
  // TabStrip::FileSupported to report the result.
  virtual void CheckFileSupported(const GURL& url) = 0;

  // Returns COLOR_TOOLBAR_TOP_SEPARATOR[,_INACTIVE] depending on the activation
  // state of the window.
  virtual SkColor GetToolbarTopSeparatorColor() const = 0;
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_TAB_STRIP_CONTROLLER_H_
