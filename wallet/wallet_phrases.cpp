// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_phrases.h"

#include <QtCore/QDate>
#include <QtCore/QTime>

namespace ph {

const auto walletCountStart = start_phrase_count();

phrase lng_wallet_window_title = "Gram Wallet";

phrase lng_wallet_intro_title = "Gram Wallet";
phrase lng_wallet_intro_description = "The gram wallet allows you to make fast and secure blockchain-based payments without intermediaries";
phrase lng_wallet_intro_create = "Create my wallet";

phrase lng_wallet_refreshing = "updating...";
phrase lng_wallet_refreshed_just_now = "updated just now";

phrase lng_wallet_cover_balance = "Your balance";
phrase lng_wallet_cover_receive_full = "Receive Grams";
phrase lng_wallet_cover_receive = "Receive";
phrase lng_wallet_cover_send = "Send";

phrase lng_wallet_empty_history_title = "Wallet Created";
phrase lng_wallet_empty_history_address = "Your wallet address";

phrase lng_wallet_row_from = "from:";
phrase lng_wallet_row_to = "to:";
phrase lng_wallet_row_fees = "blockchain fees: <0>";

phrase lng_wallet_view_title = "Transaction";
phrase lng_wallet_view_transaction_fee = "<0> transaction fee";
phrase lng_wallet_view_storage_fee = "<0> storage fee";
phrase lng_wallet_view_sender = "Sender";
phrase lng_wallet_view_recipient = "Recipient";
phrase lng_wallet_view_date = "Date";
phrase lng_wallet_view_comment = "Comment";
phrase lng_wallet_view_send_to_address = "Send Grams to this address";
phrase lng_wallet_view_send_to_recipient = "Send Grams to this Recipient";

phrase lng_wallet_receive_title = "Receive Grams";
phrase lng_wallet_receive_description = "Share this address with other Gram wallet owners to receive Grams from them:";
phrase lng_wallet_receive_share = "Share Wallet Address";
phrase lng_wallet_receive_copied = "Transfer link copied to clipboard.";

phrase lng_wallet_menu_export = "Export wallet";
phrase lng_wallet_menu_delete = "Delete wallet";

phrase lng_wallet_delete_title = "Delete Wallet";
phrase lng_wallet_delete_about = QString::fromUtf8("This will disconnect the wallet from this app. You will be able to restore your wallet using **24 secret words** \xe2\x80\x93 or import another wallet.\n\nWallets are located in the decentralized TON Blockchain. If you want the wallet to be deleted simply transfer all the Grams from it and leave it empty.");
phrase lng_wallet_delete_disconnect = "Disconnect";
phrase lng_wallet_delete_cancel = "Cancel";

const auto walletCountValidate = check_phrase_count(Wallet::kPhrasesCount);

Fn<phrase(int)> lng_wallet_refreshed_minutes_ago = [](int minutes) {
	return (minutes == 1)
		? "updated one minute ago"
		: "updated " + QString::number(minutes) + " minutes ago";
};

Fn<phrase(QDate)> lng_wallet_short_date = [](QDate date) {
	return date.toString(Qt::SystemLocaleShortDate);
};

Fn<phrase(QTime)> lng_wallet_short_time = [](QTime time) {
	return time.toString(Qt::SystemLocaleShortDate);
};

} // namespace ph

namespace Wallet {

void SetPhrases(
		ph::details::phrase_value_array<kPhrasesCount> data,
		Fn<rpl::producer<QString>(int)> wallet_refreshed_minutes_ago,
		Fn<rpl::producer<QString>(QDate)> wallet_short_date,
		Fn<rpl::producer<QString>(QTime)> wallet_short_time) {
	ph::details::set_values(std::move(data));
	ph::lng_wallet_refreshed_minutes_ago = [=](int minutes) {
		return ph::phrase{ wallet_refreshed_minutes_ago(minutes) };
	};
	ph::lng_wallet_short_date = [=](QDate date) {
		return ph::phrase{ wallet_short_date(date) };
	};
	ph::lng_wallet_short_time = [=](QTime date) {
		return ph::phrase{ wallet_short_time(date) };
	};
}

} // namespace Wallet
