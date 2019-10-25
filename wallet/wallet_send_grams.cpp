// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_send_grams.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_common.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/buttons.h"
#include "ui/inline_diamond.h"
#include "base/algorithm.h"
#include "base/qt_signal_producer.h"
#include "styles/style_wallet.h"
#include "styles/style_layers.h"
#include "styles/palette.h"

namespace Wallet {
namespace {

struct FixedAddress {
	PreparedInvoice invoice;
	int position = 0;
};

[[nodiscard]] QString AmountSeparator() {
	return ParseAmount(1).separator;
}

[[nodiscard]] FixedAddress FixAddressInput(
		const QString &text,
		int position) {
	auto result = FixedAddress{ ParseInvoice(text), position };
	if (result.invoice.address != text) {
		const auto removed = std::max(
			int(text.size()) - int(result.invoice.address.size()),
			0);
		position = std::max(position - removed, 0);
	}
	return result;
}

} // namespace

void SendGramsBox(
		not_null<Ui::GenericBox*> box,
		const QString &invoice,
		rpl::producer<int64> balance,
		Fn<void(PreparedInvoice, Fn<void(InvoiceField)> error)> done) {
	const auto prepared = ParseInvoice(invoice);
	const auto funds = std::make_shared<int64>();

	box->setTitle(ph::lng_wallet_send_title());
	box->setStyle(st::walletBox);

	box->addTopButton(st::boxTitleClose, [=] { box->closeBox(); });

	AddBoxSubtitle(box, ph::lng_wallet_send_recipient());
	const auto address = box->addRow(object_ptr<Ui::InputField>(
		box,
		st::walletSendInput,
		Ui::InputField::Mode::NoNewlines,
		ph::lng_wallet_send_address(),
		prepared.address));
	address->rawTextEdit()->setWordWrapMode(QTextOption::WrapAnywhere);

	const auto about = box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_send_about(),
			st::walletSendAbout),
		st::walletSendAboutPadding);

	const auto subtitle = AddBoxSubtitle(box, ph::lng_wallet_send_amount());
	const auto amount = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(CreateAmountInput(
			box,
			rpl::single("0" + AmountSeparator() + "0"),
			prepared.amount)),
		st::walletSendAmountPadding);

	auto balanceText = rpl::combine(
		ph::lng_wallet_send_balance(),
		rpl::duplicate(balance)
	) | rpl::map([](QString &&phrase, int64 value) {
		return phrase.replace(
			"{amount}",
			ParseAmount(std::max(value, 0LL)).full);
	});

	const auto diamondLabel = Ui::CreateInlineDiamond(
		subtitle->parentWidget(),
		0,
		0,
		st::walletSendBalanceLabel.style.font);
	const auto balanceLabel = Ui::CreateChild<Ui::FlatLabel>(
		subtitle->parentWidget(),
		std::move(balanceText),
		st::walletSendBalanceLabel);
	rpl::combine(
		subtitle->geometryValue(),
		balanceLabel->widthValue()
	) | rpl::start_with_next([=](QRect rect, int innerWidth) {
		const auto diamondTop = rect.top()
			+ st::walletSubsectionTitle.style.font->ascent
			- st::walletDiamondAscent;
		const auto diamondRight = st::boxRowPadding.right();
		diamondLabel->moveToRight(diamondRight, diamondTop);
		const auto labelTop = rect.top()
			+ st::walletSubsectionTitle.style.font->ascent
			- st::walletSendBalanceLabel.style.font->ascent;
		const auto labelRight = diamondRight
			+ st::walletDiamondSize
			+ st::walletSendBalanceLabel.style.font->spacew;
		balanceLabel->moveToRight(labelRight, labelTop);
	}, balanceLabel->lifetime());

	const auto comment = box->addRow(
		object_ptr<Ui::InputField>::fromRaw(CreateCommentInput(
			box,
			ph::lng_wallet_send_comment(),
			prepared.comment)),
		st::walletSendCommentPadding);

	const auto checkFunds = [=](const QString &amount) {
		if (const auto value = ParseAmountString(amount)) {
			const auto insufficient = (*value > std::max(*funds, 0LL));
			balanceLabel->setTextColorOverride(insufficient
				? std::make_optional(st::boxTextFgError->c)
				: std::nullopt);
		}
	};
	std::move(
		balance
	) | rpl::start_with_next([=](int64 value) {
		*funds = value;
		checkFunds(amount->getLastText());
	}, amount->lifetime());

	Ui::Connect(amount, &Ui::InputField::changed, [=] {
		Ui::PostponeCall(amount, [=] {
			checkFunds(amount->getLastText());
		});
	});
	Ui::Connect(address, &Ui::InputField::changed, [=] {
		Ui::PostponeCall(address, [=] {
			const auto position = address->textCursor().position();
			const auto now = address->getLastText();
			const auto fixed = FixAddressInput(now, position);
			if (fixed.invoice.address != now) {
				address->setText(fixed.invoice.address);
				address->setFocusFast();
				address->setCursorPosition(fixed.position);
			}
			if (fixed.invoice.amount > 0) {
				amount->setText(ParseAmount(fixed.invoice.amount).full);
			}
			if (!fixed.invoice.comment.isEmpty()) {
				comment->setText(fixed.invoice.comment);
			}
			if (fixed.invoice.address.size() == kAddressLength
				&& address->hasFocus()) {
				if (amount->getLastText().isEmpty()) {
					amount->setFocus();
				} else {
					comment->setFocus();
				}
			}
		});
	});

	box->setFocusCallback([=] {
		if (prepared.address.isEmpty()
			|| address->getLastText() != prepared.address) {
			address->setFocusFast();
		} else {
			amount->setFocusFast();
		}
	});

	const auto showError = crl::guard(box, [=](InvoiceField field) {
		switch (field) {
		case InvoiceField::Address: address->showError(); return;
		case InvoiceField::Amount: amount->showError(); return;
		case InvoiceField::Comment: comment->showError(); return;
		}
		Unexpected("Field value in SendGramsBox error callback.");
	});
	const auto submit = [=] {
		auto collected = PreparedInvoice();
		const auto parsed = ParseAmountString(amount->getLastText());
		if (!parsed) {
			amount->showError();
			return;
		}
		collected.amount = *parsed;
		collected.address = address->getLastText();
		collected.comment = comment->getLastText();
		done(collected, showError);
	};

	Ui::Connect(address, &Ui::InputField::submitted, [=] {
		if (address->getLastText().size() != kAddressLength) {
			address->showError();
		} else {
			amount->setFocus();
		}
	});
	Ui::Connect(amount, &Ui::InputField::submitted, [=] {
		if (ParseAmountString(amount->getLastText()).value_or(0) <= 0) {
			amount->showError();
		} else {
			comment->setFocus();
		}
	});
	Ui::Connect(comment, &Ui::InputField::submitted, submit);

	const auto replaceGramsTag = [] {
		return rpl::map([=](QString &&text, const QString &grams) {
			return text.replace("{grams}", grams);
		});
	};

	auto text = rpl::single(
		rpl::empty_value()
	) | rpl::then(base::qt_signal_producer(
		amount,
		&Ui::InputField::changed
	)) | rpl::map([=]() -> rpl::producer<QString> {
		const auto text = amount->getLastText();
		const auto value = ParseAmountString(text).value_or(0);
		return (value > 0)
			? rpl::combine(
				ph::lng_wallet_send_button_amount(),
				ph::lng_wallet_grams_count(ParseAmount(value).full)()
			) | replaceGramsTag()
			: ph::lng_wallet_send_button();
	}) | rpl::flatten_latest();

	box->addButton(
		std::move(text),
		submit,
		st::walletBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
}

} // namespace Wallet
