// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_confirm_transaction.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "wallet/wallet_send_grams.h"
#include "ui/address_label.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"

namespace Wallet {

void ConfirmTransactionBox(
		not_null<Ui::GenericBox*> box,
		const PreparedInvoice &invoice,
		int64 fee,
		Fn<void()> confirmed) {
	box->setTitle(ph::lng_wallet_confirm_title());

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	const auto amount = ParseAmount(invoice.amount).full;
	auto text = ph::lng_wallet_confirm_text(
	) | rpl::map([=](QString &&text) {
		return Ui::Text::RichLangValue(text.replace("{amount}", amount));
	});
	box->addRow(object_ptr<Ui::FlatLabel>(
		box,
		std::move(text),
		st::walletLabel));

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletConfirmationSkip));

	box->addRow(
		object_ptr<Ui::RpWidget>::fromRaw(Ui::CreateAddressLabel(
			box,
			invoice.address,
			st::walletConfirmationAddressLabel)));

	const auto feeParsed = ParseAmount(fee).full;
	auto feeText = ph::lng_wallet_confirm_fee(
	) | rpl::map([=](QString &&text) {
		return text.replace("{amount}", feeParsed);
	});
	const auto feeWrap = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		(st::walletConfirmationFeeSkip
			+ st::walletConfirmationFee.style.font->height
			+ st::walletConfirmationSkip)));
	const auto feeLabel = Ui::CreateChild<Ui::FlatLabel>(
		feeWrap,
		std::move(feeText),
		st::walletConfirmationFee);
	rpl::combine(
		feeLabel->widthValue(),
		feeWrap->widthValue()
	) | rpl::start_with_next([=](int innerWidth, int outerWidth) {
		feeLabel->moveToLeft(
			(outerWidth - innerWidth) / 2,
			st::walletConfirmationFeeSkip,
			outerWidth);
	}, feeLabel->lifetime());

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletConfirmationSkip));

	box->addButton(ph::lng_wallet_confirm_send(), confirmed);
	box->addButton(ph::lng_wallet_cancel(), [=] { box->closeBox(); });
}

} // namespace Wallet
