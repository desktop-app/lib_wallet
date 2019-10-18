// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_sending_transaction.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_common.h"
#include "ton/ton_state.h"
#include "ui/widgets/buttons.h"
#include "ui/lottie_widget.h"
#include "base/timer_rpl.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"

namespace Wallet {
namespace {

constexpr auto kShowCloseDelay = 10 * crl::time(1000);

[[nodiscard]] int AskPasswordBoxHeight() {
	return st::boxLayerTitleHeight
		- st::boxTopMargin
		+ st::walletPasscodeHeight;
}

} // namespace

void SendingTransactionBox(
		not_null<Ui::GenericBox*> box,
		rpl::producer<> confirmed) {
	const auto inner = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		AskPasswordBoxHeight()));

	const auto lottie = inner->lifetime().make_state<Ui::LottieAnimation>(
		inner,
		Ui::LottieFromResource("money"));
	lottie->start();

	box->setCloseByEscape(false);
	box->setCloseByOutsideClick(false);

	rpl::merge(
		std::move(confirmed),
		base::timer_once(kShowCloseDelay)
	) | rpl::take(1) | rpl::start_with_next([=] {
		box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });
	}, box->lifetime());

	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		inner,
		ph::lng_wallet_sending_title(),
		st::walletSendingTitle);
	const auto text = Ui::CreateChild<Ui::FlatLabel>(
		inner,
		ph::lng_wallet_sending_text(),
		st::walletSendingText);

	inner->widthValue(
	) | rpl::start_with_next([=](int width) {
		lottie->setGeometry({
			(width - st::walletSendingLottieSize) / 2,
			st::walletSendingLottieTop,
			st::walletSendingLottieSize,
			st::walletSendingLottieSize });
		title->moveToLeft(
			(width - title->width()) / 2,
			st::walletSendingTitleTop,
			width);
		text->moveToLeft(
			(width - text->width()) / 2,
			st::walletSendingTextTop,
			width);
	}, inner->lifetime());
}

void SendingDoneBox(
		not_null<Ui::GenericBox*> box,
		const Ton::Transaction &result) {
	const auto inner = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		AskPasswordBoxHeight()));

	const auto lottie = inner->lifetime().make_state<Ui::LottieAnimation>(
		inner,
		Ui::LottieFromResource("done"));
	lottie->start();
	lottie->stopOnLoop(1);

	const auto amount = ParseAmount(-CalculateValue(result)).full;
	auto description = ph::lng_wallet_sent_text(
	) | rpl::map([=](QString &&value) {
		return value.replace("{amount}", amount);
	});

	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		inner,
		ph::lng_wallet_sent_title(),
		st::walletSendingTitle);
	const auto text = Ui::CreateChild<Ui::FlatLabel>(
		inner,
		std::move(description),
		st::walletSendingText);

	inner->widthValue(
	) | rpl::start_with_next([=](int width) {
		lottie->setGeometry({
			(width - st::walletSentLottieSize) / 2,
			st::walletSentLottieTop,
			st::walletSentLottieSize,
			st::walletSentLottieSize });
		title->moveToLeft(
			(width - title->width()) / 2,
			st::walletSendingTitleTop,
			width);
		text->moveToLeft(
			(width - text->width()) / 2,
			st::walletSendingTextTop,
			width);
	}, inner->lifetime());

	box->addButton(ph::lng_wallet_sent_close(), [=] { box->closeBox(); });
}

} // namespace Wallet
