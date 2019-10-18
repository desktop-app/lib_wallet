// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_check.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {

Check::Check(
	Fn<std::vector<QString>(QString)> wordsByPrefix,
	const std::vector<int> &indices)
: Step(Type::Default) {
	setTitle(ph::lng_wallet_check_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_check_description(Ui::Text::RichLangValue));
	initControls(std::move(wordsByPrefix), indices);
}

std::vector<QString> Check::words() const {
	return _words();
}

rpl::producer<> Check::submitRequests() const {
	return _submitRequests.events();
}

void Check::setFocus() {
	_setFocus();
}

bool Check::checkAll() {
	return _checkAll();
}

bool Check::allowEscapeBack() const {
	return false;
}

void Check::initControls(
		Fn<std::vector<QString>(QString)> wordsByPrefix,
		const std::vector<int> &indices) {
	showLottie(
		"test",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	stopLottieOnLoop();

	_words = [=] {
		return indices | ranges::view::transform([](int index) {
			return QString();
		}) | ranges::to_vector;
	};
	_checkAll = [] {
		return true;
	};
	_setFocus = [] {

	};
}

void Check::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
