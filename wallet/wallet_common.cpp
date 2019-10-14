// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_common.h"

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

ParsedAmount ParseAmount(int64 amount) {
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
	result.gramsString = QString::number(result.grams);
	if (zeros < kNanoDigits) {
		result.separator = separator;
		result.nanoString = QString("%1"
		).arg(nanos, kNanoDigits - zeros, 10, QChar('0'));
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

} // namespace Wallet
