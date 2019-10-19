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
} // namespace Ui

namespace Wallet {

struct PreparedInvoice;
enum class InvoiceField;

struct ParsedAmount {
	int64 grams = 0;
	int64 nano = 0;
	QString gramsString;
	QString separator;
	QString nanoString;
	QString full;
};

enum class Action {
	Refresh,
	Export,
	Send,
	Receive,
	ChangePassword,
	LogOut,
};

[[nodiscard]] ParsedAmount ParseAmount(int64 amount, bool isSigned = false);
[[nodiscard]] std::optional<int64> ParseAmountString(const QString &amount);
[[nodiscard]] int64 CalculateValue(const Ton::Transaction &data);
[[nodiscard]] QString ExtractAddress(const Ton::Transaction &data);
[[nodiscard]] QString ExtractMessage(const Ton::Transaction &data);

[[nodiscard]] QString TransferLink(const QString &address);

not_null<Ui::FlatLabel*> AddBoxSubtitle(
	not_null<Ui::GenericBox*> box,
	rpl::producer<QString> text);

[[nodiscard]] bool IsIncorrectPasswordError(const Ton::Error &error);
[[nodiscard]] bool IsIncorrectMnemonicError(const Ton::Error &error);
[[nodiscard]] std::optional<InvoiceField> ErrorInvoiceField(
	const Ton::Error &error);
[[nodiscard]] Ton::TransactionToSend TransactionFromInvoice(
	const PreparedInvoice &invoice);

} // namespace Wallet
