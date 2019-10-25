// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_create_invoice.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "base/qt_signal_producer.h"
#include "styles/style_wallet.h"
#include "styles/style_layers.h"

namespace Wallet {

void CreateInvoiceBox(
		not_null<Ui::GenericBox*> box,
		const QString &address,
		Fn<void(QString)> generateQr,
		Fn<void(QImage, QString)> share) {
	box->setTitle(ph::lng_wallet_invoice_title());
	box->setStyle(st::walletBox);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	AddBoxSubtitle(box, ph::lng_wallet_invoice_amount());
	const auto amount = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(
			CreateAmountInput(box, ph::lng_wallet_invoice_number())),
		st::walletSendAmountPadding);

	const auto comment = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(
			CreateCommentInput(box, ph::lng_wallet_invoice_comment())),
		st::walletSendCommentPadding);

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_invoice_comment_about(),
			st::walletSendAbout),
		st::walletSendAboutPadding);

	AddBoxSubtitle(box, ph::lng_wallet_invoice_url());

	auto amountValue = rpl::single(
		rpl::empty_value()
	) | rpl::then(base::qt_signal_producer(
		amount,
		&Ui::InputField::changed
	)) | rpl::map([=] {
		return ParseAmountString(amount->getLastText()).value_or(0);
	});
	auto commentValue = rpl::single(
		rpl::empty_value()
	) | rpl::then(base::qt_signal_producer(
		comment,
		&Ui::InputField::changed
	)) | rpl::map([=] {
		return comment->getLastText();
	});
	auto linkText = rpl::combine(
		std::move(amountValue),
		std::move(commentValue)
	) | rpl::map([=](int64 amount, const QString &comment) {
		return TransferLink(address, amount, comment);
	});
	const auto url = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			std::move(linkText),
			st::walletInvoiceLinkLabel),
		st::walletSendAboutPadding);
	url->setBreakEverywhere(true);
	url->setSelectable(true);
	url->setDoubleClickSelectsParagraph(true);
	url->setContextCopyText(ph::lng_wallet_invoice_copy_url(ph::now));

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_invoice_url_about(),
			st::walletSendAbout),
		st::walletSendAboutPadding);

	box->setFocusCallback([=] {
		amount->setFocusFast();
	});

	const auto collectLink = [=]() -> std::optional<QString> {
		const auto parsed = ParseAmountString(amount->getLastText());
		const auto text = comment->getLastText();
		if (parsed.value_or(0) <= 0) {
			amount->showError();
			return std::nullopt;
		} else if (text.toUtf8().size() > kMaxCommentLength) {
			comment->showError();
			return std::nullopt;
		}
		return TransferLink(address, *parsed, text);
	};
	const auto submit = [=] {
		if (const auto link = collectLink()) {
			share(QImage(), *link);
		}
	};

	Ui::Connect(amount, &Ui::InputField::submitted, [=] {
		if (ParseAmountString(amount->getLastText()).value_or(0) <= 0) {
			amount->showError();
		} else {
			comment->setFocus();
		}
	});
	Ui::Connect(comment, &Ui::InputField::submitted, submit);

	box->addButton(
		ph::lng_wallet_invoice_share(),
		submit,
		st::walletBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);

	const auto generateLinkWrap = box->addRow(
		object_ptr<Ui::FixedHeightWidget>(
			box,
			st::boxLinkButton.font->height),
		st::walletReceiveLinkPadding);
	const auto generateLink = Ui::CreateChild<Ui::LinkButton>(
		generateLinkWrap,
		ph::lng_wallet_invoice_generate_qr(ph::now),
		st::boxLinkButton);
	generateLinkWrap->widthValue(
	) | rpl::start_with_next([=](int width) {
		generateLink->move((width - generateLink->width()) / 2, 0);
	}, generateLink->lifetime());
	generateLink->setClickedCallback([=] {
		if (const auto link = collectLink()) {
			generateQr(*link);
		}
	});
}

} // namespace Wallet
