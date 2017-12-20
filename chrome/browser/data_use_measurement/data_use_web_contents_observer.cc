// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/data_use_measurement/data_use_web_contents_observer.h"

#include "chrome/browser/data_use_measurement/chrome_data_use_ascriber_service.h"
#include "chrome/browser/data_use_measurement/chrome_data_use_ascriber_service_factory.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(
    data_use_measurement::DataUseWebContentsObserver);

namespace data_use_measurement {

// static
void DataUseWebContentsObserver::CreateForWebContents(
    content::WebContents* web_contents) {
  DCHECK(web_contents);

  ChromeDataUseAscriberService* service =
      ChromeDataUseAscriberServiceFactory::GetForBrowserContext(
          web_contents->GetBrowserContext());

  // Nothing to do if there is no service (Incognito), or if instance already
  // exists.
  if (!service || FromWebContents(web_contents))
    return;

  // |DataUseWebContentsObserver| is a |WebContentsUserData| so its lifetime
  // is scoped to |web_contents|.
  // |ChromeDataUseAscriberService| is a |KeyedService| and its lifetime is
  // tied to a profile. Since profiles outlive |WebContents|, |service| will
  // outlive |DataUseWebContentsObserver|.
  web_contents->SetUserData(
      UserDataKey(), new DataUseWebContentsObserver(web_contents, service));
}

DataUseWebContentsObserver::DataUseWebContentsObserver(
    content::WebContents* web_contents,
    ChromeDataUseAscriberService* service)
    : content::WebContentsObserver(web_contents), service_(service) {
  // Call RenderFrameCreated for live frames so that |service_| knows about
  // all the RenderFrameHosts.
  for (content::RenderFrameHost* frame : web_contents->GetAllFrames()) {
    if (frame->IsRenderFrameLive()) {
      service_->RenderFrameCreated(frame);
    }
  }
}

DataUseWebContentsObserver::~DataUseWebContentsObserver() {}

void DataUseWebContentsObserver::RenderFrameCreated(
    content::RenderFrameHost* render_frame_host) {
  service_->RenderFrameCreated(render_frame_host);
}

void DataUseWebContentsObserver::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  service_->RenderFrameDeleted(render_frame_host);
}

void DataUseWebContentsObserver::DidStartNavigation(
    content::NavigationHandle* navigation_handle) {
  service_->DidStartNavigation(navigation_handle);
}

void DataUseWebContentsObserver::ReadyToCommitNavigation(
    content::NavigationHandle* navigation_handle) {
  service_->ReadyToCommitNavigation(navigation_handle);
}

void DataUseWebContentsObserver::DidRedirectNavigation(
    content::NavigationHandle* navigation_handle) {
  service_->DidRedirectNavigation(navigation_handle);
}

}  // namespace data_use_measurement
