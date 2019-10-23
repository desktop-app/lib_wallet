// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_cover.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/amount_label.h"
#include "ui/lottie_widget.h"
#include "ton/ton_state.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

namespace Wallet {

Cover::Cover(not_null<Ui::RpWidget*> parent, rpl::producer<CoverState> state)
: _widget(parent) {
	setupControls(std::move(state));
}

void Cover::setGeometry(QRect geometry) {
	_widget.setGeometry(geometry);
}

int Cover::height() const {
	return _widget.height();
}

rpl::producer<> Cover::sendRequests() const {
	return _sendRequests.events();
}

rpl::producer<> Cover::receiveRequests() const {
	return _receiveRequests.events();
}

rpl::lifetime &Cover::lifetime() {
	return _widget.lifetime();
}

void Cover::setupControls(rpl::producer<CoverState> &&state) {
	auto amount = rpl::duplicate(
		state
	) | rpl::map([](const CoverState &state) {
		return ParseAmount(std::max(state.balance, 0LL));
	});

	const auto balance = _widget.lifetime().make_state<Ui::AmountLabel>(
		&_widget,
		std::move(amount),
		st::walletCoverBalance);
	rpl::combine(
		_widget.sizeValue(),
		balance->widthValue()
	) | rpl::start_with_next([=](QSize size, int width) {
		const auto blockTop = (size.height()
			+ st::walletTopBarHeight
			- st::walletCoverHeight) / 2 - st::walletTopBarHeight;
		const auto balanceTop = blockTop + st::walletCoverBalanceTop;
		balance->move((size.width() - width) / 2, balanceTop);
	}, balance->lifetime());

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		&_widget,
		ph::lng_wallet_cover_balance(),
		st::walletCoverLabel);
	rpl::combine(
		_widget.sizeValue(),
		label->widthValue()
	) | rpl::start_with_next([=](QSize size, int width) {
		const auto blockTop = (size.height()
			+ st::walletTopBarHeight
			- st::walletCoverHeight) / 2 - st::walletTopBarHeight;
		label->moveToLeft(
			(size.width() - width) / 2,
			blockTop + st::walletCoverLabelTop,
			size.width());
	}, label->lifetime());

	auto hasFunds = rpl::duplicate(
		state
	) | rpl::map([](const CoverState &state) {
		return state.balance > 0;
	}) | rpl::distinct_until_changed();

	const auto receive = Ui::CreateChild<Ui::RoundButton>(
		&_widget,
		rpl::conditional(
			rpl::duplicate(hasFunds),
			ph::lng_wallet_cover_receive(),
			ph::lng_wallet_cover_receive_full()),
		st::walletCoverButton);
	receive->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
	const auto send = Ui::CreateChild<Ui::RoundButton>(
		&_widget,
		ph::lng_wallet_cover_send(),
		st::walletCoverButton);
	send->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);

	rpl::combine(
		_widget.sizeValue(),
		std::move(hasFunds)
	) | rpl::start_with_next([=](QSize size, bool hasFunds) {
		const auto fullWidth = st::walletCoverButtonWidthFull;
		const auto left = (size.width() - fullWidth) / 2;
		const auto top = size.height()
			- st::walletCoverButtonBottom
			- receive->height();
		send->setVisible(hasFunds);
		if (hasFunds) {
			receive->resizeToWidth(st::walletCoverButtonWidth);
			send->resizeToWidth(st::walletCoverButtonWidth);
			send->moveToLeft(
				left + fullWidth - send->width(),
				top,
				size.width());
		} else {
			receive->resizeToWidth(fullWidth);
		}
		receive->moveToLeft(left, top, size.width());
	}, receive->lifetime());

	receive->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_receiveRequests, receive->lifetime());

	send->clicks(
	) | rpl::map([] {
		return rpl::empty_value();
	}) | rpl::start_to_stream(_sendRequests, send->lifetime());

	_widget.paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		QPainter(&_widget).fillRect(clip, st::walletTopBg);
	}, lifetime());
}

rpl::producer<CoverState> MakeCoverState(
		rpl::producer<Ton::WalletViewerState> state) {
	return std::move(
		state
	) | rpl::map([](const Ton::WalletViewerState &data) {
		return CoverState{ data.wallet.account.balance };
	});
}

} // namespace Wallet
