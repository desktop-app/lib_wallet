// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/address_label.h"

#include "ui/widgets/labels.h"
#include "styles/style_widgets.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

#include <QtGui/QPainter>

namespace Ui {
namespace {

style::font MonospaceFont(const style::font &parent) {
	const auto family = style::MonospaceFont()->family();
	return style::font(parent->size(), parent->flags(), family);
}

style::TextStyle ComputeAddressStyle(const style::TextStyle &parent) {
	auto result = parent;
	result.font = MonospaceFont(result.font);
	result.linkFont = MonospaceFont(result.linkFont);
	result.linkFontOver = MonospaceFont(result.linkFontOver);
	return result;
}

} // namespace

not_null<FlatLabel*> CreateAddressLabel(
		not_null<RpWidget*> parent,
		const QString &text,
		const style::FlatLabel &st) {
	const auto mono = parent->lifetime().make_state<style::FlatLabel>(st);
	mono->style = ComputeAddressStyle(mono->style);

	const auto result = CreateChild<Ui::FlatLabel>(
		parent.get(),
		rpl::single(text),
		*mono);
	result->setBreakEverywhere(true);
	result->setDoubleClickSelectsParagraph(true);

	const auto half = text.size() / 2;
	const auto first = text.mid(0, half);
	const auto second = text.mid(half);
	const auto width = std::max(
		mono->style.font->width(first),
		mono->style.font->width(second)
	) + mono->style.font->spacew / 2;
	result->resizeToWidth(width);
	result->setSelectable(true);

	return result;
}

} // namespace Ui
