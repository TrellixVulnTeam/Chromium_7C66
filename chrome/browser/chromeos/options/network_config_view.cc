// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/options/network_config_view.h"

#include <algorithm>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/chromeos/login/ui/login_display_host.h"
#include "chrome/browser/chromeos/options/network_property_ui_data.h"
#include "chrome/browser/chromeos/options/vpn_config_view.h"
#include "chrome/browser/chromeos/options/wifi_config_view.h"
#include "chrome/browser/chromeos/options/wimax_config_view.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/ash/ash_util.h"
#include "chrome/browser/ui/ash/system_tray_client.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/locale_settings.h"
#include "chromeos/login/login_state.h"
#include "chromeos/network/network_state.h"
#include "chromeos/network/network_state_handler.h"
#include "components/device_event_log/device_event_log.h"
#include "components/user_manager/user.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/vector_icons_public.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/layout_constants.h"
#include "ui/views/widget/widget.h"

using views::Widget;

namespace chromeos {

namespace {

// Used to check if a network config dialog is already showing.
NetworkConfigView* g_instance = nullptr;

gfx::NativeWindow GetParentForUnhostedDialog() {
  if (LoginDisplayHost::default_host()) {
    // TODO(jamescook): LoginDisplayHost has the wrong native window in mash.
    // This will fix itself when mash converts from ui::Window to aura::Window.
    // http://crbug.com/659155
    if (!chrome::IsRunningInMash())
      return LoginDisplayHost::default_host()->GetNativeWindow();
  } else {
    Browser* browser = chrome::FindTabbedBrowser(
        ProfileManager::GetPrimaryUserProfile(), true);
    if (browser)
      return browser->window()->GetNativeWindow();
  }
  return nullptr;
}

}  // namespace

// static
const int ChildNetworkConfigView::kInputFieldMinWidth = 270;

NetworkConfigView::NetworkConfigView()
    : child_config_view_(nullptr),
      delegate_(nullptr),
      advanced_button_(nullptr) {
  DCHECK(!g_instance);
  g_instance = this;
}

bool NetworkConfigView::InitWithNetworkState(const NetworkState* network) {
  DCHECK(network);
  std::string service_path = network->path();
  if (network->type() == shill::kTypeWifi ||
      network->type() == shill::kTypeEthernet) {
    child_config_view_ = new WifiConfigView(this, service_path, false);
  } else if (network->type() == shill::kTypeWimax) {
    child_config_view_ = new WimaxConfigView(this, service_path);
  } else if (network->type() == shill::kTypeVPN) {
    child_config_view_ = new VPNConfigView(this, service_path);
  }
  return child_config_view_ != nullptr;
}

bool NetworkConfigView::InitWithType(const std::string& type) {
  if (type == shill::kTypeWifi) {
    child_config_view_ = new WifiConfigView(this,
                                            "" /* service_path */,
                                            false /* show_8021x */);
    advanced_button_ = views::MdTextButton::CreateSecondaryUiButton(
        this, l10n_util::GetStringUTF16(
                  IDS_OPTIONS_SETTINGS_INTERNET_OPTIONS_ADVANCED_BUTTON));
  } else if (type == shill::kTypeVPN) {
    child_config_view_ = new VPNConfigView(this,
                                           "" /* service_path */);
  }
  return child_config_view_ != nullptr;
}

NetworkConfigView::~NetworkConfigView() {
  DCHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

// static
void NetworkConfigView::ShowForNetworkId(const std::string& network_id,
                                         gfx::NativeWindow parent) {
  if (g_instance)
    return;
  const NetworkState* network =
      NetworkHandler::Get()->network_state_handler()->GetNetworkStateFromGuid(
          network_id);
  if (!network) {
    LOG(ERROR)
        << "NetworkConfigView::ShowForNetworkId called with invalid network";
    return;
  }
  NetworkConfigView* view = new NetworkConfigView();
  if (!view->InitWithNetworkState(network)) {
    LOG(ERROR) << "NetworkConfigView::ShowForNetworkId called with invalid "
                  "network type: "
               << network->type();
    delete view;
    return;
  }
  NET_LOG(USER) << "NetworkConfigView::ShowForNetworkId: " << network->path();
  view->ShowDialog(parent);
}

// static
void NetworkConfigView::ShowForType(const std::string& type,
                                    gfx::NativeWindow parent) {
  if (g_instance)
    return;
  NetworkConfigView* view = new NetworkConfigView();
  if (!view->InitWithType(type)) {
    LOG(ERROR) << "NetworkConfigView::ShowForType called with invalid type: "
               << type;
    delete view;
    return;
  }
  NET_LOG(USER) << "NetworkConfigView::ShowForType: " << type;
  view->ShowDialog(parent);
}

gfx::NativeWindow NetworkConfigView::GetNativeWindow() const {
  return GetWidget()->GetNativeWindow();
}

base::string16 NetworkConfigView::GetDialogButtonLabel(
    ui::DialogButton button) const {
  if (button == ui::DIALOG_BUTTON_OK) {
    if (child_config_view_->IsConfigureDialog())
      return l10n_util::GetStringUTF16(IDS_OPTIONS_SETTINGS_CONFIGURE);
    return l10n_util::GetStringUTF16(IDS_OPTIONS_SETTINGS_CONNECT);
  }
  return views::DialogDelegateView::GetDialogButtonLabel(button);
}

bool NetworkConfigView::IsDialogButtonEnabled(ui::DialogButton button) const {
  // Disable connect button if cannot login.
  if (button == ui::DIALOG_BUTTON_OK)
    return child_config_view_->CanLogin();
  return true;
}

bool NetworkConfigView::Cancel() {
  if (delegate_)
    delegate_->OnDialogCancelled();
  child_config_view_->Cancel();
  return true;
}

bool NetworkConfigView::Accept() {
  // Do not attempt login if it is guaranteed to fail, keep the dialog open.
  if (!child_config_view_->CanLogin())
    return false;
  bool result = child_config_view_->Login();
  if (result && delegate_)
    delegate_->OnDialogAccepted();
  return result;
}

views::View* NetworkConfigView::CreateExtraView() {
  return advanced_button_;
}

views::View* NetworkConfigView::GetInitiallyFocusedView() {
  return child_config_view_->GetInitiallyFocusedView();
}

base::string16 NetworkConfigView::GetWindowTitle() const {
  DCHECK(!child_config_view_->GetTitle().empty());
  return child_config_view_->GetTitle();
}

ui::ModalType NetworkConfigView::GetModalType() const {
  return ui::MODAL_TYPE_SYSTEM;
}

void NetworkConfigView::GetAccessibleNodeData(ui::AXNodeData* node_data) {
  views::DialogDelegateView::GetAccessibleNodeData(node_data);
  node_data->SetName(
      l10n_util::GetStringUTF8(IDS_OPTIONS_SETTINGS_OTHER_WIFI_NETWORKS));
}

void NetworkConfigView::ButtonPressed(views::Button* sender,
                                      const ui::Event& event) {
  if (advanced_button_ && sender == advanced_button_) {
    advanced_button_->SetVisible(false);
    ShowAdvancedView();
  }
}

void NetworkConfigView::ShowAdvancedView() {
  // Clear out the old widgets and build new ones.
  RemoveChildView(child_config_view_);
  delete child_config_view_;
  // For now, there is only an advanced view for Wi-Fi 802.1X.
  child_config_view_ = new WifiConfigView(this,
                                          "" /* service_path */,
                                          true /* show_8021x */);
  AddChildView(child_config_view_);
  // Resize the window to be able to hold the new widgets.
  gfx::Size size = views::Widget::GetLocalizedContentsSize(
      IDS_JOIN_WIFI_NETWORK_DIALOG_ADVANCED_WIDTH_CHARS,
      IDS_JOIN_WIFI_NETWORK_DIALOG_ADVANCED_MINIMUM_HEIGHT_LINES);
  // Get the new bounds with desired size at the same center point.
  gfx::Rect bounds = GetWidget()->GetWindowBoundsInScreen();
  int horiz_padding = bounds.width() - size.width();
  int vert_padding = bounds.height() - size.height();
  bounds.Inset(horiz_padding / 2, vert_padding / 2,
               horiz_padding / 2, vert_padding / 2);
  GetWidget()->SetBoundsConstrained(bounds);
  Layout();
  child_config_view_->InitFocus();
}

void NetworkConfigView::Layout() {
  child_config_view_->SetBounds(0, 0, width(), height());
}

gfx::Size NetworkConfigView::GetPreferredSize() const {
  gfx::Size result(views::Widget::GetLocalizedContentsSize(
      IDS_JOIN_WIFI_NETWORK_DIALOG_WIDTH_CHARS,
      IDS_JOIN_WIFI_NETWORK_DIALOG_MINIMUM_HEIGHT_LINES));
  gfx::Size size = child_config_view_->GetPreferredSize();
  result.set_height(size.height());
  if (size.width() > result.width())
    result.set_width(size.width());
  return result;
}

void NetworkConfigView::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  // Can't init before we're inserted into a Container, because we require
  // a HWND to parent native child controls to.
  views::DialogDelegateView::ViewHierarchyChanged(details);
  if (details.is_add && details.child == this) {
    AddChildView(child_config_view_);
  }
}

void NetworkConfigView::ShowDialog(gfx::NativeWindow parent) {
  // Attempt to find a fallback parent window.
  if (parent == nullptr)
    parent = GetParentForUnhostedDialog();

  Widget* window = nullptr;
  if (parent) {
    // Create as a child of |parent|.
    window = DialogDelegate::CreateDialogWidget(this, nullptr, parent);
  } else {
    // Fall back to default window container on primary display.
    window = SystemTrayClient::CreateUnownedDialogWidget(this);
  }
  window->SetAlwaysOnTop(true);
  window->Show();
}

// ChildNetworkConfigView

ChildNetworkConfigView::ChildNetworkConfigView(
    NetworkConfigView* parent,
    const std::string& service_path)
    : parent_(parent),
      service_path_(service_path) {
}

ChildNetworkConfigView::~ChildNetworkConfigView() {
}

bool ChildNetworkConfigView::IsConfigureDialog() {
  return false;
}

// static
void ChildNetworkConfigView::GetShareStateForLoginState(bool* default_value,
                                                        bool* modifiable) {
  *default_value = !LoginState::Get()->UserHasNetworkProfile();
  // Allow only authenticated user to change the share state.
  *modifiable = LoginState::Get()->IsUserAuthenticated();
}

// ControlledSettingIndicatorView

ControlledSettingIndicatorView::ControlledSettingIndicatorView(
    const NetworkPropertyUIData& ui_data)
    : managed_(ui_data.IsManaged()), image_view_(nullptr) {
  image_view_ = new views::ImageView();
  // Disable |image_view_| so mouse events propagate to the parent.
  image_view_->SetEnabled(false);
  image_view_->SetImage(gfx::CreateVectorIcon(gfx::VectorIconId::BUSINESS, 16,
                                              gfx::kChromeIconGrey));
  image_view_->SetTooltipText(
      l10n_util::GetStringUTF16(IDS_OPTIONS_CONTROLLED_SETTING_POLICY));
  AddChildView(image_view_);
  SetLayoutManager(new views::FillLayout());
}

ControlledSettingIndicatorView::~ControlledSettingIndicatorView() {}

gfx::Size ControlledSettingIndicatorView::GetPreferredSize() const {
  return (managed_ && visible()) ? image_view_->GetPreferredSize()
                                 : gfx::Size();
}

}  // namespace chromeos
