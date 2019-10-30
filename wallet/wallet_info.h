// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ton/ton_state.h"
#include "ton/ton_result.h"

namespace Ui {
class RpWidget;
class ScrollArea;
} // namespace Ui

namespace Wallet {

enum class Action;

class Info final {
public:
	struct Data {
		bool justCreated = false;
		rpl::producer<Ton::WalletViewerState> state;
		rpl::producer<Ton::Result<Ton::LoadedSlice>> loaded;
		rpl::producer<Ton::Update> updates;
	};
	Info(not_null<QWidget*> parent, Data data);
	~Info();

	void setGeometry(QRect geometry);

	[[nodiscard]] rpl::producer<Action> actionRequests() const;
	[[nodiscard]] rpl::producer<Ton::TransactionId> preloadRequests() const;
	[[nodiscard]] rpl::producer<Ton::Transaction> viewRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls(Data &&data);

	const std::unique_ptr<Ui::RpWidget> _widget;
	const not_null<Ui::ScrollArea*> _scroll;
	const not_null<Ui::RpWidget*> _inner;

	rpl::event_stream<Action> _actionRequests;
	rpl::event_stream<Ton::TransactionId> _preloadRequests;
	rpl::event_stream<Ton::Transaction> _viewRequests;

};

} // namespace Wallet
