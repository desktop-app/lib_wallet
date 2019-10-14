// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/ph.h"

namespace ph {

extern phrase lng_wallet_window_title;

extern phrase lng_wallet_intro_title;
extern phrase lng_wallet_intro_description;
extern phrase lng_wallet_intro_create;

extern phrase lng_wallet_refreshing;
extern phrase lng_wallet_refreshed_just_now;

extern phrase lng_wallet_cover_balance;
extern phrase lng_wallet_cover_receive_full;
extern phrase lng_wallet_cover_receive;
extern phrase lng_wallet_cover_send;

extern Fn<phrase(int)> lng_wallet_refreshed_minutes_ago;

} // namespace ph

namespace Wallet {

inline constexpr auto kPhrasesCount = 10;

void SetPhrases(
	ph::details::phrase_value_array<kPhrasesCount> data,
	Fn<rpl::producer<QString>(int)> wallet_refreshed_minutes_ago);

} // namespace Wallet
