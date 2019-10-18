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

void Intro::initControls() {
	wallet_InitResource();

	auto termsText = rpl::combine(
		ph::lng_wallet_intro_accept_text(),
		ph::lng_wallet_intro_accept_terms()
	) | rpl::map([](QString text, const QString &link) {
		const auto full = text.replace(
			"{terms_link}",
			textcmdLink("https://telegram.org/tos/wallet", link));
		return TextUtilities::ParseEntities(full, TextParseRichText);
	});

	showLottie(
		"intro",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	showNextButton(ph::lng_wallet_intro_create());
	showBelowNextButton(object_ptr<Ui::PaddingWrap<Ui::FlatLabel>>(
		inner().get(),
		object_ptr<Ui::FlatLabel>(
			inner().get(),
			std::move(termsText),
			st::walletStepIntroTerms),
		QMargins{ 0, st::walletStepIntroTermsSkip, 0, 0 }));
}

void Intro::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
