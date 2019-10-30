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

struct EmptyHistoryState {
	QString address;
	bool justCreated = false;
};

class EmptyHistory final {
public:
	EmptyHistory(
		not_null<Ui::RpWidget*> parent,
		rpl::producer<EmptyHistoryState> state);

	void setGeometry(QRect geometry);
	void setVisible(bool visible);

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void setupControls(rpl::producer<EmptyHistoryState> &&state);

	Ui::RpWidget _widget;

};

[[nodiscard]] rpl::producer<EmptyHistoryState> MakeEmptyHistoryState(
	rpl::producer<Ton::WalletViewerState> state,
	bool justCreated);

} // namespace Wallet
