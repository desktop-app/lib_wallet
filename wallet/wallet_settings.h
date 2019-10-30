// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"
#include "ton/ton_settings.h"

namespace Ton {
struct Settings;
} // namespace ton

namespace Wallet {

class UpdateInfo;

void SettingsBox(
	not_null<Ui::GenericBox*> box,
	const Ton::Settings &settings,
	UpdateInfo *updateInfo,
	Fn<QByteArray(QString)> checkConfig,
	Fn<void(Ton::Settings)> save);

} // namespace Wallet
