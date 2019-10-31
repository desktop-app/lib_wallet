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
#include "ui/text/text_utilities.h"
#include "ui/basic_click_handlers.h"
#include "base/qt_signal_producer.h"
#include "styles/style_wallet.h"
#include "styles/style_layers.h"

namespace Wallet {
namespace {

class InvoiceHandler final : public UrlClickHandler {
public:
	explicit InvoiceHandler(const QString &url) : UrlClickHandler(url) {
	}

	QString copyToClipboardContextItemText() const override {
		return QString();
	}

};

} // namespace

void CreateInvoiceBox(
		not_null<Ui::GenericBox*> box,
		const QString &address,
		Fn<void(QString)> generateQr,
		Fn<void(QImage, QString)> share) {
	box->setTitle(ph::lng_wallet_invoice_title());
	box->setStyle(st::walletInvoiceBox);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	AddBoxSubtitle(box, ph::lng_wallet_invoice_amount());
	const auto amount = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(
			CreateAmountInput(box, ph::lng_wallet_invoice_number())),
		st::walletSendAmountPadding);

	const auto comment = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(
			CreateCommentInput(box, ph::lng_wallet_invoice_comment())));

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_invoice_comment_about(),
			st::walletSendAbout),
		st::walletInvoiceAboutCommentPadding);

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
	const auto url = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			QString(),
			st::walletInvoiceLinkLabel),
		st::walletInvoiceLinkPadding);
	url->setBreakEverywhere(true);
	url->setSelectable(true);
	url->setDoubleClickSelectsParagraph(true);
	url->setContextCopyText(ph::lng_wallet_invoice_copy_url(ph::now));
	url->setClickHandlerFilter([=](const ClickHandlerPtr&, Qt::MouseButton) {
		submit();
		return false;
	});
	url->setMinimumHeight(st::walletInvoiceLinkLabel.maxHeight);

	rpl::combine(
		std::move(amountValue),
		std::move(commentValue)
	) | rpl::map([=](int64 amount, const QString &comment) {
		const auto link = TransferLink(address, amount, comment);
		return (amount > 0)
			? Ui::Text::Link(link)
			: TextWithEntities{ link };
	}) | rpl::start_with_next([=](TextWithEntities &&text) {
		url->setMarkedText(std::move(text));
		url->setLink(1, std::make_shared<InvoiceHandler>(text.text));
	}, url->lifetime());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_invoice_url_about(),
			st::walletSendAbout),
		st::walletSendAboutPadding);

	box->setFocusCallback([=] {
		amount->setFocusFast();
	});

	Ui::Connect(amount, &Ui::InputField::submitted, [=] {
		if (ParseAmountString(amount->getLastText()).value_or(0) <= 0) {
			amount->showError();
		} else {
			comment->setFocus();
		}
	});
	Ui::Connect(comment, &Ui::InputField::submitted, submit);

	const auto button = box->addButton(
		ph::lng_wallet_invoice_share(),
		submit,
		st::walletBottomButton);
	button->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);

	const auto parent = button->parentWidget();
	const auto generateLink = Ui::CreateChild<Ui::LinkButton>(
		parent,
		ph::lng_wallet_invoice_generate_qr(ph::now),
		st::boxLinkButton);
	rpl::combine(
		button->geometryValue(),
		generateLink->widthValue()
	) | rpl::start_with_next([=](QRect button, int width) {
		generateLink->move(
			(parent->width() - width) / 2,
			button.top() - st::walletGenerateQrLinkTop);
	}, generateLink->lifetime());
	generateLink->setClickedCallback([=] {
		if (const auto link = collectLink()) {
			generateQr(*link);
		}
	});
}

} // namespace Wallet
