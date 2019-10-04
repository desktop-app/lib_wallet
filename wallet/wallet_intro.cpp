// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_intro.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/buttons.h"
#include "styles/style_wallet.h"

namespace Wallet {

Intro::Intro(not_null<QWidget*> parent, Mode mode)
: _widget(std::make_unique<Ui::RpWidget>(parent))
, _mode(mode) {
	setupControls();
	_widget->show();
}

void Intro::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
}

rpl::producer<Intro::Action> Intro::actionRequests() const {
	return _actionRequests.events();
}

void Intro::setupControls() {
	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		_widget.get(),
		ph::lng_wallet_intro_title(Ui::Text::RichLangValue));
	const auto description = Ui::CreateChild<Ui::FlatLabel>(
		_widget.get(),
		ph::lng_wallet_intro_description(Ui::Text::RichLangValue));
	const auto next = Ui::CreateChild<Ui::RoundButton>(
		_widget.get(),
		ph::lng_wallet_intro_create(),
		st::walletNextButton);

	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		title->move((size.width() - title->width()) / 2, size.height() / 4);
		description->move((size.width() - description->width()) / 2, size.height() / 2);
		next->move((size.width() - next->width()) / 2, (size.height() * 3) / 4);
	}, _widget->lifetime());

	next->setClickedCallback([=] {
		_actionRequests.fire(Action::CreateWallet);
	});
}

rpl::lifetime &Intro::lifetime() {
	return _widget->lifetime();
}

} // namespace Wallet
