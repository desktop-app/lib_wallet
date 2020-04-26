// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_confirm_transaction.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "wallet/wallet_send_grams.h"
#include "ui/address_label.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

#include <QtGui/QtEvents>

namespace Wallet {
namespace {

constexpr auto kWarningPreviewLength = 30;

[[nodiscard]] rpl::producer<TextWithEntities> PrepareEncryptionWarning(
		const PreparedInvoice &invoice) {
	const auto text = (invoice.comment.size() > kWarningPreviewLength)
		? (invoice.comment.mid(0, kWarningPreviewLength - 3) + "...")
		: invoice.comment;
	return ph::lng_wallet_confirm_warning(
		Ui::Text::RichLangValue
	) | rpl::map([=](TextWithEntities value) {
		const auto was = QString("{comment}");
		const auto wasLength = was.size();
		const auto nowLength = text.size();
		const auto position = value.text.indexOf(was);
		if (position >= 0) {
			value.text = value.text.mid(0, position)
				+ text
				+ value.text.mid(position + wasLength);
			auto entities = EntitiesInText();
			for (auto &entity : value.entities) {
				const auto from = entity.offset();
				const auto till = from + entity.length();
				if (till < position + wasLength) {
					if (from < position) {
						entity.shrinkFromRight(std::max(till - position, 0));
						entities.push_back(std::move(entity));
					}
				} else if (from > position) {
					if (till > position + wasLength) {
						entity.extendToLeft(
							std::min(from - (position + wasLength), 0));
						entity.shiftRight(nowLength - wasLength);
						entities.push_back(std::move(entity));
					}
				} else {
					entity.shrinkFromRight(wasLength - nowLength);
					entities.push_back(std::move(entity));
				}
			}
			value.entities = std::move(entities);
		}
		return value;
	});
}

} // namespace

void ConfirmTransactionBox(
		not_null<Ui::GenericBox*> box,
		const PreparedInvoice &invoice,
		int64 fee,
		Fn<void()> confirmed) {
	box->setTitle(ph::lng_wallet_confirm_title());

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });
	box->setCloseByOutsideClick(false);

	const auto amount = FormatAmount(invoice.amount).full;
	auto text = rpl::combine(
		ph::lng_wallet_confirm_text(),
		ph::lng_wallet_grams_count(amount)()
	) | rpl::map([=](QString &&text, const QString &grams) {
		return Ui::Text::RichLangValue(text.replace("{grams}", grams));
	});
	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			std::move(text),
			st::walletLabel),
		st::walletConfirmationLabelPadding);

	box->addRow(
		object_ptr<Ui::RpWidget>::fromRaw(Ui::CreateAddressLabel(
			box,
			invoice.address,
			st::walletConfirmationAddressLabel,
			nullptr,
			st::windowBgOver->c)),
		st::walletConfirmationAddressPadding);

	const auto feeParsed = FormatAmount(fee).full;
	auto feeText = rpl::combine(
		ph::lng_wallet_confirm_fee(),
		ph::lng_wallet_grams_count(feeParsed)()
	) | rpl::map([=](QString &&text, const QString &grams) {
		return text.replace("{grams}", grams);
	});
	const auto feeWrap = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		(st::walletConfirmationFee.style.font->height
			+ st::walletConfirmationSkip)));
	const auto feeLabel = Ui::CreateChild<Ui::FlatLabel>(
		feeWrap,
		std::move(feeText),
		st::walletConfirmationFee);
	rpl::combine(
		feeLabel->widthValue(),
		feeWrap->widthValue()
	) | rpl::start_with_next([=](int innerWidth, int outerWidth) {
		feeLabel->moveToLeft(
			(outerWidth - innerWidth) / 2,
			0,
			outerWidth);
	}, feeLabel->lifetime());

	if (invoice.sendUnencryptedText && !invoice.comment.isEmpty()) {
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			PrepareEncryptionWarning(invoice),
			st::walletLabel));
	}

	box->events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		if (e->type() == QEvent::KeyPress) {
			const auto key = static_cast<QKeyEvent*>(e.get())->key();
			if (key == Qt::Key_Enter || key == Qt::Key_Return) {
				confirmed();
			}
		}
	}, box->lifetime());

	box->addButton(ph::lng_wallet_confirm_send(), confirmed);
	box->addButton(ph::lng_wallet_cancel(), [=] { box->closeBox(); });
}

} // namespace Wallet
