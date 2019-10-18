// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_passcode.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {

Passcode::Passcode() : Step(Type::Default) {
	setTitle(ph::lng_wallet_set_passcode_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_set_passcode_description(Ui::Text::RichLangValue));
	initControls();
}

void Passcode::initControls() {
	showLottie(
		"lock",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	stopLottieOnLoop();
}

void Passcode::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
