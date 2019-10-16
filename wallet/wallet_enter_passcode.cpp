// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_enter_passcode.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_enter_passcode.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "styles/style_wallet.h"

namespace Wallet {

void EnterPasscodeBox(
		not_null<Ui::GenericBox*> box,
		Fn<void(QByteArray password, Fn<void(QString)> error)> submit) {
	box->setTitle(ph::lng_wallet_passcode_title());

	const auto inner = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::walletPasscodeHeight));

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

	Ui::Connect(input, &Ui::PasswordInput::changed, [=] {
		error->hide(anim::type::normal);
	});

	box->setFocusCallback([=] {
		input->setFocusFast();
	});

	box->addButton(ph::lng_wallet_passcode_next(), [=] {
		const auto value = input->getLastText().toUtf8();
		if (value.isEmpty()) {
			input->showError();
			return;
		}
		submit(value, crl::guard(box, [=](QString text) {
			input->showError();
			error->entity()->setText(text);
			error->show(anim::type::normal);
		}));
	});
	box->addButton(ph::lng_wallet_cancel(), [=] {
		box->closeBox();
	});
}

} // namespace Wallet
