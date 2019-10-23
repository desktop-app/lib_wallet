// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_view_transaction.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "ui/amount_label.h"
#include "ui/address_label.h"
#include "ui/lottie_widget.h"
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
	const auto feeSkip = st::walletTransactionFeeSkip;
	const auto secondFeeSkip = st::walletTransactionSecondFeeSkip;
	const auto height = st::walletTransactionSummaryHeight
		+ (data.otherFee ? (st::normalFont->height + feeSkip) : 0)
		+ (data.storageFee
			? (st::normalFont->height
				+ (data.otherFee ? secondFeeSkip : feeSkip))
			: 0);
	auto result = object_ptr<Ui::FixedHeightWidget>(
		parent,
		height);

	const auto balance = result->lifetime().make_state<Ui::AmountLabel>(
		result.data(),
		rpl::single(ParseAmount(CalculateValue(data), true)),
		st::walletTransactionValue);
	const auto otherFee = data.otherFee
		? Ui::CreateChild<Ui::FlatLabel>(
			result.data(),
			ph::lng_wallet_view_transaction_fee(ph::now).replace(
				"{amount}",
				ParseAmount(data.otherFee).full),
			st::walletTransactionFee)
		: nullptr;
	const auto storageFee = data.storageFee
		? Ui::CreateChild<Ui::FlatLabel>(
			result.data(),
			ph::lng_wallet_view_storage_fee(ph::now).replace(
				"{amount}",
				ParseAmount(data.storageFee).full),
			st::walletTransactionFee)
		: nullptr;
	rpl::combine(
		result->widthValue(),
		balance->widthValue(),
		otherFee ? otherFee->widthValue() : rpl::single(0),
		storageFee ? storageFee->widthValue() : rpl::single(0)
	) | rpl::start_with_next([=](int width, int bwidth, int, int) {
		auto top = st::walletTransactionValueTop;

		balance->move((width - bwidth) / 2, top);

		top += balance->height() + feeSkip;
		if (otherFee) {
			otherFee->move((width - otherFee->width()) / 2, top);
			top += otherFee->height() + secondFeeSkip;
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
	box->setStyle(st::walletBox);

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
			st::walletTransactionAddress)),
		{
			st::boxRowPadding.left(),
			st::boxRowPadding.top(),
			st::boxRowPadding.right(),
			st::walletTransactionDateTop,
		});

	AddBoxSubtitle(box, ph::lng_wallet_view_date());
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			base::unixtime::parse(
				data.time
			).toString(Qt::DefaultLocaleLongDate),
			st::walletLabel),
		{
			st::boxRowPadding.left(),
			st::boxRowPadding.top(),
			st::boxRowPadding.right(),
			(message.isEmpty()
				? st::boxRowPadding.bottom()
				: st::walletTransactionCommentTop),
		});

	if (!message.isEmpty()) {
		AddBoxSubtitle(box, ph::lng_wallet_view_comment());
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			message,
			st::walletLabel))->setSelectable(true);
	}

	box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletTransactionBottomSkip));

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
