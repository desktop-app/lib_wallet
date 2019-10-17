// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_common.h"

#include "wallet/wallet_send_grams.h"
#include "ton/ton_state.h"
#include "ton/ton_result.h"
#include "ui/layers/generic_box.h"
#include "styles/style_wallet.h"

#include <QtCore/QLocale>

namespace Wallet {
namespace {

constexpr auto kOneGram = 1'000'000'000;
constexpr auto kNanoDigits = 9;

std::optional<int64> ParseAmountGrams(const QString &trimmed) {
	auto ok = false;
	const auto grams = int64(trimmed.toLongLong(&ok));
	return (ok
		&& (grams <= std::numeric_limits<int64>::max() / kOneGram)
		&& (grams >= std::numeric_limits<int64>::min() / kOneGram))
		? std::make_optional(grams * kOneGram)
		: std::nullopt;
}

std::optional<int64> ParseAmountNano(QString trimmed) {
	while (trimmed.size() < kNanoDigits) {
		trimmed.append('0');
	}
	auto zeros = 0;
	for (const auto ch : trimmed) {
		if (ch == '0') {
			++zeros;
		} else {
			break;
		}
	}
	if (zeros == trimmed.size()) {
		return 0;
	} else if (trimmed.size() > kNanoDigits) {
		return std::nullopt;
	}
	auto ok = false;
	const auto value = trimmed.mid(zeros).toLongLong(&ok);
	return (ok && value > 0 && value < kOneGram)
		? std::make_optional(value)
		: std::nullopt;
}

} // namespace

ParsedAmount ParseAmount(int64 amount, bool isSigned) {
	auto result = ParsedAmount();
	result.grams = amount / kOneGram;
	auto nanos = result.nano = std::abs(amount) % kOneGram;
	auto zeros = 0;
	while (zeros < kNanoDigits && nanos % 10 == 0) {
		nanos /= 10;
		++zeros;
	}
	const auto &locale = QLocale::system();
	const auto separator = locale.toString(0.1f
	).replace('0', QString()
	).replace('1', QString());
	result.full = result.gramsString = QString::number(result.grams);
	if (isSigned && amount > 0) {
		result.full = result.gramsString = '+' + result.gramsString;
	} else if (amount < 0 && result.grams == 0) {
		result.full = result.gramsString = '-' + result.gramsString;
	}
	if (zeros < kNanoDigits) {
		result.separator = separator;
		result.nanoString = QString("%1"
		).arg(nanos, kNanoDigits - zeros, 10, QChar('0'));
		result.full += separator + result.nanoString;
	}
	return result;
}

std::optional<int64> ParseAmountString(const QString &amount) {
	const auto trimmed = amount.trimmed();
	const auto &locale = QLocale::system();
	const auto separator = locale.toString(0.1f
	).replace('0', QString()
	).replace('1', QString());
	const auto index1 = trimmed.indexOf('.');
	const auto index2 = trimmed.indexOf(',');
	const auto index3 = (separator == "." || separator == ",")
		? -1
		: trimmed.indexOf(separator);
	const auto found = (index1 >= 0 ? 1 : 0)
		+ (index2 >= 0 ? 1 : 0)
		+ (index3 >= 0 ? 1 : 0);
	if (found > 1) {
		return std::nullopt;
	}
	const auto index = (index1 >= 0)
		? index1
		: (index2 >= 0)
		? index2
		: index3;
	const auto used = (index1 >= 0)
		? "."
		: (index2 >= 0)
		? ","
		: separator;
	const auto grams = ParseAmountGrams(trimmed.mid(0, index));
	const auto nano = ParseAmountNano(trimmed.mid(index + used.size()));
	if (index < 0 || index == trimmed.size() - used.size()) {
		return grams;
	} else if (index == 0) {
		return nano;
	} else if (!nano || !grams) {
		return std::nullopt;
	}
	return *grams + (*grams < 0 ? (-*nano) : (*nano));
}

int64 CalculateValue(const Ton::Transaction &data) {
	const auto outgoing = ranges::accumulate(
		data.outgoing,
		int64(0),
		ranges::plus(),
		&Ton::Message::value);
	return data.incoming.value - outgoing;
}

QString ExtractAddress(const Ton::Transaction &data) {
	return data.outgoing.empty()
		? data.incoming.source
		: data.outgoing.front().destination;
}

QString ExtractMessage(const Ton::Transaction &data) {
	return data.outgoing.empty()
		? data.incoming.message
		: data.outgoing.front().message;
}

QString TransferLink(const QString &address) {
	return "ton://transfer/" + address;
}

not_null<Ui::FlatLabel*> AddBoxSubtitle(
		not_null<Ui::GenericBox*> box,
		rpl::producer<QString> text) {
	return box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			std::move(text),
			st::walletSubsectionTitle),
		st::walletSubsectionTitlePadding);
}

bool IsIncorrectPasswordError(const Ton::Error &error) {
	return error.details.startsWith(qstr("KEY_DECRYPT"));
}

std::optional<Wallet::InvoiceField> ErrorInvoiceField(
		const Ton::Error &error) {
	const auto text = error.details;
	if (text.startsWith(qstr("NOT_ENOUGH_FUNDS"))) {
		return InvoiceField::Amount;
	} else if (text.startsWith(qstr("MESSAGE_TOO_LONG"))) {
		return InvoiceField::Comment;
	} else if (text.startsWith(qstr("INVALID_ACCOUNT_ADDRESS"))) {
		return InvoiceField::Address;
	}
	return std::nullopt;
}

Ton::TransactionToSend TransactionFromInvoice(
		const PreparedInvoice &invoice) {
	auto result = Ton::TransactionToSend();
	result.recipient = invoice.address;
	result.amount = invoice.amount;
	result.comment = invoice.comment;
	result.allowSendToUninited = true;
	return result;
}

} // namespace Wallet
