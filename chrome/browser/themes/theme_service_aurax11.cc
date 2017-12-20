// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/themes/theme_service_aurax11.h"

#include "base/bind.h"
#include "base/macros.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/themes/custom_theme_supplier.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "ui/gfx/image/image.h"
#include "ui/native_theme/native_theme_aura.h"
#include "ui/views/linux_ui/linux_ui.h"

namespace {

class SystemThemeX11 : public CustomThemeSupplier {
 public:
  explicit SystemThemeX11(PrefService* pref_service);

  // Overridden from CustomThemeSupplier:
  void StartUsingTheme() override;
  void StopUsingTheme() override;
  bool GetTint(int id, color_utils::HSL* hsl) const override;
  bool GetColor(int id, SkColor* color) const override;
  gfx::Image GetImageNamed(int id) override;
  bool HasCustomImage(int id) const override;

 private:
  ~SystemThemeX11() override;

  // These pointers are not owned by us.
  views::LinuxUI* const linux_ui_;
  PrefService* const pref_service_;

  DISALLOW_COPY_AND_ASSIGN(SystemThemeX11);
};

SystemThemeX11::SystemThemeX11(PrefService* pref_service)
    : CustomThemeSupplier(NATIVE_X11),
      linux_ui_(views::LinuxUI::instance()),
      pref_service_(pref_service) {}

void SystemThemeX11::StartUsingTheme() {
  pref_service_->SetBoolean(prefs::kUsesSystemTheme, true);
  // Have the former theme notify its observers of change.
  ui::NativeThemeAura::instance()->NotifyObservers();
}

void SystemThemeX11::StopUsingTheme() {
  pref_service_->SetBoolean(prefs::kUsesSystemTheme, false);
  // Have the former theme notify its observers of change.
  if (linux_ui_)
    linux_ui_->GetNativeTheme(NULL)->NotifyObservers();
}

bool SystemThemeX11::GetTint(int id, color_utils::HSL* hsl) const {
  return linux_ui_ && linux_ui_->GetTint(id, hsl);
}

bool SystemThemeX11::GetColor(int id, SkColor* color) const {
  return linux_ui_ && linux_ui_->GetColor(id, color);
}

gfx::Image SystemThemeX11::GetImageNamed(int id) {
  return gfx::Image();
}

bool SystemThemeX11::HasCustomImage(int id) const {
  return false;
}

SystemThemeX11::~SystemThemeX11() {}

}  // namespace

ThemeServiceAuraX11::ThemeServiceAuraX11() {}

ThemeServiceAuraX11::~ThemeServiceAuraX11() {}

bool ThemeServiceAuraX11::ShouldInitWithSystemTheme() const {
  return profile()->GetPrefs()->GetBoolean(prefs::kUsesSystemTheme);
}

void ThemeServiceAuraX11::UseSystemTheme() {
  SetCustomDefaultTheme(new SystemThemeX11(profile()->GetPrefs()));
}

bool ThemeServiceAuraX11::IsSystemThemeDistinctFromDefaultTheme() const {
  return true;
}

bool ThemeServiceAuraX11::UsingDefaultTheme() const {
  return ThemeService::UsingDefaultTheme() && !UsingSystemTheme();
}

bool ThemeServiceAuraX11::UsingSystemTheme() const {
  const CustomThemeSupplier* theme_supplier = get_theme_supplier();
  return theme_supplier &&
         theme_supplier->get_theme_type() == CustomThemeSupplier::NATIVE_X11;
}
