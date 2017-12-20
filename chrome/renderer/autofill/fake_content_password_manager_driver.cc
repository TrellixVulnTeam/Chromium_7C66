// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/autofill/fake_content_password_manager_driver.h"

FakeContentPasswordManagerDriver::FakeContentPasswordManagerDriver() {}

FakeContentPasswordManagerDriver::~FakeContentPasswordManagerDriver() {}

void FakeContentPasswordManagerDriver::BindRequest(
    autofill::mojom::PasswordManagerDriverRequest request) {
  bindings_.AddBinding(this, std::move(request));
}

// mojom::PasswordManagerDriver:
void FakeContentPasswordManagerDriver::PasswordFormsParsed(
    const std::vector<autofill::PasswordForm>& forms) {}

void FakeContentPasswordManagerDriver::PasswordFormsRendered(
    const std::vector<autofill::PasswordForm>& visible_forms,
    bool did_stop_loading) {
  called_password_forms_rendered_ = true;
  password_forms_rendered_ = visible_forms;
}

void FakeContentPasswordManagerDriver::PasswordFormSubmitted(
    const autofill::PasswordForm& password_form) {
  called_password_form_submitted_ = true;
  password_form_submitted_ = password_form;
}

void FakeContentPasswordManagerDriver::InPageNavigation(
    const autofill::PasswordForm& password_form) {
  called_inpage_navigation_ = true;
  password_form_inpage_navigation_ = password_form;
}

void FakeContentPasswordManagerDriver::PresaveGeneratedPassword(
    const autofill::PasswordForm& password_form) {
  called_presave_generated_password_ = true;
}

void FakeContentPasswordManagerDriver::PasswordNoLongerGenerated(
    const autofill::PasswordForm& password_form) {
  called_password_no_longer_generated_ = true;
}

void FakeContentPasswordManagerDriver::ShowPasswordSuggestions(
    int key,
    base::i18n::TextDirection text_direction,
    const base::string16& typed_username,
    int options,
    const gfx::RectF& bounds) {
  called_show_pw_suggestions_ = true;
  show_pw_suggestions_key_ = key;
  show_pw_suggestions_username_ = typed_username;
  show_pw_suggestions_options_ = options;
}

void FakeContentPasswordManagerDriver::PasswordAutofillAgentConstructed() {
  called_agent_constructed_ = true;
}

void FakeContentPasswordManagerDriver::RecordSavePasswordProgress(
    const std::string& log) {
  called_record_save_progress_ = true;
}

void FakeContentPasswordManagerDriver::SaveGenerationFieldDetectedByClassifier(
    const autofill::PasswordForm& password_form,
    const base::string16& generation_field) {
  called_save_generation_field_ = true;
  save_generation_field_ = generation_field;
}
