// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_step.h"

#include "ui/rp_widget.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/effects/slide_animation.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

namespace Wallet::Create {
namespace {

QImage AddImageMargins(const QImage &source, QMargins margins) {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto was = source.size() / pixelRatio;
	const auto size = QRect({}, was).marginsAdded(margins).size();
	auto large = QImage(
		size * pixelRatio,
		QImage::Format_ARGB32_Premultiplied);
	large.setDevicePixelRatio(pixelRatio);
	large.fill(Qt::transparent);
	{
		auto p = QPainter(&large);
		p.drawImage(
			QRect({ margins.left(), margins.top() }, was),
			source);
	}
	return large;
}

} // namespace

struct Step::SlideAnimationData {
	Type type = Type();
	std::unique_ptr<Ui::LottieAnimation> lottie;
	int lottieTop = 0;
	int lottieSize = 0;
	QImage content;
	int contentTop = 0;
};

Step::SlideAnimation::~SlideAnimation() = default;

Step::Step(Type type)
: _type(type)
, _widget(std::make_unique<Ui::RpWidget>())
, _scroll(resolveScrollArea())
, _inner(resolveInner()) {
	initGeometry();

	_widget->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		paintContent(clip);
	}, _widget->lifetime());
}

Step::~Step() = default;

Ui::ScrollArea *Step::resolveScrollArea() {
	return (_type == Type::Scroll)
		? Ui::CreateChild<Ui::ScrollArea>(_widget.get(), st::walletScrollArea)
		: nullptr;
}

not_null<Ui::RpWidget*> Step::resolveInner() {
	return _scroll
		? _scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll)).data()
		: Ui::CreateChild<Ui::RpWidget>(_widget.get());
}

void Step::initGeometry() {
	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		if (_scroll) {
			_scroll->setGeometry({ QPoint(), size });
		}
		_inner->setGeometry(
			0,
			0,
			size.width(),
			std::max(desiredHeight(), size.height()));
	}, _inner->lifetime());
}

void Step::paintContent(QRect clip) {
	if (_slideAnimation.slide) {
		if (!_slideAnimation.slide->animating()) {
			showFinished();
			return;
		}
	}

	if (_slideAnimation.slide) {
		auto p = QPainter(_widget.get());
		paintSlideAnimation(p, clip);
	}
}

void Step::paintSlideAnimation(QPainter &p, QRect clip) {
	const auto left = (_widget->width() - _slideAnimation.slideWidth) / 2;
	const auto top = _slideAnimation.slideTop;
	_slideAnimation.slide->paintFrame(p, left, top, _widget->width());
}

void Step::showFinished() {
	if (_slideAnimation.lottieNow) {
		_lottie = std::move(_slideAnimation.lottieNow);
		_lottie->attach(inner());
		_lottie->setOpacity(1.);
		_lottie->setGeometry(lottieGeometry(
			contentTop() + _lottieTop,
			_lottieSize));
	}
	_slideAnimation = SlideAnimation();
	if (_scroll) {
		_scroll->show();
	}
	inner()->show();
	showFinishedHook();
	setFocus();
}

not_null<Ui::RpWidget*> Step::widget() const {
	return _widget.get();
}

rpl::producer<> Step::nextRequests() const {
	return _nextButton
		? _nextButton->clicks() | rpl::map([] { return rpl::empty_value(); })
		: rpl::producer<>();
}

int Step::desiredHeight() const {
	return st::walletStepHeight;
}

bool Step::allowEscapeBack() const {
	return true;
}

Step::SlideAnimationData Step::prepareSlideAnimationData() {
	Expects(_title != nullptr);

	const auto scrollTop = (_scroll ? _scroll->scrollTop() : 0);

	auto result = SlideAnimationData();
	result.type = _type;
	if (_lottie) {
		_lottie->detach();
		result.lottie = std::move(_lottie);
		result.lottieTop = contentTop() + _lottieTop - scrollTop;
		result.lottieSize = _lottieSize;
	}
	result.content = prepareSlideAnimationContent();
	result.contentTop = slideAnimationContentTop() - scrollTop;
	return result;
}

QImage Step::prepareSlideAnimationContent() const {
	Expects(_title != nullptr);

	const auto contentTop = slideAnimationContentTop();
	const auto contentWidth = std::max(
		_description->naturalWidth(),
		st::walletWindowSize.width());
	const auto contentRect = QRect(
		(inner()->width() - contentWidth) / 2,
		contentTop,
		contentWidth,
		animationContentBottom() - contentTop);
	return grabForAnimation(contentRect);
}

not_null<Ui::RpWidget*> Step::inner() const {
	return _inner;
}

int Step::contentTop() const {
	const auto desired = desiredHeight();
	return (std::max(desired, inner()->height()) - desired) / 2;
}

int Step::slideAnimationContentTop() const {
	Expects(_title != nullptr);

	return std::max(
		_title->y(),
		_scroll ? _scroll->scrollTop() : 0);
}

int Step::animationContentBottom() const {
	const auto bottom = contentTop() + desiredHeight();
	return _scroll
		? std::min(bottom, _scroll->scrollTop() + _scroll->height())
		: bottom;
}

QImage Step::grabForAnimation(QRect rect) const {
	return Ui::GrabWidgetToImage(inner(), rect);
}

void Step::showFinishedHook() {
}

void Step::setTitle(rpl::producer<TextWithEntities> text, int top) {
	_title.emplace(
		inner(),
		std::move(text),
		st::walletStepTitle);

	if (!top) {
		top = (_type == Type::Scroll)
			? st::walletStepScrollTitleTop
			: st::walletStepTitleTop;
	}
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_title->resizeToWidth(size.width());
		_title->move(0, contentTop() + top);
	}, _title->lifetime());
}

void Step::setDescription(rpl::producer<TextWithEntities> text, int top) {
	Expects(_title != nullptr);

	_description.emplace(
		inner(),
		std::move(text),
		st::walletStepDescription);

	if (!top) {
		top = (_type == Type::Scroll)
			? st::walletStepScrollDescriptionTop
			: st::walletStepDescriptionTop;
	}
	_title->geometryValue(
	) | rpl::start_with_next([=](QRect geometry) {
		_description->resizeToWidth(geometry.width());
		_description->move(0, geometry.y() + top);
	}, _description->lifetime());
}

void Step::showAnimated(not_null<Step*> previous, Direction direction) {
	showAnimatedSlide(previous, direction);
}

void Step::showFast() {
	showFinished();
}

void Step::showNextButton(rpl::producer<QString> text) {
	_nextButton.emplace(inner(), std::move(text), st::walletNextButton);
	_nextButton->setTextTransform(
		Ui::RoundButton::TextTransform::NoTransform);
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto skip = (_type == Type::Scroll)
			? st::walletWordsNextBottomSkip
			: (st::walletStepHeight - st::walletStepNextTop);
		_nextButton->move(
			(size.width() - _nextButton->width()) / 2,
			contentTop() + desiredHeight() - skip);
	}, _nextButton->lifetime());
}

void Step::showBelowNextButton(object_ptr<Ui::RpWidget> widget) {
	Expects(_nextButton != nullptr);

	_belowNextButton.reset(widget.release());
	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_belowNextButton->move(
			(size.width() - _belowNextButton->width()) / 2,
			_nextButton->y() + _nextButton->height());
	}, _belowNextButton->lifetime());
}

void Step::adjustSlideSnapshots(
		SlideAnimationData &was,
		SlideAnimationData &now) {
	const auto pixelRatio = style::DevicePixelRatio();
	const auto wasSize = was.content.size() / pixelRatio;
	const auto nowSize = now.content.size() / pixelRatio;
	const auto wasBottom = was.contentTop + wasSize.height();
	const auto nowBottom = now.contentTop + nowSize.height();
	const auto widthDelta = nowSize.width() - wasSize.width();
	auto wasMargins = QMargins();
	auto nowMargins = QMargins();
	wasMargins.setTop(std::max(was.contentTop - now.contentTop, 0));
	nowMargins.setTop(std::max(now.contentTop - was.contentTop, 0));
	wasMargins.setLeft(std::max(widthDelta / 2, 0));
	nowMargins.setLeft(-std::min(widthDelta / 2, 0));
	wasMargins.setRight(std::max(widthDelta - (widthDelta / 2), 0));
	nowMargins.setRight(-std::min(widthDelta - (widthDelta / 2), 0));
	wasMargins.setBottom(std::max(nowBottom - wasBottom, 0));
	nowMargins.setBottom(std::max(wasBottom - nowBottom, 0));
	was.content = AddImageMargins(was.content, wasMargins);
	now.content = AddImageMargins(now.content, nowMargins);
	was.contentTop = now.contentTop = std::min(
		was.contentTop,
		now.contentTop);
}

void Step::showAnimatedSlide(not_null<Step*> previous, Direction direction) {
	const auto pixelRatio = style::DevicePixelRatio();
	auto was = previous->prepareSlideAnimationData();
	auto now = prepareSlideAnimationData();
	_slideAnimation.slide = std::make_unique<Ui::SlideAnimation>();
	_slideAnimation.direction = direction;

	adjustSlideSnapshots(was, now);
	Assert(now.contentTop == was.contentTop);
	Assert(now.content.size() == was.content.size());

	_slideAnimation.slideTop = was.contentTop;
	_slideAnimation.slideWidth = was.content.width() / pixelRatio;
	_slideAnimation.lottieWas = std::move(was.lottie);
	_slideAnimation.lottieWasTop = was.lottieTop;
	_slideAnimation.lottieWasSize = was.lottieSize;
	_slideAnimation.lottieNow = std::move(now.lottie);
	_slideAnimation.lottieNowTop = now.lottieTop;
	_slideAnimation.lottieNowSize = now.lottieSize;
	_slideAnimation.slide->setSnapshots(
		Ui::PixmapFromImage(std::move(was.content)),
		Ui::PixmapFromImage(std::move(now.content)));

	if (_slideAnimation.lottieWas) {
		_slideAnimation.lottieWas->attach(_widget.get());
	}
	if (_slideAnimation.lottieNow) {
		_slideAnimation.lottieNow->attach(_widget.get());
	}

	inner()->hide();
	if (_scroll) {
		_scroll->hide();
	}
	_slideAnimation.slide->start(
		(direction == Direction::Backward),
		[=] { slideAnimationCallback(); },
		st::slideDuration);
	slideAnimationCallback();
}

void Step::slideAnimationCallback() {
	const auto state = _slideAnimation.slide->state();
	const auto forward = (_slideAnimation.direction == Direction::Forward);
	if (_slideAnimation.lottieWas) {
		const auto shown = 1. - (forward
			? state.leftProgress
			: state.rightProgress);
		const auto scale = forward ? state.leftAlpha : state.rightAlpha;
		const auto fullHeight = _slideAnimation.lottieWasSize;
		const auto height = scale * fullHeight;
		const auto delta = (1. - shown) * _slideAnimation.slideWidth;
		_slideAnimation.lottieWas->setOpacity(scale);
		_slideAnimation.lottieWas->setGeometry(lottieGeometry(
			_slideAnimation.lottieWasTop + (1. - scale) * fullHeight / 2.,
			height).translated(forward ? -delta : delta, 0));
	}
	if (_slideAnimation.lottieNow) {
		const auto shown = forward
			? state.rightProgress
			: state.leftProgress;
		const auto scale = forward ? state.rightAlpha : state.leftAlpha;
		const auto fullHeight = _slideAnimation.lottieNowSize;
		const auto height = scale * fullHeight;
		const auto delta = (1. - shown) * _slideAnimation.slideWidth;
		_slideAnimation.lottieNow->setOpacity(scale);
		_slideAnimation.lottieNow->setGeometry(lottieGeometry(
			_slideAnimation.lottieNowTop + (1. - scale) * fullHeight / 2.,
			height).translated(forward ? delta : -delta, 0));
	}
	_widget->update();
}

void Step::setFocus() {
	inner()->setFocus();
}

rpl::lifetime &Step::lifetime() {
	return widget()->lifetime();
}

void Step::ensureVisible(int top, int height) {
	Expects(_type == Type::Scroll);
	Expects(_scroll != nullptr);

	_scroll->scrollToY(top, top + height);
}

void Step::showLottie(const QString &name, int top, int size) {
	_lottie = std::make_unique<Ui::LottieAnimation>(
		inner(),
		Ui::LottieFromResource(name));
	_lottieTop = top;
	_lottieSize = size;

	inner()->sizeValue(
	) | rpl::filter([=] {
		return (_lottie->parent() == inner());
	}) | rpl::start_with_next([=](QSize size) {
		_lottie->setGeometry(lottieGeometry(
			contentTop() + _lottieTop,
			_lottieSize));
	}, inner()->lifetime());
}

void Step::startLottie() {
	Expects(_lottie != nullptr);

	_lottie->start();
}

void Step::stopLottieOnLoop(int loop) {
	Expects(_lottie != nullptr);

	_lottie->stopOnLoop(loop);
}

QRect Step::lottieGeometry(int top, int size) const {
	return {
		(inner()->width() - size) / 2,
		top,
		size,
		size
	};
}

} // namespace Wallet::Create
