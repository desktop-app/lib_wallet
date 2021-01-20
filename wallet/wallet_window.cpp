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
#include "wallet/wallet_update_info.h"
#include "wallet/create/wallet_create_manager.h"
#include "ton/ton_wallet.h"
#include "ton/ton_account_viewer.h"
#include "base/platform/base_platform_process.h"
#include "base/qt_signal_producer.h"
#include "base/last_user_input.h"
#include "base/algorithm.h"
#include "ui/address_label.h"
#include "ui/widgets/window.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/input_fields.h"
#include "ui/widgets/buttons.h"
#include "ui/layers/layer_manager.h"
#include "ui/layers/generic_box.h"
#include "ui/toast/toast.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

#include <QtCore/QMimeData>
#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtGui/QtEvents>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Wallet {
namespace {

constexpr auto kRefreshEachDelay = 10 * crl::time(1000);
constexpr auto kRefreshInactiveDelay = 60 * crl::time(1000);
constexpr auto kRefreshWhileSendingDelay = 3 * crl::time(1000);

[[nodiscard]] bool ValidateTransferLink(const QString &link) {
	return QRegularExpression(
		QString("^((ton://)?transfer/)?[a-z0-9_\\-]{%1}/?($|\\?)"
		).arg(kAddressLength),
		QRegularExpression::CaseInsensitiveOption
	).match(link.trimmed()).hasMatch();
}

} // namespace

Window::Window(
	not_null<Ton::Wallet*> wallet,
	UpdateInfo *updateInfo)
: _wallet(wallet)
, _window(std::make_unique<Ui::Window>())
, _layers(std::make_unique<Ui::LayerManager>(_window->body()))
, _updateInfo(updateInfo) {
	init();
	const auto keys = _wallet->publicKeys();
	if (keys.empty()) {
		showCreate();
	} else {
		showAccount(keys[0]);
	}
}

Window::~Window() = default;

void Window::init() {
	_window->setTitle(QString());
	_window->setGeometry(style::centerrect(
		qApp->primaryScreen()->geometry(),
		QRect(QPoint(), st::walletWindowSize)));
	_window->setMinimumSize(st::walletWindowSize);

	_layers->setHideByBackgroundClick(true);

	updatePalette();
	style::PaletteChanged(
	) | rpl::start_with_next([=] {
		updatePalette();
	}, _window->lifetime());

	startWallet();
}

void Window::startWallet() {
	const auto &was = _wallet->settings().net();
	if (was.useCustomConfig) {
		return;
	}
	const auto loaded = [=](Ton::Result<QByteArray> result) {
		auto copy = _wallet->settings();
		if (result
			&& !copy.net().useCustomConfig
			&& copy.net().configUrl == was.configUrl
			&& *result != copy.net().config) {
			copy.net().config = *result;
			saveSettingsSure(copy, [=] {
				if (_viewer) {
					refreshNow();
				}
			});
		}
		if (!_viewer) {
			_wallet->sync();
		}
	};
	_wallet->loadWebResource(was.configUrl, std::move(loaded));
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
	_updateButton.destroy();

	_window->setTitleStyle(st::defaultWindowTitle);
	_importing = false;
	_createManager = std::make_unique<Create::Manager>(
		_window->body(),
		_updateInfo);
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
			showAccount(_createManager->publicKey(), !_importing);
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
		if (result) {
			_createSyncing = rpl::event_stream<QString>();
			_createManager->showPasscode(_createSyncing.events());
		} else if (IsIncorrectMnemonicError(result.error())) {
			_importing = false;
			createShowIncorrectImport();
		} else {
			_importing = false;
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
		case Ton::Error::Type::Web: return "Request Error";
		}
		Unexpected("Error type in Window::showGenericError.");
	}();
	showSimpleError(
		rpl::single(QString(title)),
		rpl::single((error.details + "\n\n" + additional).trimmed()),
		ph::lng_wallet_ok());
}

void Window::showSendingError(const Ton::Error &error) {
	const auto additional = ""
		"Possible error, please wait. If your transaction disappears "
		"from the \"Pending\" list and does not appear "
		"in the list of recent transactions, try again.";
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
	if (!_importing) {
		createSaveKey(passcode, QString(), std::move(guard));
		return;
	}
	rpl::single(
		Ton::Update{ Ton::SyncState() }
	) | rpl::then(
		_wallet->updates()
	) | rpl::map([](const Ton::Update &update) {
		return v::match(update.data, [&](const Ton::SyncState &data) {
			if (!data.valid()
				|| data.current == data.to
				|| data.current == data.from) {
				return ph::lng_wallet_sync();
			} else {
				const auto percent = QString::number(
					(100 * (data.current - data.from)
						/ (data.to - data.from)));
				return ph::lng_wallet_sync_percent(
				) | rpl::map([=](QString &&text) {
					return text.replace("{percent}", percent);
				}) | rpl::type_erased();
			}
		}, [&](auto&&) {
			return ph::lng_wallet_sync();
		});
	}) | rpl::flatten_latest(
	) | rpl::start_to_stream(_createSyncing, _createManager->lifetime());

	const auto done = [=](Ton::Result<QString> result) {
		if (!result) {
			*guard = false;
			showGenericError(result.error());
			return;
		}
		createSaveKey(passcode, *result, guard);
	};
	_wallet->queryWalletAddress(crl::guard(this, done));
}

void Window::createSaveKey(
		const QByteArray &passcode,
		const QString &address,
		std::shared_ptr<bool> guard) {
	const auto done = [=](Ton::Result<QByteArray> result) {
		*guard = false;
		if (!result) {
			showGenericError(result.error());
			return;
		}
		_createManager->showReady(*result);
	};
	_wallet->saveKey(passcode, address, crl::guard(this, done));
}

void Window::showAccount(const QByteArray &publicKey, bool justCreated) {
	_layers->hideAll();
	_importing = false;
	_createManager = nullptr;

	_address = _wallet->getUsedAddress(publicKey);
	_viewer = _wallet->createAccountViewer(publicKey, _address);
	_state = _viewer->state() | rpl::map([](Ton::WalletViewerState &&state) {
		return std::move(state.wallet);
	});
	_syncing = false;
	_syncing = _wallet->updates() | rpl::map([](const Ton::Update &update) {
		return v::match(update.data, [&](const Ton::SyncState &data) {
			return data.valid() && (data.current != data.to);
		}, [&](auto&&) {
			return false;
		});
	});

	_window->setTitleStyle(st::walletWindowTitle);
	auto data = Info::Data();
	data.justCreated = justCreated;
	data.state = _viewer->state();
	data.loaded = _viewer->loaded();
	data.updates = _wallet->updates();
	data.collectEncrypted = _collectEncryptedRequests.events();
	data.updateDecrypted = _decrypted.events();
	data.share = shareAddressCallback();
	data.useTestNetwork = _wallet->settings().useTestNetwork;
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

	setupUpdateWithInfo();

	_info->actionRequests(
	) | rpl::start_with_next([=](Action action) {
		switch (action) {
		case Action::Refresh: refreshNow(); return;
		case Action::Export: askExportPassword(); return;
		case Action::Send: sendGrams(); return;
		case Action::Receive: receiveGrams(); return;
		case Action::ChangePassword: changePassword(); return;
		case Action::ShowSettings: showSettings(); return;
		case Action::LogOut: logoutWithConfirmation(); return;
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
		_layers->showBox(Box(
			ViewTransactionBox,
			std::move(data),
			_collectEncryptedRequests.events(),
			_decrypted.events(),
			shareAddressCallback(),
			[=] { decryptEverything(publicKey); },
			send));
	}, _info->lifetime());

	_info->decryptRequests(
	) | rpl::start_with_next([=] {
		decryptEverything(publicKey);
	}, _info->lifetime());

	_wallet->updates(
	) | rpl::filter([](const Ton::Update &update) {
		return v::is<Ton::DecryptPasswordNeeded>(update.data);
	}) | rpl::start_with_next([=](const Ton::Update &update) {
		askDecryptPassword(v::get<Ton::DecryptPasswordNeeded>(update.data));
	}, _info->lifetime());

	_wallet->updates(
	) | rpl::filter([](const Ton::Update &update) {
		return v::is<Ton::DecryptPasswordGood>(update.data);
	}) | rpl::start_with_next([=](const Ton::Update &update) {
		doneDecryptPassword(v::get<Ton::DecryptPasswordGood>(update.data));
	}, _info->lifetime());
}

void Window::decryptEverything(const QByteArray &publicKey) {
	auto transactions = std::vector<Ton::Transaction>();
	_collectEncryptedRequests.fire(&transactions);
	if (transactions.empty()) {
		return;
	}
	const auto done = [=](
			const Ton::Result<std::vector<Ton::Transaction>> &result) {
		if (!result) {
			showGenericError(result.error());
			return;
		}
		_decrypted.fire(&result.value());
	};
	_wallet->decrypt(
		publicKey,
		std::move(transactions),
		crl::guard(this, done));
}

void Window::askDecryptPassword(const Ton::DecryptPasswordNeeded &data) {
	const auto key = data.publicKey;
	const auto generation = data.generation;
	const auto already = (_decryptPasswordState
		&& _decryptPasswordState->box)
		? _decryptPasswordState->generation
		: 0;
	if (already == generation) {
		return;
	} else if (!_decryptPasswordState) {
		_decryptPasswordState = std::make_unique<DecryptPasswordState>();
	}
	_decryptPasswordState->generation = generation;
	if (!_decryptPasswordState->box) {
		auto box = Box(EnterPasscodeBox, [=](
				const QByteArray &passcode,
				Fn<void(QString)> showError) {
			_decryptPasswordState->showError = showError;
			_wallet->updateViewersPassword(key, passcode);
		});
		QObject::connect(box, &QObject::destroyed, [=] {
			if (!_decryptPasswordState->success) {
				_wallet->updateViewersPassword(key, QByteArray());
			}
			_decryptPasswordState = nullptr;
		});
		_decryptPasswordState->box = box.data();
		_layers->showBox(std::move(box));
	} else if (_decryptPasswordState->showError) {
		_decryptPasswordState->showError(
			ph::lng_wallet_passcode_incorrect(ph::now));
	}
}

void Window::doneDecryptPassword(const Ton::DecryptPasswordGood &data) {
	if (_decryptPasswordState
		&& _decryptPasswordState->generation < data.generation) {
		_decryptPasswordState->success = true;
		_decryptPasswordState->box->closeBox();
	}
}

void Window::setupUpdateWithInfo() {
	Expects(_info != nullptr);

	rpl::combine(
		_window->body()->sizeValue(),
		_updateButtonHeight.events() | rpl::flatten_latest()
	) | rpl::start_with_next([=](QSize size, int height) {
		_info->setGeometry({ 0, 0, size.width(), size.height() - height });
		if (height > 0) {
			_updateButton->setGeometry(
				0,
				size.height() - height,
				size.width(),
				height);
		}
	}, _info->lifetime());

	if (!_updateInfo) {
		_updateButtonHeight.fire(rpl::single(0));
		return;
	}

	rpl::merge(
		rpl::single(rpl::empty_value()),
		_updateInfo->isLatest(),
		_updateInfo->failed(),
		_updateInfo->ready()
	) | rpl::start_with_next([=] {
		if (_updateInfo->state() == UpdateState::Ready) {
			if (_updateButton) {
				return;
			}
			_updateButton.create(
				_window->body(),
				ph::lng_wallet_update(ph::now).toUpper(),
				st::walletUpdateButton);
			_updateButton->show();
			_updateButton->setClickedCallback([=] {
				_updateInfo->install();
			});
			_updateButtonHeight.fire(_updateButton->heightValue());

			_layers->raise();
		} else {
			_updateButtonHeight.fire(rpl::single(0));
			if (!_updateButton) {
				return;
			}
			_updateButton.destroy();
		}
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
	if (_viewer && ValidateTransferLink(link)) {
		sendGrams(link);
	}
	return true;
}

void Window::showConfigUpgrade(Ton::ConfigUpgrade upgrade) {
	if (upgrade == Ton::ConfigUpgrade::TestnetToTestnet2) {
		const auto message = "The TON test network has been reset.\n"
			"TON testnet2 is now operational.";
		showSimpleError(
			ph::lng_wallet_warning(),
			rpl::single(QString(message)),
			ph::lng_wallet_ok());
	} else if (upgrade == Ton::ConfigUpgrade::TestnetToMainnet) {
		const auto message = "The Gram Wallet has switched "
			"from the testing to the main network.\n\nIn case you want "
			"to perform more testing you can switch back "
			"to the Test Gram network in Settings "
			"and reconnect your wallet using 24 secret words.";
		showSimpleError(
			ph::lng_wallet_warning(),
			rpl::single(QString(message)),
			ph::lng_wallet_ok());
	}
}

void Window::sendGrams(const QString &invoice) {
	if (_sendConfirmBox) {
		_sendConfirmBox->closeBox();
	}
	if (_sendBox) {
		_sendBox->closeBox();
	}
	if (!_state.current().pendingTransactions.empty()) {
		showSimpleError(
			ph::lng_wallet_warning(),
			ph::lng_wallet_wait_pending(),
			ph::lng_wallet_ok());
		return;
	} else if (_syncing.current()) {
		showSimpleError(
			ph::lng_wallet_warning(),
			ph::lng_wallet_wait_syncing(),
			ph::lng_wallet_ok());
		return;
	}
	const auto checking = std::make_shared<bool>();
	const auto send = [=](
			const PreparedInvoice &invoice,
			Fn<void(InvoiceField)> showError) {
		const auto account = _state.current().account;
		const auto available = account.fullBalance - account.lockedBalance;
		if (!Ton::Wallet::CheckAddress(invoice.address)) {
			showError(InvoiceField::Address);
		} else if (invoice.amount > available || invoice.amount <= 0) {
			showError(InvoiceField::Amount);
		} else {
			confirmTransaction(invoice, showError, checking);
		}
	};
	auto unlockedBalance = _state.value(
	) | rpl::map([](const Ton::WalletState &state) {
		return state.account.fullBalance - state.account.lockedBalance;
	});
	auto box = Box(
		SendGramsBox,
		invoice,
		std::move(unlockedBalance),
		send);
	_sendBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::confirmTransaction(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError,
		std::shared_ptr<bool> guard) {
	if (*guard || !_sendBox) {
		return;
	}
	*guard = true;
	auto done = [=](Ton::Result<Ton::TransactionCheckResult> result) {
		*guard = false;
		if (!result) {
			if (const auto field = ErrorInvoiceField(result.error())) {
				showInvoiceError(*field);
			} else if (!invoice.sendUnencryptedText
				&& result.error().details.startsWith("MESSAGE_ENCRYPTION")) {
				auto copy = invoice;
				copy.sendUnencryptedText = true;
				confirmTransaction(copy, showInvoiceError, guard);
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
		crl::guard(_sendBox.data(), done));
}

void Window::askSendPassword(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError) {
	const auto publicKey = _wallet->publicKeys().front();
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
			_wallet->updateViewersPassword(publicKey, passcode);
			decryptEverything(publicKey);
		};
		const auto sent = [=](Ton::Result<> result) {
			if (!result) {
				showSendingError(result.error());
				return;
			}
			confirmations->fire({});
		};
		_wallet->sendGrams(
			publicKey,
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
	const auto account = _state.current().account;
	const auto available = account.fullBalance - account.lockedBalance;
	// This may be enabled in the future, but right now it is not safe.
	// You could think that you transfer specific amount, but really
	// you're transferring all the remaining funds, even if they change
	// while the transfer request is already being sent.
	//
	//if (invoice.amount == available && account.lockedBalance == 0) {
	//	// Special case transaction where we transfer all that is left.
	//} else
	if (invoice.amount + checkResult.sourceFees.sum() > available) {
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
		_testnet,
		[=] { createInvoice(); },
		shareAddressCallback()));
}

void Window::createInvoice() {
	_layers->showBox(Box(
		CreateInvoiceBox,
		_address,
		_testnet,
		[=](const QString &link) { showInvoiceQr(link); },
		shareCallback(
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_receive_copied_qr(ph::now))));
}

void Window::showInvoiceQr(const QString &link) {
	_layers->showBox(Box(
		InvoiceQrBox,
		link,
		shareCallback(
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_invoice_copied(ph::now),
			ph::lng_wallet_receive_copied_qr(ph::now))));
}

Fn<void(QImage, QString)> Window::shareCallback(
		const QString &linkCopied,
		const QString &textCopied,
		const QString &qr) {
	return [=](const QImage &image, const QString &text) {
		if (!image.isNull()) {
			auto mime = std::make_unique<QMimeData>();
			if (!text.isEmpty()) {
				mime->setText(text);
			}
			mime->setImageData(image);
			QGuiApplication::clipboard()->setMimeData(mime.release());
			showToast(qr);
		} else {
			QGuiApplication::clipboard()->setText(text);
			showToast((text.indexOf("://") >= 0) ? linkCopied : textCopied);
		}
	};
}

Fn<void(QImage, QString)> Window::shareAddressCallback() {
	return shareCallback(
		ph::lng_wallet_receive_copied(ph::now),
		ph::lng_wallet_receive_address_copied(ph::now),
		ph::lng_wallet_receive_copied_qr(ph::now));
}

void Window::showToast(const QString &text) {
	Ui::Toast::Show(_window.get(), text);
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
	const auto checkConfig = [=](QString path, Fn<void(QByteArray)> good) {
		checkConfigFromContent([&] {
			auto file = QFile(path);
			file.open(QIODevice::ReadOnly);
			return file.readAll();
		}(), std::move(good));
	};
	auto box = Box(
		SettingsBox,
		_wallet->settings(),
		_updateInfo,
		checkConfig,
		[=](const Ton::Settings &settings) { saveSettings(settings); });
	_settingsBox = box.data();
	_layers->showBox(std::move(box));
}

void Window::checkConfigFromContent(
		QByteArray bytes,
		Fn<void(QByteArray)> good) {
	_wallet->checkConfig(bytes, [=](Ton::Result<> result) {
		if (result) {
			good(bytes);
		} else {
			showSimpleError(
				ph::lng_wallet_error(),
				ph::lng_wallet_bad_config(),
				ph::lng_wallet_ok());
		}
	});
}

void Window::saveSettings(const Ton::Settings &settings) {
	if (settings.net().useCustomConfig) {
		saveSettingsWithLoaded(settings);
		return;
	}
	const auto loaded = [=](Ton::Result<QByteArray> result) {
		if (!result) {
			if (result.error().type == Ton::Error::Type::Web) {
				using namespace rpl::mappers;
				showSimpleError(
					ph::lng_wallet_error(),
					ph::lng_wallet_bad_config_url(
					) | rpl::map(_1 + "\n\n" + result.error().details),
					ph::lng_wallet_ok());
			} else {
				showGenericError(result.error());
			}
			return;
		}
		checkConfigFromContent(*result, [=](QByteArray config) {
			auto copy = settings;
			copy.net().config = config;
			saveSettingsWithLoaded(copy);
		});
	};
	_wallet->loadWebResource(settings.net().configUrl, loaded);
}

void Window::saveSettingsWithLoaded(const Ton::Settings &settings) {
	const auto &current = _wallet->settings();
	const auto change = (settings.useTestNetwork != current.useTestNetwork);
	if (change) {
		showSwitchTestNetworkWarning(settings);
		return;
	}
	const auto detach = (settings.net().blockchainName
		!= current.net().blockchainName);
	if (detach) {
		showBlockchainNameWarning(settings);
		return;
	}
	saveSettingsSure(settings, [=] {
		if (_settingsBox) {
			_settingsBox->closeBox();
		}
		if (_viewer) {
			refreshNow();
		}
	});
}

void Window::saveSettingsSure(
		const Ton::Settings &settings,
		Fn<void()> done) {
	const auto showError = [=](Ton::Error error) {
		if (_saveConfirmBox) {
			_saveConfirmBox->closeBox();
		}
		showGenericError(error);
	};
	_wallet->updateSettings(settings, [=](Ton::Result<> result) {
		if (!result) {
			if (_wallet->publicKeys().empty()) {
				showCreate();
			}
			showError(result.error());
		} else {
			done();
		}
	});
}

void Window::refreshNow() {
	_viewer->refreshNow([=](Ton::Result<> result) {
		if (!result) {
			showGenericError(result.error());
		}
	});
}

void Window::showSwitchTestNetworkWarning(const Ton::Settings &settings) {
	showSettingsWithLogoutWarning(
		settings,
		(settings.useTestNetwork
			? ph::lng_wallet_warning_to_testnet()
			: ph::lng_wallet_warning_to_mainnet()));
}

void Window::showBlockchainNameWarning(const Ton::Settings &settings) {
	Expects(settings.useTestNetwork);

	showSettingsWithLogoutWarning(
		settings,
		ph::lng_wallet_warning_blockchain_name());
}

void Window::showSettingsWithLogoutWarning(
		const Ton::Settings &settings,
		rpl::producer<QString> text) {
	using namespace rpl::mappers;

	const auto saving = std::make_shared<bool>();
	auto box = Box([=](not_null<Ui::GenericBox*> box) mutable {
		box->setTitle(ph::lng_wallet_warning());
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			rpl::combine(
				std::move(text),
				ph::lng_wallet_warning_reconnect()
			) | rpl::map(_1 + "\n\n" + _2),
			st::walletLabel));
		box->addButton(ph::lng_wallet_continue(), [=] {
			if (std::exchange(*saving, true)) {
				return;
			}
			saveSettingsSure(settings, [=] {
				logout();
			});
		}, st::attentionBoxButton);
		box->addButton(ph::lng_wallet_cancel(), [=] {
			box->closeBox();
		});
	});
	_saveConfirmBox = box.data();
	_layers->showBox(std::move(box));
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

void Window::logoutWithConfirmation() {
	_layers->showBox(Box(DeleteWalletBox, [=] { logout(); }));
}

void Window::logout() {
	_wallet->deleteAllKeys(crl::guard(this, [=](Ton::Result<> result) {
		if (!result) {
			showGenericError(result.error());
			return;
		}
		showCreate();
	}));
}

} // namespace Wallet
