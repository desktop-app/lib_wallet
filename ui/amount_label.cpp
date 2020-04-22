// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/amount_label.h"

#include "wallet/wallet_common.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Ui {
namespace {

using Wallet::FormattedAmount;

rpl::producer<QString> LargeText(rpl::producer<FormattedAmount> amount) {
	return std::move(
		amount
	) | rpl::map([](const FormattedAmount &amount) {
		return amount.gramsString;
	});
}

rpl::producer<QString> SmallText(rpl::producer<FormattedAmount> amount) {
	return std::move(
		amount
	) | rpl::map([](const FormattedAmount &amount) {
		return amount.separator + amount.nanoString;
	});
}

} // namespace

AmountLabel::AmountLabel(
	not_null<QWidget*> parent,
	rpl::producer<FormattedAmount> amount,
	const style::WalletAmountLabel &st)
: _st(st)
, _large(parent, LargeText(rpl::duplicate(amount)), st.large)
, _small(parent, SmallText(rpl::duplicate(amount)), st.small)
, _diamond(!st.diamond
	? nullptr
	: std::make_unique<LottieAnimation>(
		parent,
		LottieFromResource("diamond"))) {
	if (_diamond) {
		_diamond->start();
	}
	_large.show();
	_small.show();
}

AmountLabel::~AmountLabel() = default;

rpl::producer<int> AmountLabel::widthValue() const {
	using namespace rpl::mappers;
	return rpl::combine(
		_large.widthValue(),
		_small.widthValue(),
		_1 + _2 + (_diamond ? (_st.diamond + _st.diamondPosition.x()) : 0));
}

int AmountLabel::height() const {
	return _large.height();
}

void AmountLabel::move(int x, int y) {
	_large.move(x, y);
	x += _large.width();
	_small.move(
		x,
		y + _st.large.style.font->ascent - _st.small.style.font->ascent);
	x += _small.width();
	if (_diamond) {
		const auto size = QSize(_st.diamond, _st.diamond);
		_diamond->setGeometry(
			{ QPoint(x, y) + _st.diamondPosition, size });
	}
}

rpl::lifetime &AmountLabel::lifetime() {
	return _lifetime;
}

} // namespace Ui
