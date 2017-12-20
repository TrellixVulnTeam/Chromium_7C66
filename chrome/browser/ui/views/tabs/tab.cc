// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/tabs/tab.h"

#include <stddef.h>
#include <limits>
#include <utility>

#include "base/command_line.h"
#include "base/debug/alias.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/themes/theme_properties.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/layout_constants.h"
#include "chrome/browser/ui/tab_contents/core_tab_helper.h"
#include "chrome/browser/ui/tabs/tab_utils.h"
#include "chrome/browser/ui/view_ids.h"
#include "chrome/browser/ui/views/tabs/alert_indicator_button.h"
#include "chrome/browser/ui/views/tabs/tab_controller.h"
#include "chrome/browser/ui/views/touch_uma/touch_uma.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/theme_resources.h"
#include "components/grit/components_scaled_resources.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/user_metrics.h"
#include "content/public/common/url_constants.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/pathops/SkPathOps.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/list_selection_model.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/theme_provider.h"
#include "ui/gfx/animation/animation_container.h"
#include "ui/gfx/animation/throb_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_analysis.h"
#include "ui/gfx/favicon_size.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/image/image_skia_operations.h"
#include "ui/gfx/paint_vector_icon.h"
#include "ui/gfx/path.h"
#include "ui/gfx/scoped_canvas.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vector_icons_public.h"
#include "ui/resources/grit/ui_resources.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/rect_based_targeting_utils.h"
#include "ui/views/view_targeter.h"
#include "ui/views/widget/tooltip_manager.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/non_client_view.h"

#if defined(USE_AURA)
#include "ui/aura/env.h"
#endif

using base::UserMetricsAction;

namespace {

// Width of touch tabs.
const int kTouchWidth = 120;

const int kExtraLeftPaddingToBalanceCloseButtonPadding = 2;
const int kAfterTitleSpacing = 4;

// When a non-pinned tab becomes a pinned tab the width of the tab animates. If
// the width of a pinned tab is at least kPinnedTabExtraWidthToRenderAsNormal
// larger than the desired pinned tab width then the tab is rendered as a normal
// tab. This is done to avoid having the title immediately disappear when
// transitioning a tab from normal to pinned tab.
const int kPinnedTabExtraWidthToRenderAsNormal = 30;

// How opaque to make the hover state (out of 1).
const double kHoverOpacity = 0.33;

// Opacity of the active tab background painted over inactive selected tabs.
const double kSelectedTabOpacity = 0.3;

// Inactive selected tabs have their throb value scaled by this.
const double kSelectedTabThrobScale = 0.95 - kSelectedTabOpacity;

// Max number of images to cache. This has to be at least two since rounding
// errors may lead to tabs in the same tabstrip having different sizes.
// 8 = normal/incognito, active/inactive, 2 sizes within tabstrip.
const size_t kMaxImageCacheSize = 8;

// Height of the miniature tab strip in immersive mode.
const int kImmersiveTabHeight = 3;

// Height of the small tab indicator rectangles in immersive mode.
const int kImmersiveBarHeight = 2;

// Color for active and inactive tabs in the immersive mode light strip. These
// should be a little brighter than the color of the normal art assets for tabs,
// which for active tabs is 230, 230, 230 and for inactive is 184, 184, 184.
const SkColor kImmersiveActiveTabColor = SkColorSetRGB(235, 235, 235);
const SkColor kImmersiveInactiveTabColor = SkColorSetRGB(190, 190, 190);

// The minimum opacity (out of 1) when a tab (either active or inactive) is
// throbbing in the immersive mode light strip.
const double kImmersiveTabMinThrobOpacity = 0.66;

// Number of steps in the immersive mode loading animation.
const int kImmersiveLoadingStepCount = 32;

const char kTabCloseButtonName[] = "TabCloseButton";

////////////////////////////////////////////////////////////////////////////////
// ImageCacheEntryMetadata
//
// All metadata necessary to uniquely identify a cached image.
struct ImageCacheEntryMetadata {
  ImageCacheEntryMetadata(SkColor fill_color,
                          SkColor stroke_color,
                          bool use_fill_and_stroke_images,
                          float scale_factor,
                          const gfx::Size& size);

  ~ImageCacheEntryMetadata();

  bool operator==(const ImageCacheEntryMetadata& rhs) const;

  SkColor fill_color;
  SkColor stroke_color;
  bool use_fill_and_stroke_images;
  float scale_factor;
  gfx::Size size;
};

ImageCacheEntryMetadata::ImageCacheEntryMetadata(
    SkColor fill_color,
    SkColor stroke_color,
    bool use_fill_and_stroke_images,
    float scale_factor,
    const gfx::Size& size)
    : fill_color(fill_color),
      stroke_color(stroke_color),
      use_fill_and_stroke_images(use_fill_and_stroke_images),
      scale_factor(scale_factor),
      size(size) {}

ImageCacheEntryMetadata::~ImageCacheEntryMetadata() {}

bool ImageCacheEntryMetadata::operator==(
    const ImageCacheEntryMetadata& rhs) const {
  return fill_color == rhs.fill_color && stroke_color == rhs.stroke_color &&
         use_fill_and_stroke_images == rhs.use_fill_and_stroke_images &&
         scale_factor == rhs.scale_factor && size == rhs.size;
}

////////////////////////////////////////////////////////////////////////////////
// ImageCacheEntry and cache management
//
// A cached image and the metadata used to generate it.
struct ImageCacheEntry {
  ImageCacheEntry(const ImageCacheEntryMetadata& metadata,
                  const gfx::ImageSkia& fill_image,
                  const gfx::ImageSkia& stroke_image);
  ~ImageCacheEntry();

  ImageCacheEntryMetadata metadata;
  gfx::ImageSkia fill_image;
  gfx::ImageSkia stroke_image;
};

ImageCacheEntry::ImageCacheEntry(const ImageCacheEntryMetadata& metadata,
                                 const gfx::ImageSkia& fill_image,
                                 const gfx::ImageSkia& stroke_image)
    : metadata(metadata), fill_image(fill_image), stroke_image(stroke_image) {}

ImageCacheEntry::~ImageCacheEntry() {}

typedef std::list<ImageCacheEntry> ImageCache;

// As the majority of the tabs are inactive, and painting tabs is slowish,
// we cache a handful of the inactive tab backgrounds here.
static ImageCache* g_image_cache = nullptr;

// Performs a one-time initialization of static resources such as tab images.
void InitTabResources() {
  static bool initialized = false;
  if (initialized)
    return;

  initialized = true;
  g_image_cache = new ImageCache();
}

////////////////////////////////////////////////////////////////////////////////
// Drawing and utility functions

// Returns the width of the tab endcap at scale 1.  More precisely, this is the
// width of the curve making up either the outer or inner edge of the stroke;
// since these two curves are horizontally offset by 1 px (regardless of scale),
// the total width of the endcap from tab outer edge to the inside end of the
// stroke inner edge is (GetUnscaledEndcapWidth() * scale) + 1.
float GetUnscaledEndcapWidth() {
  return GetLayoutInsets(TAB).left() - 0.5f;
}

void DrawHighlight(gfx::Canvas* canvas,
                   const SkPoint& p,
                   SkScalar radius,
                   SkColor color) {
  const SkColor colors[2] = { color, SkColorSetA(color, 0) };
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setShader(SkGradientShader::MakeRadial(p, radius, colors, nullptr, 2,
                                               SkShader::kClamp_TileMode));
  canvas->sk_canvas()->drawRect(
      SkRect::MakeXYWH(p.x() - radius, p.y() - radius, radius * 2, radius * 2),
      paint);
}

// Returns whether the favicon for the given URL should be colored according to
// the browser theme.
bool ShouldThemifyFaviconForUrl(const GURL& url) {
  return url.SchemeIs(content::kChromeUIScheme) &&
         url.host() != chrome::kChromeUIHelpHost &&
         url.host() != chrome::kChromeUIUberHost &&
         url.host() != chrome::kChromeUIAppLauncherPageHost;
}

// Returns a path corresponding to the tab's content region inside the outer
// stroke.
gfx::Path GetFillPath(float scale, const gfx::Size& size) {
  const float right = size.width() * scale;
  // The bottom of the tab needs to be pixel-aligned or else when we call
  // ClipPath with anti-aliasing enabled it can cause artifacts.
  const float bottom = std::ceil(size.height() * scale);
  const float unscaled_endcap_width = GetUnscaledEndcapWidth();

  gfx::Path fill;
  fill.moveTo(right - 1, bottom);
  fill.rCubicTo(-0.75 * scale, 0, -1.625 * scale, -0.5 * scale, -2 * scale,
                -1.5 * scale);
  fill.lineTo(right - 1 - (unscaled_endcap_width - 2) * scale, 2.5 * scale);
  // Prevent overdraw in the center near minimum width (only happens if
  // scale < 2).  We could instead avoid this by increasing the tab inset
  // values, but that would shift all the content inward as well, unless we
  // then overlapped the content on the endcaps, by which point we'd have a
  // huge mess.
  const float scaled_endcap_width = 1 + unscaled_endcap_width * scale;
  const float overlap = scaled_endcap_width * 2 - right;
  const float offset = (overlap > 0) ? (overlap / 2) : 0;
  fill.rCubicTo(-0.375 * scale, -1 * scale, -1.25 * scale + offset,
                -1.5 * scale, -2 * scale + offset, -1.5 * scale);
  if (overlap < 0)
    fill.lineTo(scaled_endcap_width, scale);
  fill.rCubicTo(-0.75 * scale, 0, -1.625 * scale - offset, 0.5 * scale,
                -2 * scale - offset, 1.5 * scale);
  fill.lineTo(1 + 2 * scale, bottom - 1.5 * scale);
  fill.rCubicTo(-0.375 * scale, scale, -1.25 * scale, 1.5 * scale, -2 * scale,
                1.5 * scale);
  fill.close();
  return fill;
}

// Returns a path corresponding to the tab's outer border for a given tab |size|
// and |scale|.  If |unscale_at_end| is true, this path will be normalized to a
// 1x scale by scaling by 1/scale before returning.  If |extend_to_top| is true,
// the path is extended vertically to the top of the tab bounds.  The caller
// uses this for Fitts' Law purposes in maximized/fullscreen mode.
gfx::Path GetBorderPath(float scale,
                        bool unscale_at_end,
                        bool extend_to_top,
                        const gfx::Size& size) {
  const float top = scale - 1;
  const float right = size.width() * scale;
  const float bottom = size.height() * scale;
  const float unscaled_endcap_width = GetUnscaledEndcapWidth();

  gfx::Path path;
  path.moveTo(0, bottom);
  path.rLineTo(0, -1);
  path.rCubicTo(0.75 * scale, 0, 1.625 * scale, -0.5 * scale, 2 * scale,
                -1.5 * scale);
  path.lineTo((unscaled_endcap_width - 2) * scale, top + 1.5 * scale);
  if (extend_to_top) {
    // Create the vertical extension by extending the side diagonals until
    // they reach the top of the bounds.
    const float dy = 2.5 * scale - 1;
    const float dx = Tab::GetInverseDiagonalSlope() * dy;
    path.rLineTo(dx, -dy);
    path.lineTo(right - (unscaled_endcap_width - 2) * scale - dx, 0);
    path.rLineTo(dx, dy);
  } else {
    path.rCubicTo(0.375 * scale, -scale, 1.25 * scale, -1.5 * scale, 2 * scale,
                  -1.5 * scale);
    path.lineTo(right - unscaled_endcap_width * scale, top);
    path.rCubicTo(0.75 * scale, 0, 1.625 * scale, 0.5 * scale, 2 * scale,
                  1.5 * scale);
  }
  path.lineTo(right - 2 * scale, bottom - 1 - 1.5 * scale);
  path.rCubicTo(0.375 * scale, scale, 1.25 * scale, 1.5 * scale, 2 * scale,
                1.5 * scale);
  path.rLineTo(0, 1);
  path.close();

  if (unscale_at_end && (scale != 1))
    path.transform(SkMatrix::MakeScale(1.f / scale));

  return path;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// FaviconCrashAnimation
//
//  A custom animation subclass to manage the favicon crash animation.
class Tab::FaviconCrashAnimation : public gfx::LinearAnimation,
                                   public gfx::AnimationDelegate {
 public:
  explicit FaviconCrashAnimation(Tab* target)
      : gfx::LinearAnimation(1000, 25, this),
        target_(target) {
  }
  ~FaviconCrashAnimation() override {}

  // gfx::Animation overrides:
  void AnimateToState(double state) override {
    const double kHidingOffset =
        Tab::GetMinimumInactiveSize().height() - GetLayoutInsets(TAB).height();

    if (state < .5) {
      // Animate the normal icon down.
      target_->SetFaviconHidingOffset(
          static_cast<int>(floor(kHidingOffset * 2.0 * state)));
    } else {
      // Animate the crashed icon up.
      target_->SetShouldDisplayCrashedFavicon(true);
      target_->SetFaviconHidingOffset(
          static_cast<int>(
              floor(kHidingOffset - ((state - .5) * 2.0 * kHidingOffset))));
    }
  }

 private:
  Tab* target_;

  DISALLOW_COPY_AND_ASSIGN(FaviconCrashAnimation);
};

////////////////////////////////////////////////////////////////////////////////
// TabCloseButton
//
//  This is a Button subclass that causes middle clicks to be forwarded to the
//  parent View by explicitly not handling them in OnMousePressed.
class Tab::TabCloseButton : public views::ImageButton,
                            public views::MaskedTargeterDelegate {
 public:
  explicit TabCloseButton(Tab* tab)
      : views::ImageButton(tab),
        tab_(tab) {
    SetEventTargeter(
        std::unique_ptr<views::ViewTargeter>(new views::ViewTargeter(this)));
  }

  ~TabCloseButton() override {}

  // views::View:
  View* GetTooltipHandlerForPoint(const gfx::Point& point) override {
    // Tab close button has no children, so tooltip handler should be the same
    // as the event handler.
    // In addition, a hit test has to be performed for the point (as
    // GetTooltipHandlerForPoint() is responsible for it).
    if (!HitTestPoint(point))
      return NULL;
    return GetEventHandlerForPoint(point);
  }

  bool OnMousePressed(const ui::MouseEvent& event) override {
    tab_->controller_->OnMouseEventInTab(this, event);

    bool handled = ImageButton::OnMousePressed(event);
    // Explicitly mark midle-mouse clicks as non-handled to ensure the tab
    // sees them.
    return !event.IsMiddleMouseButton() && handled;
  }

  void OnMouseMoved(const ui::MouseEvent& event) override {
    tab_->controller_->OnMouseEventInTab(this, event);
    CustomButton::OnMouseMoved(event);
  }

  void OnMouseReleased(const ui::MouseEvent& event) override {
    tab_->controller_->OnMouseEventInTab(this, event);
    CustomButton::OnMouseReleased(event);
  }

  void OnGestureEvent(ui::GestureEvent* event) override {
    // Consume all gesture events here so that the parent (Tab) does not
    // start consuming gestures.
    ImageButton::OnGestureEvent(event);
    event->SetHandled();
  }

  const char* GetClassName() const override { return kTabCloseButtonName; }

 private:
  // views::MaskedTargeterDelegate:
  View* TargetForRect(View* root, const gfx::Rect& rect) override {
    CHECK_EQ(root, this);

    if (!views::UsePointBasedTargeting(rect))
      return ViewTargeterDelegate::TargetForRect(root, rect);

    // Ignore the padding set on the button.
    gfx::Rect contents_bounds = GetContentsBounds();
    contents_bounds.set_x(GetMirroredXForRect(contents_bounds));

#if defined(USE_AURA)
    // Include the padding in hit-test for touch events.
    // TODO(pkasting): It seems like touch events would generate rects rather
    // than points and thus use the TargetForRect() call above.  If this is
    // reached, it may be from someone calling GetEventHandlerForPoint() while a
    // touch happens to be occurring.  In such a case, maybe we don't want this
    // code to run?  It's possible this block should be removed, or maybe this
    // whole function deleted.  Note that in these cases, we should probably
    // also remove the padding on the close button bounds (see Tab::Layout()),
    // as it will be pointless.
    if (aura::Env::GetInstance()->is_touch_down())
      contents_bounds = GetLocalBounds();
#endif

    return contents_bounds.Intersects(rect) ? this : parent();
  }

  // We need to define this so hit-testing won't include the border region.
  bool GetHitTestMask(gfx::Path* mask) const override {
    gfx::Rect button_bounds(GetContentsBounds());
    button_bounds.set_x(GetMirroredXForRect(button_bounds));
    mask->addRect(gfx::RectToSkRect(button_bounds));
    return true;
  }

  Tab* tab_;

  DISALLOW_COPY_AND_ASSIGN(TabCloseButton);
};

////////////////////////////////////////////////////////////////////////////////
// ThrobberView
//
// A Layer-backed view for updating a waiting or loading tab throbber.
class Tab::ThrobberView : public views::View {
 public:
  explicit ThrobberView(Tab* owner);

  // Resets the times tracking when the throbber changes state.
  void ResetStartTimes();

 private:
  // views::View:
  bool CanProcessEventsWithinSubtree() const override;
  void OnPaint(gfx::Canvas* canvas) override;

  Tab* owner_;  // Weak. Owns |this|.

  // The point in time when the tab icon was first painted in the waiting state.
  base::TimeTicks waiting_start_time_;

  // The point in time when the tab icon was first painted in the loading state.
  base::TimeTicks loading_start_time_;

  // Paint state for the throbber after the most recent waiting paint.
  gfx::ThrobberWaitingState waiting_state_;

  DISALLOW_COPY_AND_ASSIGN(ThrobberView);
};

Tab::ThrobberView::ThrobberView(Tab* owner) : owner_(owner) {}

void Tab::ThrobberView::ResetStartTimes() {
  waiting_start_time_ = base::TimeTicks();
  loading_start_time_ = base::TimeTicks();
  waiting_state_ = gfx::ThrobberWaitingState();
}

bool Tab::ThrobberView::CanProcessEventsWithinSubtree() const {
  return false;
}

void Tab::ThrobberView::OnPaint(gfx::Canvas* canvas) {
  const TabRendererData::NetworkState state = owner_->data().network_state;
  if (state == TabRendererData::NETWORK_STATE_NONE)
    return;

  const ui::ThemeProvider* tp = GetThemeProvider();
  const gfx::Rect bounds = GetLocalBounds();
  if (state == TabRendererData::NETWORK_STATE_WAITING) {
    if (waiting_start_time_ == base::TimeTicks())
      waiting_start_time_ = base::TimeTicks::Now();

    waiting_state_.elapsed_time = base::TimeTicks::Now() - waiting_start_time_;
    gfx::PaintThrobberWaiting(
        canvas, bounds,
        tp->GetColor(ThemeProperties::COLOR_TAB_THROBBER_WAITING),
        waiting_state_.elapsed_time);
  } else {
    if (loading_start_time_ == base::TimeTicks())
      loading_start_time_ = base::TimeTicks::Now();

    waiting_state_.color =
        tp->GetColor(ThemeProperties::COLOR_TAB_THROBBER_WAITING);
    gfx::PaintThrobberSpinningAfterWaiting(
        canvas, bounds,
        tp->GetColor(ThemeProperties::COLOR_TAB_THROBBER_SPINNING),
        base::TimeTicks::Now() - loading_start_time_, &waiting_state_);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Tab, public:

// static
const char Tab::kViewClassName[] = "Tab";

Tab::Tab(TabController* controller, gfx::AnimationContainer* container)
    : controller_(controller),
      closing_(false),
      dragging_(false),
      detached_(false),
      favicon_hiding_offset_(0),
      immersive_loading_step_(0),
      should_display_crashed_favicon_(false),
      pulse_animation_(new gfx::ThrobAnimation(this)),
      crash_icon_animation_(new FaviconCrashAnimation(this)),
      animation_container_(container),
      throbber_(nullptr),
      alert_indicator_button_(nullptr),
      close_button_(nullptr),
      title_(new views::Label()),
      tab_activated_with_last_tap_down_(false),
      hover_controller_(this),
      showing_icon_(false),
      showing_alert_indicator_(false),
      showing_close_button_(false),
      button_color_(SK_ColorTRANSPARENT) {
  DCHECK(controller);
  InitTabResources();

  // So we get don't get enter/exit on children and don't prematurely stop the
  // hover.
  set_notify_enter_exit_on_child(true);

  set_id(VIEW_ID_TAB);

  SetBorder(views::CreateEmptyBorder(GetLayoutInsets(TAB)));

  title_->SetHorizontalAlignment(gfx::ALIGN_TO_HEAD);
  title_->SetElideBehavior(gfx::FADE_TAIL);
  title_->SetHandlesTooltips(false);
  title_->SetAutoColorReadabilityEnabled(false);
  title_->SetText(CoreTabHelper::GetDefaultTitle());
  AddChildView(title_);

  SetEventTargeter(
      std::unique_ptr<views::ViewTargeter>(new views::ViewTargeter(this)));

  throbber_ = new ThrobberView(this);
  throbber_->SetVisible(false);
  AddChildView(throbber_);

  alert_indicator_button_ = new AlertIndicatorButton(this);
  AddChildView(alert_indicator_button_);

  close_button_ = new TabCloseButton(this);
  close_button_->SetAccessibleName(
      l10n_util::GetStringUTF16(IDS_ACCNAME_CLOSE));
  // The normal image is set by OnButtonColorMaybeChanged() because it depends
  // on the current theme and active state.  The hovered and pressed images
  // don't depend on the these, so we can set them here.
  const gfx::ImageSkia& hovered = gfx::CreateVectorIcon(
      gfx::VectorIconId::TAB_CLOSE_HOVERED_PRESSED,
      SkColorSetRGB(0xDB, 0x44, 0x37));
  const gfx::ImageSkia& pressed = gfx::CreateVectorIcon(
      gfx::VectorIconId::TAB_CLOSE_HOVERED_PRESSED,
      SkColorSetRGB(0xA8, 0x35, 0x2A));
  close_button_->SetImage(views::CustomButton::STATE_HOVERED, &hovered);
  close_button_->SetImage(views::CustomButton::STATE_PRESSED, &pressed);

  // Disable animation so that the red danger sign shows up immediately
  // to help avoid mis-clicks.
  close_button_->SetAnimationDuration(0);
  AddChildView(close_button_);

  set_context_menu_controller(this);

  const int kPulseDurationMs = 200;
  pulse_animation_->SetSlideDuration(kPulseDurationMs);
  pulse_animation_->SetContainer(animation_container_.get());

  hover_controller_.SetAnimationContainer(animation_container_.get());
}

Tab::~Tab() {
}

bool Tab::IsActive() const {
  return controller_->IsActiveTab(this);
}

void Tab::ActiveStateChanged() {
  // The pinned tab title changed indicator is only shown for inactive tabs.
  // When transitioning between active and inactive always reset the state
  // to enforce that.
  SetPinnedTabTitleChangedIndicatorVisible(false);
  OnButtonColorMaybeChanged();
  alert_indicator_button_->UpdateEnabledForMuteToggle();
  Layout();
}

void Tab::AlertStateChanged() {
  Layout();
}

bool Tab::IsSelected() const {
  return controller_->IsTabSelected(this);
}

void Tab::SetData(const TabRendererData& data) {
  DCHECK(GetWidget());

  if (data_.Equals(data))
    return;

  TabRendererData old(data_);
  UpdateLoadingAnimation(data.network_state);
  data_ = data;

  base::string16 title = data_.title;
  if (title.empty()) {
    title = data_.loading ?
        l10n_util::GetStringUTF16(IDS_TAB_LOADING_TITLE) :
        CoreTabHelper::GetDefaultTitle();
  } else {
    Browser::FormatTitleForDisplay(&title);
  }
  title_->SetText(title);

  if (!data_.IsCrashed()) {
    crash_icon_animation_->Stop();
    SetShouldDisplayCrashedFavicon(false);
    favicon_hiding_offset_ = 0;
  } else if (!should_display_crashed_favicon_ &&
             !crash_icon_animation_->is_animating()) {
    data_.alert_state = TabAlertState::NONE;
    crash_icon_animation_->Start();
  }

  if (data_.alert_state != old.alert_state)
    alert_indicator_button_->TransitionToAlertState(data_.alert_state);

  if (old.pinned != data_.pinned)
    showing_pinned_tab_title_changed_indicator_ = false;

  DataChanged(old);

  Layout();
  SchedulePaint();
}

void Tab::UpdateLoadingAnimation(TabRendererData::NetworkState state) {
  if (state == data_.network_state &&
      state == TabRendererData::NETWORK_STATE_NONE) {
    // If the network state is none and hasn't changed, do nothing. Otherwise we
    // need to advance the animation frame.
    return;
  }

  data_.network_state = state;
  AdvanceLoadingAnimation();
}

void Tab::StartPulse() {
  pulse_animation_->StartThrobbing(std::numeric_limits<int>::max());
}

void Tab::StopPulse() {
  pulse_animation_->Stop();
}

void Tab::SetPinnedTabTitleChangedIndicatorVisible(bool value) {
  if (value == showing_pinned_tab_title_changed_indicator_)
    return;

  DCHECK(!value || data().pinned);

  showing_pinned_tab_title_changed_indicator_ = value;
  SchedulePaint();
}

int Tab::GetWidthOfLargestSelectableRegion() const {
  // Assume the entire region to the left of the alert indicator and/or close
  // buttons is available for click-to-select.  If neither are visible, the
  // entire tab region is available.
  const int indicator_left =
      showing_alert_indicator_ ? alert_indicator_button_->x() : width();
  const int close_button_left = showing_close_button_ ?
      close_button_->x() : width();
  return std::min(indicator_left, close_button_left);
}

// static
gfx::Size Tab::GetMinimumInactiveSize() {
  return gfx::Size(GetLayoutInsets(TAB).width(), GetLayoutConstant(TAB_HEIGHT));
}

// static
gfx::Size Tab::GetMinimumActiveSize() {
  gfx::Size minimum_size = GetMinimumInactiveSize();
  minimum_size.Enlarge(gfx::kFaviconSize, 0);
  return minimum_size;
}

// static
gfx::Size Tab::GetStandardSize() {
  const int kNetTabWidth = 193;
  return gfx::Size(kNetTabWidth + GetLayoutConstant(TABSTRIP_TAB_OVERLAP),
                   GetMinimumInactiveSize().height());
}

// static
int Tab::GetTouchWidth() {
  return kTouchWidth;
}

// static
int Tab::GetPinnedWidth() {
  return GetMinimumInactiveSize().width() +
         GetLayoutConstant(TAB_PINNED_CONTENT_WIDTH);
}

// static
int Tab::GetImmersiveHeight() {
  return kImmersiveTabHeight;
}

// static
float Tab::GetInverseDiagonalSlope() {
  // This is computed from the border path as follows:
  // * The unscaled endcap width is enough for the whole stroke outer curve,
  //   i.e. the side diagonal plus the curves on both its ends.
  // * The bottom and top curve are each (2 * scale) px wide, so the diagonal is
  //   (unscaled endcap width - 2 - 2) * scale px wide.
  // * The bottom and top curve are each 1.5 px high.  Additionally, there is an
  //   extra 1 px below the bottom curve and (scale - 1) px above the top curve,
  //   so the diagonal is ((height - 1.5 - 1.5) * scale - 1 - (scale - 1)) px
  //   high.
  // Simplifying these gives the expression below.
  return (GetUnscaledEndcapWidth() - 4) /
      (GetMinimumInactiveSize().height() - 4);
}

////////////////////////////////////////////////////////////////////////////////
// Tab, AnimationDelegate overrides:

void Tab::AnimationProgressed(const gfx::Animation* animation) {
  // Ignore if the pulse animation is being performed on active tab because
  // it repaints the same image. See PaintTab().
  if ((animation != pulse_animation_.get()) || !IsActive())
    SchedulePaint();
}

void Tab::AnimationCanceled(const gfx::Animation* animation) {
  SchedulePaint();
}

void Tab::AnimationEnded(const gfx::Animation* animation) {
  SchedulePaint();
}

////////////////////////////////////////////////////////////////////////////////
// Tab, views::ButtonListener overrides:

void Tab::ButtonPressed(views::Button* sender, const ui::Event& event) {
  if (!alert_indicator_button_ || !alert_indicator_button_->visible())
    content::RecordAction(UserMetricsAction("CloseTab_NoAlertIndicator"));
  else if (alert_indicator_button_->enabled())
    content::RecordAction(UserMetricsAction("CloseTab_MuteToggleAvailable"));
  else if (data_.alert_state == TabAlertState::AUDIO_PLAYING)
    content::RecordAction(UserMetricsAction("CloseTab_AudioIndicator"));
  else
    content::RecordAction(UserMetricsAction("CloseTab_RecordingIndicator"));

  const CloseTabSource source =
      (event.type() == ui::ET_MOUSE_RELEASED &&
       !(event.flags() & ui::EF_FROM_TOUCH)) ? CLOSE_TAB_FROM_MOUSE
                                             : CLOSE_TAB_FROM_TOUCH;
  DCHECK_EQ(close_button_, sender);
  controller_->CloseTab(this, source);
  if (event.type() == ui::ET_GESTURE_TAP)
    TouchUMA::RecordGestureAction(TouchUMA::GESTURE_TABCLOSE_TAP);
}

////////////////////////////////////////////////////////////////////////////////
// Tab, views::ContextMenuController overrides:

void Tab::ShowContextMenuForView(views::View* source,
                                 const gfx::Point& point,
                                 ui::MenuSourceType source_type) {
  if (!closing())
    controller_->ShowContextMenuForTab(this, point, source_type);
}

////////////////////////////////////////////////////////////////////////////////
// Tab, views::MaskedTargeterDelegate overrides:

bool Tab::GetHitTestMask(gfx::Path* mask) const {
  // When the window is maximized we don't want to shave off the edges or top
  // shadow of the tab, such that the user can click anywhere along the top
  // edge of the screen to select a tab. Ditto for immersive fullscreen.
  const views::Widget* widget = GetWidget();
  *mask = GetBorderPath(
      GetWidget()->GetCompositor()->device_scale_factor(), true,
      widget && (widget->IsMaximized() || widget->IsFullscreen()), size());
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Tab, views::View overrides:

void Tab::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
  // If this hierarchy changed has resulted in us being part of a widget
  // hierarchy for the first time, we can now get at the theme provider, and
  // should recalculate the button color.
  if (details.is_add)
    OnButtonColorMaybeChanged();
}

void Tab::OnPaint(gfx::Canvas* canvas) {
  // Don't paint if we're narrower than we can render correctly. (This should
  // only happen during animations).
  if (width() < GetMinimumInactiveSize().width() && !data().pinned)
    return;

  gfx::Path clip;
  if (!controller_->ShouldPaintTab(
          this, base::Bind(&GetBorderPath, canvas->image_scale(), true, false),
          &clip))
    return;

  if (controller_->IsImmersiveStyle())
    PaintImmersiveTab(canvas);
  else
    PaintTab(canvas, clip);
}

void Tab::Layout() {
  const gfx::Rect lb = GetContentsBounds();
  showing_icon_ = ShouldShowIcon();
  // See comments in IconCapacity().
  const int extra_padding =
      (controller_->ShouldHideCloseButtonForInactiveTabs() ||
       (IconCapacity() < 3)) ? 0 : kExtraLeftPaddingToBalanceCloseButtonPadding;
  const int start = lb.x() + extra_padding;
  favicon_bounds_.SetRect(start, lb.y(), 0, 0);
  if (showing_icon_) {
    favicon_bounds_.set_size(gfx::Size(gfx::kFaviconSize, gfx::kFaviconSize));
    favicon_bounds_.set_y(lb.y() + (lb.height() - gfx::kFaviconSize + 1) / 2);
    MaybeAdjustLeftForPinnedTab(&favicon_bounds_);
  }
  throbber_->SetBoundsRect(favicon_bounds_);

  showing_close_button_ = ShouldShowCloseBox();
  if (showing_close_button_) {
    // If the ratio of the close button size to tab width exceeds the maximum.
    // The close button should be as large as possible so that there is a larger
    // hit-target for touch events. So the close button bounds extends to the
    // edges of the tab. However, the larger hit-target should be active only
    // for touch events, and the close-image should show up in the right place.
    // So a border is added to the button with necessary padding. The close
    // button (Tab::TabCloseButton) makes sure the padding is a hit-target only
    // for touch events.
    // TODO(pkasting): The padding should maybe be removed, see comments in
    // TabCloseButton::TargetForRect().
    close_button_->SetBorder(views::NullBorder());
    const gfx::Size close_button_size(close_button_->GetPreferredSize());
    const int top = lb.y() + (lb.height() - close_button_size.height() + 1) / 2;
    const int left = kAfterTitleSpacing;
    const int close_button_end = lb.right();
    close_button_->SetPosition(
        gfx::Point(close_button_end - close_button_size.width() - left, 0));
    const int bottom = height() - close_button_size.height() - top;
    const int right = width() - close_button_end;
    close_button_->SetBorder(
        views::CreateEmptyBorder(top, left, bottom, right));
    close_button_->SizeToPreferredSize();
  }
  close_button_->SetVisible(showing_close_button_);

  showing_alert_indicator_ = ShouldShowAlertIndicator();
  if (showing_alert_indicator_) {
    const gfx::Size image_size(alert_indicator_button_->GetPreferredSize());
    const int right = showing_close_button_ ?
        close_button_->x() + close_button_->GetInsets().left() : lb.right();
    gfx::Rect bounds(
        std::max(lb.x(), right - image_size.width()),
        lb.y() + (lb.height() - image_size.height() + 1) / 2,
        image_size.width(),
        image_size.height());
    MaybeAdjustLeftForPinnedTab(&bounds);
    alert_indicator_button_->SetBoundsRect(bounds);
  }
  alert_indicator_button_->SetVisible(showing_alert_indicator_);

  // Size the title to fill the remaining width and use all available height.
  const bool show_title = ShouldRenderAsNormalTab();
  if (show_title) {
    const int title_spacing = GetLayoutConstant(TAB_FAVICON_TITLE_SPACING);
    int title_left = showing_icon_ ?
        (favicon_bounds_.right() + title_spacing) : start;
    int title_width = lb.right() - title_left;
    if (showing_alert_indicator_) {
      title_width =
          alert_indicator_button_->x() - kAfterTitleSpacing - title_left;
    } else if (showing_close_button_) {
      // Allow the title to overlay the close button's empty border padding.
      title_width = close_button_->x() + close_button_->GetInsets().left() -
          kAfterTitleSpacing - title_left;
    }
    // The Label will automatically center the font's cap height within the
    // provided vertical space.
    title_->SetBoundsRect(
        gfx::Rect(title_left, lb.y(), std::max(title_width, 0), lb.height()));
  }
  title_->SetVisible(show_title);
}

void Tab::OnThemeChanged() {
  OnButtonColorMaybeChanged();
  favicon_ = gfx::ImageSkia();
}

const char* Tab::GetClassName() const {
  return kViewClassName;
}

bool Tab::GetTooltipText(const gfx::Point& p, base::string16* tooltip) const {
  // Note: Anything that affects the tooltip text should be accounted for when
  // calling TooltipTextChanged() from Tab::DataChanged().
  *tooltip = chrome::AssembleTabTooltipText(data_.title, data_.alert_state);
  return !tooltip->empty();
}

bool Tab::GetTooltipTextOrigin(const gfx::Point& p, gfx::Point* origin) const {
  origin->set_x(title_->x() + 10);
  origin->set_y(-4);
  return true;
}

bool Tab::OnMousePressed(const ui::MouseEvent& event) {
  controller_->OnMouseEventInTab(this, event);

  // Allow a right click from touch to drag, which corresponds to a long click.
  if (event.IsOnlyLeftMouseButton() ||
      (event.IsOnlyRightMouseButton() && event.flags() & ui::EF_FROM_TOUCH)) {
    ui::ListSelectionModel original_selection;
    original_selection.Copy(controller_->GetSelectionModel());
    // Changing the selection may cause our bounds to change. If that happens
    // the location of the event may no longer be valid. Create a copy of the
    // event in the parents coordinate, which won't change, and recreate an
    // event after changing so the coordinates are correct.
    ui::MouseEvent event_in_parent(event, static_cast<View*>(this), parent());
    if (controller_->SupportsMultipleSelection()) {
      if (event.IsShiftDown() && event.IsControlDown()) {
        controller_->AddSelectionFromAnchorTo(this);
      } else if (event.IsShiftDown()) {
        controller_->ExtendSelectionTo(this);
      } else if (event.IsControlDown()) {
        controller_->ToggleSelected(this);
        if (!IsSelected()) {
          // Don't allow dragging non-selected tabs.
          return false;
        }
      } else if (!IsSelected()) {
        controller_->SelectTab(this);
        content::RecordAction(UserMetricsAction("SwitchTab_Click"));
      }
    } else if (!IsSelected()) {
      controller_->SelectTab(this);
      content::RecordAction(UserMetricsAction("SwitchTab_Click"));
    }
    ui::MouseEvent cloned_event(event_in_parent, parent(),
                                static_cast<View*>(this));
    controller_->MaybeStartDrag(this, cloned_event, original_selection);
  }
  return true;
}

bool Tab::OnMouseDragged(const ui::MouseEvent& event) {
  controller_->ContinueDrag(this, event);
  return true;
}

void Tab::OnMouseReleased(const ui::MouseEvent& event) {
  controller_->OnMouseEventInTab(this, event);

  // Notify the drag helper that we're done with any potential drag operations.
  // Clean up the drag helper, which is re-created on the next mouse press.
  // In some cases, ending the drag will schedule the tab for destruction; if
  // so, bail immediately, since our members are already dead and we shouldn't
  // do anything else except drop the tab where it is.
  if (controller_->EndDrag(END_DRAG_COMPLETE))
    return;

  // Close tab on middle click, but only if the button is released over the tab
  // (normal windows behavior is to discard presses of a UI element where the
  // releases happen off the element).
  if (event.IsMiddleMouseButton()) {
    if (HitTestPoint(event.location())) {
      controller_->CloseTab(this, CLOSE_TAB_FROM_MOUSE);
    } else if (closing_) {
      // We're animating closed and a middle mouse button was pushed on us but
      // we don't contain the mouse anymore. We assume the user is clicking
      // quicker than the animation and we should close the tab that falls under
      // the mouse.
      Tab* closest_tab = controller_->GetTabAt(this, event.location());
      if (closest_tab)
        controller_->CloseTab(closest_tab, CLOSE_TAB_FROM_MOUSE);
    }
  } else if (event.IsOnlyLeftMouseButton() && !event.IsShiftDown() &&
             !event.IsControlDown()) {
    // If the tab was already selected mouse pressed doesn't change the
    // selection. Reset it now to handle the case where multiple tabs were
    // selected.
    controller_->SelectTab(this);

    if (alert_indicator_button_ && alert_indicator_button_->visible() &&
        alert_indicator_button_->bounds().Contains(event.location())) {
      content::RecordAction(UserMetricsAction("TabAlertIndicator_Clicked"));
    }
  }
}

void Tab::OnMouseCaptureLost() {
  controller_->EndDrag(END_DRAG_CAPTURE_LOST);
}

void Tab::OnMouseEntered(const ui::MouseEvent& event) {
  hover_controller_.Show(views::GlowHoverController::SUBTLE);
}

void Tab::OnMouseMoved(const ui::MouseEvent& event) {
  hover_controller_.SetLocation(event.location());
  controller_->OnMouseEventInTab(this, event);
}

void Tab::OnMouseExited(const ui::MouseEvent& event) {
  hover_controller_.Hide();
}

void Tab::OnGestureEvent(ui::GestureEvent* event) {
  switch (event->type()) {
    case ui::ET_GESTURE_TAP_DOWN: {
      // TAP_DOWN is only dispatched for the first touch point.
      DCHECK_EQ(1, event->details().touch_points());

      // See comment in OnMousePressed() as to why we copy the event.
      ui::GestureEvent event_in_parent(*event, static_cast<View*>(this),
                                       parent());
      ui::ListSelectionModel original_selection;
      original_selection.Copy(controller_->GetSelectionModel());
      tab_activated_with_last_tap_down_ = !IsActive();
      if (!IsSelected())
        controller_->SelectTab(this);
      gfx::Point loc(event->location());
      views::View::ConvertPointToScreen(this, &loc);
      ui::GestureEvent cloned_event(event_in_parent, parent(),
                                    static_cast<View*>(this));
      controller_->MaybeStartDrag(this, cloned_event, original_selection);
      break;
    }

    case ui::ET_GESTURE_END:
      controller_->EndDrag(END_DRAG_COMPLETE);
      break;

    case ui::ET_GESTURE_SCROLL_UPDATE:
      controller_->ContinueDrag(this, *event);
      break;

    default:
      break;
  }
  event->SetHandled();
}

void Tab::GetAccessibleNodeData(ui::AXNodeData* node_data) {
  node_data->role = ui::AX_ROLE_TAB;
  node_data->SetName(data_.title);
  node_data->AddStateFlag(ui::AX_STATE_MULTISELECTABLE);
  node_data->AddStateFlag(ui::AX_STATE_SELECTABLE);
  controller_->UpdateTabAccessibilityState(this, node_data);
  if (IsSelected())
    node_data->AddStateFlag(ui::AX_STATE_SELECTED);
}

////////////////////////////////////////////////////////////////////////////////
// Tab, private

void Tab::MaybeAdjustLeftForPinnedTab(gfx::Rect* bounds) const {
  if (ShouldRenderAsNormalTab())
    return;
  const int ideal_delta = width() - GetPinnedWidth();
  const int ideal_x = (GetPinnedWidth() - bounds->width()) / 2;
  bounds->set_x(
      bounds->x() + static_cast<int>(
          (1 - static_cast<float>(ideal_delta) /
              static_cast<float>(kPinnedTabExtraWidthToRenderAsNormal)) *
          (ideal_x - bounds->x())));
}

void Tab::DataChanged(const TabRendererData& old) {
  // We may overzealously reset the favicon cache here but this check eliminates
  // at least some unnecessary re-computations and fixes the behavior of
  // about:crash.
  if (!old.favicon.BackedBySameObjectAs(data().favicon))
    favicon_ = gfx::ImageSkia();

  if (data().alert_state != old.alert_state || data().title != old.title)
    TooltipTextChanged();

  if (data().blocked == old.blocked)
    return;

  if (data().blocked)
    StartPulse();
  else
    StopPulse();
}

void Tab::PaintTab(gfx::Canvas* canvas, const gfx::Path& clip) {
  const int kActiveTabFillId =
      GetThemeProvider()->HasCustomImage(IDR_THEME_TOOLBAR) ? IDR_THEME_TOOLBAR
                                                            : 0;
  const int y_offset = -GetLayoutInsets(TAB).top();
  if (IsActive()) {
    PaintTabBackgroundUsingFillId(canvas, canvas, true, kActiveTabFillId,
                                  y_offset);
  } else {
    PaintInactiveTabBackground(canvas, clip);

    const double throb_value = GetThrobValue();
    if (throb_value > 0) {
      canvas->SaveLayerAlpha(gfx::ToRoundedInt(throb_value * 0xff),
                             GetLocalBounds());
      PaintTabBackgroundUsingFillId(canvas, canvas, true, kActiveTabFillId,
                                    y_offset);
      canvas->Restore();
    }
  }

  if (showing_icon_)
    PaintIcon(canvas);
}

void Tab::PaintImmersiveTab(gfx::Canvas* canvas) {
  // Use transparency for the draw-attention animation.
  int alpha = 255;
  if (pulse_animation_->is_animating() && !data().pinned) {
    alpha = pulse_animation_->CurrentValueBetween(
        255, gfx::ToRoundedInt(255 * kImmersiveTabMinThrobOpacity));
  }

  // Draw a gray rectangle to represent the tab. This works for pinned tabs as
  // well as regular ones. The active tab has a brigher bar.
  SkColor color =
      IsActive() ? kImmersiveActiveTabColor : kImmersiveInactiveTabColor;
  gfx::Rect bar_rect = GetImmersiveBarRect();
  canvas->FillRect(bar_rect, SkColorSetA(color, alpha));

  // Paint network activity indicator.
  // TODO(jamescook): Replace this placeholder animation with a real one.
  // For now, let's go with a Cylon eye effect, but in blue.
  if (data().network_state != TabRendererData::NETWORK_STATE_NONE) {
    const SkColor kEyeColor = SkColorSetARGB(alpha, 71, 138, 217);
    int eye_width = bar_rect.width() / 3;
    int eye_offset = bar_rect.width() * immersive_loading_step_ /
        kImmersiveLoadingStepCount;
    if (eye_offset + eye_width < bar_rect.width()) {
      // Draw a single indicator strip because it fits inside |bar_rect|.
      gfx::Rect eye_rect(
          bar_rect.x() + eye_offset, 0, eye_width, kImmersiveBarHeight);
      canvas->FillRect(eye_rect, kEyeColor);
    } else {
      // Draw two indicators to simulate the eye "wrapping around" to the left
      // side. The first part fills the remainder of the bar.
      int right_eye_width = bar_rect.width() - eye_offset;
      gfx::Rect right_eye_rect(
          bar_rect.x() + eye_offset, 0, right_eye_width, kImmersiveBarHeight);
      canvas->FillRect(right_eye_rect, kEyeColor);
      // The second part parts the remaining |eye_width| on the left.
      int left_eye_width = eye_offset + eye_width - bar_rect.width();
      gfx::Rect left_eye_rect(
          bar_rect.x(), 0, left_eye_width, kImmersiveBarHeight);
      canvas->FillRect(left_eye_rect, kEyeColor);
    }
  }
}

void Tab::PaintInactiveTabBackground(gfx::Canvas* canvas,
                                     const gfx::Path& clip) {
  bool has_custom_image;
  int fill_id = controller_->GetBackgroundResourceId(&has_custom_image);
  const ui::ThemeProvider* tp = GetThemeProvider();

  // We only cache the image when it's the default image and we're not hovered,
  // to avoid caching a background image that isn't the same for all tabs.
  if (has_custom_image) {
    // If the theme is providing a custom background image, then its top edge
    // should be at the top of the tab. Otherwise, we assume that the background
    // image is a composited foreground + frame image.  Note that if the theme
    // is only providing a custom frame image, |has_custom_image| will be true,
    // but we should use the |background_offset_| here.
    const int y_offset =
        tp->HasCustomImage(fill_id) ? 0 : background_offset_.y();
    PaintTabBackgroundUsingFillId(canvas, canvas, false, fill_id, y_offset);
    return;
  }
  if (hover_controller_.ShouldDraw()) {
    PaintTabBackgroundUsingFillId(canvas, canvas, false, 0, 0);
    return;
  }

  // For efficiency, we don't use separate fill and stroke images unless we
  // really need to clip the stroke and not the fill (for stacked tabs).  This
  // saves memory and avoids an extra image draw at the cost of recalculating
  // the images when MaySetClip() toggles.
  const bool use_fill_and_stroke_images = controller_->MaySetClip();

  const ImageCacheEntryMetadata metadata(
      tp->GetColor(ThemeProperties::COLOR_BACKGROUND_TAB),
      controller_->GetToolbarTopSeparatorColor(), use_fill_and_stroke_images,
      canvas->image_scale(), size());
  auto it = std::find_if(
      g_image_cache->begin(), g_image_cache->end(),
      [&metadata](const ImageCacheEntry& e) { return e.metadata == metadata; });
  if (it == g_image_cache->end()) {
    gfx::Canvas tmp_canvas(size(), canvas->image_scale(), false);
    if (use_fill_and_stroke_images) {
      gfx::Canvas tmp_fill_canvas(size(), canvas->image_scale(), false);
      PaintTabBackgroundUsingFillId(&tmp_fill_canvas, &tmp_canvas, false, 0, 0);
      g_image_cache->emplace_front(
          metadata, gfx::ImageSkia(tmp_fill_canvas.ExtractImageRep()),
          gfx::ImageSkia(tmp_canvas.ExtractImageRep()));
    } else {
      PaintTabBackgroundUsingFillId(&tmp_canvas, &tmp_canvas, false, 0, 0);
      g_image_cache->emplace_front(
          metadata, gfx::ImageSkia(),
          gfx::ImageSkia(tmp_canvas.ExtractImageRep()));
    }
    if (g_image_cache->size() > kMaxImageCacheSize)
      g_image_cache->pop_back();
    it = g_image_cache->begin();
  }

  gfx::ScopedCanvas scoped_canvas(
      use_fill_and_stroke_images ? canvas : nullptr);
  if (use_fill_and_stroke_images) {
    canvas->DrawImageInt(it->fill_image, 0, 0);
    canvas->sk_canvas()->clipPath(clip, SkRegion::kDifference_Op, true);
  }
  canvas->DrawImageInt(it->stroke_image, 0, 0);
}

void Tab::PaintTabBackgroundUsingFillId(gfx::Canvas* fill_canvas,
                                        gfx::Canvas* stroke_canvas,
                                        bool is_active,
                                        int fill_id,
                                        int y_offset) {
  gfx::Path fill;
  SkPaint paint;
  paint.setAntiAlias(true);

  // Draw the fill.
  {
    gfx::ScopedCanvas scoped_canvas(fill_canvas);
    const float scale = fill_canvas->UndoDeviceScaleFactor();
    const ui::ThemeProvider* tp = GetThemeProvider();
    const SkColor toolbar_color = tp->GetColor(ThemeProperties::COLOR_TOOLBAR);

    fill = GetFillPath(scale, size());
    {
      gfx::ScopedCanvas clip_scoper(fill_canvas);
      fill_canvas->ClipPath(fill, true);
      if (fill_id) {
        gfx::ScopedCanvas scale_scoper(fill_canvas);
        fill_canvas->sk_canvas()->scale(scale, scale);
        fill_canvas->TileImageInt(*tp->GetImageSkiaNamed(fill_id),
                                  GetMirroredX() + background_offset_.x(),
                                  y_offset, 0, 0, width(), height());
      } else {
        paint.setColor(
            is_active ? toolbar_color
                      : tp->GetColor(ThemeProperties::COLOR_BACKGROUND_TAB));
        fill_canvas->DrawRect(
            gfx::ScaleToEnclosingRect(GetLocalBounds(), scale), paint);
      }

      if (!is_active && hover_controller_.ShouldDraw()) {
        SkPoint hover_location(
            gfx::PointToSkPoint(hover_controller_.location()));
        hover_location.scale(SkFloatToScalar(scale));
        const SkScalar kMinHoverRadius = 16;
        const SkScalar radius =
            std::max(SkFloatToScalar(width() / 4.f), kMinHoverRadius);
        DrawHighlight(fill_canvas, hover_location, radius * scale,
                      SkColorSetA(toolbar_color, hover_controller_.GetAlpha()));
      }
    }
  }

  // Draw the stroke.
  {
    gfx::ScopedCanvas scoped_canvas(stroke_canvas);
    const float scale = stroke_canvas->UndoDeviceScaleFactor();

    gfx::Path stroke = GetBorderPath(scale, false, false, size());
    Op(stroke, fill, kDifference_SkPathOp, &stroke);
    if (!is_active) {
      // Clip out the bottom line; this will be drawn for us by
      // TabStrip::PaintChildren().
      stroke_canvas->ClipRect(
          gfx::RectF(width() * scale, height() * scale - 1));
    }
    paint.setColor(controller_->GetToolbarTopSeparatorColor());
    stroke_canvas->DrawPath(stroke, paint);
  }
}

void Tab::PaintPinnedTabTitleChangedIndicatorAndIcon(
    gfx::Canvas* canvas,
    const gfx::Rect& favicon_draw_bounds) {
  // The pinned tab title changed indicator consists of two parts:
  // . a clear (totally transparent) part over the bottom right (or left in rtl)
  //   of the favicon. This is done by drawing the favicon to a canvas, then
  //   drawing the clear part on top of the favicon.
  // . a circle in the bottom right (or left in rtl) of the favicon.
  if (!favicon_.isNull()) {
    const float kIndicatorCropRadius = 4.5;
    gfx::Canvas icon_canvas(gfx::Size(gfx::kFaviconSize, gfx::kFaviconSize),
                            canvas->image_scale(), false);
    icon_canvas.DrawImageInt(favicon_, 0, 0);
    SkPaint clear_paint;
    clear_paint.setAntiAlias(true);
    clear_paint.setBlendMode(SkBlendMode::kClear);
    const int circle_x = base::i18n::IsRTL() ? 0 : gfx::kFaviconSize;
    icon_canvas.DrawCircle(gfx::PointF(circle_x, gfx::kFaviconSize),
                           kIndicatorCropRadius, clear_paint);
    canvas->DrawImageInt(gfx::ImageSkia(icon_canvas.ExtractImageRep()), 0, 0,
                         favicon_draw_bounds.width(),
                         favicon_draw_bounds.height(), favicon_draw_bounds.x(),
                         favicon_draw_bounds.y(), favicon_draw_bounds.width(),
                         favicon_draw_bounds.height(), false);
  }

  // Draws the actual pinned tab title changed indicator.
  const int kIndicatorRadius = 3;
  SkPaint indicator_paint;
  indicator_paint.setColor(GetNativeTheme()->GetSystemColor(
      ui::NativeTheme::kColorId_ProminentButtonColor));
  indicator_paint.setAntiAlias(true);
  const int indicator_x = GetMirroredXWithWidthInView(
      favicon_bounds_.right() - kIndicatorRadius, kIndicatorRadius * 2);
  const int indicator_y = favicon_bounds_.bottom() - kIndicatorRadius;
  canvas->DrawCircle(gfx::Point(indicator_x + kIndicatorRadius,
                                indicator_y + kIndicatorRadius),
                     kIndicatorRadius, indicator_paint);
}

void Tab::PaintIcon(gfx::Canvas* canvas) {
  gfx::Rect bounds = favicon_bounds_;
  bounds.set_x(GetMirroredXForRect(bounds));
  bounds.Offset(0, favicon_hiding_offset_);
  bounds.Intersect(GetContentsBounds());
  if (bounds.IsEmpty())
    return;

  // Throbber will do its own painting.
  if (data().network_state != TabRendererData::NETWORK_STATE_NONE)
    return;

  // Ensure that |favicon_| is created.
  if (favicon_.isNull()) {
    ui::ResourceBundle* rb = &ui::ResourceBundle::GetSharedInstance();
    favicon_ = should_display_crashed_favicon_
                   ? *rb->GetImageSkiaNamed(IDR_CRASH_SAD_FAVICON)
                   : data().favicon;
    // Themify the icon if it's a chrome:// page or if it's the sadtab favicon.
    // This ensures chrome:// pages are visible over the tab background. This is
    // similar to code in the bookmarks bar.
    if (!favicon_.isNull() &&
        (should_display_crashed_favicon_ ||
         favicon_.BackedBySameObjectAs(
             *rb->GetImageSkiaNamed(IDR_DEFAULT_FAVICON)) ||
         ShouldThemifyFaviconForUrl(data().url))) {
      favicon_ = gfx::ImageSkiaOperations::CreateHSLShiftedImage(
          favicon_, GetThemeProvider()->GetTint(ThemeProperties::TINT_BUTTONS));
    }
  }

  if (showing_pinned_tab_title_changed_indicator_ &&
      !should_display_crashed_favicon_) {
    PaintPinnedTabTitleChangedIndicatorAndIcon(canvas, bounds);
  } else if (!favicon_.isNull()) {
    canvas->DrawImageInt(favicon_, 0, 0, bounds.width(), bounds.height(),
                         bounds.x(), bounds.y(), bounds.width(),
                         bounds.height(), false);
  }
}

void Tab::AdvanceLoadingAnimation() {
  const TabRendererData::NetworkState state = data().network_state;
  if (controller_->IsImmersiveStyle()) {
    throbber_->SetVisible(false);
    if (state == TabRendererData::NETWORK_STATE_WAITING) {
      // Waiting steps backwards.
      immersive_loading_step_ =
          (immersive_loading_step_ - 1 + kImmersiveLoadingStepCount) %
          kImmersiveLoadingStepCount;
    } else if (state == TabRendererData::NETWORK_STATE_LOADING) {
      immersive_loading_step_ =
          (immersive_loading_step_ + 1) % kImmersiveLoadingStepCount;
    } else {
      immersive_loading_step_ = 0;
    }

    SchedulePaintInRect(GetImmersiveBarRect());
    return;
  }

  if (state == TabRendererData::NETWORK_STATE_NONE) {
    throbber_->ResetStartTimes();
    throbber_->SetVisible(false);
    ScheduleIconPaint();
    return;
  }

  // Since the throbber can animate for a long time, paint to a separate layer
  // when possible to reduce repaint overhead.
  const bool paint_to_layer = controller_->CanPaintThrobberToLayer();
  if (paint_to_layer != !!throbber_->layer()) {
    throbber_->SetPaintToLayer(paint_to_layer);
    if (paint_to_layer) {
      throbber_->layer()->SetFillsBoundsOpaquely(false);
      ScheduleIconPaint();  // Ensure the non-layered throbber goes away.
    }
  }
  if (!throbber_->visible()) {
    ScheduleIconPaint();  // Repaint the icon area to hide the favicon.
    throbber_->SetVisible(true);
  }
  throbber_->SchedulePaint();
}

int Tab::IconCapacity() const {
  const gfx::Size min_size(GetMinimumInactiveSize());
  if (height() < min_size.height())
    return 0;
  const int available_width = std::max(0, width() - min_size.width());
  // All icons are the same size as the favicon.
  const int icon_width = gfx::kFaviconSize;
  // We need enough space to display the icons flush against each other.
  const int visible_icons = available_width / icon_width;
  // When the close button will be visible on inactive tabs, we add additional
  // padding to the left of the favicon to balance the whitespace inside the
  // non-hovered close button image; otherwise, the tab contents look too close
  // to the left edge.  If the tab close button isn't visible on inactive tabs,
  // we let the tab contents take the full width of the tab, to maximize visible
  // content on tiny tabs.  We base the determination on the inactive tab close
  // button state so that when a tab is activated its contents don't suddenly
  // shift.
  if (visible_icons < 3)
    return visible_icons;
  const int padding = controller_->ShouldHideCloseButtonForInactiveTabs() ?
      0 : kExtraLeftPaddingToBalanceCloseButtonPadding;
  return (available_width - padding) / icon_width;
}

bool Tab::ShouldShowIcon() const {
  return chrome::ShouldTabShowFavicon(
      IconCapacity(), data().pinned, IsActive(), data().show_icon,
      alert_indicator_button_ ? alert_indicator_button_->showing_alert_state()
                              : data_.alert_state);
}

bool Tab::ShouldShowAlertIndicator() const {
  return chrome::ShouldTabShowAlertIndicator(
      IconCapacity(), data().pinned, IsActive(), data().show_icon,
      alert_indicator_button_ ? alert_indicator_button_->showing_alert_state()
                              : data_.alert_state);
}

bool Tab::ShouldShowCloseBox() const {
  if (!IsActive() && controller_->ShouldHideCloseButtonForInactiveTabs())
    return false;

  return chrome::ShouldTabShowCloseButton(
      IconCapacity(), data().pinned, IsActive());
}

bool Tab::ShouldRenderAsNormalTab() const {
  return !data().pinned ||
      (width() >= (GetPinnedWidth() + kPinnedTabExtraWidthToRenderAsNormal));
}

double Tab::GetThrobValue() {
  const bool is_selected = IsSelected();
  double val = is_selected ? kSelectedTabOpacity : 0;
  const double offset =
      is_selected ? (kSelectedTabThrobScale * kHoverOpacity) : kHoverOpacity;

  if (pulse_animation_->is_animating())
    val += pulse_animation_->GetCurrentValue() * offset;
  else if (hover_controller_.ShouldDraw())
    val += hover_controller_.GetAnimationValue() * offset;
  return val;
}

void Tab::SetShouldDisplayCrashedFavicon(bool value) {
  if (value == should_display_crashed_favicon_)
    return;

  should_display_crashed_favicon_ = value;
  favicon_ = gfx::ImageSkia();
}

void Tab::SetFaviconHidingOffset(int offset) {
  favicon_hiding_offset_ = offset;
  ScheduleIconPaint();
}

void Tab::OnButtonColorMaybeChanged() {
  // The theme provider may be null if we're not currently in a widget
  // hierarchy.
  const ui::ThemeProvider* theme_provider = GetThemeProvider();
  if (!theme_provider)
    return;

  const SkColor title_color = theme_provider->GetColor(IsActive() ?
      ThemeProperties::COLOR_TAB_TEXT :
      ThemeProperties::COLOR_BACKGROUND_TAB_TEXT);
  const SkColor new_button_color = SkColorSetA(title_color, 0xA0);
  if (button_color_ != new_button_color) {
    button_color_ = new_button_color;
    title_->SetEnabledColor(title_color);
    alert_indicator_button_->OnParentTabButtonColorChanged();
    const gfx::ImageSkia& close_button_normal_image = gfx::CreateVectorIcon(
        gfx::VectorIconId::TAB_CLOSE_NORMAL, button_color_);
    close_button_->SetImage(views::CustomButton::STATE_NORMAL,
                            &close_button_normal_image);
  }
}

void Tab::ScheduleIconPaint() {
  gfx::Rect bounds = favicon_bounds_;
  if (bounds.IsEmpty())
    return;

  // Extends the area to the bottom when the crash animation is in progress.
  if (crash_icon_animation_->is_animating())
    bounds.set_height(height() - bounds.y());
  bounds.set_x(GetMirroredXForRect(bounds));
  SchedulePaintInRect(bounds);
}

gfx::Rect Tab::GetImmersiveBarRect() const {
  // The main bar is as wide as the normal tab's horizontal top line.
  gfx::Rect contents = GetContentsBounds();
  contents.set_y(0);
  contents.set_height(kImmersiveBarHeight);
  return contents;
}
