// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Ui {

class RpWidget;
class ScrollArea;

class TonWordSuggestions final {
public:
	explicit TonWordSuggestions(not_null<QWidget*> parent);

	void setGeometry(QPoint position, int width);
	void show(std::vector<QString> &&words);
	void hide();

	void selectDown();
	void selectUp();
	void select(int index);
	void choose();

	[[nodiscard]] rpl::producer<QString> chosen() const;
	[[nodiscard]] rpl::producer<> hidden() const;

	[[nodiscard]] rpl::lifetime &lifetime();

private:
	void paintRows();
	void ensureSelectedVisible();
	void selectByMouse(QPoint position);
	[[nodiscard]] QImage prepareFrame() const;

	const std::unique_ptr<RpWidget> _widget;
	const not_null<ScrollArea*> _scroll;
	const not_null<RpWidget*> _inner;

	mutable QImage _frame;
	std::vector<QString> _words;
	int _selected = -1;
	int _pressed = -1;

	rpl::event_stream<QString> _chosen;
	rpl::event_stream<> _hidden;

};

} // namespace Ui
