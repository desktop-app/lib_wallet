// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/rp_widget.h"
#include "ton/ton_state.h"

class Painter;

namespace Wallet {

struct HistoryState {
	Ton::TransactionsSlice lastTransactions;
	std::vector<Ton::PendingTransaction> pendingTransactions;
};

class HistoryRow;

class History final {
public:
	History(
		not_null<Ui::RpWidget*> parent,
		rpl::producer<HistoryState> state,
		rpl::producer<Ton::LoadedSlice> loaded);
	~History();

	void updateGeometry(QPoint position, int width);
	[[nodiscard]] rpl::producer<int> heightValue() const;
	void setVisibleTopBottom(int top, int bottom);

	[[nodiscard]] rpl::producer<Ton::TransactionId> preloadRequests() const;
	[[nodiscard]] rpl::producer<Ton::Transaction> viewRequests() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	struct ScrollState {
		Ton::TransactionId top;
		int offset = 0;
	};

	void setupContent(
		rpl::producer<HistoryState> &&state,
		rpl::producer<Ton::LoadedSlice> &&loaded);
	void resizeToWidth(int width);
	void mergeState(HistoryState &&state);
	bool mergePendingChanged(std::vector<Ton::PendingTransaction> &&list);
	bool mergeListChanged(Ton::TransactionsSlice &&data);
	void refresh();
	void paint(Painter &p, QRect clip);
	[[nodiscard]] ScrollState computeScrollState() const;

	void selectRow(int selected);
	void selectRowByMouse();
	void pressRow();
	void releaseRow();

	Ui::RpWidget _widget;

	std::vector<Ton::PendingTransaction> _pendingData;
	std::vector<Ton::Transaction> _listData;
	Ton::TransactionId _previousId;

	std::vector<std::unique_ptr<HistoryRow>> _rows;
	int _visibleTop = 0;
	int _visibleBottom = 0;
	int _selected = -1;
	int _pressed = -1;

	rpl::event_stream<Ton::TransactionId> _preloadRequests;
	rpl::event_stream<Ton::Transaction> _viewRequests;

};

[[nodiscard]] rpl::producer<HistoryState> MakeHistoryState(
	rpl::producer<Ton::WalletViewerState> state);

} // namespace Wallet
