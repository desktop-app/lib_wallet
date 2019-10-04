// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_phrases.h"

namespace ph {

const auto walletCountStart = start_phrase_count();

phrase lng_wallet_intro_title = "Gram Wallet";
phrase lng_wallet_intro_description = "The gram wallet allows you to make fast and secure blockchain-based payments without intermediaries";
phrase lng_wallet_intro_create = "Create my wallet";

const auto walletCountValidate = check_phrase_count(Wallet::kPhrasesCount);

} // namespace ph
