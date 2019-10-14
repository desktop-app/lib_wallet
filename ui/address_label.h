// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

class QWidget;
class QString;

namespace style {
struct FlatLabel;
} // namespace style

namespace Ui {

class RpWidget;
class FlatLabel;

[[nodiscard]] not_null<FlatLabel*> CreateAddressLabel(
	not_null<RpWidget*> parent,
	const QString &text,
	const style::FlatLabel &st);

} // namespace Ui
