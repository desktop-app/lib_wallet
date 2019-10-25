// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/style/style_core.h"

class QPainter;

namespace Ui {

class RpWidget;

void PaintInlineDiamond(QPainter &p, int x, int y, const style::font &font);

[[nodiscard]] QImage InlineDiamondImage(int size);

not_null<RpWidget*> CreateInlineDiamond(
	not_null<QWidget*> parent,
	int x,
	int y,
	const style::font &font);

[[nodiscard]] QImage DiamondQr(const QString &text, int pixel, int max = 0);
[[nodiscard]] QImage DiamondQrForShare(const QString &text);

} // namespace Ui
