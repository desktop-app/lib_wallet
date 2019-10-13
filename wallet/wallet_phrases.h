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

} // namespace ph

namespace Wallet {

inline constexpr auto kPhrasesCount = 4;

inline void SetPhrases(ph::details::phrase_value_array<kPhrasesCount> data) {
	ph::details::set_values(std::move(data));
}

} // namespace Wallet
