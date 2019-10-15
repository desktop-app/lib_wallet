// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_info.h"

#include "wallet/wallet_top_bar.h"
#include "wallet/wallet_cover.h"
#include "wallet/wallet_empty_history.h"
#include "wallet/wallet_history.h"
#include "wallet/wallet_common.h"
#include "ui/rp_widget.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "styles/style_wallet.h"

#include <QtCore/QDateTime>

namespace Wallet {
namespace {

QString AmountToString(int64 amount) {
	const auto parsed = ParseAmount(amount);
	return parsed.gramsString + parsed.separator + parsed.nanoString;
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

QString PrintAddress(const Ton::WalletViewerState &state) {
	return state.wallet.address;
}

QString PrintData(const Ton::WalletViewerState &full) {
	const auto &state = full.wallet;
	auto result = "Balance: " + AmountToString(
		std::max(state.account.balance, 0LL));
	for (const auto &transaction : state.lastTransactions.list) {
		result += "\n\n";
		const auto value = transaction.incoming.value
			- ranges::accumulate(
				transaction.outgoing,
				int64(0),
				ranges::plus(),
				&Ton::Message::value);
		if (value) {
			result += (value > 0 ? '+' : '-') + AmountToString(value);
		} else {
			result += AmountToString(0);
		}
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
, _data(std::move(data))
, _scroll(
	Ui::CreateChild<Ui::ScrollArea>(_widget.get(), st::walletScrollArea))
, _inner(_scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll.get()))) {
	setupControls();
	_widget->show();
}

Info::~Info() = default;

void Info::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
}

rpl::producer<Info::Action> Info::actionRequests() const {
	return _actionRequests.events();
}

rpl::producer<Ton::TransactionId> Info::preloadRequests() const {
	return _preloadRequests.events();
}

rpl::producer<Ton::Transaction> Info::viewRequests() const {
	return _viewRequests.events();
}

void Info::setupControls() {
	const auto &state = _data.state;
	const auto topBar = _widget->lifetime().make_state<TopBar>(
		_widget.get(),
		MakeTopBarState(rpl::duplicate(state), _widget->lifetime()));
	topBar->refreshRequests(
	) | rpl::map([] {
		return Action::Refresh;
	}) | rpl::start_to_stream(_actionRequests, topBar->lifetime());

	const auto cover = _widget->lifetime().make_state<Cover>(
		_inner.get(),
		MakeCoverState(rpl::duplicate(state)));
	rpl::merge(
		cover->sendRequests() | rpl::map([] { return Action::Send; }),
		cover->receiveRequests() | rpl::map([] { return Action::Receive; })
	) | rpl::start_to_stream(_actionRequests, cover->lifetime());

	const auto history = _widget->lifetime().make_state<History>(
		_inner.get(),
		MakeHistoryState(rpl::duplicate(state)),
		rpl::duplicate(_data.loaded));
	const auto emptyHistory = _widget->lifetime().make_state<EmptyHistory>(
		_inner.get(),
		MakeEmptyHistoryState(rpl::duplicate(state)));

	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_scroll->setGeometry(QRect(
			QPoint(),
			size
		).marginsRemoved({ 0, st::walletTopBarHeight, 0, 0 }));
	}, _scroll->lifetime());

	_scroll->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto coverHeight = (size.height() + st::walletTopBarHeight) / 2
			- st::walletTopBarHeight;
		cover->setGeometry(QRect(0, 0, size.width(), coverHeight));
		const auto contentGeometry = QRect(
			0,
			coverHeight,
			size.width(),
			size.height() - coverHeight);
		history->updateGeometry({ 0, coverHeight }, size.width());
		emptyHistory->setGeometry(contentGeometry);
	}, cover->lifetime());

	rpl::combine(
		_scroll->sizeValue(),
		history->heightValue()
	) | rpl::start_with_next([=](QSize size, int height) {
		const auto innerHeight = std::max(
			size.height(),
			cover->height() + height);
		_inner->setGeometry({ 0, 0, size.width(), innerHeight });
		emptyHistory->setVisible(height == 0);
	}, _inner->lifetime());

	rpl::combine(
		_scroll->scrollTopValue(),
		_scroll->heightValue()
	) | rpl::start_with_next([=](int scrollTop, int scrollHeight) {
		history->setVisibleTopBottom(scrollTop, scrollTop + scrollHeight);
	}, history->lifetime());

	history->preloadRequests(
	) | rpl::start_to_stream(_preloadRequests, history->lifetime());

	history->viewRequests(
	) | rpl::start_to_stream(_viewRequests, history->lifetime());
}

rpl::lifetime &Info::lifetime() {
	return _widget->lifetime();
}

} // namespace Wallet
