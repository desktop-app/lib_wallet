// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Ton {
struct Transaction;
} // namespace Ton

namespace Ui {
class GenericBox;
} // namespace Ui

namespace Wallet {

struct ParsedAmount {
	int64 grams = 0;
	int64 nano = 0;
	QString gramsString;
	QString separator;
	QString nanoString;
	QString full;
};

[[nodiscard]] ParsedAmount ParseAmount(int64 amount, bool isSigned = false);
[[nodiscard]] std::optional<int64> ParseAmountString(const QString &amount);
[[nodiscard]] int64 CalculateValue(const Ton::Transaction &data);
[[nodiscard]] QString ExtractAddress(const Ton::Transaction &data);
[[nodiscard]] QString ExtractMessage(const Ton::Transaction &data);

[[nodiscard]] QString TransferLink(const QString &address);

void AddBoxSubtitle(
	not_null<Ui::GenericBox*> box,
	rpl::producer<QString> text);

} // namespace Wallet
