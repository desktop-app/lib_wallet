// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ton/ton_state.h"

namespace Ui {
class RpWidget;
class ScrollArea;
} // namespace Ui

namespace Wallet {

class Info final {
public:
	struct Data {
		QString address;
		rpl::producer<int64> balance;
		rpl::producer<Ton::TransactionsSlice> lastTransactions;
	};
	enum class Action {
		Refresh,
		Send,
		ChangePassword,
		LogOut,
	};
	Info(not_null<QWidget*> parent, Data data);

	void setGeometry(QRect geometry);

	[[nodiscard]] rpl::producer<Action> actionRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls(Data &&data);

	const std::unique_ptr<Ui::RpWidget> _widget;
	const not_null<Ui::ScrollArea*> _scroll;
	const not_null<Ui::RpWidget*> _inner;

	rpl::event_stream<Action> _actionRequests;

};

} // namespace Wallet
