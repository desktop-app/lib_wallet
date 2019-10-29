// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Wallet {

struct UpdateProgress {
	int64 already = 0;
	int64 size = 0;
};

enum class UpdateState {
	None,
	Download,
	Ready,
};

class UpdateInfo {
public:
	[[nodiscard]] virtual rpl::producer<> checking() = 0;
	[[nodiscard]] virtual rpl::producer<> isLatest() = 0;
	[[nodiscard]] virtual rpl::producer<UpdateProgress> progress() = 0;
	[[nodiscard]] virtual rpl::producer<> failed() = 0;
	[[nodiscard]] virtual rpl::producer<> ready() = 0;

	[[nodiscard]] virtual UpdateState state() = 0;
	[[nodiscard]] virtual int64 already() = 0;
	[[nodiscard]] virtual int64 size() = 0;

	virtual void toggle(bool enabled) = 0;
	virtual void test() = 0;
	virtual void install() = 0;

	[[nodiscard]] virtual bool toggled() = 0;
	[[nodiscard]] virtual int currentVersion() = 0;

	virtual ~UpdateInfo() = default;
};

} // namespace Wallet
