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

phrase lng_wallet_cancel = "Cancel";
phrase lng_wallet_continue = "Continue";

phrase lng_wallet_intro_title = "Gram Wallet";
phrase lng_wallet_intro_description = "Gram wallet allows you to make fast and\nsecure blockchain-based payments\nwithout intermediaries.";
phrase lng_wallet_intro_create = "Create My Wallet";
phrase lng_wallet_intro_accept_text = "By creating the wallet you accept\n{terms_link}.";
phrase lng_wallet_intro_accept_terms = "Terms and Conditions";

phrase lng_wallet_created_title = "Congratulations";
phrase lng_wallet_created_description = "Your Gram wallet has just been created.\nOnly you control it.\n\nTo be able to always have access to it,\nplease set up a secure passcode and write\ndown secret words.";

phrase lng_wallet_words_title = "24 secret words";
phrase lng_wallet_words_description = "Write down these 24 words in the correct\norder and store them in a secret place.\n\nUse these secret words to restore access to\nyour wallet if you lose your passcode or\naccess to this device.";
phrase lng_wallet_words_sure_title = "Sure done?";
phrase lng_wallet_words_sure_text = "You didn't have enough time to write these words down.";
phrase lng_wallet_words_sure_ok = "OK, Sorry";

phrase lng_wallet_check_title = "Test Time!";
phrase lng_wallet_check_description = "Now let's check that you wrote your secret\nwords correctly.\n\nPlease enter words {index1}, {index2} and {index3} below:";
phrase lng_wallet_check_incorrect_title = "Incorrect words";
phrase lng_wallet_check_incorrect_text = "The secret words you have entered do not match the ones in the list.";
phrase lng_wallet_check_incorrect_view = "See words";
phrase lng_wallet_check_incorrect_retry = "Try again";

phrase lng_wallet_set_passcode_title = "Secure Passcode";
phrase lng_wallet_set_passcode_description = "Please choose a secure passcode\nfor your payments confirmation.";
phrase lng_wallet_set_passcode_enter = "Enter your passcode";
phrase lng_wallet_set_passcode_repeat = "Repeat your passcode";

phrase lng_wallet_ready_title = "Ready to go!";
phrase lng_wallet_ready_description = "You're all set. Now you have a wallet that\nonly you control \xe2\x80\x93 directly, without\nmiddlemen or bankers.";
phrase lng_wallet_ready_show_account = "View My Wallet";

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
phrase lng_wallet_row_fees = "blockchain fees: {amount}";
phrase lng_wallet_row_pending_date = "Pending";

phrase lng_wallet_view_title = "Transaction";
phrase lng_wallet_view_transaction_fee = "{amount} transaction fee";
phrase lng_wallet_view_storage_fee = "{amount} storage fee";
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
phrase lng_wallet_delete_about = "This will disconnect the wallet from this app. You will be able to restore your wallet using **24 secret words** \xe2\x80\x93 or import another wallet.\n\nWallets are located in the decentralized TON Blockchain. If you want the wallet to be deleted simply transfer all the Grams from it and leave it empty.";
phrase lng_wallet_delete_disconnect = "Disconnect";

phrase lng_wallet_send_title = "Send Grams";
phrase lng_wallet_send_recipient = "Recipient wallet address";
phrase lng_wallet_send_address = "Enter wallet address";
phrase lng_wallet_send_about = "Copy the 48-letter wallet address of the recipient here or ask them to send you a ton:// link.";
phrase lng_wallet_send_amount = "Amount";
phrase lng_wallet_send_balance = "Balance: {amount}";
phrase lng_wallet_send_comment = "Comment (optional)";
phrase lng_wallet_send_button = "Send Grams";
phrase lng_wallet_send_button_amount = "Send {amount} Grams";

phrase lng_wallet_confirm_title = "Confirmation";
phrase lng_wallet_confirm_text = "Do you want to send **{amount} Grams** to:";
phrase lng_wallet_confirm_fee = "Fee: ~{amount} Grams";
phrase lng_wallet_confirm_send = "Send Grams";

phrase lng_wallet_passcode_title = "Passcode";
phrase lng_wallet_passcode_enter = "Enter your passcode";
phrase lng_wallet_passcode_next = "Next";
phrase lng_wallet_passcode_incorrect = "This is a wrong password.";

phrase lng_wallet_sending_title = "Sending Grams";
phrase lng_wallet_sending_text = "Please wait a few seconds for your\ntransaction to be processed...";

phrase lng_wallet_sent_title = "Done!";
phrase lng_wallet_sent_text = "{amount} Grams have been sent.";
phrase lng_wallet_sent_close = "Close";

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
