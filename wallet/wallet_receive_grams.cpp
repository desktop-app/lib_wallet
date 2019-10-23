// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_receive_grams.h"

#include "wallet/wallet_phrases.h"
#include "ui/address_label.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"

namespace Wallet {

void ReceiveGramsBox(
		not_null<Ui::GenericBox*> box,
		const QString &address,
		Fn<void(QString)> share) {
	box->setTitle(ph::lng_wallet_receive_title());
	box->setStyle(st::walletBox);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	box->addRow(object_ptr<Ui::FlatLabel>(
		box,
		ph::lng_wallet_receive_description(),
		st::walletLabel));

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletTransactionSkip));

	box->addRow(
		object_ptr<Ui::RpWidget>::fromRaw(Ui::CreateAddressLabel(
			box,
			address,
			st::walletReceiveAddressLabel)));

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletTransactionSkip));

	box->addButton(
		ph::lng_wallet_receive_share(),
		[=] { share(address); },
		st::walletBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
}

} // namespace Wallet
