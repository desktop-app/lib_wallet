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
#include "ui/widgets/input_fields.h"
#include "base/qthelp_url.h"
#include "styles/style_wallet.h"

#include <QtCore/QLocale>

namespace Wallet {
namespace {

constexpr auto kOneGram = 1'000'000'000;
constexpr auto kNanoDigits = 9;

struct FixedAmount {
	QString text;
	int position = 0;
};

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

[[nodiscard]] FixedAmount FixAmountInput(
		const QString &was,
		const QString &text,
		int position) {
	constexpr auto kMaxDigitsCount = 9;
	const auto separator = ParseAmount(1).separator;

	auto result = FixedAmount{ text, position };
	if (text.isEmpty()) {
		return result;
	} else if (text.startsWith('.')
		|| text.startsWith(',')
		|| text.startsWith(separator)) {
		result.text.prepend('0');
		++result.position;
	}
	auto separatorFound = false;
	auto digitsCount = 0;
	for (auto i = 0; i != result.text.size();) {
		const auto ch = result.text[i];
		const auto atSeparator = result.text.midRef(i).startsWith(separator);
		if (ch >= '0' && ch <= '9' && digitsCount < kMaxDigitsCount) {
			++i;
			++digitsCount;
			continue;
		} else if (!separatorFound
			&& (atSeparator || ch == '.' || ch == ',')) {
			separatorFound = true;
			if (!atSeparator) {
				result.text.replace(i, 1, separator);
			}
			digitsCount = 0;
			i += separator.size();
			continue;
		}
		result.text.remove(i, 1);
		if (result.position > i) {
			--result.position;
		}
	}
	if (result.text == "0" && result.position > 0) {
		if (was.startsWith('0')) {
			result.text = QString();
			result.position = 0;
		} else {
			result.text += separator;
			result.position += separator.size();
		}
	}
	return result;
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
	const auto separator = QLocale::system().decimalPoint();
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

PreparedInvoice ParseInvoice(QString invoice) {
	const auto prefix = qstr("transfer/");
	auto result = PreparedInvoice();

	const auto position = invoice.indexOf(prefix, 0, Qt::CaseInsensitive);
	if (position >= 0) {
		invoice = invoice.mid(position + prefix.size());
	}
	const auto paramsPosition = invoice.indexOf('?');
	if (paramsPosition >= 0) {
		const auto params = qthelp::url_parse_params(
			invoice.mid(paramsPosition + 1),
			qthelp::UrlParamNameTransform::ToLower);
		result.amount = params.value("amount").toULongLong();
		result.comment = params.value("text");
	}
	result.address = invoice.mid(0, paramsPosition).replace(
		QRegularExpression("[^a-zA-Z0-9_\\-]"),
		QString()
	).mid(0, kAddressLength);
	return result;
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
	return !data.outgoing.empty()
		? data.outgoing.front().destination
		: !data.incoming.source.isEmpty()
		? data.incoming.source
		: data.incoming.destination;
}

QString ExtractMessage(const Ton::Transaction &data) {
	const auto &message = data.outgoing.empty()
		? data.incoming.message
		: data.outgoing.front().message;
	if (!message.encrypted.isEmpty()) {
		return "<encrypted text>";
	} else if (message.decrypted) {
		return "<decrypted text>\n\n" + message.text;
	}
	return "<plain text>\n\n" + message.text;
}

QString TransferLink(
		const QString &address,
		int64 amount,
		const QString &comment) {
	const auto base = "ton://transfer/" + address;
	auto params = QStringList();
	if (amount > 0) {
		params.push_back("amount=" + QString::number(amount));
	}
	if (!comment.isEmpty()) {
		params.push_back("text=" + qthelp::url_encode(comment));
	}
	return params.isEmpty()
		? base
		: (base + '?' + params.join('&'));
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

not_null<Ui::InputField*> CreateAmountInput(
		not_null<QWidget*> parent,
		rpl::producer<QString> placeholder,
		int64 amount) {
	const auto result = Ui::CreateChild<Ui::InputField>(
		parent.get(),
		st::walletInput,
		Ui::InputField::Mode::SingleLine,
		std::move(placeholder),
		(amount > 0 ? ParseAmount(amount).full : QString()));
	const auto lastAmountValue = std::make_shared<QString>();
	Ui::Connect(result, &Ui::InputField::changed, [=] {
		Ui::PostponeCall(result, [=] {
			const auto position = result->textCursor().position();
			const auto now = result->getLastText();
			const auto fixed = FixAmountInput(
				*lastAmountValue,
				now,
				position);
			*lastAmountValue = fixed.text;
			if (fixed.text == now) {
				return;
			}
			result->setText(fixed.text);
			result->setFocusFast();
			result->setCursorPosition(fixed.position);
		});
	});
	return result;
}

not_null<Ui::InputField*> CreateCommentInput(
		not_null<QWidget*> parent,
		rpl::producer<QString> placeholder,
		const QString &value) {
	const auto result = Ui::CreateChild<Ui::InputField>(
		parent.get(),
		st::walletInput,
		Ui::InputField::Mode::MultiLine,
		std::move(placeholder),
		value);
	result->setMaxLength(kMaxCommentLength);
	Ui::Connect(result, &Ui::InputField::changed, [=] {
		Ui::PostponeCall(result, [=] {
			const auto text = result->getLastText();
			const auto utf = text.toUtf8();
			if (utf.size() <= kMaxCommentLength) {
				return;
			}
			const auto position = result->textCursor().position();
			const auto update = [&](const QString &text, int position) {
				result->setText(text);
				result->setCursorPosition(position);
			};
			const auto after = text.midRef(position).toUtf8();
			if (after.size() <= kMaxCommentLength) {
				const auto remove = utf.size() - kMaxCommentLength;
				const auto inutf = text.midRef(0, position).toUtf8().size();
				const auto inserted = utf.mid(inutf - remove, remove);
				auto cut = QString::fromUtf8(inserted).size();
				auto updated = text.mid(0, position - cut)
					+ text.midRef(position);
				while (updated.toUtf8().size() > kMaxCommentLength) {
					++cut;
					updated = text.mid(0, position - cut)
						+ text.midRef(position);
				}
				update(updated, position - cut);
			} else {
				update(after.mid(after.size() - kMaxCommentLength), 0);
			}
		});
	});
	return result;
}

bool IsIncorrectPasswordError(const Ton::Error &error) {
	return error.details.startsWith(qstr("KEY_DECRYPT"));
}

bool IsIncorrectMnemonicError(const Ton::Error &error) {
	return error.details.startsWith(qstr("INVALID_MNEMONIC"))
		|| error.details.startsWith(qstr("NEED_MNEMONIC_PASSWORD"));
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
	result.sendUnencryptedText = invoice.sendUnencryptedText;
	return result;
}

} // namespace Wallet
