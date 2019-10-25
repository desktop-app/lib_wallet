// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_invoice_qr.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "ui/widgets/buttons.h"
#include "ui/inline_diamond.h"
#include "styles/style_wallet.h"
#include "styles/style_layers.h"

namespace Wallet {

void InvoiceQrBox(
		not_null<Ui::GenericBox*> box,
		const QString &link,
		Fn<void(QImage, QString)> share) {
	box->setTitle(ph::lng_wallet_invoice_qr_title());
	box->setStyle(st::walletBox);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	const auto qr = Ui::DiamondQr(
		link,
		st::walletInvoiceQrPixel,
		st::boxWidth - st::boxRowPadding.left() - st::boxRowPadding.right());
	const auto size = qr.width() / style::DevicePixelRatio();
	const auto height = st::walletInvoiceQrSkip * 2 + size;
	const auto container = box->addRow(
		object_ptr<Ui::BoxContentDivider>(box, height),
		st::walletInvoiceQrMargin);
	const auto button = Ui::CreateChild<Ui::AbstractButton>(container);
	button->resize(size, size);
	button->paintRequest(
	) | rpl::start_with_next([=] {
		QPainter(button).drawImage(QRect(0, 0, size, size), qr);
	}, button->lifetime());
	container->widthValue(
	) | rpl::start_with_next([=](int width) {
		button->move((width - size) / 2, st::walletInvoiceQrSkip);
	}, button->lifetime());
	button->setClickedCallback([=] {
		share(Ui::DiamondQrForShare(link), QString());
	});

	const auto prepared = ParseInvoice(link);

	AddBoxSubtitle(box, ph::lng_wallet_invoice_qr_amount());

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ParseAmount(prepared.amount).full,
			st::walletLabel),
		st::walletInvoiceQrValuePadding);

	if (!prepared.comment.isEmpty()) {
		AddBoxSubtitle(box, ph::lng_wallet_invoice_qr_comment());

		box->addRow(
			object_ptr<Ui::FlatLabel>(
				box,
				prepared.comment,
				st::walletLabel),
			st::walletInvoiceQrValuePadding);
	}

	box->addButton(
		ph::lng_wallet_invoice_qr_share(),
		[=] { share(Ui::DiamondQrForShare(link), QString()); },
		st::walletBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
}

} // namespace Wallet
