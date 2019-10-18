// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/ton_word_input.h"

#include "ui/widgets/input_fields.h"
#include "ui/widgets/labels.h"
#include "ui/ton_word_suggestions.h"
#include "base/event_filter.h"
#include "base/qt_signal_producer.h"
#include "base/platform/base_platform_layout_switch.h"
#include "styles/style_wallet.h"

#include <QtGui/QtEvents>

namespace Ui {

const QString TonWordInput::kSkipPassword = "speakfriendandenter";

TonWordInput::TonWordInput(
	not_null<QWidget*> parent,
	int index,
	Fn<std::vector<QString>(QString)> wordsByPrefix)
: _index(parent, QString::number(index + 1) + '.', st::walletWordIndexLabel)
, _word(parent, st::walletCheckInputField, rpl::single(QString()), QString())
, _wordsByPrefix(std::move(wordsByPrefix)) {
	_word->customUpDown(true);
	base::install_event_filter(_word.data(), [=](not_null<QEvent*> e) {
		if (e->type() != QEvent::KeyPress) {
			return base::EventFilterResult::Continue;
		}
		const auto ev = static_cast<QKeyEvent*>(e.get());
		if ((ev->key() != Qt::Key_Tab) && (ev->key() != Qt::Key_Backtab)) {
			return base::EventFilterResult::Continue;
		}
		const auto direction =  ((ev->key() == Qt::Key_Tab)
			&& !(ev->modifiers() & Qt::ShiftModifier))
			? TabDirection::Forward
			: TabDirection::Backward;
		_wordTabbed.fire_copy(direction);
		return base::EventFilterResult::Cancel;
	});
	setupSuggestions();
}

TonWordInput::~TonWordInput() = default;

void TonWordInput::setupSuggestions() {
	base::qt_signal_producer(
		_word.data(),
		&InputField::changed
	) | rpl::start_with_next([=] {
		_chosen = false;
		showSuggestions(word());
	}, _word->lifetime());

	focused(
	) | rpl::filter([=] {
		return !_chosen;
	}) | rpl::start_with_next([=] {
		showSuggestions(word());
	}, _word->lifetime());

	base::install_event_filter(_word.data(), [=](not_null<QEvent*> e) {
		if (e->type() != QEvent::KeyPress || !_suggestions) {
			return base::EventFilterResult::Continue;
		}
		const auto key = static_cast<QKeyEvent*>(e.get())->key();
		if (key == Qt::Key_Up) {
			_suggestions->selectUp();
			return base::EventFilterResult::Cancel;
		} else if (key == Qt::Key_Down) {
			_suggestions->selectDown();
			return base::EventFilterResult::Cancel;
		}
		return base::EventFilterResult::Continue;
	});
}

void TonWordInput::showSuggestions(const QString &word) {
	auto list = _wordsByPrefix(word);
	if (list.empty() || (list.size() == 1 && list.front() == word) || word.size() < 3) {
		if (_suggestions) {
			_suggestions->hide();
		}
	} else {
		if (!_suggestions) {
			createSuggestionsWidget();
		}
		_suggestions->show(std::move(list));
	}
}

void TonWordInput::createSuggestionsWidget() {
	_suggestions = std::make_unique<TonWordSuggestions>(
		_word->parentWidget());

	_suggestions->chosen(
	) | rpl::start_with_next([=](QString word) {
		_chosen = true;
		_word->setText(word);
		_word->setFocus();
		_word->setCursorPosition(word.size());
		_suggestions = nullptr;
		emit _word->submitted(Qt::KeyboardModifiers());
	}, _suggestions->lifetime());

	_suggestions->hidden(
	) | rpl::start_with_next([=] {
		_suggestions = nullptr;
	}, _suggestions->lifetime());

	_word->geometryValue(
	) | rpl::start_with_next([=](QRect geometry) {
		_suggestions->setGeometry(
			geometry.topLeft() + QPoint(0, geometry.height()),
			geometry.width());
	}, _suggestions->lifetime());

	_word->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return (e->type() == QEvent::KeyPress);
	}) | rpl::map([=](not_null<QEvent*> e) {
		return static_cast<QKeyEvent*>(e.get())->key();
	}) | rpl::start_with_next([=](int key) {
		if (key == Qt::Key_Up) {
			_suggestions->selectUp();
		} else if (key == Qt::Key_Down) {
			_suggestions->selectDown();
		}
	}, _suggestions->lifetime());

	blurred(
	) | rpl::start_with_next([=] {
		_suggestions->hide();
	}, _suggestions->lifetime());
}

void TonWordInput::move(int left, int top) const {
	_index->move(left - _index->width() - st::walletWordIndexSkip, top);
	_word->move(left, top - st::walletCheckInputSkip);
}

void TonWordInput::setFocus() const {
	base::Platform::SwitchKeyboardLayoutToEnglish();
	_word->setFocus();
}

void TonWordInput::showError() const {
	_word->showError();
}

void TonWordInput::showErrorNoFocus() const {
	_word->showErrorNoFocus();
}

rpl::producer<> TonWordInput::focused() const {
	return base::qt_signal_producer(_word.data(), &InputField::focused);
}

rpl::producer<> TonWordInput::blurred() const {
	return base::qt_signal_producer(_word.data(), &InputField::blurred);
}

rpl::producer<TonWordInput::TabDirection> TonWordInput::tabbed() const {
	return _wordTabbed.events();
}

rpl::producer<> TonWordInput::submitted() const {
	return base::qt_signal_producer(
		_word.data(),
		&InputField::submitted
	) | rpl::filter([=] {
		if (_suggestions) {
			_suggestions->choose();
			return false;
		}
		return true;
	}) | rpl::map([] {
		return rpl::empty_value();
	});
}

int TonWordInput::top() const {
	return _index->y();
}

QString TonWordInput::word() const {
	return _word->getLastText();
}

} // namespace Ui
