// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "wallet/create/wallet_create_step.h"

namespace Wallet::Create {

class Intro final : public Step {
public:
	Intro();

private:
	void initControls();
	void showFinishedHook() override;

};

} // namespace Wallet::Create
