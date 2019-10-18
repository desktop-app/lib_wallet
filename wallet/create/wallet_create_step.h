// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "base/unique_qptr.h"
#include "ui/effects/animations.h"
#include "ui/widgets/labels.h"

struct TextWithEntities;

namespace Ui {
class LottieAnimation;
class SlideAnimation;
class CrossFadeAnimation;
class RpWidget;
class FlatLabel;
class ScrollArea;
class RoundButton;
template <typename Widget>
class FadeWrap;
} // namespace Ui

namespace Wallet::Create {

enum class Direction {
	Forward,
	Backward,
};

class Step {
public:
	enum class Type {
		Scroll,
		Default
	};

	Step(Type type);
	Step(const Step &other) = delete;
	Step &operator=(const Step &other) = delete;
	virtual ~Step() = 0;

	[[nodiscard]] virtual int desiredHeight() const;
	[[nodiscard]] virtual bool allowEscapeBack() const;
	[[nodiscard]] not_null<Ui::RpWidget*> widget() const;
	[[nodiscard]] rpl::producer<> nextRequests() const;

	void showAnimated(not_null<Step*> previous, Direction direction);
	void showFast();
	virtual void setFocus();

	[[nodiscard]] rpl::lifetime &lifetime();

protected:
	[[nodiscard]] not_null<Ui::RpWidget*> inner() const;
	[[nodiscard]] int contentTop() const;

	void setTitle(rpl::producer<TextWithEntities> text, int top = 0);
	void setDescription(rpl::producer<TextWithEntities> text, int top = 0);
	void ensureVisible(int top, int height);

	void showLottie(const QString &name, int top, int size);
	void startLottie();
	void stopLottieOnLoop(int loop = 1);

	void showNextButton(rpl::producer<QString> text);
	void showBelowNextButton(object_ptr<Ui::RpWidget> widget);

	[[nodiscard]] virtual QImage grabForAnimation(QRect rect) const;
	virtual void showFinishedHook();

private:
	struct SlideAnimationData;
	struct SlideAnimation {
		SlideAnimation() = default;
		SlideAnimation(SlideAnimation&&) = default;
		SlideAnimation &operator=(SlideAnimation&&) = default;
		~SlideAnimation();

		std::unique_ptr<Ui::LottieAnimation> lottieWas;
		std::unique_ptr<Ui::LottieAnimation> lottieNow;

		std::unique_ptr<Ui::SlideAnimation> slide;
		int slideTop = 0;
		int slideWidth = 0;
		int lottieWasTop = 0;
		int lottieWasSize = 0;
		int lottieNowTop = 0;
		int lottieNowSize = 0;
	};

	[[nodiscard]] Ui::ScrollArea *resolveScrollArea();
	[[nodiscard]] not_null<Ui::RpWidget*> resolveInner();
	[[nodiscard]] auto resolveNextButton()
		->std::unique_ptr<Ui::FadeWrap<Ui::RoundButton>>;
	void initGeometry();

	void showAnimatedSlide(not_null<Step*> previous, Direction direction);
	void paintContent(QRect clip);
	void showFinished();

	[[nodiscard]] int slideAnimationContentTop() const;
	[[nodiscard]] int animationContentBottom() const;

	[[nodiscard]] SlideAnimationData prepareSlideAnimationData();
	[[nodiscard]] QImage prepareSlideAnimationContent() const;
	void adjustSlideSnapshots(
		SlideAnimationData &was,
		SlideAnimationData &now);
	void slideAnimationCallback();
	void paintSlideAnimation(QPainter &p, QRect clip);

	[[nodiscard]] QRect lottieGeometry(int top, int size) const;

	const Type _type = Type();
	const std::unique_ptr<Ui::RpWidget> _widget;
	Ui::ScrollArea * const _scroll = nullptr;
	const not_null<Ui::RpWidget*> _inner;

	std::unique_ptr<Ui::LottieAnimation> _lottie;
	int _lottieTop = 0;
	int _lottieSize = 0;
	base::unique_qptr<Ui::FlatLabel> _title;
	base::unique_qptr<Ui::FlatLabel> _description;
	base::unique_qptr<Ui::RoundButton> _nextButton;
	base::unique_qptr<Ui::RpWidget> _belowNextButton;

	SlideAnimation _slideAnimation;

};

} // namespace Wallet::Create
