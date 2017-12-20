// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/passwords/manage_passwords_state.h"

#include <algorithm>
#include <utility>

#include "base/memory/ptr_util.h"
#include "components/password_manager/core/browser/browser_save_password_progress_logger.h"
#include "components/password_manager/core/browser/log_manager.h"
#include "components/password_manager/core/browser/password_form_manager.h"
#include "components/password_manager/core/browser/password_manager.h"
#include "components/password_manager/core/browser/password_manager_client.h"

using password_manager::PasswordFormManager;

namespace {

std::vector<std::unique_ptr<autofill::PasswordForm>> DeepCopyMapToVector(
    const std::map<base::string16, const autofill::PasswordForm*>&
        password_form_map) {
  std::vector<std::unique_ptr<autofill::PasswordForm>> ret;
  ret.reserve(password_form_map.size());
  for (const auto& form_pair : password_form_map)
    ret.push_back(base::MakeUnique<autofill::PasswordForm>(*form_pair.second));
  return ret;
}

void AppendDeepCopyVector(
    const std::vector<const autofill::PasswordForm*>& forms,
    std::vector<std::unique_ptr<autofill::PasswordForm>>* result) {
  result->reserve(result->size() + forms.size());
  for (const auto& form : forms)
    result->push_back(base::MakeUnique<autofill::PasswordForm>(*form));
}

// Updates one form in |forms| that has the same unique key as |updated_form|.
// Returns true if the form was found and updated.
bool UpdateFormInVector(
    const autofill::PasswordForm& updated_form,
    std::vector<std::unique_ptr<autofill::PasswordForm>>* forms) {
  auto it = std::find_if(
      forms->begin(), forms->end(),
      [&updated_form](const std::unique_ptr<autofill::PasswordForm>& form) {
        return ArePasswordFormUniqueKeyEqual(*form, updated_form);
      });
  if (it != forms->end()) {
    **it = updated_form;
    return true;
  }
  return false;
}

// Removes a form from |forms| that has the same unique key as |form_to_delete|.
void RemoveFormFromVector(
    const autofill::PasswordForm& form_to_delete,
    std::vector<std::unique_ptr<autofill::PasswordForm>>* forms) {
  auto it = std::find_if(
      forms->begin(), forms->end(),
      [&form_to_delete](const std::unique_ptr<autofill::PasswordForm>& form) {
        return ArePasswordFormUniqueKeyEqual(*form, form_to_delete);
      });
  if (it != forms->end())
    forms->erase(it);
}

}  // namespace

ManagePasswordsState::ManagePasswordsState()
    : state_(password_manager::ui::INACTIVE_STATE),
      client_(nullptr) {
}

ManagePasswordsState::~ManagePasswordsState() {}

void ManagePasswordsState::OnPendingPassword(
    std::unique_ptr<password_manager::PasswordFormManager> form_manager) {
  ClearData();
  form_manager_ = std::move(form_manager);
  local_credentials_forms_ = DeepCopyMapToVector(form_manager_->best_matches());
  AppendDeepCopyVector(form_manager_->form_fetcher()->GetFederatedMatches(),
                       &local_credentials_forms_);
  origin_ = form_manager_->observed_form().origin;
  SetState(password_manager::ui::PENDING_PASSWORD_STATE);
}

void ManagePasswordsState::OnUpdatePassword(
    std::unique_ptr<password_manager::PasswordFormManager> form_manager) {
  ClearData();
  form_manager_ = std::move(form_manager);
  local_credentials_forms_ = DeepCopyMapToVector(form_manager_->best_matches());
  AppendDeepCopyVector(form_manager_->form_fetcher()->GetFederatedMatches(),
                       &local_credentials_forms_);
  origin_ = form_manager_->observed_form().origin;
  SetState(password_manager::ui::PENDING_PASSWORD_UPDATE_STATE);
}

void ManagePasswordsState::OnRequestCredentials(
    std::vector<std::unique_ptr<autofill::PasswordForm>> local_credentials,
    std::vector<std::unique_ptr<autofill::PasswordForm>> federation_providers,
    const GURL& origin) {
  ClearData();
  local_credentials_forms_ = std::move(local_credentials);
  federation_providers_forms_ = std::move(federation_providers);
  origin_ = origin;
  SetState(password_manager::ui::CREDENTIAL_REQUEST_STATE);
}

void ManagePasswordsState::OnAutoSignin(
    std::vector<std::unique_ptr<autofill::PasswordForm>> local_forms,
    const GURL& origin) {
  DCHECK(!local_forms.empty());
  ClearData();
  local_credentials_forms_ = std::move(local_forms);
  origin_ = origin;
  SetState(password_manager::ui::AUTO_SIGNIN_STATE);
}

void ManagePasswordsState::OnAutomaticPasswordSave(
    std::unique_ptr<PasswordFormManager> form_manager) {
  ClearData();
  form_manager_ = std::move(form_manager);
  local_credentials_forms_.reserve(form_manager_->best_matches().size());
  bool updated = false;
  for (const auto& form : form_manager_->best_matches()) {
    if (form_manager_->pending_credentials().username_value == form.first) {
      local_credentials_forms_.push_back(
          base::MakeUnique<autofill::PasswordForm>(
              form_manager_->pending_credentials()));
      updated = true;
    } else {
      local_credentials_forms_.push_back(
          base::MakeUnique<autofill::PasswordForm>(*form.second));
    }
  }
  if (!updated) {
    local_credentials_forms_.push_back(base::MakeUnique<autofill::PasswordForm>(
        form_manager_->pending_credentials()));
  }
  AppendDeepCopyVector(form_manager_->form_fetcher()->GetFederatedMatches(),
                       &local_credentials_forms_);
  origin_ = form_manager_->observed_form().origin;
  SetState(password_manager::ui::CONFIRMATION_STATE);
}

void ManagePasswordsState::OnPasswordAutofilled(
    const std::map<base::string16, const autofill::PasswordForm*>&
        password_form_map,
    const GURL& origin,
    const std::vector<const autofill::PasswordForm*>* federated_matches) {
  DCHECK(!password_form_map.empty());
  ClearData();
  bool only_PSL_matches = std::all_of(
      password_form_map.begin(), password_form_map.end(),
      [](const std::map<base::string16,
                        const autofill::PasswordForm*>::value_type& p) {
        return p.second->is_public_suffix_match;
      });
  if (only_PSL_matches) {
    // Don't show the UI for PSL matched passwords. They are not stored for this
    // page and cannot be deleted.
    origin_ = GURL();
    SetState(password_manager::ui::INACTIVE_STATE);
  } else {
    local_credentials_forms_ = DeepCopyMapToVector(password_form_map);
    if (federated_matches)
      AppendDeepCopyVector(*federated_matches, &local_credentials_forms_);
    origin_ = origin;
    SetState(password_manager::ui::MANAGE_STATE);
  }
}

void ManagePasswordsState::OnInactive() {
  ClearData();
  origin_ = GURL();
  SetState(password_manager::ui::INACTIVE_STATE);
}

void ManagePasswordsState::TransitionToState(
    password_manager::ui::State state) {
  DCHECK_NE(password_manager::ui::INACTIVE_STATE, state_);
  DCHECK_EQ(password_manager::ui::MANAGE_STATE, state);
  if (state_ == password_manager::ui::CREDENTIAL_REQUEST_STATE) {
    if (!credentials_callback_.is_null()) {
      credentials_callback_.Run(nullptr);
      credentials_callback_.Reset();
    }
    federation_providers_forms_.clear();
  }
  SetState(state);
}

void ManagePasswordsState::ProcessLoginsChanged(
    const password_manager::PasswordStoreChangeList& changes) {
  if (state() == password_manager::ui::INACTIVE_STATE)
    return;

  for (const password_manager::PasswordStoreChange& change : changes) {
    const autofill::PasswordForm& changed_form = change.form();
    if (changed_form.blacklisted_by_user)
      continue;
    if (change.type() == password_manager::PasswordStoreChange::REMOVE) {
      DeleteForm(changed_form);
    } else {
      if (change.type() == password_manager::PasswordStoreChange::UPDATE)
        UpdateForm(changed_form);
      else
        AddForm(changed_form);
    }
  }
}

void ManagePasswordsState::ChooseCredential(
    const autofill::PasswordForm* form) {
  DCHECK_EQ(password_manager::ui::CREDENTIAL_REQUEST_STATE, state());
  DCHECK(!credentials_callback().is_null());

  credentials_callback().Run(form);
  set_credentials_callback(ManagePasswordsState::CredentialsCallback());
}

void ManagePasswordsState::ClearData() {
  form_manager_.reset();
  local_credentials_forms_.clear();
  federation_providers_forms_.clear();
  credentials_callback_.Reset();
}

void ManagePasswordsState::AddForm(const autofill::PasswordForm& form) {
  if (form.origin != origin_)
    return;
  if (UpdateForm(form))
    return;
  local_credentials_forms_.push_back(
      base::MakeUnique<autofill::PasswordForm>(form));
}

bool ManagePasswordsState::UpdateForm(const autofill::PasswordForm& form) {
  bool updated_locals = UpdateFormInVector(form, &local_credentials_forms_);
  return (UpdateFormInVector(form, &federation_providers_forms_) ||
          updated_locals);
}

void ManagePasswordsState::DeleteForm(const autofill::PasswordForm& form) {
  RemoveFormFromVector(form, &local_credentials_forms_);
  RemoveFormFromVector(form, &federation_providers_forms_);
}

void ManagePasswordsState::SetState(password_manager::ui::State state) {
  DCHECK(client_);
  if (client_->GetLogManager()->IsLoggingActive()) {
    password_manager::BrowserSavePasswordProgressLogger logger(
        client_->GetLogManager());
    logger.LogNumber(
        autofill::SavePasswordProgressLogger::STRING_NEW_UI_STATE,
        state);
  }
  state_ = state;
}
