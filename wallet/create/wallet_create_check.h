// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "wallet/create/wallet_create_step.h"

namespace Wallet::Create {

class Check final : public Step {
public:
	Check(
		Fn<std::vector<QString>(QString)> wordsByPrefix,
		const std::vector<int> &indices);

	bool allowEscapeBack() const override;

	[[nodiscard]] std::vector<QString> words() const;
	[[nodiscard]] rpl::producer<> submitRequests() const;

	void setFocus() override;
	bool checkAll();

private:
	void initControls(
		Fn<std::vector<QString>(QString)> wordsByPrefix,
		const std::vector<int> &indices);
	void showFinishedHook() override;

	int _desiredHeight = 0;
	Fn<std::vector<QString>()> _words;
	Fn<void()> _setFocus;
	Fn<bool()> _checkAll;

	rpl::event_stream<> _submitRequests;

};

} // namespace Wallet::Create
