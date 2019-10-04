// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Wallet {

class Intro final {
public:
	enum class Mode {
		Standalone
	};
	enum class Action {
		ShowTerms,
		ImportWords,
		CreateWallet,
	};
	Intro(not_null<QWidget*> parent, Mode mode);

	void setGeometry(QRect geometry);

	[[nodiscard]] rpl::producer<Action> actionRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls();

	const std::unique_ptr<Ui::RpWidget> _widget;
	//const Mode _mode = Mode::Standalone;

	rpl::event_stream<Action> _actionRequests;

};

} // namespace Wallet
