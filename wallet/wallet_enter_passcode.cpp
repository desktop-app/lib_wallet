// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_enter_passcode.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_enter_passcode.h"
#include "ui/lottie_widget.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "base/platform/base_platform_layout_switch.h"
#include "styles/style_wallet.h"

namespace Wallet {

void EnterPasscodeBox(
		not_null<Ui::GenericBox*> box,
		Fn<void(QByteArray password, Fn<void(QString)> error)> submit) {
	box->setTitle(ph::lng_wallet_passcode_title());

	const auto inner = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletPasscodeHeight));

	const auto lottie = inner->lifetime().make_state<Ui::LottieAnimation>(
		inner,
		Ui::LottieFromResource("lock"));
	lottie->start();
	lottie->stopOnLoop(1);

	const auto input = Ui::CreateChild<Ui::PasswordInput>(
		inner,
		st::walletPasscodeInput,
		ph::lng_wallet_passcode_enter());
	const auto error = Ui::CreateChild<Ui::FadeWrap<Ui::FlatLabel>>(
		inner,
		object_ptr<Ui::FlatLabel>(
			inner,
			QString(),
			st::walletPasscodeError));

	inner->widthValue(
	) | rpl::start_with_next([=](int width) {
		lottie->setGeometry({
			(width - st::walletPasscodeLottieSize) / 2,
			st::walletPasscodeLottieTop,
			st::walletPasscodeLottieSize,
			st::walletPasscodeLottieSize });
		input->move(
			(width - input->width()) / 2,
			st::walletPasscodeInputTop);
	}, input->lifetime());

	inner->widthValue(
	) | rpl::start_with_next([=](int width) {
		error->resizeToWidth(width);
		error->moveToLeft(0, st::walletPasscodeErrorTop, width);
	}, error->lifetime());
	error->hide(anim::type::instant);

	const auto next = [=] {
		const auto value = input->getLastText().toUtf8();
		if (value.isEmpty()) {
			input->showError();
			return;
		}
		submit(value, crl::guard(box, [=](QString text) {
			input->showError();
			input->setSelection(0, input->getLastText().size());
			error->entity()->setText(text);
			error->show(anim::type::normal);
		}));
	};

	Ui::Connect(input, &Ui::PasswordInput::changed, [=] {
		error->hide(anim::type::normal);
	});
	Ui::Connect(input, &Ui::PasswordInput::submitted, next);

	box->setFocusCallback([=] {
		base::Platform::SwitchKeyboardLayoutToEnglish();
		input->setFocusFast();
	});

	box->addButton(ph::lng_wallet_passcode_next(), next);
	box->addButton(ph::lng_wallet_cancel(), [=] {
		box->closeBox();
	});
}

} // namespace Wallet
