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

struct CoverState {
	int64 unlockedBalance = 0;
	int64 lockedBalance = 0;
	bool justCreated = false;
	bool useTestNetwork = false;
};

class Cover final {
public:
	Cover(not_null<Ui::RpWidget*> parent, rpl::producer<CoverState> state);

	void setGeometry(QRect geometry);
	[[nodiscard]] int height() const;

	[[nodiscard]] rpl::producer<> sendRequests() const;
	[[nodiscard]] rpl::producer<> receiveRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls();
	void setupBalance();

	Ui::RpWidget _widget;

	rpl::variable<CoverState> _state;
	rpl::event_stream<> _sendRequests;
	rpl::event_stream<> _receiveRequests;

};

[[nodiscard]] rpl::producer<CoverState> MakeCoverState(
	rpl::producer<Ton::WalletViewerState> state,
	bool justCreated,
	bool useTestNetwork);

} // namespace Wallet
