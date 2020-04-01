// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "ton/ton_state.h"
#include "base/weak_ptr.h"
#include "base/object_ptr.h"

#include <QtCore/QPointer>

namespace Ton {
class Wallet;
class AccountViewer;
struct TransactionCheckResult;
struct PendingTransaction;
struct Transaction;
struct WalletState;
struct Error;
struct Settings;
enum class ConfigUpgrade;
} // namespace Ton

namespace Ui {
class Window;
class LayerManager;
class GenericBox;
class RpWidget;
class FlatButton;
} // namespace Ui

namespace Wallet {
namespace Create {
class Manager;
} // namespace Create

class Info;
struct PreparedInvoice;
enum class InvoiceField;
class UpdateInfo;

class Window final : public base::has_weak_ptr {
public:
	Window(not_null<Ton::Wallet*> wallet, UpdateInfo *updateInfo = nullptr);
	~Window();

	void showAndActivate();
	[[nodiscard]] not_null<Ui::RpWidget*> widget() const;
	bool handleLinkOpen(const QString &link);
	void showConfigUpgrade(Ton::ConfigUpgrade upgrade);

private:
	struct DecryptPasswordState {
		int generation = 0;
		bool success = false;
		QPointer<Ui::GenericBox> box;
		Fn<void(QString)> showError;
	};

	void init();
	void updatePalette();
	void showSimpleError(
		rpl::producer<QString> title,
		rpl::producer<QString> text,
		rpl::producer<QString> button);
	void showGenericError(
		const Ton::Error &error,
		const QString &additional = QString());
	void showSendingError(const Ton::Error &error);
	void showToast(const QString &text);
	void startWallet();

	void showCreate();
	void createImportKey(const std::vector<QString> &words);
	void createKey(std::shared_ptr<bool> guard);
	void createShowIncorrectWords();
	void createShowTooFastWords();
	void createShowIncorrectImport();
	void createShowImportFail();
	void createSavePasscode(
		const QByteArray &passcode,
		std::shared_ptr<bool> guard);

	void decryptEverything();
	void askDecryptPassword(const Ton::DecryptPasswordNeeded &data);
	void doneDecryptPassword(const Ton::DecryptPasswordGood &data);

	void showAccount(const QByteArray &publicKey, bool justCreated = false);
	void setupUpdateWithInfo();
	void setupRefreshEach();
	void sendGrams(const QString &invoice = QString());
	void confirmTransaction(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError,
		std::shared_ptr<bool> guard);
	void showSendConfirmation(
		const PreparedInvoice &invoice,
		const Ton::TransactionCheckResult &checkResult,
		Fn<void(InvoiceField)> showInvoiceError);
	void askSendPassword(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError);
	void showSendingTransaction(
		const Ton::PendingTransaction &transaction,
		rpl::producer<> confirmed);
	void showSendingDone(std::optional<Ton::Transaction> result);
	void refreshNow();
	void receiveGrams();
	void createInvoice();
	void showInvoiceQr(const QString &link);
	void changePassword();
	void askExportPassword();
	void showExported(const std::vector<QString> &words);
	void showSettings();
	void checkConfigFromContent(QByteArray bytes, Fn<void(QByteArray)> good);
	void saveSettings(const Ton::Settings &settings);
	void saveSettingsWithLoaded(const Ton::Settings &settings);
	void saveSettingsSure(const Ton::Settings &settings, Fn<void()> done);
	void showBlockchainNameWarning(const Ton::Settings &settings);
	Fn<void(QImage, QString)> shareCallback(
		const QString &copied,
		const QString &qr);
	void logoutWithConfirmation();
	void logout();

	// Before _layers, because box destructor can set this pointer.
	std::unique_ptr<DecryptPasswordState> _decryptPasswordState;

	const not_null<Ton::Wallet*> _wallet;
	const std::unique_ptr<Ui::Window> _window;
	const std::unique_ptr<Ui::LayerManager> _layers;
	UpdateInfo * const _updateInfo = nullptr;

	std::unique_ptr<Create::Manager> _createManager;
	bool _importing = false;

	QString _address;
	std::unique_ptr<Ton::AccountViewer> _viewer;
	rpl::variable<Ton::WalletState> _state;
	rpl::variable<bool> _syncing;
	std::unique_ptr<Info> _info;
	object_ptr<Ui::FlatButton> _updateButton = { nullptr };
	rpl::event_stream<rpl::producer<int>> _updateButtonHeight;

	QPointer<Ui::GenericBox> _sendBox;
	QPointer<Ui::GenericBox> _sendConfirmBox;
	QPointer<Ui::GenericBox> _simpleErrorBox;
	QPointer<Ui::GenericBox> _settingsBox;
	QPointer<Ui::GenericBox> _saveConfirmBox;

};

} // namespace Wallet
