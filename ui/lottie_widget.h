// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Lottie {
class SinglePlayer;
struct Information;
} // namespace Lottie

namespace Ui {

class RpWidget;

class LottieAnimation final {
public:
	LottieAnimation(not_null<QWidget*> parent, const QByteArray &content);
	~LottieAnimation();

	void setGeometry(QRect geometry);
	void setOpacity(float64 opacity);

	void detach();
	void attach(not_null<QWidget*> parent);
	[[nodiscard]] not_null<QWidget*> parent() const;

	void start();
	void stopOnFrame(int frame);
	void stopOnLoop(int loop);

private:
	void paintFrame();

	const std::unique_ptr<RpWidget> _widget;
	const std::unique_ptr<Lottie::SinglePlayer> _lottie;

	float64 _opacity = 1.;
	int _stopOnFrame = 0;
	int _stopOnLoop = 0;
	int _loop = 0;
	int _framesInLoop = 0;
	bool _startPlaying = false;

};

[[nodiscard]] QByteArray LottieFromResource(const QString &name);

} // namespace Ui
