// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_view_transaction.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "ui/address_label.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/widgets/buttons.h"
#include "ton/ton_state.h"
#include "base/unixtime.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"

#include <QtCore/QDateTime>

namespace Wallet {
namespace {

object_ptr<Ui::RpWidget> CreateSummary(
		not_null<Ui::RpWidget*> parent,
		const Ton::Transaction &data) {
	const auto feeSkip = st::transactionFeeSkip;
	const auto height = st::transactionSummaryHeight
		+ (data.otherFee ? (st::normalFont->height + feeSkip) : 0)
		+ (data.storageFee ? (st::normalFont->height + feeSkip) : 0);
	auto result = object_ptr<Ui::FixedHeightWidget>(
		parent,
		height);
	const auto balance = Ui::CreateChild<Ui::FlatLabel>(
		result.data(),
		ParseAmount(CalculateValue(data), true).full,
		st::transactionValue);
	const auto otherFee = data.otherFee
		? Ui::CreateChild<Ui::FlatLabel>(
			result.data(),
			ph::lng_wallet_view_transaction_fee(ph::now).replace(
				"<0>",
				ParseAmount(data.otherFee).full),
			st::transactionFee)
		: nullptr;
	const auto storageFee = data.storageFee
		? Ui::CreateChild<Ui::FlatLabel>(
			result.data(),
			ph::lng_wallet_view_storage_fee(ph::now).replace(
				"<0>",
				ParseAmount(data.storageFee).full),
			st::transactionFee)
		: nullptr;
	rpl::combine(
		result->widthValue(),
		balance->widthValue(),
		otherFee ? otherFee->widthValue() : rpl::single(0),
		storageFee ? storageFee->widthValue() : rpl::single(0)
	) | rpl::start_with_next([=](int width, int, int, int) {
		auto top = st::transactionValueTop;
		balance->move((width - balance->width()) / 2, top);
		top += balance->height() + feeSkip;
		if (otherFee) {
			otherFee->move((width - otherFee->width()) / 2, top);
			top += otherFee->height() + feeSkip;
		}
		if (storageFee) {
			storageFee->move((width - storageFee->width()) / 2, top);
		}
	}, result->lifetime());

	return result;
}

} // namespace

void ViewTransactionBox(
		not_null<Ui::GenericBox*> box,
		Ton::Transaction &&data,
		Fn<void(QString)> send) {
	box->setTitle(ph::lng_wallet_view_title());

	const auto address = ExtractAddress(data);
	const auto incoming = data.outgoing.empty();
	const auto message = ExtractMessage(data);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	box->addRow(CreateSummary(box, data));

	AddBoxSubtitle(box, incoming
		? ph::lng_wallet_view_sender()
		: ph::lng_wallet_view_recipient());
	box->addRow(
		object_ptr<Ui::RpWidget>::fromRaw(Ui::CreateAddressLabel(
			box,
			address,
			st::walletLabel)));

	AddBoxSubtitle(box, ph::lng_wallet_view_date());
	box->addRow(object_ptr<Ui::FlatLabel>(
		box,
		base::unixtime::parse(
			data.time
		).toString(Qt::DefaultLocaleLongDate),
		st::walletLabel));

	if (!message.isEmpty()) {
		AddBoxSubtitle(box, ph::lng_wallet_view_comment());
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			message,
			st::walletLabel)
		)->setSelectable(true);
	}

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::transactionSkip));

	auto text = incoming
		? ph::lng_wallet_view_send_to_address()
		: ph::lng_wallet_view_send_to_recipient();
	box->addButton(
		std::move(text),
		[=] { send(address); },
		st::walletBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
}

} // namespace Wallet
