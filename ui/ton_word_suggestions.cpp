// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/ton_word_suggestions.h"

#include "base/object_ptr.h"
#include "ui/widgets/scroll_area.h"
#include "ui/rp_widget.h"
#include "ui/painter.h"
#include "styles/style_wallet.h"
#include "styles/style_widgets.h"
#include "styles/palette.h"

#include <QtGui/QPainter>

namespace Ui {

TonWordSuggestions::TonWordSuggestions(not_null<QWidget*> parent)
: _widget(std::make_unique<RpWidget>(parent))
, _scroll(Ui::CreateChild<ScrollArea>(
	_widget.get(),
	st::walletSuggestionsScroll))
, _inner(_scroll->setOwnedWidget(object_ptr<RpWidget>(_widget.get()))) {
	_widget->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_scroll->setGeometry({ QPoint(), size });
	}, _widget->lifetime());

	_widget->paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		paintRows();
	}, _widget->lifetime());

	_inner->setMouseTracking(true);
	_inner->events(
	) | rpl::start_with_next([=](not_null<QEvent*> e) {
		if (e->type() == QEvent::MouseMove) {
			selectByMouse(static_cast<QMouseEvent*>(e.get())->pos());
		} else if (e->type() == QEvent::MouseButtonPress) {
			_pressed = _selected;
		} else if (e->type() == QEvent::MouseButtonRelease) {
			_widget->update();
			if (std::exchange(_pressed, -1) == _selected) {
				choose();
			}
		}
	}, _inner->lifetime());
}

void TonWordSuggestions::show(std::vector<QString> &&words) {
	if (_words == words) {
		return;
	}
	_words = std::move(words);
	select(0);
	const auto height = st::walletSuggestionsSkip * 2
		+ int(_words.size()) * st::walletSuggestionHeight
		+ st::walletSuggestionShadowWidth;
	const auto outerHeight = std::min(
		height,
		st::walletSuggestionsHeightMax);
	_inner->resize(_widget->width(), height);
	_widget->resize(_widget->width(), outerHeight);
	_widget->update();
	_widget->show();
}

void TonWordSuggestions::hide() {
	_hidden.fire({});
}

void TonWordSuggestions::selectDown() {
	Expects(!_words.empty());

	select(_selected + 1);
	ensureSelectedVisible();
}

void TonWordSuggestions::selectUp() {
	select(_selected - 1);
	ensureSelectedVisible();
}

void TonWordSuggestions::select(int index) {
	Expects(!_words.empty());

	index = std::clamp(index, 0, int(_words.size()) - 1);
	if (_selected == index) {
		return;
	}
	_selected = index;
	_inner->update();
}

void TonWordSuggestions::selectByMouse(QPoint position) {
	select((position.y() - st::walletSuggestionsSkip)
		/ st::walletSuggestionHeight);
}

void TonWordSuggestions::ensureSelectedVisible() {
	const auto skip = st::walletSuggestionsSkip;
	const auto top = skip + _selected * st::walletSuggestionHeight;
	_scroll->scrollToY(top - skip, top + st::walletSuggestionHeight + skip);
}

void TonWordSuggestions::choose() {
	Expects(!_words.empty());
	Expects(_selected >= 0 && _selected < _words.size());

	_chosen.fire_copy(_words[_selected]);
}

void TonWordSuggestions::setGeometry(QPoint position, int width) {
	_widget->setGeometry({ position, QSize{ width, _widget->height() } });
	_inner->resize(width, _inner->height());
}

void TonWordSuggestions::paintRows() {
	QPainter(_widget.get()).drawImage(0, 0, prepareFrame());
}

QImage TonWordSuggestions::prepareFrame() const {
	const auto pixelRatio = style::DevicePixelRatio();
	if (_frame.size() != _widget->size() * pixelRatio) {
		_frame = QImage(
			_widget->size() * pixelRatio,
			QImage::Format_ARGB32_Premultiplied);
	}
	_frame.fill(st::windowBg->c);
	_frame.setDevicePixelRatio(pixelRatio);
	{
		auto p = QPainter(&_frame);

		const auto thickness = st::walletSuggestionShadowWidth;

		p.setPen(st::windowFg);
		p.setFont(st::normalFont);
		auto index = 0;
		const auto wordLeft = thickness;
		auto wordTop = st::walletSuggestionsSkip - _scroll->scrollTop();
		const auto wordWidth = _widget->width() - 2 * thickness;
		const auto wordHeight = st::walletSuggestionHeight;
		for (const auto &word : _words) {
			if (index == (_pressed >= 0 ? _pressed : _selected)) {
				p.fillRect(
					wordLeft,
					wordTop,
					wordWidth,
					wordHeight,
					st::windowBgOver);
			}
			p.drawText(
				wordLeft + st::walletSuggestionLeft,
				wordTop + st::walletSuggestionTop + st::normalFont->ascent,
				word);
			wordTop += wordHeight;
			++index;
		}

		const auto radius = st::walletSuggestionsRadius;
		const auto left = float64(thickness) / 2;
		const auto top = -2. * radius;
		const auto width = float64(_widget->width()) - thickness;
		const auto height = float64(_widget->height())
			- top
			+ ((thickness / 2.) - thickness);

		PainterHighQualityEnabler hq(p);
		p.setBrush(Qt::NoBrush);
		auto pen = st::defaultInputField.borderFg->p;
		pen.setWidth(thickness);
		p.setPen(pen);
		p.drawRoundedRect(QRectF{ left, top, width, height }, radius, radius);
	}
	return _frame;
}

rpl::producer<QString> TonWordSuggestions::chosen() const {
	return _chosen.events();
}

rpl::producer<> TonWordSuggestions::hidden() const {
	return _hidden.events();
}

rpl::lifetime &TonWordSuggestions::lifetime() {
	return _widget->lifetime();
}

} // namespace Ui
