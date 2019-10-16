// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/layers/generic_box.h"

namespace Wallet {

struct PreparedInvoice {
	QString address;
	int64 amount;
	QString comment;
};

enum class InvoiceField {
	Address,
	Amount,
	Comment,
};

void SendGramsBox(
	not_null<Ui::GenericBox*> box,
	const QString &invoice,
	rpl::producer<int64> balance,
	Fn<void(PreparedInvoice, Fn<void(InvoiceField)> error)> done);

} // namespace Wallet
