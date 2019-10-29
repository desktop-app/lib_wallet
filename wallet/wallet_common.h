// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Ton {
struct Error;
struct Transaction;
struct TransactionToSend;
} // namespace Ton

namespace Ui {
class GenericBox;
class FlatLabel;
class InputField;
} // namespace Ui

namespace Wallet {

struct PreparedInvoice;
enum class InvoiceField;

inline constexpr auto kMaxCommentLength = 500;
inline constexpr auto kAddressLength = 48;

struct ParsedAmount {
	int64 grams = 0;
	int64 nano = 0;
	QString gramsString;
	QString separator;
	QString nanoString;
	QString full;
};

struct PreparedInvoice {
	QString address;
	int64 amount;
	QString comment;
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

[[nodiscard]] ParsedAmount ParseAmount(int64 amount, bool isSigned = false);
[[nodiscard]] std::optional<int64> ParseAmountString(const QString &amount);
[[nodiscard]] PreparedInvoice ParseInvoice(QString invoice);
[[nodiscard]] int64 CalculateValue(const Ton::Transaction &data);
[[nodiscard]] QString ExtractAddress(const Ton::Transaction &data);
[[nodiscard]] QString ExtractMessage(const Ton::Transaction &data);

[[nodiscard]] QString TransferLink(
	const QString &address,
	int64 amount = 0,
	const QString &comment = QString());

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
