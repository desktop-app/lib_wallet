// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "wallet/create/wallet_create_step.h"
#include "ui/effects/animations.h"
#include "base/unique_qptr.h"

namespace Ui {
class RpWidget;
class RoundButton;
class LinkButton;
class IconButton;
template <typename Widget>
class FadeWrap;
} // namespace Ui

namespace Wallet::Create {

class Manager final {
public:
	Manager(
		not_null<QWidget*> parent,
		Fn<std::vector<QString>(QString)> wordsByPrefix);
	Manager(const Manager &other) = delete;
	Manager &operator=(const Manager &other) = delete;
	~Manager();

	[[nodiscard]] not_null<Ui::RpWidget*> content() const;

	void setGeometry(QRect geometry);

	enum class Action {
		CreateKey,
		ShowAccount,
		ShowCheckTooSoon,
		ShowCheckIncorrect,
	};

	[[nodiscard]] rpl::producer<Action> actionRequests() const;

	void next();
	void back();
	void backByEscape();

	void showIntro();
	void showCreated(std::vector<QString> &&words);
	void showWords(Direction direction);
	void showCheck();
	void showPasscode();
	void showReady();

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void showStep(
		std::unique_ptr<Step> step,
		Direction direction,
		FnMut<void()> next = nullptr,
		FnMut<void()> back = nullptr);
	void initButtons();

	const std::unique_ptr<Ui::RpWidget> _content;
	const base::unique_qptr<Ui::FadeWrap<Ui::IconButton>> _backButton;

	const Fn<std::vector<QString>(QString)> _wordsByPrefix;

	std::unique_ptr<Step> _step;
	std::vector<QString> _words;

	FnMut<void()> _next;
	FnMut<void()> _back;

	rpl::event_stream<Action> _actionRequests;

};

} // namespace Wallet::Create
