// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/rp_widget.h"

namespace Ton {
struct WalletViewerState;
} // namespace Ton

namespace Wallet {

struct TopBarState {
	QString text;
	bool refreshing = false;
};

class TopBar final {
public:
	TopBar(not_null<Ui::RpWidget*> parent, rpl::producer<TopBarState> state);

	[[nodiscard]] rpl::producer<> refreshRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls(rpl::producer<TopBarState> &&state);

	Ui::RpWidget _widget;
	rpl::event_stream<> _refreshRequests;

};

[[nodiscard]] rpl::producer<TopBarState> MakeTopBarState(
	rpl::producer<Ton::WalletViewerState> &&state,
	rpl::lifetime &alive);

} // namespace Wallet
