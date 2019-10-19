// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_passcode.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/input_fields.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "base/platform/base_platform_layout_switch.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {

Passcode::Passcode() : Step(Type::Default) {
	setTitle(ph::lng_wallet_set_passcode_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_set_passcode_description(Ui::Text::RichLangValue));
	initControls();
}

QByteArray Passcode::passcode() const {
	return _passcode();
}

void Passcode::setFocus() {
	_setFocus();
}

void Passcode::initControls() {
	showLottie(
		"lock",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	stopLottieOnLoop();

	const auto enter = Ui::CreateChild<Ui::PasswordInput>(
		inner().get(),
		st::walletPasscodeInput,
		ph::lng_wallet_set_passcode_enter());
	const auto repeat = Ui::CreateChild<Ui::PasswordInput>(
		inner().get(),
		st::walletPasscodeInput,
		ph::lng_wallet_set_passcode_repeat());

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		enter->move(
			(size.width() - enter->width()) / 2,
			contentTop() + st::walletSetPasscodeEnterTop);
		repeat->move(
			(size.width() - repeat->width()) / 2,
			contentTop() + st::walletSetPasscodeRepeatTop);
	}, inner()->lifetime());

	showNextButton(ph::lng_wallet_continue());

	_passcode = [=] {
		if (enter->getLastText().isEmpty()) {
			enter->showError();
			return QByteArray();
		} else if (repeat->getLastText() != enter->getLastText()) {
			repeat->showError();
			return QByteArray();
		}
		return enter->getLastText().toUtf8();
	};
	_setFocus = [=] {
		base::Platform::SwitchKeyboardLayoutToEnglish();
		enter->setFocusFast();
	};
}

void Passcode::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
