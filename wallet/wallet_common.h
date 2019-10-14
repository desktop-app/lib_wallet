// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Wallet {

struct ParsedAmount {
	int64 grams = 0;
	int64 nano = 0;
	QString gramsString;
	QString separator;
	QString nanoString;
};

[[nodiscard]] ParsedAmount ParseAmount(int64 amount);
std::optional<int64> ParseAmountString(const QString &amount);

} // namespace Wallet
