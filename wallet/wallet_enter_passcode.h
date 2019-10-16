// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"

namespace Wallet {

void EnterPasscodeBox(
	not_null<Ui::GenericBox*> box,
	Fn<void(QByteArray password, Fn<void(QString)> error)> submit);

} // namespace Wallet
