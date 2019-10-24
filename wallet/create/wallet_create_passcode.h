// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "wallet/create/wallet_create_step.h"

namespace Wallet::Create {

class Passcode final : public Step {
public:
	Passcode();

	[[nodiscard]] QByteArray passcode() const;
	int desiredHeight() const override;

	void setFocus() override;

private:
	void initControls();
	void showFinishedHook() override;

	Fn<QByteArray()> _passcode;
	Fn<void()> _setFocus;

};

} // namespace Wallet::Create
