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

ReceiveGramsBox ReceiveGrams(const QString &address) {
	auto result = ReceiveGramsBox();
	auto shareRequests = std::make_unique<rpl::event_stream<QString>>();
	result.shareRequests = shareRequests->events();
	result.box = Box([=, stream = std::move(shareRequests)](
			not_null<Ui::GenericBox*> box) mutable {
		const auto shareRequests = stream.get();
		Ui::AttachAsChild(box, std::move(stream));

		box->setTitle(ph::lng_wallet_receive_title());

		box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_receive_description(),
			st::walletLabel));

		box->addRow(object_ptr<Ui::FixedHeightWidget>(
			box,
			st::transactionSkip));

		box->addRow(
			object_ptr<Ui::RpWidget>::fromRaw(Ui::CreateAddressLabel(
				box,
				address,
				st::walletReceiveAddressLabel)));

		box->addRow(object_ptr<Ui::FixedHeightWidget>(
			box,
			st::transactionSkip));

		box->addButton(
			ph::lng_wallet_receive_share(),
			[=] { shareRequests->fire_copy(address); },
			st::walletBottomButton
		)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
	});
	return result;

}

} // namespace Wallet
