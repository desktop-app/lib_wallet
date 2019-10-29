// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"

namespace Wallet {
class UpdateInfo;
} // namespace Wallet

namespace Wallet::Settings {

struct ToggleUpdates {
	bool enabled = false;
};
struct AllowTestUpdates {};
struct InstallUpdate {};

using ActionEnum = base::variant<
	ToggleUpdates,
	AllowTestUpdates,
	InstallUpdate>;

struct Action : ActionEnum {
	using ActionEnum::ActionEnum;
};

void CreateBox(
	not_null<Ui::GenericBox*> box,
	UpdateInfo *updateInfo,
	Fn<void(Action)> callback);

} // namespace Wallet::Settings
