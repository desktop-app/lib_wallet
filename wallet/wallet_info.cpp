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
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "styles/style_wallet.h"

#include <QtCore/QDateTime>

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

QString DateToString(const QDateTime &timestamp) {
	return QString("%1.%2.%3 %4:%5:%6"
	).arg(timestamp.date().day(), 2, 10, QChar('0')
	).arg(timestamp.date().month(), 2, 10, QChar('0')
	).arg(timestamp.date().year()
	).arg(timestamp.time().hour(), 2, 10, QChar('0')
	).arg(timestamp.time().minute(), 2, 10, QChar('0')
	).arg(timestamp.time().second(), 2, 10, QChar('0'));
}

QString ConcatenateData(int64 amount, Ton::TransactionsSlice slice) {
	auto result = "Balance: " + AmountToString(amount);
	for (const auto &transaction : slice.list) {
		result += "\n\n";
		const auto value = transaction.incoming.value
			- ranges::accumulate(
				transaction.outgoing,
				int64(0),
				ranges::plus(),
				&Ton::Message::value);
		result += (value > 0 ? '+' : '-') + AmountToString(value);
		if (value > 0 && !transaction.incoming.source.isEmpty()) {
			result += " from " + transaction.incoming.source;
		} else if (value < 0
			&& !transaction.outgoing.empty()
			&& !transaction.outgoing.front().destination.isEmpty()) {
			result += " to " + transaction.outgoing.front().destination;
		}
		result += "\n" + DateToString(
			QDateTime::fromTime_t(transaction.time));
		if (transaction.fee) {
			result += " (fee: " + AmountToString(transaction.fee) + ")";
		}
		if (!transaction.incoming.message.isEmpty()) {
			result += "\nComment: " + transaction.incoming.message;
		} else if (!transaction.outgoing.empty()
			&& !transaction.outgoing.front().message.isEmpty()) {
			result += "\nComment: " + transaction.outgoing.front().message;
		}
	}
	return result;
}

} // namespace

Info::Info(not_null<QWidget*> parent, Data data)
: _widget(std::make_unique<Ui::RpWidget>(parent))
, _scroll(Ui::CreateChild<Ui::ScrollArea>(_widget.get()))
, _inner(_scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll.get()))) {
	setupControls(std::move(data));
	_widget->show();
}

void Info::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
	_scroll->setGeometry({ QPoint(), geometry.size() });
	_inner->resizeToWidth(geometry.width());
}

rpl::producer<Info::Action> Info::actionRequests() const {
	return _actionRequests.events();
}

void Info::setupControls(Data &&data) {
	const auto title = Ui::CreateChild<Ui::FlatLabel>(
		_inner.get(),
		rpl::single(data.address));
	title->setSelectable(true);
	const auto description = Ui::CreateChild<Ui::FlatLabel>(
		_inner.get(),
		rpl::combine(
			std::move(data.balance),
			std::move(data.lastTransactions)
		) | rpl::map(ConcatenateData));
	const auto next = Ui::CreateChild<Ui::RoundButton>(
		_inner.get(),
		rpl::single(QString("Refresh")),
		st::walletNextButton);

	rpl::combine(
		_inner->widthValue(),
		description->widthValue()
	) | rpl::start_with_next([=](int width, int descriptionWidth) {
		title->move((width - title->width()) / 2, width / 10);
		description->move((width - descriptionWidth) / 2, width / 5);
		const auto bottom = description->y() + description->height();
		next->move((width - next->width()) / 2, bottom + width / 10);
		_inner->resize(width, next->y() + next->height() + width / 10);
	}, _inner->lifetime());

	next->setClickedCallback([=] {
		_actionRequests.fire(Action::Refresh);
	});
}

rpl::lifetime &Info::lifetime() {
	return _widget->lifetime();
}

} // namespace Wallet
