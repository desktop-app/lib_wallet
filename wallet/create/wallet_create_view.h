// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "wallet/create/wallet_create_step.h"

namespace Wallet::Create {

class View final : public Step {
public:
	View(const std::vector<QString> &words);

	int desiredHeight() const override;

private:
	void initControls(const std::vector<QString> &words);
	void showFinishedHook() override;

	int _desiredHeight = 0;

};

} // namespace Wallet::Create
