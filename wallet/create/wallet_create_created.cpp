// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_created.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {

Created::Created() : Step(Type::Default) {
	setTitle(ph::lng_wallet_created_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_created_description(Ui::Text::RichLangValue));
	initControls();
}

void Created::initControls() {
	showLottie(
		"created",
		st::walletStepCreatedLottiePosition,
		st::walletStepCreatedLottieSize);
	stopLottieOnLoop();
	showNextButton(ph::lng_wallet_continue());
}

void Created::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
