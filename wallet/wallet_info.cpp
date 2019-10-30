// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_info.h"

#include "wallet/wallet_top_bar.h"
#include "wallet/wallet_common.h"
#include "wallet/wallet_cover.h"
#include "wallet/wallet_empty_history.h"
#include "wallet/wallet_history.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "styles/style_wallet.h"

#include <QtCore/QDateTime>

namespace Wallet {

Info::Info(not_null<QWidget*> parent, Data data)
: _widget(std::make_unique<Ui::RpWidget>(parent))
, _scroll(
	Ui::CreateChild<Ui::ScrollArea>(_widget.get(), st::walletScrollArea))
, _inner(_scroll->setOwnedWidget(object_ptr<Ui::RpWidget>(_scroll.get()))) {
	setupControls(std::move(data));
	_widget->show();
}

Info::~Info() = default;

void Info::setGeometry(QRect geometry) {
	_widget->setGeometry(geometry);
}

rpl::producer<Action> Info::actionRequests() const {
	return _actionRequests.events();
}

rpl::producer<Ton::TransactionId> Info::preloadRequests() const {
	return _preloadRequests.events();
}

rpl::producer<Ton::Transaction> Info::viewRequests() const {
	return _viewRequests.events();
}

void Info::setupControls(Data &&data) {
	const auto &state = data.state;
	const auto topBar = _widget->lifetime().make_state<TopBar>(
		_widget.get(),
		MakeTopBarState(
			rpl::duplicate(state),
			rpl::duplicate(data.updates),
			_widget->lifetime()));
	topBar->actionRequests(
	) | rpl::start_to_stream(_actionRequests, topBar->lifetime());

	const auto cover = _widget->lifetime().make_state<Cover>(
		_inner.get(),
		MakeCoverState(rpl::duplicate(state), data.justCreated));
	rpl::merge(
		cover->sendRequests() | rpl::map([] { return Action::Send; }),
		cover->receiveRequests() | rpl::map([] { return Action::Receive; })
	) | rpl::start_to_stream(_actionRequests, cover->lifetime());

	auto loaded = std::move(
		data.loaded
	) | rpl::filter([](const Ton::Result<Ton::LoadedSlice> &value) {
		return value.has_value();
	}) | rpl::map([](Ton::Result<Ton::LoadedSlice> &&value) {
		return std::move(*value);
	});
	const auto history = _widget->lifetime().make_state<History>(
		_inner.get(),
		MakeHistoryState(rpl::duplicate(state)),
		std::move(loaded));
	const auto emptyHistory = _widget->lifetime().make_state<EmptyHistory>(
		_inner.get(),
		MakeEmptyHistoryState(rpl::duplicate(state), data.justCreated));

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
