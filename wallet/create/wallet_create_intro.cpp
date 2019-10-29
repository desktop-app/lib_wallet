// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_intro.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "ui/basic_click_handlers.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "ui/text/text_entity.h"
#include "styles/style_wallet.h"

void wallet_InitResource() {
	Q_INIT_RESOURCE(wallet);
}

namespace Wallet::Create {

Intro::Intro() : Step(Type::Default) {
	setTitle(ph::lng_wallet_intro_title(Ui::Text::RichLangValue));
	setDescription(
		ph::lng_wallet_intro_description(Ui::Text::RichLangValue));
	initControls();
}

rpl::producer<> Intro::importClicks() const {
	return rpl::duplicate(_importClicks);
}

void Intro::initControls() {
	wallet_InitResource();

	showLottie(
		"intro",
		st::walletStepIntroLottiePosition,
		st::walletStepIntroLottieSize);
	showNextButton(ph::lng_wallet_intro_create());
	auto importButton = object_ptr<Ui::PaddingWrap<Ui::LinkButton>>(
		inner().get(),
		object_ptr<Ui::LinkButton>(
			inner().get(),
			ph::lng_wallet_intro_import(ph::now),
			st::walletStepIntroImportLink),
		QMargins{ 0, st::walletStepIntroImportSkip, 0, 0 });
	_importClicks = importButton->entity()->clicks(
	) | rpl::map([] { return rpl::empty_value(); });
	showBelowNextButton(std::move(importButton));
}

void Intro::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
