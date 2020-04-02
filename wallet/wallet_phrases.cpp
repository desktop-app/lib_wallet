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
phrase lng_wallet_done = "Done";
phrase lng_wallet_save = "Save";
phrase lng_wallet_warning = "Warning";
phrase lng_wallet_error = "Error";
phrase lng_wallet_ok = "OK";

phrase lng_wallet_copy_address = "Copy Wallet Address";

phrase lng_wallet_intro_title = "Gram Wallet";
phrase lng_wallet_intro_description = "Gram wallet allows you to make fast and\nsecure blockchain-based payments\nwithout intermediaries.";
phrase lng_wallet_intro_create = "Create My Wallet";
phrase lng_wallet_intro_import = "Import existing wallet";

phrase lng_wallet_import_title = "24 Secret Words";
phrase lng_wallet_import_description = "Please restore access to your wallet by\nentering the 24 secret words you wrote\ndown when creating the wallet.";
phrase lng_wallet_import_dont_have = "I don't have them";
phrase lng_wallet_import_incorrect_title = "Incorrect words";
phrase lng_wallet_import_incorrect_text = "Sorry, you have entered incorrect secret words. Please double check and try again.";
phrase lng_wallet_import_incorrect_retry = "Try again";

phrase lng_wallet_too_bad_title = "Too Bad";
phrase lng_wallet_too_bad_description = "Without the secret words, you can't\nrestore access to your wallet.";
phrase lng_wallet_too_bad_enter_words = "Enter words";

phrase lng_wallet_created_title = "Congratulations";
phrase lng_wallet_created_description = "Your Gram wallet has just been created.\nOnly you control it.\n\nTo be able to always have access to it,\nplease set up a secure password and write\ndown secret words.";

phrase lng_wallet_words_title = "24 secret words";
phrase lng_wallet_words_description = "Write down these 24 words in the correct\norder and store them in a secret place.\n\nUse these secret words to restore access to\nyour wallet if you lose your password or\naccess to this device.";
phrase lng_wallet_words_sure_title = "Sure done?";
phrase lng_wallet_words_sure_text = "You didn't have enough time to write these words down.";
phrase lng_wallet_words_sure_ok = "OK, Sorry";

phrase lng_wallet_check_title = "Test Time!";
phrase lng_wallet_check_description = "Now let's check that you wrote your secret\nwords correctly.\n\nPlease enter the words {index1}, {index2} and {index3} below:";
phrase lng_wallet_check_incorrect_title = "Incorrect words";
phrase lng_wallet_check_incorrect_text = "The secret words you have entered do not match the ones in the list.";
phrase lng_wallet_check_incorrect_view = "See words";
phrase lng_wallet_check_incorrect_retry = "Try again";

phrase lng_wallet_set_passcode_title = "Secure Password";
phrase lng_wallet_set_passcode_description = "Please choose a secure password\nfor confirming your payments.";
phrase lng_wallet_set_passcode_enter = "Enter your password";
phrase lng_wallet_set_passcode_repeat = "Repeat your password";

phrase lng_wallet_change_passcode_title = "Change Password";
phrase lng_wallet_change_passcode_old = "Enter your old password";
phrase lng_wallet_change_passcode_new = "Enter a new password";
phrase lng_wallet_change_passcode_repeat = "Repeat the new password";
phrase lng_wallet_change_passcode_done = "Password changed successfully.";

phrase lng_wallet_ready_title = "Ready to go!";
phrase lng_wallet_ready_description = "You're all set. Now you have a wallet that\nonly you control \xe2\x80\x93 directly, without\nmiddlemen or bankers.";
phrase lng_wallet_ready_show_account = "View My Wallet";

phrase lng_wallet_sync = "syncing...";
phrase lng_wallet_sync_percent = "syncing... {percent}%";
phrase lng_wallet_refreshing = "updating...";
phrase lng_wallet_refreshed_just_now = "updated just now";

phrase lng_wallet_cover_balance = "Your balance";
phrase lng_wallet_cover_receive_full = "Receive Grams";
phrase lng_wallet_cover_receive = "Receive";
phrase lng_wallet_cover_send = "Send";

phrase lng_wallet_update = "Update Wallet";
phrase lng_wallet_update_short = "Update";

phrase lng_wallet_empty_history_title = "Wallet Created";
phrase lng_wallet_empty_history_welcome = "Welcome";
phrase lng_wallet_empty_history_address = "Your wallet address";

phrase lng_wallet_row_from = "from:";
phrase lng_wallet_row_to = "to:";
phrase lng_wallet_row_fees = "blockchain fees: {amount}";
phrase lng_wallet_row_pending_date = "Pending";
phrase lng_wallet_click_to_decrypt = "Click to show encrypted comment";
phrase lng_wallet_decrypt_failed = "Decryption failed :(";

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
phrase lng_wallet_receive_description = "Share this address to receive Test Grams. Note: this link won't work for real Grams.";
phrase lng_wallet_receive_create_invoice = "Create Invoice";
phrase lng_wallet_receive_share = "Share Wallet Address";
phrase lng_wallet_receive_copied = "Transfer link copied to clipboard.";
phrase lng_wallet_receive_copied_qr = "QR Code copied to clipboard.";

phrase lng_wallet_invoice_title = "Create Invoice";
phrase lng_wallet_invoice_amount = "Amount";
phrase lng_wallet_invoice_number = "Amount in grams you expect to receive";
phrase lng_wallet_invoice_comment = "Comment (optional)";
phrase lng_wallet_invoice_comment_about = "You can specify the amount and purpose of the payment to save the sender some time.";
phrase lng_wallet_invoice_url = "Invoice URL";
phrase lng_wallet_invoice_copy_url = "Copy Invoice URL";
phrase lng_wallet_invoice_url_about = "Share this address to receive Test Grams. Note: this link won't work for real Grams.";
phrase lng_wallet_invoice_generate_qr = "Generate QR Code";
phrase lng_wallet_invoice_share = "Share Invoice URL";
phrase lng_wallet_invoice_qr_title = "Invoice QR";
phrase lng_wallet_invoice_qr_amount = "Expected amount";
phrase lng_wallet_invoice_qr_comment = "Comment";
phrase lng_wallet_invoice_qr_share = "Share QR Code";
phrase lng_wallet_invoice_copied = "Invoice link copied to clipboard.";

phrase lng_wallet_menu_settings = "Settings";
phrase lng_wallet_menu_change_passcode = "Change password";
phrase lng_wallet_menu_export = "Back up wallet";
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
phrase lng_wallet_send_button_amount = "Send {grams}";

phrase lng_wallet_send_failed_title = "Sending failed";
phrase lng_wallet_send_failed_text = "Could not perform the transaction. Please check your wallet's balance and try again.";

phrase lng_wallet_confirm_title = "Confirmation";
phrase lng_wallet_confirm_text = "Do you want to send **{grams}** to:";
phrase lng_wallet_confirm_fee = "Fee: ~{grams}";
phrase lng_wallet_confirm_send = "Send Grams";
phrase lng_wallet_confirm_warning = "**Note:** your comment \xC2\xAB{comment}\xC2\xBB **will not be encrypted**.";

phrase lng_wallet_same_address_title = "Warning";
phrase lng_wallet_same_address_text = "Sending Grams from a wallet to the same wallet doesn't make sense, you will simply waste a portion of the value on blockchain fees.";
phrase lng_wallet_same_address_proceed = "Proceed";

phrase lng_wallet_passcode_title = "Password";
phrase lng_wallet_passcode_enter = "Enter your password";
phrase lng_wallet_passcode_next = "Next";
phrase lng_wallet_passcode_incorrect = "Incorrect password.";

phrase lng_wallet_sending_title = "Sending Grams";
phrase lng_wallet_sending_text = "Please wait a few seconds for your\ntransaction to be processed...";

phrase lng_wallet_sent_title = "Done!";
phrase lng_wallet_sent_close = "Close";

phrase lng_wallet_settings_title = "Settings";
phrase lng_wallet_settings_version_title = "Version and updates";
phrase lng_wallet_settings_autoupdate = "Update automatically";
phrase lng_wallet_settings_version = "Version {version}";
phrase lng_wallet_settings_checking = "Checking for updates...";
phrase lng_wallet_settings_latest = "Latest version is installed";
phrase lng_wallet_settings_check = "Check for updates";
phrase lng_wallet_settings_downloading = "Downloading update {progress}...";
phrase lng_wallet_settings_ready = "New version is ready";
phrase lng_wallet_settings_fail = "Update check failed :(";
phrase lng_wallet_settings_update = "Update Wallet";
phrase lng_wallet_settings_configuration = "Server Settings";
phrase lng_wallet_settings_update_config = "Update config automatically";
phrase lng_wallet_settings_config_url = "Config update URL";
phrase lng_wallet_settings_config_from_file = "Load from file";
phrase lng_wallet_settings_blockchain_name = "Blockchain ID";

phrase lng_wallet_warning_blockchain_name = "Are you sure you want to change the blockchain ID? You don't need this unless you're testing your own TON network.\n\nIf you proceed, you will need to reconnect your wallet using 24 secret words.";
phrase lng_wallet_bad_config = "Sorry, this config is invalid.";
phrase lng_wallet_bad_config_url = "Could not load config from URL.";
phrase lng_wallet_wait_pending = "Please wait until the current transaction is completed.";
phrase lng_wallet_wait_syncing = "Please wait until the synchronisation is completed.";

phrase lng_wallet_downloaded = "{ready} / {total} {mb}";

const auto walletCountValidate = check_phrase_count(Wallet::kPhrasesCount);

Fn<phrase(int)> lng_wallet_refreshed_minutes_ago = [](int minutes) {
	return (minutes == 1)
		? "updated one minute ago"
		: "updated " + QString::number(minutes) + " minutes ago";
};

Fn<phrase(QDate)> lng_wallet_short_date = [](QDate date) {
	const auto month = date.month();
	const auto result = [&]() -> QString {
		switch (month) {
		case 1: return "January";
		case 2: return "February";
		case 3: return "March";
		case 4: return "April";
		case 5: return "May";
		case 6: return "June";
		case 7: return "July";
		case 8: return "August";
		case 9: return "September";
		case 10: return "October";
		case 11: return "November";
		case 12: return "December";
		}
		return QString();
	}();
	if (result.isEmpty()) {
		return result;
	}
	const auto small = result + ' ' + QString::number(date.day());
	const auto year = date.year();
	const auto current = QDate::currentDate();
	const auto currentYear = current.year();
	const auto currentMonth = current.month();
	if (year == currentYear) {
		return small;
	}
	const auto yearIsMuchGreater = [](int year, int otherYear) {
		return (year > otherYear + 1);
	};
	const auto monthIsMuchGreater = [](
			int year,
			int month,
			int otherYear,
			int otherMonth) {
		return (year == otherYear + 1) && (month + 12 > otherMonth + 3);
	};
	if (false
		|| yearIsMuchGreater(year, currentYear)
		|| yearIsMuchGreater(currentYear, year)
		|| monthIsMuchGreater(year, month, currentYear, currentMonth)
		|| monthIsMuchGreater(currentYear, currentMonth, year, month)) {
		return small + ", " + QString::number(year);
	}
	return small;
};

Fn<phrase(QTime)> lng_wallet_short_time = [](QTime time) {
	return time.toString(Qt::SystemLocaleShortDate);
};

Fn<phrase(QString)> lng_wallet_grams_count = [](QString text) {
	return text + ((text == "1") ? " Gram" : " Grams");
};

Fn<phrase(QString)> lng_wallet_grams_count_sent = [](QString text) {
	return text + ((text == "1")
		? " Gram has been sent."
		: " Grams have been sent.");
};

} // namespace ph

namespace Wallet {

void SetPhrases(
		ph::details::phrase_value_array<kPhrasesCount> data,
		Fn<rpl::producer<QString>(int)> wallet_refreshed_minutes_ago,
		Fn<rpl::producer<QString>(QDate)> wallet_short_date,
		Fn<rpl::producer<QString>(QTime)> wallet_short_time,
		Fn<rpl::producer<QString>(QString)> wallet_grams_count) {
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
	ph::lng_wallet_grams_count = [=](QString text) {
		return ph::phrase{ wallet_grams_count(text) };
	};
}

} // namespace Wallet
