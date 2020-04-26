// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "base/flags.h"

namespace Ton {
struct Error;
struct Transaction;
struct TransactionToSend;
} // namespace Ton

namespace Ui {
class GenericBox;
class FlatLabel;
class InputField;
class VerticalLayout;
} // namespace Ui

namespace Wallet {

struct PreparedInvoice;
enum class InvoiceField;

inline constexpr auto kMaxCommentLength = 500;
inline constexpr auto kAddressLength = 48;

struct FormattedAmount {
	QString gramsString;
	QString separator;
	QString nanoString;
	QString full;
};

struct PreparedInvoice {
	int64 amount;
	QString address;
	QString comment;
	bool sendUnencryptedText = false;
};

enum class Action {
	Refresh,
	Export,
	Send,
	Receive,
	ChangePassword,
	ShowSettings,
	LogOut,
};

enum class FormatFlag {
	Signed = 0x01,
	Rounded = 0x02,
	Simple = 0x04,
};
constexpr bool is_flag_type(FormatFlag) { return true; };
using FormatFlags = base::flags<FormatFlag>;

[[nodiscard]] FormattedAmount FormatAmount(
	int64 amount,
	FormatFlags flags = FormatFlags());
[[nodiscard]] std::optional<int64> ParseAmountString(const QString &amount);
[[nodiscard]] PreparedInvoice ParseInvoice(QString invoice);
[[nodiscard]] int64 CalculateValue(const Ton::Transaction &data);
[[nodiscard]] QString ExtractAddress(const Ton::Transaction &data);
[[nodiscard]] bool IsEncryptedMessage(const Ton::Transaction &data);
[[nodiscard]] QString ExtractMessage(const Ton::Transaction &data);

[[nodiscard]] QString TransferLink(
	const QString &address,
	int64 amount = 0,
	const QString &comment = QString());

not_null<Ui::FlatLabel*> AddBoxSubtitle(
	not_null<Ui::VerticalLayout*> box,
	rpl::producer<QString> text);
not_null<Ui::FlatLabel*> AddBoxSubtitle(
	not_null<Ui::GenericBox*> box,
	rpl::producer<QString> text);

[[nodiscard]] not_null<Ui::InputField*> CreateAmountInput(
	not_null<QWidget*> parent,
	rpl::producer<QString> placeholder,
	int64 amount = 0);
[[nodiscard]] not_null<Ui::InputField*> CreateCommentInput(
	not_null<QWidget*> parent,
	rpl::producer<QString> placeholder,
	const QString &value = QString());

[[nodiscard]] bool IsIncorrectPasswordError(const Ton::Error &error);
[[nodiscard]] bool IsIncorrectMnemonicError(const Ton::Error &error);
[[nodiscard]] std::optional<InvoiceField> ErrorInvoiceField(
	const Ton::Error &error);
[[nodiscard]] Ton::TransactionToSend TransactionFromInvoice(
	const PreparedInvoice &invoice);

} // namespace Wallet
