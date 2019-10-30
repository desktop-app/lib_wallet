// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_empty_history.h"

#include "wallet/wallet_phrases.h"
#include "ton/ton_state.h"
#include "ui/widgets/labels.h"
#include "ui/address_label.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet {

EmptyHistory::EmptyHistory(
	not_null<Ui::RpWidget*> parent,
	rpl::producer<EmptyHistoryState> state)
: _widget(parent) {
	setupControls(std::move(state));
}

void EmptyHistory::setGeometry(QRect geometry) {
	_widget.setGeometry(geometry);
}

void EmptyHistory::setVisible(bool visible) {
	_widget.setVisible(visible);
}

rpl::lifetime &EmptyHistory::lifetime() {
	return _widget.lifetime();
}

void EmptyHistory::setupControls(rpl::producer<EmptyHistoryState> &&state) {
	const auto lottie = _widget.lifetime().make_state<Ui::LottieAnimation>(
		&_widget,
		Ui::LottieFromResource("empty"));
	lottie->stopOnLoop(1);
	lottie->start();

	auto justCreated = rpl::duplicate(
		state
	) | rpl::map([](const EmptyHistoryState &state) {
		return state.justCreated;
	});
	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		&_widget,
		rpl::conditional(
			std::move(justCreated),
			ph::lng_wallet_empty_history_title(),
			ph::lng_wallet_empty_history_welcome()),
		st::walletEmptyHistoryTitle);
	rpl::combine(
		_widget.sizeValue(),
		title->widthValue()
	) | rpl::start_with_next([=](QSize size, int width) {
		const auto blockTop = (size.height()
			- st::walletEmptyHistoryHeight) / 2;

		lottie->setGeometry({
			(size.width() - st::walletEmptyLottieSize) / 2,
			blockTop + st::walletEmptyLottieTop,
			st::walletEmptyLottieSize,
			st::walletEmptyLottieSize });

		title->moveToLeft(
			(size.width() - width) / 2,
			blockTop + st::walletEmptyHistoryTitleTop,
			size.width());
	}, title->lifetime());

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		&_widget,
		ph::lng_wallet_empty_history_address(),
		st::walletEmptyHistoryLabel);
	rpl::combine(
		_widget.sizeValue(),
		label->widthValue()
	) | rpl::start_with_next([=](QSize size, int width) {
		const auto blockTop = (size.height()
			- st::walletEmptyHistoryHeight) / 2;
		label->moveToLeft(
			(size.width() - width) / 2,
			blockTop + st::walletEmptyHistoryLabelTop,
			size.width());
	}, label->lifetime());

	std::move(
		state
	) | rpl::map([](const EmptyHistoryState &state) {
		return state.address;
	}) | rpl::take(1) | rpl::start_with_next([=](const QString &text) {
		const auto address = Ui::CreateAddressLabel(
			&_widget,
			text,
			st::walletEmptyHistoryAddress);
		rpl::combine(
			_widget.sizeValue(),
			address->widthValue()
		) | rpl::start_with_next([=](QSize size, int width) {
			const auto blockTop = (size.height()
				- st::walletEmptyHistoryHeight) / 2;
			address->moveToLeft(
				(size.width() - address->widthNoMargins()) / 2,
				blockTop + st::walletEmptyHistoryAddressTop,
				size.width());
		}, address->lifetime());
	}, lifetime());
}

rpl::producer<EmptyHistoryState> MakeEmptyHistoryState(
		rpl::producer<Ton::WalletViewerState> state,
		bool justCreated) {
	return std::move(
		state
	) | rpl::map([=](const Ton::WalletViewerState &state) {
		return EmptyHistoryState{ state.wallet.address, justCreated };
	});
}
} // namespace Wallet
