// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_ready.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/text/text_utilities.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {

Ready::Ready() : Step(Type::Default) {
	setTitle(ph::lng_wallet_ready_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_ready_description(Ui::Text::RichLangValue));
	initControls();
}

int Ready::desiredHeight() const {
	return st::walletChecksHeight;
}

void Ready::initControls() {
	showLottie(
		"done",
		st::walletStepReadyLottiePosition,
		st::walletStepReadyLottieSize);
	stopLottieOnLoop();

	showNextButton(ph::lng_wallet_ready_show_account());
}

void Ready::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
