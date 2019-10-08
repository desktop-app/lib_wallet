// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_info.h"

#include "wallet/wallet_phrases.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/buttons.h"
#include "styles/style_wallet.h"

namespace Wallet {
namespace {

constexpr auto kOneGram = 1'000'000'000;
constexpr auto kNanoDigits = 9;

QString AmountToString(int64 amount) {
	const auto grams = amount / kOneGram;
	auto nanos = amount % kOneGram;
	auto zeros = 0;
	while (zeros + 1 < kNanoDigits && nanos % 10 == 0) {
		nanos /= 10;
		++zeros;
	}
	return QString("%1.%2"
	).arg(grams
	).arg(nanos, kNanoDigits - zeros, 10, QChar('0'));
}

} // namespace

Info::Info(not_null<QWidget*> parent, Data data)
: _widget(std::make_unique<Ui::RpWidget>(parent)) {
	setupControls(std::move(data));
	_widget->show();
}

void Info::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
}

rpl::producer<Info::Action> Info::actionRequests() const {
	return _actionRequests.events();
}

void Info::setupControls(Data &&data) {
	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		_widget.get(),
		rpl::single(data.address));
	title->setSelectable(true);
	const auto description = Ui::CreateChild<Ui::FlatLabel>(
		_widget.get(),
		std::move(data.balance) | rpl::map(AmountToString));
	const auto next = Ui::CreateChild<Ui::RoundButton>(
		_widget.get(),
		rpl::single(QString("Refresh")),
		st::walletNextButton);

	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		title->move((size.width() - title->width()) / 2, size.height() / 4);
		description->move(size.width() / 10, size.height() / 2);
		next->move((size.width() - next->width()) / 2, (size.height() * 3) / 4);
	}, _widget->lifetime());

	next->setClickedCallback([=] {
		_actionRequests.fire(Action::Refresh);
	});
}

rpl::lifetime &Info::lifetime() {
	return _widget->lifetime();
}

} // namespace Wallet
