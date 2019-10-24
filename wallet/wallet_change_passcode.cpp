// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_change_passcode.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_enter_passcode.h"
#include "ui/lottie_widget.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "base/platform/base_platform_layout_switch.h"
#include "styles/style_wallet.h"

namespace Wallet {

void ChangePasscodeBox(
		not_null<Ui::GenericBox*> box,
		Fn<void(
			QByteArray old,
			QByteArray now,
			Fn<void(QString)> error)> submit) {
	box->setTitle(ph::lng_wallet_change_passcode_title());

	const auto inner = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletChangePasscodeHeight));

	const auto lottie = inner->lifetime().make_state<Ui::LottieAnimation>(
		inner,
		Ui::LottieFromResource("lock"));
	lottie->start();
	lottie->stopOnLoop(1);

	const auto old = Ui::CreateChild<Ui::PasswordInput>(
		inner,
		st::walletPasscodeInput,
		ph::lng_wallet_change_passcode_old());
	const auto error = Ui::CreateChild<Ui::FadeWrap<Ui::FlatLabel>>(
		inner,
		object_ptr<Ui::FlatLabel>(
			inner,
			QString(),
			st::walletPasscodeError));

	const auto now = Ui::CreateChild<Ui::PasswordInput>(
		inner,
		st::walletPasscodeInput,
		ph::lng_wallet_change_passcode_new());

	const auto repeat = Ui::CreateChild<Ui::PasswordInput>(
		inner,
		st::walletPasscodeInput,
		ph::lng_wallet_change_passcode_repeat());

	inner->widthValue(
	) | rpl::start_with_next([=](int width) {
		lottie->setGeometry({
			(width - st::walletPasscodeLottieSize) / 2,
			st::walletPasscodeLottieTop,
			st::walletPasscodeLottieSize,
			st::walletPasscodeLottieSize });
		old->move(
			(width - old->width()) / 2,
			st::walletChangePasscodeOldTop);

		error->resizeToWidth(width);
		error->moveToLeft(0, st::walletChangePasscodeErrorTop, width);

		now->move(
			(width - now->width()) / 2,
			st::walletChangePasscodeNowTop);
		repeat->move(
			(width - repeat->width()) / 2,
			st::walletChangePasscodeRepeatTop);
	}, inner->lifetime());

	error->hide(anim::type::instant);

	const auto save = [=] {
		const auto oldPassword = old->getLastText().toUtf8();
		const auto nowPassword = now->getLastText().toUtf8();
		if (oldPassword.isEmpty()) {
			old->showError();
			return;
		} else if (nowPassword.isEmpty()) {
			now->showError();
			return;
		} else if (repeat->getLastText().toUtf8() != nowPassword) {
			repeat->showError();
			return;
		}

		submit(oldPassword, nowPassword, crl::guard(box, [=](QString text) {
			old->showError();
			error->entity()->setText(text);
			error->show(anim::type::normal);
		}));
	};

	Ui::Connect(old, &Ui::PasswordInput::changed, [=] {
		error->hide(anim::type::normal);
	});

	Ui::Connect(old, &Ui::PasswordInput::submitted, [=] {
		if (old->getLastText().isEmpty()) {
			old->showError();
		} else {
			now->setFocus();
		}
	});
	Ui::Connect(now, &Ui::PasswordInput::submitted, [=] {
		if (now->getLastText().isEmpty()) {
			now->showError();
		} else {
			repeat->setFocus();
		}
	});
	Ui::Connect(repeat, &Ui::PasswordInput::submitted, save);

	box->setFocusCallback([=] {
		base::Platform::SwitchKeyboardLayoutToEnglish();
		old->setFocusFast();
	});

	box->addButton(ph::lng_wallet_save(), save);
	box->addButton(ph::lng_wallet_cancel(), [=] {
		box->closeBox();
	});
}

} // namespace Wallet
