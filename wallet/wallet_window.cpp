// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_window.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_common.h"
#include "wallet/wallet_info.h"
#include "wallet/wallet_view_transaction.h"
#include "wallet/wallet_receive_grams.h"
#include "wallet/wallet_create_invoice.h"
#include "wallet/wallet_invoice_qr.h"
#include "wallet/wallet_send_grams.h"
#include "wallet/wallet_enter_passcode.h"
#include "wallet/wallet_change_passcode.h"
#include "wallet/wallet_confirm_transaction.h"
#include "wallet/wallet_sending_transaction.h"
#include "wallet/wallet_delete.h"
#include "wallet/wallet_export.h"
#include "wallet/wallet_update_info.h"
#include "wallet/wallet_settings.h"
#include "wallet/create/wallet_create_manager.h"
#include "ton/ton_wallet.h"
#include "ton/ton_account_viewer.h"
#include "base/platform/base_platform_process.h"
#include "base/qt_signal_producer.h"
#include "base/last_user_input.h"
#include "ui/address_label.h"
#include "ui/widgets/window.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/input_fields.h"
#include "ui/layers/layer_manager.h"
#include "ui/layers/generic_box.h"
#include "ui/toast/toast.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

#include <QtCore/QMimeData>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtGui/QClipboard>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Wallet {
namespace {

constexpr auto kRefreshEachDelay = 10 * crl::time(1000);
constexpr auto kRefreshInactiveDelay = 60 * crl::time(1000);
constexpr auto kRefreshWhileSendingDelay = 3 * crl::time(1000);

} // namespace

Window::Window(
	not_null<Ton::Wallet*> wallet,
	UpdateInfo *updateInfo)
: _wallet(wallet)
, _window(std::make_unique<Ui::Window>())
, _layers(std::make_unique<Ui::LayerManager>(_window->body()))
, _updateInfo(updateInfo) {
	init();
	if (_wallet->publicKeys().empty()) {
		showCreate();
	} else {
		showAccount(_wallet->publicKeys()[0]);
	}
}

Window::~Window() = default;

void Window::init() {
	_window->setTitle(QString());
	_window->setGeometry(style::centerrect(
		QApplication::desktop()->geometry(),
		QRect(QPoint(), st::walletWindowSize)));
	_window->setFixedSize(st::walletWindowSize);

	_layers->setHideByBackgroundClick(true);

	updatePalette();
	style::PaletteChanged(
	) | rpl::start_with_next([=] {
		updatePalette();
	}, _window->lifetime());
}

void Window::updatePalette() {
	auto palette = _window->palette();
	palette.setColor(QPalette::Window, st::windowBg->c);
	_window->setPalette(palette);
	Ui::ForceFullRepaint(_window.get());
}

void Window::showCreate() {
	_layers->hideAll();
	_info = nullptr;
	_viewer = nullptr;

	_window->setTitleStyle(st::defaultWindowTitle);
	_createManager = std::make_unique<Create::Manager>(_window->body());
	_layers->raise();

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_createManager->setGeometry({ QPoint(), size });
	}, _createManager->lifetime());

	const auto creating = std::make_shared<bool>();
	_createManager->actionRequests(
	) | rpl::start_with_next([=](Create::Manager::Action action) {
		switch (action) {
		case Create::Manager::Action::NewKey:
			if (!_importing) {
				_createManager->showIntro();
			}
			return;
		case Create::Manager::Action::CreateKey:
			createKey(creating);
			return;
		case Create::Manager::Action::ShowCheckIncorrect:
			createShowIncorrectWords();
			return;
		case Create::Manager::Action::ShowCheckTooSoon:
			createShowTooFastWords();
			return;
		case Create::Manager::Action::ShowImportFail:
			createShowImportFail();
			return;
		case Create::Manager::Action::ShowAccount:
			showAccount(_createManager->publicKey());
			return;
		}
		Unexpected("Action in Create::Manager::actionRequests().");
	}, _createManager->lifetime());

	_createManager->importRequests(
	) | rpl::start_with_next([=](const std::vector<QString> &words) {
		createImportKey(words);
	}, _createManager->lifetime());

	const auto saving = std::make_shared<bool>();
	_createManager->passcodeChosen(
	) | rpl::start_with_next([=](const QByteArray &passcode) {
		createSavePasscode(passcode, saving);
	}, _createManager->lifetime());
}

void Window::createImportKey(const std::vector<QString> &words) {
	if (std::exchange(_importing, true)) {
		return;
	}
	_wallet->importKey(words, crl::guard(this, [=](Ton::Result<> result) {
		_importing = false;
		if (result) {
			_createManager->showPasscode();
		} else if (IsIncorrectMnemonicError(result.error())) {
			createShowIncorrectImport();
		} else {
			showGenericError(result.error());
		}
	}));
}

void Window::createKey(std::shared_ptr<bool> guard) {
	if (std::exchange(*guard, true)) {
		return;
	}
	const auto done = [=](Ton::Result<std::vector<QString>> result) {
		Expects(result.has_value());

		*guard = false;
		_createManager->showCreated(std::move(*result));
	};
	_wallet->createKey(crl::guard(this, done));
}

void Window::createShowIncorrectWords() {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(ph::lng_wallet_check_incorrect_title());
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_check_incorrect_text(),
			st::walletLabel));
		box->addButton(ph::lng_wallet_check_incorrect_retry(), [=] {
			box->closeBox();
			_createManager->setFocus();
		});
		box->addButton(ph::lng_wallet_check_incorrect_view(), [=] {
			box->closeBox();
			_createManager->showWords(Create::Direction::Backward);
		});
	}));
}

void Window::createShowTooFastWords() {
	showSimpleError(
		ph::lng_wallet_words_sure_title(),
		ph::lng_wallet_words_sure_text(),
		ph::lng_wallet_words_sure_ok());
}

void Window::createShowIncorrectImport() {
	showSimpleError(
		ph::lng_wallet_import_incorrect_title(),
		ph::lng_wallet_import_incorrect_text(),
		ph::lng_wallet_import_incorrect_retry());
}

void Window::createShowImportFail() {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(ph::lng_wallet_too_bad_title());
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			ph::lng_wallet_too_bad_description(),
			st::walletLabel));
		box->addButton(ph::lng_wallet_too_bad_enter_words(), [=] {
			box->closeBox();
			_createManager->setFocus();
		});
		box->addButton(ph::lng_wallet_cancel(), [=] {
			box->closeBox();
			_createManager->showIntro();
		});
	}));
}

void Window::showSimpleError(
		rpl::producer<QString> title,
		rpl::producer<QString> text,
		rpl::producer<QString> button) {
	if (_simpleErrorBox) {
		_simpleErrorBox->closeBox();
	}
	auto box = Box([&](not_null<Ui::GenericBox*> box) mutable {
		box->setTitle(std::move(title));
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			std::move(text),
			st::walletLabel));
		box->addButton(std::move(button), [=] {
			box->closeBox();
			if (_createManager) {
				_createManager->setFocus();
			}
		});
	});
	_simpleErrorBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::showGenericError(
		const Ton::Error &error,
		const QString &additional) {
	const auto title = [&] {
		switch (error.type) {
		case Ton::Error::Type::IO: return "Disk Error";
		case Ton::Error::Type::TonLib: return "Library Error";
		case Ton::Error::Type::WrongPassword: return "Encryption Error";
		}
		Unexpected("Error type in Window::showGenericError.");
	}();
	showSimpleError(
		rpl::single(QString(title)),
		rpl::single((error.details + "\n\n" + additional).trimmed()),
		rpl::single(QString("OK")));
}

void Window::showSendingError(const Ton::Error &error) {
	const auto additional = ""
		"Your transaction may or may not be processed successfully, "
		"so please wait till it disappears from the 'Pending' block "
		"and see if it appears in the recent transactions list.";
	showGenericError(error, additional);
	if (_sendBox) {
		_sendBox->closeBox();
	}
}

void Window::createSavePasscode(
		const QByteArray &passcode,
		std::shared_ptr<bool> guard) {
	if (std::exchange(*guard, true)) {
		return;
	}
	const auto done = [=](Ton::Result<QByteArray> result) {
		*guard = false;
		if (!result) {
			showGenericError(result.error());
			return;
		}
		_createManager->showReady(*result);
	};
	_wallet->saveKey(passcode, crl::guard(this, done));
}

void Window::showAccount(const QByteArray &publicKey) {
	_layers->hideAll();
	_createManager = nullptr;

	_address = Ton::Wallet::GetAddress(publicKey);
	_viewer = _wallet->createAccountViewer(_address);
	_state = _viewer->state() | rpl::map([](Ton::WalletViewerState &&state) {
		return std::move(state.wallet);
	});

	_window->setTitleStyle(st::walletWindowTitle);
	auto data = Info::Data();
	data.state = _viewer->state();
	data.loaded = _viewer->loaded();
	data.updates = _wallet->updates();
	_info = std::make_unique<Info>(_window->body(), std::move(data));
	_layers->raise();

	setupRefreshEach();

	_viewer->loaded(
	) | rpl::filter([](const Ton::Result<Ton::LoadedSlice> &value) {
		return !value;
	}) | rpl::map([](Ton::Result<Ton::LoadedSlice> &&value) {
		return std::move(value.error());
	}) | rpl::start_with_next([=](const Ton::Error &error) {
		showGenericError(error);
	}, _info->lifetime());

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_info->setGeometry({ QPoint(), size });
	}, _info->lifetime());

	_info->actionRequests(
	) | rpl::start_with_next([=](Action action) {
		switch (action) {
		case Action::Refresh: _viewer->refreshNow(nullptr); return;
		case Action::Export: askExportPassword(); return;
		case Action::Send: sendGrams(); return;
		case Action::Receive: receiveGrams(); return;
		case Action::ChangePassword: changePassword(); return;
		case Action::ShowSettings: showSettings(); return;
		case Action::LogOut: logout(); return;
		}
		Unexpected("Action in Info::actionRequests().");
	}, _info->lifetime());

	_info->preloadRequests(
	) | rpl::start_with_next([=](const Ton::TransactionId &id) {
		_viewer->preloadSlice(id);
	}, _info->lifetime());

	_info->viewRequests(
	) | rpl::start_with_next([=](Ton::Transaction &&data) {
		const auto send = [=](const QString &address) {
			sendGrams(address);
		};
		_layers->showBox(Box(ViewTransactionBox, std::move(data), send));
	}, _info->lifetime());
}

void Window::setupRefreshEach() {
	Expects(_viewer != nullptr);
	Expects(_info != nullptr);

	const auto basedOnActivity = _viewer->state(
	) | rpl::map([] {
		return (base::SinceLastUserInput() > kRefreshEachDelay)
			? kRefreshInactiveDelay
			: kRefreshEachDelay;
	});

	const auto basedOnWindowActive = rpl::single(
		rpl::empty_value()
	) | rpl::then(base::qt_signal_producer(
		_window->windowHandle(),
		&QWindow::activeChanged
	)) | rpl::map([=]() -> rpl::producer<crl::time> {
		if (!_window->isActiveWindow()) {
			return rpl::single(kRefreshInactiveDelay);
		}
		return rpl::duplicate(basedOnActivity);
	}) | rpl::flatten_latest();

	const auto basedOnPending = _viewer->state(
	) | rpl::map([=](const Ton::WalletViewerState &state) {
		return !state.wallet.pendingTransactions.empty();
	}) | rpl::distinct_until_changed(
	) | rpl::map([=](bool hasPending) -> rpl::producer<crl::time> {
		if (hasPending) {
			return rpl::single(kRefreshWhileSendingDelay);
		}
		return rpl::duplicate(basedOnWindowActive);
	}) | rpl::flatten_latest(
	);

	rpl::duplicate(
		basedOnPending
	) | rpl::distinct_until_changed(
	) | rpl::start_with_next([=](crl::time delay) {
		_viewer->setRefreshEach(delay);
	}, _info->lifetime());
}

void Window::showAndActivate() {
	_window->show();
	base::Platform::ActivateThisProcessWindow(_window->winId());
	_window->activateWindow();
	if (_createManager) {
		_createManager->setFocus();
	} else {
		_window->setFocus();
	}
}

not_null<Ui::RpWidget*> Window::widget() const {
	return _window.get();
}

bool Window::handleLinkOpen(const QString &link) {
	if (_viewer) {
		sendGrams(link);
	}
	return true;
}

void Window::sendGrams(const QString &invoice) {
	if (_sendConfirmBox) {
		_sendConfirmBox->closeBox();
	}
	if (_sendBox) {
		_sendBox->closeBox();
	}
	const auto checking = std::make_shared<bool>();
	const auto send = [=](
			const PreparedInvoice &invoice,
			Fn<void(InvoiceField)> showError) {
		if (!Ton::Wallet::CheckAddress(invoice.address)) {
			showError(InvoiceField::Address);
		} else if (invoice.amount > _state.current().account.balance) {
			showError(InvoiceField::Amount);
		} else {
			confirmTransaction(invoice, showError, checking);
		}
	};
	auto balance = _state.value(
	) | rpl::map([](const Ton::WalletState &state) {
		return state.account.balance;
	});
	auto box = Box(
		SendGramsBox,
		invoice,
		std::move(balance),
		send);
	_sendBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::confirmTransaction(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError,
		std::shared_ptr<bool> guard) {
	if (*guard) {
		return;
	}
	*guard = true;
	auto done = [=](Ton::Result<Ton::TransactionCheckResult> result) {
		*guard = false;
		if (!result) {
			if (const auto field = ErrorInvoiceField(result.error())) {
				showInvoiceError(*field);
			} else {
				showGenericError(result.error());
			}
			return;
		}
		showSendConfirmation(
			invoice,
			*result,
			showInvoiceError);
	};
	_wallet->checkSendGrams(
		_wallet->publicKeys().front(),
		TransactionFromInvoice(invoice),
		crl::guard(this, done));
}

void Window::askSendPassword(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError) {
	const auto sending = std::make_shared<bool>();
	const auto ready = [=](
			const QByteArray &passcode,
			const PreparedInvoice &invoice,
			Fn<void(QString)> showError) {
		if (*sending) {
			return;
		}
		const auto confirmations = std::make_shared<rpl::event_stream<>>();
		*sending = true;
		auto ready = [=](Ton::Result<Ton::PendingTransaction> result) {
			if (!result && IsIncorrectPasswordError(result.error())) {
				*sending = false;
				showError(ph::lng_wallet_passcode_incorrect(ph::now));
				return;
			}
			if (_sendConfirmBox) {
				_sendConfirmBox->closeBox();
			}
			if (!result) {
				if (const auto field = ErrorInvoiceField(result.error())) {
					showInvoiceError(*field);
				} else {
					showGenericError(result.error());
				}
				return;
			}
			showSendingTransaction(*result, confirmations->events());
		};
		const auto sent = [=](Ton::Result<> result) {
			if (!result) {
				showSendingError(result.error());
				return;
			}
			confirmations->fire({});
		};
		_wallet->sendGrams(
			_wallet->publicKeys().front(),
			passcode,
			TransactionFromInvoice(invoice),
			crl::guard(this, ready),
			crl::guard(this, sent));
	};
	if (_sendConfirmBox) {
		_sendConfirmBox->closeBox();
	}
	auto box = Box(EnterPasscodeBox, [=](
			const QByteArray &passcode,
			Fn<void(QString)> showError) {
		ready(passcode, invoice, showError);
	});
	_sendConfirmBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::showSendConfirmation(
		const PreparedInvoice &invoice,
		const Ton::TransactionCheckResult &checkResult,
		Fn<void(InvoiceField)> showInvoiceError) {
	const auto balance = _state.current().account.balance;
	if (invoice.amount + checkResult.sourceFees.sum() > balance) {
		showInvoiceError(InvoiceField::Amount);
		return;
	}
	const auto confirmed = [=] {
		if (invoice.address == _address) {
			_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
				box->setTitle(ph::lng_wallet_same_address_title());
				box->addRow(object_ptr<Ui::FlatLabel>(
					box,
					ph::lng_wallet_same_address_text(),
					st::walletLabel));
				box->addButton(ph::lng_wallet_same_address_proceed(), [=] {
					box->closeBox();
					askSendPassword(invoice, showInvoiceError);
				});
				box->addButton(ph::lng_wallet_cancel(), [=] {
					box->closeBox();
					if (_sendConfirmBox) {
						_sendConfirmBox->closeBox();
					}
				});
			}));
		} else {
			askSendPassword(invoice, showInvoiceError);
		}
	};
	auto box = Box(
		ConfirmTransactionBox,
		invoice,
		checkResult.sourceFees.sum(),
		confirmed);
	_sendConfirmBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::showSendingTransaction(
		const Ton::PendingTransaction &transaction,
		rpl::producer<> confirmed) {
	if (_sendBox) {
		_sendBox->closeBox();
	}
	auto box = Box(SendingTransactionBox, std::move(confirmed));
	_sendBox = box.data();
	_state.value(
	) | rpl::filter([=](const Ton::WalletState &state) {
		return ranges::find(state.pendingTransactions, transaction)
			== end(state.pendingTransactions);
	}) | rpl::map([=](const Ton::WalletState &state) {
		const auto i = ranges::find(
			state.lastTransactions.list,
			transaction.fake);
		return (i != end(state.lastTransactions.list))
			? std::make_optional(*i)
			: std::nullopt;
	}) | rpl::start_with_next([=](std::optional<Ton::Transaction> &&result) {
		showSendingDone(std::move(result));
	}, _sendBox->lifetime());
	_layers->showBox(std::move(box));

	if (_sendConfirmBox) {
		_sendConfirmBox->closeBox();
	}
}

void Window::showSendingDone(std::optional<Ton::Transaction> result) {
	if (result) {
		_layers->showBox(Box(SendingDoneBox, *result));
	} else {
		showSimpleError(
			ph::lng_wallet_send_failed_title(),
			ph::lng_wallet_send_failed_text(),
			ph::lng_wallet_continue());
	}

	if (_sendBox) {
		_sendBox->closeBox();
	}
}

void Window::receiveGrams() {
	_layers->showBox(Box(
		ReceiveGramsBox,
		_address,
		TransferLink(_address),
		[=] { createInvoice(); },
		shareCallback(
			ph::lng_wallet_receive_copied(ph::now),
			ph::lng_wallet_receive_copied_qr(ph::now))));
}

void Window::createInvoice() {
	_layers->showBox(Box(
		CreateInvoiceBox,
		_address,
		[=](const QString &link) { showInvoiceQr(link); },
		shareCallback(
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_receive_copied_qr(ph::now))));
}

void Window::showInvoiceQr(const QString &link) {
	_layers->showBox(Box(
		InvoiceQrBox,
		link,
		shareCallback(
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_receive_copied_qr(ph::now))));
}

Fn<void(QImage, QString)> Window::shareCallback(
		const QString &copied,
		const QString &qr) {
	return [=](const QImage &image, const QString &link) {
		if (!image.isNull()) {
			auto mime = std::make_unique<QMimeData>();
			if (!link.isEmpty()) {
				mime->setText(link);
			}
			mime->setImageData(image);
			QGuiApplication::clipboard()->setMimeData(mime.release());
			showToast(qr);
		} else {
			QGuiApplication::clipboard()->setText(link);
			showToast(copied);
		}
	};
}

void Window::showToast(const QString &text) {
	auto toast = Ui::Toast::Config();
	toast.text = text;
	Ui::Toast::Show(_window.get(), toast);
}

void Window::changePassword() {
	const auto saving = std::make_shared<bool>();
	const auto weakBox = std::make_shared<QPointer<Ui::GenericBox>>();
	auto box = Box(ChangePasscodeBox, [=](
			const QByteArray &old,
			const QByteArray &now,
			Fn<void(QString)> showError) {
		if (std::exchange(*saving, true)) {
			return;
		}
		const auto done = [=](Ton::Result<> result) {
			if (!result) {
				*saving = false;
				if (IsIncorrectPasswordError(result.error())) {
					showError(ph::lng_wallet_passcode_incorrect(ph::now));
				} else {
					showGenericError(result.error());
				}
				return;
			}
			if (*weakBox) {
				(*weakBox)->closeBox();
			}
			showToast(ph::lng_wallet_change_passcode_done(ph::now));
		};
		_wallet->changePassword(old, now, crl::guard(this, done));
	});
	*weakBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::showSettings() {
	const auto callback = [=](Settings::Action action) {
		using namespace Settings;
		action.match([&](ToggleUpdates data) {
			_updateInfo->toggle(data.enabled);
		}, [&](InstallUpdate) {
			_updateInfo->install();
		}, [&](AllowTestUpdates) {
			_updateInfo->test();
		});
	};
	_layers->showBox(Box(Settings::CreateBox, _updateInfo, callback));
}

void Window::askExportPassword() {
	const auto exporting = std::make_shared<bool>();
	const auto weakBox = std::make_shared<QPointer<Ui::GenericBox>>();
	const auto ready = [=](
			const QByteArray &passcode,
			Fn<void(QString)> showError) {
		if (*exporting) {
			return;
		}
		*exporting = true;
		const auto ready = [=](Ton::Result<std::vector<QString>> result) {
			*exporting = false;
			if (!result) {
				if (IsIncorrectPasswordError(result.error())) {
					showError(ph::lng_wallet_passcode_incorrect(ph::now));
				} else {
					showGenericError(result.error());
				}
				return;
			}
			if (*weakBox) {
				(*weakBox)->closeBox();
			}
			showExported(*result);
		};
		_wallet->exportKey(
			_wallet->publicKeys().front(),
			passcode,
			crl::guard(this, ready));
	};
	auto box = Box(EnterPasscodeBox, [=](
			const QByteArray &passcode,
			Fn<void(QString)> showError) {
		ready(passcode, showError);
	});
	*weakBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::showExported(const std::vector<QString> &words) {
	_layers->showBox(Box(ExportedBox, words));
}

void Window::logout() {
	_layers->showBox(Box(DeleteWalletBox, [=] {
		_wallet->deleteAllKeys(crl::guard(this, [=](Ton::Result<> result) {
			if (!result) {
				showGenericError(result.error());
				return;
			}
			showCreate();
		}));
	}));
}

} // namespace Wallet
