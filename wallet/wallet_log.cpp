// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_log.h"

#include "base/integration.h"

namespace Wallet::details {

void LogMessage(const QString &text) {
	base::Integration::Instance().logMessage(text);
}

} // namespace Wallet::details
