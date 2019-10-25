// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"

namespace Wallet {

void InvoiceQrBox(
	not_null<Ui::GenericBox*> box,
	const QString &link,
	Fn<void(QImage, QString)> share);

} // namespace Wallet
