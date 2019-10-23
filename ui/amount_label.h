// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/widgets/labels.h"

namespace style {
struct WalletAmountLabel;
} // namespace style

namespace Wallet {
struct ParsedAmount;
} // namespace Wallet

namespace Ui {

class LottieAnimation;

class AmountLabel final {
public:
	AmountLabel(
		not_null<QWidget*> parent,
		rpl::producer<Wallet::ParsedAmount> amount,
		const style::WalletAmountLabel &st);
	~AmountLabel();

	rpl::producer<int> widthValue() const;
	int height() const;
	void move(int x, int y);

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	const style::WalletAmountLabel &_st;
	Ui::FlatLabel _large;
	Ui::FlatLabel _small;
	const std::unique_ptr<Ui::LottieAnimation> _diamond;

	rpl::lifetime _lifetime;

};

} // namespace Ui
