// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_delete.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "styles/style_layers.h"

namespace Wallet {

void DeleteWalletBox(not_null<Ui::GenericBox*> box, Fn<void()> logout) {
	box->setTitle(ph::lng_wallet_delete_title());

	box->addRow(object_ptr<Ui::FlatLabel>(
		box,
		ph::lng_wallet_delete_about(Ui::Text::RichLangValue),
		st::boxLabel));

	box->addButton(ph::lng_wallet_delete_disconnect(), [=] {
		logout();
	}, st::attentionBoxButton);
	box->addButton(ph::lng_wallet_delete_cancel(), [=] {
		box->closeBox();
	});
}

} // namespace Wallet
