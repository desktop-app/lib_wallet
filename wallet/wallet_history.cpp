// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_history.h"

#include "wallet/wallet_common.h"
#include "wallet/wallet_phrases.h"
#include "base/unixtime.h"
#include "ui/address_label.h"
#include "ui/inline_diamond.h"
#include "ui/painter.h"
#include "ui/text/text.h"
#include "ui/text/text_utilities.h"
#include "ui/effects/animations.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

#include <QtCore/QDateTime>

namespace Wallet {
namespace {

constexpr auto kPreloadScreens = 3;
constexpr auto kCommentLinesMax = 3;

struct TransactionLayout {
	TimeId serverTime = 0;
	QDateTime dateTime;
	Ui::Text::String date;
	Ui::Text::String time;
	Ui::Text::String amountGrams;
	Ui::Text::String amountNano;
	Ui::Text::String address;
	Ui::Text::String comment;
	Ui::Text::String fees;
	ClickHandlerPtr decryptLink;
	int addressWidth = 0;
	int addressHeight = 0;
	bool incoming = false;
	bool pending = false;
};

[[nodiscard]] const style::TextStyle &AddressStyle() {
	const static auto result = Ui::ComputeAddressStyle(st::defaultTextStyle);
	return result;
}

void RefreshTimeTexts(
		TransactionLayout &layout,
		bool forceDateText = false) {
	layout.dateTime = base::unixtime::parse(layout.serverTime);
	layout.time.setText(
		st::defaultTextStyle,
		ph::lng_wallet_short_time(layout.dateTime.time())(ph::now));
	if (layout.date.isEmpty() && !forceDateText) {
		return;
	}
	if (layout.pending) {
		layout.date.setText(
			st::semiboldTextStyle,
			ph::lng_wallet_row_pending_date(ph::now));
	} else {
		layout.date.setText(
			st::semiboldTextStyle,
			ph::lng_wallet_short_date(layout.dateTime.date())(ph::now));
	}
}

[[nodiscard]] TransactionLayout PrepareLayout(
		const Ton::Transaction &data,
		Fn<void()> decrypt) {
	auto result = TransactionLayout();
	result.serverTime = data.time;
	const auto amount = ParseAmount(CalculateValue(data), true);
	result.amountGrams.setText(st::walletRowGramsStyle, amount.gramsString);
	result.amountNano.setText(
		st::walletRowNanoStyle,
		amount.separator + amount.nanoString);
	const auto address = ExtractAddress(data);
	const auto addressPartWidth = [&](int from, int length = -1) {
		return AddressStyle().font->width(address.mid(from, length));
	};
	result.address = Ui::Text::String(
		AddressStyle(),
		address,
		_defaultOptions,
		st::walletAddressWidthMin);
	result.addressWidth = (AddressStyle().font->spacew / 2) + std::max(
		addressPartWidth(0, address.size() / 2),
		addressPartWidth(address.size() / 2));
	result.addressHeight = AddressStyle().font->height * 2;
	result.comment = Ui::Text::String(st::walletAddressWidthMin);
	const auto encrypted = IsEncryptedMessage(data) && decrypt;
	result.comment.setMarkedText(
		st::defaultTextStyle,
		(encrypted
			? Ui::Text::Link(ph::lng_wallet_click_to_decrypt(ph::now))
			: Ui::Text::WithEntities(ExtractMessage(data))),
		_textPlainOptions);
	if (encrypted) {
		result.decryptLink = std::make_shared<LambdaClickHandler>(decrypt);
		result.comment.setLink(1, result.decryptLink);
	}
	if (data.fee) {
		const auto fee = ParseAmount(data.fee).full;
		result.fees.setText(
			st::defaultTextStyle,
			ph::lng_wallet_row_fees(ph::now).replace("{amount}", fee));
	}
	result.incoming = !data.incoming.source.isEmpty();
	result.pending = (data.id.lt == 0);

	RefreshTimeTexts(result);
	return result;
}

} // namespace

class HistoryRow final {
public:
	HistoryRow(const Ton::Transaction &transaction, Fn<void()> decrypt);
	HistoryRow(const HistoryRow &) = delete;
	HistoryRow &operator=(const HistoryRow &) = delete;

	[[nodiscard]] Ton::TransactionId id() const;

	void refreshDate();
	[[nodiscard]] QDateTime date() const;
	void setShowDate(bool show, Fn<void()> repaintDate);
	void setDecryptionFailed();
	bool showDate() const;

	[[nodiscard]] int top() const;
	void setTop(int top);

	void resizeToWidth(int width);
	[[nodiscard]] int height() const;
	[[nodiscard]] int bottom() const;

	void paint(Painter &p, int x, int y);
	void paintDate(Painter &p, int x, int y);
	[[nodiscard]] bool isUnderCursor(QPoint point) const;
	[[nodiscard]] ClickHandlerPtr handlerUnderCursor(QPoint point) const;

private:
	[[nodiscard]] QRect computeInnerRect() const;

	Ton::TransactionId _id;
	TransactionLayout _layout;
	int _top = 0;
	int _width = 0;
	int _height = 0;
	int _commentHeight = 0;

	Ui::Animations::Simple _dateShadowShown;
	Fn<void()> _repaintDate;
	bool _dateHasShadow = false;
	bool _decryptionFailed = false;

};

HistoryRow::HistoryRow(
	const Ton::Transaction &transaction,
	Fn<void()> decrypt)
: _id(transaction.id)
, _layout(PrepareLayout(transaction, decrypt)) {
}

Ton::TransactionId HistoryRow::id() const {
	return _id;
}

void HistoryRow::refreshDate() {
	RefreshTimeTexts(_layout);
}

QDateTime HistoryRow::date() const {
	return _layout.dateTime;
}

void HistoryRow::setShowDate(bool show, Fn<void()> repaintDate) {
	_width = 0;
	if (!show) {
		_layout.date.clear();
	} else {
		_repaintDate = std::move(repaintDate);
		RefreshTimeTexts(_layout, true);
	}
}

void HistoryRow::setDecryptionFailed() {
	_width = 0;
	_decryptionFailed = true;
	_layout.comment.setText(
		st::defaultTextStyle,
		"Decryption failed :(",
		_textPlainOptions);
}

bool HistoryRow::showDate() const {
	return !_layout.date.isEmpty();
}

int HistoryRow::top() const {
	return _top;
}

void HistoryRow::setTop(int top) {
	_top = top;
}

void HistoryRow::resizeToWidth(int width) {
	if (_width == width) {
		return;
	}
	_width = width;
	const auto padding = st::walletRowPadding;
	const auto use = std::min(_width, st::walletRowWidthMax);
	const auto avail = use - padding.left() - padding.right();
	_height = 0;
	if (!_layout.date.isEmpty()) {
		_height += st::walletRowDateSkip;
	}
	_height += padding.top() + _layout.amountGrams.minHeight();
	_height += st::walletRowAddressTop + _layout.addressHeight;
	if (!_layout.comment.isEmpty()) {
		_commentHeight = std::min(
			_layout.comment.countHeight(avail),
			st::defaultTextStyle.font->height * kCommentLinesMax);
		_height += st::walletRowCommentTop + _commentHeight;
	}
	if (!_layout.fees.isEmpty()) {
		_height += st::walletRowFeesTop + _layout.fees.minHeight();
	}
	_height += padding.bottom();
}

int HistoryRow::height() const {
	return _height;
}

int HistoryRow::bottom() const {
	return _top + _height;
}

void HistoryRow::paint(Painter &p, int x, int y) {
	const auto padding = st::walletRowPadding;
	const auto use = std::min(_width, st::walletRowWidthMax);
	const auto avail = use - padding.left() - padding.right();
	x += (_width - use) / 2 + padding.left();

	if (!_layout.date.isEmpty()) {
		y += st::walletRowDateSkip;
	} else {
		const auto shadowLeft = (use < _width)
			? (x - st::walletRowShadowAdd)
			: x;
		const auto shadowWidth = (use < _width)
			? (avail + 2 * st::walletRowShadowAdd)
			: _width - padding.left();
		p.fillRect(shadowLeft, y, shadowWidth, st::lineWidth, st::shadowFg);
	}
	y += padding.top();

	p.setPen(_layout.incoming ? st::boxTextFgGood : st::boxTextFgError);
	_layout.amountGrams.draw(p, x, y, avail);

	const auto nanoTop = y
		+ st::walletRowGramsStyle.font->ascent
		- st::walletRowNanoStyle.font->ascent;
	const auto nanoLeft = x + _layout.amountGrams.maxWidth();
	_layout.amountNano.draw(p, nanoLeft, nanoTop, avail);

	const auto diamondTop = y
		+ st::walletRowGramsStyle.font->ascent
		- st::normalFont->ascent;
	const auto diamondLeft = nanoLeft
		+ _layout.amountNano.maxWidth()
		+ st::normalFont->spacew;
	Ui::PaintInlineDiamond(p, diamondLeft, diamondTop, st::normalFont);

	const auto labelTop = diamondTop;
	const auto labelLeft = diamondLeft
		+ st::walletDiamondSize
		+ st::normalFont->spacew;
	p.setPen(st::windowFg);
	p.setFont(st::normalFont);
	p.drawText(
		labelLeft,
		labelTop + st::normalFont->ascent,
		(_layout.incoming
			? ph::lng_wallet_row_from(ph::now)
			: ph::lng_wallet_row_to(ph::now)));

	const auto timeTop = labelTop;
	const auto timeLeft = x + avail - _layout.time.maxWidth();
	p.setPen(st::windowSubTextFg);
	_layout.time.draw(p, timeLeft, timeTop, avail);
	if (_layout.pending) {
		st::walletRowPending.paint(
			p,
			(timeLeft
				- st::walletRowPendingPosition.x()
				- st::walletRowPending.width()),
			timeTop + st::walletRowPendingPosition.y(),
			avail);
	}
	y += _layout.amountGrams.minHeight();

	p.setPen(st::windowFg);
	y += st::walletRowAddressTop;
	_layout.address.drawElided(
		p,
		x,
		y,
		_layout.addressWidth,
		2,
		style::al_topleft,
		0,
		-1,
		0,
		true);
	y += _layout.addressHeight;

	if (!_layout.comment.isEmpty()) {
		y += st::walletRowCommentTop;
		if (_decryptionFailed) {
			p.setPen(st::boxTextFgError);
		}
		_layout.comment.drawElided(p, x, y, avail, kCommentLinesMax);
		y += _commentHeight;
	}
	if (!_layout.fees.isEmpty()) {
		p.setPen(st::windowSubTextFg);
		y += st::walletRowFeesTop;
		_layout.fees.draw(p, x, y, avail);
	}
}

void HistoryRow::paintDate(Painter &p, int x, int y) {
	Expects(!_layout.date.isEmpty());
	Expects(_repaintDate != nullptr);

	const auto hasShadow = (y != top());
	if (_dateHasShadow != hasShadow) {
		_dateHasShadow = hasShadow;
		_dateShadowShown.start(
			_repaintDate,
			hasShadow ? 0. : 1.,
			hasShadow ? 1. : 0.,
			st::widgetFadeDuration);
	}
	const auto line = st::lineWidth;
	const auto noShadowHeight = st::walletRowDateHeight - line;

	if (_dateHasShadow || _dateShadowShown.animating()) {
		p.setOpacity(_dateShadowShown.value(_dateHasShadow ? 1. : 0.));
		p.fillRect(x, y + noShadowHeight, _width, line, st::shadowFg);
	}

	const auto padding = st::walletRowPadding;
	const auto use = std::min(_width, st::walletRowWidthMax);
	x += (_width - use) / 2;

	p.setOpacity(0.9);
	p.fillRect(x, y, use, noShadowHeight, st::windowBg);

	const auto avail = use - padding.left() - padding.right();
	x += padding.left();
	p.setOpacity(1.);
	p.setPen(st::windowFg);
	_layout.date.draw(p, x, y + st::walletRowDateTop, avail);
}

QRect HistoryRow::computeInnerRect() const {
	const auto padding = st::walletRowPadding;
	const auto use = std::min(_width, st::walletRowWidthMax);
	const auto avail = use - padding.left() - padding.right();
	const auto left = (use < _width)
		? ((_width - use) / 2 + padding.left() - st::walletRowShadowAdd)
		: 0;
	const auto width = (use < _width)
		? (avail + 2 * st::walletRowShadowAdd)
		: _width;
	auto y = top();
	if (!_layout.date.isEmpty()) {
		y += st::walletRowDateSkip;
	}
	return QRect(left, y, width, bottom() - y);
}

bool HistoryRow::isUnderCursor(QPoint point) const {
	return computeInnerRect().contains(point);
}

ClickHandlerPtr HistoryRow::handlerUnderCursor(QPoint point) const {
	if (!_layout.decryptLink || _layout.comment.isEmpty()) {
		return nullptr;
	}
	const auto inner = computeInnerRect();
	const auto commentLeft = (inner.width() < _width)
		? (inner.left() + st::walletRowShadowAdd)
		: st::walletRowPadding.left();
	const auto commentTop = inner.top()
		+ st::walletRowPadding.top()
		+ _layout.amountGrams.minHeight()
		+ st::walletRowAddressTop
		+ _layout.addressHeight
		+ st::walletRowCommentTop;
	const auto comment = QRect(
		commentLeft,
		commentTop,
		_layout.comment.maxWidth(),
		_commentHeight);
	return comment.contains(point) ? _layout.decryptLink : nullptr;
}

History::History(
	not_null<Ui::RpWidget*> parent,
	rpl::producer<HistoryState> state,
	rpl::producer<Ton::LoadedSlice> loaded,
	rpl::producer<not_null<std::vector<Ton::Transaction>*>> collectEncrypted,
	rpl::producer<
		not_null<const std::vector<Ton::Transaction>*>> updateDecrypted)
: _widget(parent) {
	setupContent(std::move(state), std::move(loaded));

	base::unixtime::updates(
	) | rpl::start_with_next([=] {
		for (const auto &row : ranges::view::concat(_pendingRows, _rows)) {
			row->refreshDate();
		}
		refreshShowDates();
		_widget.update();
	}, _widget.lifetime());

	std::move(
		collectEncrypted
	) | rpl::start_with_next([=](
			not_null<std::vector<Ton::Transaction>*> list) {
		auto &&encrypted = ranges::view::all(
			_listData
		) | ranges::view::filter(IsEncryptedMessage);
		list->insert(list->end(), encrypted.begin(), encrypted.end());
	}, _widget.lifetime());

	std::move(
		updateDecrypted
	) | rpl::start_with_next([=](
			not_null<const std::vector<Ton::Transaction>*> list) {
		auto changed = false;
		for (auto i = 0, count = int(_listData.size()); i != count; ++i) {
			if (IsEncryptedMessage(_listData[i])) {
				if (takeDecrypted(i, *list)) {
					changed = true;
				}
			}
		}
		if (changed) {
			refreshShowDates();
			_widget.update();
		}
	}, _widget.lifetime());
}

History::~History() = default;

void History::updateGeometry(QPoint position, int width) {
	_widget.move(position);
	resizeToWidth(width);
}

void History::resizeToWidth(int width) {
	if (!width) {
		return;
	}
	auto height = (_pendingRows.empty() && _rows.empty())
		? 0
		: st::walletRowsSkip;
	for (const auto &row : ranges::view::concat(_pendingRows, _rows)) {
		row->setTop(height);
		row->resizeToWidth(width);
		height += row->height();
	}
	if (height > 0) {
		height += st::walletRowsSkip;
	}
	_widget.resize(width, height);
}

rpl::producer<int> History::heightValue() const {
	return _widget.heightValue();
}

void History::setVisibleTopBottom(int top, int bottom) {
	_visibleTop = top - _widget.y();
	_visibleBottom = bottom - _widget.y();
	if (_visibleBottom <= _visibleTop || !_previousId.lt || _rows.empty()) {
		return;
	}
	const auto visibleHeight = (_visibleBottom - _visibleTop);
	const auto preloadHeight = kPreloadScreens * visibleHeight;
	if (_visibleBottom + preloadHeight >= _widget.height()) {
		_preloadRequests.fire_copy(_previousId);
	}
}

rpl::producer<Ton::TransactionId> History::preloadRequests() const {
	return _preloadRequests.events();
}

rpl::producer<Ton::Transaction> History::viewRequests() const {
	return _viewRequests.events();
}

rpl::producer<Ton::Transaction> History::decryptRequests() const {
	return _decryptRequests.events();
}

rpl::lifetime &History::lifetime() {
	return _widget.lifetime();
}

void History::setupContent(
		rpl::producer<HistoryState> &&state,
		rpl::producer<Ton::LoadedSlice> &&loaded) {
	std::move(
		state
	) | rpl::start_with_next([=](HistoryState &&state) {
		mergeState(std::move(state));
	}, lifetime());

	std::move(
		loaded
	) | rpl::filter([=](const Ton::LoadedSlice &slice) {
		return (slice.after == _previousId);
	}) | rpl::start_with_next([=](Ton::LoadedSlice &&slice) {
		_previousId = slice.data.previousId;
		_listData.insert(
			end(_listData),
			slice.data.list.begin(),
			slice.data.list.end());
		refreshRows();
	}, lifetime());

	_widget.paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		auto p = Painter(&_widget);
		paint(p, clip);
	}, lifetime());

	_widget.setAttribute(Qt::WA_MouseTracking);
	_widget.events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		switch (e->type()) {
		case QEvent::Leave: selectRow(-1, nullptr); return;
		case QEvent::Enter:
		case QEvent::MouseMove: selectRowByMouse(); return;
		case QEvent::MouseButtonPress: pressRow(); return;
		case QEvent::MouseButtonRelease: releaseRow(); return;
		}
	}, lifetime());
}

void History::selectRow(int selected, ClickHandlerPtr handler) {
	Expects(selected >= 0 || !handler);

	if (_selected != selected) {
		const auto was = (_selected >= 0 && _selected < int(_rows.size()))
			? _rows[_selected].get()
			: nullptr;
		if (was) repaintRow(was);
		_selected = selected;
		_widget.setCursor((_selected >= 0)
			? style::cur_pointer
			: style::cur_default);
	}
	if (ClickHandler::getActive() != handler) {
		const auto now = (_selected >= 0 && _selected < int(_rows.size()))
			? _rows[_selected].get()
			: nullptr;
		if (now) repaintRow(now);
		ClickHandler::setActive(handler);
	}
}

void History::selectRowByMouse() {
	const auto point = _widget.mapFromGlobal(QCursor::pos());
	const auto from = ranges::upper_bound(
		_rows,
		point.y(),
		ranges::less(),
		&HistoryRow::bottom);
	const auto till = ranges::lower_bound(
		_rows,
		point.y(),
		ranges::less(),
		&HistoryRow::top);

	if (from != till && (*from)->isUnderCursor(point)) {
		selectRow(from - begin(_rows), (*from)->handlerUnderCursor(point));
	} else {
		selectRow(-1, nullptr);
	}
}

void History::pressRow() {
	_pressed = _selected;
	ClickHandler::pressed();
}

void History::releaseRow() {
	Expects(_selected < int(_rows.size()));

	const auto handler = ClickHandler::unpressed();
	if (std::exchange(_pressed, -1) != _selected || _selected < 0) {
		if (handler) handler->onClick(ClickContext());
		return;
	}
	if (handler) {
		handler->onClick(ClickContext());
	} else {
		const auto i = ranges::find(
			_listData,
			_rows[_selected]->id(),
			&Ton::Transaction::id);
		Assert(i != end(_listData));
		_viewRequests.fire_copy(*i);
	}
}

void History::decryptById(const Ton::TransactionId &id) {
	const auto i = ranges::find(_listData, id, &Ton::Transaction::id);
	Assert(i != end(_listData));
	_decryptRequests.fire_copy(*i);
}

void History::paint(Painter &p, QRect clip) {
	if (_pendingRows.empty() && _rows.empty()) {
		return;
	}
	const auto paintRows = [&](
			const std::vector<std::unique_ptr<HistoryRow>> &rows) {
		const auto from = ranges::upper_bound(
			rows,
			clip.top(),
			ranges::less(),
			&HistoryRow::bottom);
		const auto till = ranges::lower_bound(
			rows,
			clip.top() + clip.height(),
			ranges::less(),
			&HistoryRow::top);
		if (from == till) {
			return;
		}
		for (const auto &row : ranges::make_subrange(from, till)) {
			row->paint(p, 0, row->top());
		}
		auto lastDateTop = rows.back()->bottom();
		const auto dates = ranges::make_subrange(begin(rows), till);
		for (const auto &row : dates | ranges::view::reverse) {
			if (!row->showDate()) {
				continue;
			}
			const auto top = std::max(
				std::min(_visibleTop, lastDateTop - st::walletRowDateHeight),
				row->top());
			row->paintDate(p, 0, top);
			if (row->top() <= _visibleTop) {
				break;
			}
			lastDateTop = top;
		}
	};
	paintRows(_pendingRows);
	paintRows(_rows);
}

History::ScrollState History::computeScrollState() const {
	const auto item = ranges::upper_bound(
		_rows,
		_visibleTop,
		ranges::less(),
		&HistoryRow::bottom);
	if (item == _rows.end()
		|| (item == _rows.begin()
			&& _rows.front()->id() == _listData.front().id)) {
		return ScrollState();
	}
	auto result = ScrollState();
	result.top = (*item)->id();
	result.offset = _visibleTop - (*item)->top();
	return result;
}

void History::mergeState(HistoryState &&state) {
	if (mergePendingChanged(std::move(state.pendingTransactions))) {
		refreshPending();
	}
	if (mergeListChanged(std::move(state.lastTransactions))) {
		refreshRows();
	}
}

bool History::mergePendingChanged(
		std::vector<Ton::PendingTransaction> &&list) {
	if (_pendingData == list) {
		return false;
	}
	_pendingData = std::move(list);
	return true;
}

bool History::mergeListChanged(Ton::TransactionsSlice &&data) {
	const auto i = _listData.empty()
		? data.list.cend()
		: ranges::find(std::as_const(data.list), _listData.front());
	if (i == data.list.cend()) {
		_listData = data.list | ranges::to_vector;
		_previousId = std::move(data.previousId);
		return true;
	} else if (i != data.list.cbegin()) {
		_listData.insert(begin(_listData), data.list.cbegin(), i);
		return true;
	}
	return false;
}

void History::setRowShowDate(
		const std::unique_ptr<HistoryRow> &row,
		bool show) {
	const auto raw = row.get();
	row->setShowDate(show, [=] { repaintShadow(raw); });
}

bool History::takeDecrypted(
		int index,
		const std::vector<Ton::Transaction> &decrypted) {
	Expects(index >= 0 && index < _listData.size());
	Expects(index >= 0 && index < _rows.size());
	Expects(_rows[index]->id() == _listData[index].id);

	const auto i = ranges::find(
		decrypted,
		_listData[index].id,
		&Ton::Transaction::id);
	if (i == end(decrypted)) {
		return false;
	}
	if (IsEncryptedMessage(*i)) {
		_rows[index]->setDecryptionFailed();
	} else {
		_listData[index] = *i;
		_rows[index] = makeRow(*i);
	}
	return true;
}

std::unique_ptr<HistoryRow> History::makeRow(const Ton::Transaction &data) {
	const auto id = data.id;
	if (const auto pending = (id.lt == 0)) {
		return std::make_unique<HistoryRow>(data, nullptr);
	}
	return std::make_unique<HistoryRow>(data, [=] { decryptById(id); });
}

void History::refreshShowDates() {
	auto previous = QDate();
	for (const auto &row : _rows) {
		const auto current = row->date().date();
		setRowShowDate(row, current != previous);
		previous = current;
	}
	resizeToWidth(_widget.width());
}

void History::refreshPending() {
	_pendingRows = ranges::view::all(
		_pendingData
	) | ranges::view::transform([&](const Ton::PendingTransaction &data) {
		return makeRow(data.fake);
	}) | ranges::to_vector;

	if (!_pendingRows.empty()) {
		setRowShowDate(_pendingRows.front());
	}
	resizeToWidth(_widget.width());
}

void History::refreshRows() {
	auto addedFront = std::vector<std::unique_ptr<HistoryRow>>();
	auto addedBack = std::vector<std::unique_ptr<HistoryRow>>();
	for (const auto &element : _listData) {
		if (!_rows.empty() && element.id == _rows.front()->id()) {
			break;
		}
		addedFront.push_back(makeRow(element));
	}
	if (!_rows.empty()) {
		const auto from = ranges::find(
			_listData,
			_rows.back()->id(),
			&Ton::Transaction::id);
		if (from != end(_listData)) {
			addedBack = ranges::make_subrange(
				from + 1,
				end(_listData)
			) | ranges::view::transform([=](const Ton::Transaction &data) {
				return makeRow(data);
			}) | ranges::to_vector;
		}
	}
	if (addedFront.empty() && addedBack.empty()) {
		return;
	} else if (!addedFront.empty()) {
		if (addedFront.size() < _listData.size()) {
			addedFront.insert(
				end(addedFront),
				std::make_move_iterator(begin(_rows)),
				std::make_move_iterator(end(_rows)));
		}
		_rows = std::move(addedFront);
	}
	_rows.insert(
		end(_rows),
		std::make_move_iterator(begin(addedBack)),
		std::make_move_iterator(end(addedBack)));

	refreshShowDates();
}

void History::repaintRow(not_null<HistoryRow*> row) {
	_widget.update(0, row->top(), _widget.width(), row->height());
}

void History::repaintShadow(not_null<HistoryRow*> row) {
	const auto min = std::min(row->top(), _visibleTop);
	const auto delta = std::max(row->top(), _visibleTop) - min;
	_widget.update(0, min, _widget.width(), delta + st::walletRowDateHeight);
}

rpl::producer<HistoryState> MakeHistoryState(
		rpl::producer<Ton::WalletViewerState> state) {
	return std::move(
		state
	) | rpl::map([](Ton::WalletViewerState &&state) {
		return HistoryState{
			std::move(state.wallet.lastTransactions),
			std::move(state.wallet.pendingTransactions)
		};
	});
}

} // namespace Wallet
