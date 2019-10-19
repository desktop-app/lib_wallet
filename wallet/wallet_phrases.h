// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ui/ph.h"

class QDate;
class QTime;

namespace ph {

extern phrase lng_wallet_cancel;
extern phrase lng_wallet_continue;
extern phrase lng_wallet_done;

extern phrase lng_wallet_intro_title;
extern phrase lng_wallet_intro_description;
extern phrase lng_wallet_intro_create;
extern phrase lng_wallet_intro_accept_text;
extern phrase lng_wallet_intro_accept_terms;

extern phrase lng_wallet_created_title;
extern phrase lng_wallet_created_description;

extern phrase lng_wallet_words_title;
extern phrase lng_wallet_words_description;
extern phrase lng_wallet_words_sure_title;
extern phrase lng_wallet_words_sure_text;
extern phrase lng_wallet_words_sure_ok;

extern phrase lng_wallet_check_title;
extern phrase lng_wallet_check_description;
extern phrase lng_wallet_check_incorrect_title;
extern phrase lng_wallet_check_incorrect_text;
extern phrase lng_wallet_check_incorrect_view;
extern phrase lng_wallet_check_incorrect_retry;

extern phrase lng_wallet_set_passcode_title;
extern phrase lng_wallet_set_passcode_description;
extern phrase lng_wallet_set_passcode_enter;
extern phrase lng_wallet_set_passcode_repeat;

extern phrase lng_wallet_ready_title;
extern phrase lng_wallet_ready_description;
extern phrase lng_wallet_ready_show_account;

extern phrase lng_wallet_refreshing;
extern phrase lng_wallet_refreshed_just_now;

extern phrase lng_wallet_cover_balance;
extern phrase lng_wallet_cover_receive_full;
extern phrase lng_wallet_cover_receive;
extern phrase lng_wallet_cover_send;

extern phrase lng_wallet_empty_history_title;
extern phrase lng_wallet_empty_history_address;

extern phrase lng_wallet_row_from;
extern phrase lng_wallet_row_to;
extern phrase lng_wallet_row_fees;
extern phrase lng_wallet_row_pending_date;

extern phrase lng_wallet_view_title;
extern phrase lng_wallet_view_transaction_fee;
extern phrase lng_wallet_view_storage_fee;
extern phrase lng_wallet_view_sender;
extern phrase lng_wallet_view_recipient;
extern phrase lng_wallet_view_date;
extern phrase lng_wallet_view_comment;
extern phrase lng_wallet_view_send_to_address;
extern phrase lng_wallet_view_send_to_recipient;

extern phrase lng_wallet_receive_title;
extern phrase lng_wallet_receive_description;
extern phrase lng_wallet_receive_share;
extern phrase lng_wallet_receive_copied;

extern phrase lng_wallet_menu_export;
extern phrase lng_wallet_menu_delete;

extern phrase lng_wallet_delete_title;
extern phrase lng_wallet_delete_about;
extern phrase lng_wallet_delete_disconnect;

extern phrase lng_wallet_send_title;
extern phrase lng_wallet_send_recipient;
extern phrase lng_wallet_send_address;
extern phrase lng_wallet_send_about;
extern phrase lng_wallet_send_amount;
extern phrase lng_wallet_send_balance;
extern phrase lng_wallet_send_comment;
extern phrase lng_wallet_send_button;
extern phrase lng_wallet_send_button_amount;

extern phrase lng_wallet_confirm_title;
extern phrase lng_wallet_confirm_text;
extern phrase lng_wallet_confirm_fee;
extern phrase lng_wallet_confirm_send;

extern phrase lng_wallet_passcode_title;
extern phrase lng_wallet_passcode_enter;
extern phrase lng_wallet_passcode_next;
extern phrase lng_wallet_passcode_incorrect;

extern phrase lng_wallet_sending_title;
extern phrase lng_wallet_sending_text;

extern phrase lng_wallet_sent_title;
extern phrase lng_wallet_sent_text;
extern phrase lng_wallet_sent_close;

extern Fn<phrase(int)> lng_wallet_refreshed_minutes_ago;
extern Fn<phrase(QDate)> lng_wallet_short_date;
extern Fn<phrase(QTime)> lng_wallet_short_time;

} // namespace ph

namespace Wallet {

inline constexpr auto kPhrasesCount = 80;

void SetPhrases(
	ph::details::phrase_value_array<kPhrasesCount> data,
	Fn<rpl::producer<QString>(int)> wallet_refreshed_minutes_ago,
	Fn<rpl::producer<QString>(QDate)> wallet_short_date,
	Fn<rpl::producer<QString>(QTime)> wallet_short_time);

} // namespace Wallet
