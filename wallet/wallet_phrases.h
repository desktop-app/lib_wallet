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

extern phrase lng_wallet_empty_history_title;
extern phrase lng_wallet_empty_history_address;

extern phrase lng_wallet_row_from;
extern phrase lng_wallet_row_to;
extern phrase lng_wallet_row_fees;

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
extern phrase lng_wallet_delete_cancel;

extern Fn<phrase(int)> lng_wallet_refreshed_minutes_ago;
extern Fn<phrase(QDate)> lng_wallet_short_date;
extern Fn<phrase(QTime)> lng_wallet_short_time;

} // namespace ph

namespace Wallet {

inline constexpr auto kPhrasesCount = 34;

void SetPhrases(
	ph::details::phrase_value_array<kPhrasesCount> data,
	Fn<rpl::producer<QString>(int)> wallet_refreshed_minutes_ago,
	Fn<rpl::producer<QString>(QDate)> wallet_short_date,
	Fn<rpl::producer<QString>(QTime)> wallet_short_time);

} // namespace Wallet
