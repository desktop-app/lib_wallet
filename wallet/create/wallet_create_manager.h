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
#include "base/timer.h"

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
	Manager(not_null<QWidget*> parent);
	Manager(const Manager &other) = delete;
	Manager &operator=(const Manager &other) = delete;
	~Manager();

	[[nodiscard]] not_null<Ui::RpWidget*> content() const;

	void setGeometry(QRect geometry);

	enum class Action {
		NewKey,
		CreateKey,
		ImportKey,
		ShowAccount,
		ShowCheckTooSoon,
		ShowCheckIncorrect,
	};

	[[nodiscard]] rpl::producer<Action> actionRequests() const;
	[[nodiscard]] rpl::producer<std::vector<QString>> importRequests() const;
	[[nodiscard]] rpl::producer<QByteArray> passcodeChosen() const;
	[[nodiscard]] QByteArray publicKey() const;

	void next();
	void back();
	void backByEscape();
	void setFocus();

	void showIntro();
	void showImport();
	void showCreated(std::vector<QString> &&words);
	void showWords(Direction direction);
	void showCheck();
	void showPasscode();
	void showReady(const QByteArray &publicKey);

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void showStep(
		std::unique_ptr<Step> step,
		Direction direction,
		FnMut<void()> next = nullptr,
		FnMut<void()> back = nullptr);
	[[nodiscard]] std::vector<QString> wordsByPrefix(
		const QString &word) const;
	void initButtons();
	void acceptWordsDelayByModifiers(Qt::KeyboardModifiers modifiers);

	const std::unique_ptr<Ui::RpWidget> _content;
	const base::unique_qptr<Ui::FadeWrap<Ui::IconButton>> _backButton;
	const base::flat_set<QString> _validWords;
	const Fn<std::vector<QString>(QString)> _wordsByPrefix;

	std::unique_ptr<Step> _step;
	std::vector<QString> _words;
	base::Timer _waitForWords;
	bool _wordsShouldBeReady = false;
	QByteArray _publicKey;

	FnMut<void()> _next;
	FnMut<void()> _back;

	rpl::event_stream<Action> _actionRequests;
	rpl::event_stream<std::vector<QString>> _importRequests;
	rpl::event_stream<QByteArray> _passcodeChosen;

};

} // namespace Wallet::Create
