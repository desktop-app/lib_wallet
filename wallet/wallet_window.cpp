// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_window.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_common.h"
#include "wallet/wallet_intro.h"
#include "wallet/wallet_info.h"
#include "wallet/wallet_view_transaction.h"
#include "wallet/wallet_receive_grams.h"
#include "wallet/wallet_send_grams.h"
#include "wallet/wallet_enter_passcode.h"
#include "wallet/wallet_confirm_transaction.h"
#include "wallet/wallet_sending_transaction.h"
#include "wallet/wallet_delete.h"
#include "ton/ton_wallet.h"
#include "ton/ton_account_viewer.h"
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

#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QtEvents>
#include <QtGui/QClipboard>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Wallet {
namespace {

constexpr auto kRefreshEachDelay = 10 * crl::time(1000);
constexpr auto kRefreshWhileSendingDelay = 3 * crl::time(1000);

} // namespace

Window::Window(not_null<Ton::Wallet*> wallet)
: _wallet(wallet)
, _window(std::make_unique<Ui::Window>())
, _layers(std::make_unique<Ui::LayerManager>(_window->body())) {
	init();
	if (_wallet->publicKeys().empty()) {
		showIntro();
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
	_window->setTitleStyle(st::walletWindowTitle);
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

void Window::showIntro() {
	_layers->hideAll();
	_info = nullptr;
	_viewer = nullptr;

	_intro = std::make_unique<Intro>(_window->body());
	_layers->raise();

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_intro->setGeometry({ QPoint(), size });
	}, _intro->lifetime());

	_intro->actionRequests(
	) | rpl::start_with_next([=](Intro::Action action) {
		switch (action) {
		case Intro::Action::CreateWallet: {
			_wallet->createKey([=](
				Ton::Result<std::vector<QString>> result) {
				if (result) {
					saveKey(*result);
				}
			});
		} break;
		}
	}, _intro->lifetime());
}

void Window::showAccount(const QByteArray &publicKey) {
	_intro = nullptr;

	_address = Ton::Wallet::GetAddress(publicKey);
	_viewer = _wallet->createAccountViewer(_address);
	_viewer->setRefreshEach(kRefreshEachDelay);
	_state = _viewer->state() | rpl::map([](Ton::WalletViewerState &&state) {
		return std::move(state.wallet);
	});

	auto data = Info::Data();
	data.state = _viewer->state();
	data.loaded = _viewer->loaded();
	_info = std::make_unique<Info>(_window->body(), data);

	_layers->raise();
	_layers->hideAll();

	_window->body()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		_info->setGeometry({ QPoint(), size });
	}, _info->lifetime());

	_info->actionRequests(
	) | rpl::start_with_next([=](Action action) {
		switch (action) {
		case Action::Refresh: _viewer->refreshNow(nullptr); break;
		case Action::Export: /*export();*/ break;
		case Action::Send: sendGrams(); break;
		case Action::Receive: receiveGrams(); break;
		case Action::ChangePassword: changePassword(); break;
		case Action::LogOut: logout(); break;
		}
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

void Window::show() {
	_window->show();
}

void Window::setFocus() {
	_window->setFocus();
}

void Window::saveKey(const std::vector<QString> &words) {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Words")));
		for (const auto &word : words) {
			box->addRow(object_ptr<Ui::FlatLabel>(
				box,
				rpl::single(word),
				st::boxLabel));
		}
		const auto passwordWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto password = Ui::CreateChild<Ui::PasswordInput>(
			passwordWrap,
			st::defaultInputField,
			rpl::single(QString("Password")));
		passwordWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			password->resize(width, password->height());
		}, password->lifetime());
		password->heightValue(
		) | rpl::start_with_next([=](int height) {
			passwordWrap->resize(passwordWrap->width(), height);
		}, password->lifetime());
		password->move(0, 0);
		const auto saving = box->lifetime().make_state<bool>(false);
		box->setCloseByEscape(false);
		box->setCloseByOutsideClick(false);
		box->addButton(rpl::single(QString("Save")), [=] {
			if (*saving) {
				return;
			}
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*saving = true;
			_wallet->saveKey(pwd.toUtf8(), [=](Ton::Result<QByteArray> r) {
				*saving = false;
				if (r) {
					box->closeBox();
					showAccount(_wallet->publicKeys()[0]);
				} else {
					password->showError();
					return;
				}
			});
		});
	}));
}

void Window::sendGrams(const QString &invoice) {
	const auto send = [=](
			const PreparedInvoice &invoice,
			Fn<void(InvoiceField)> showError) {
		if (!Ton::Wallet::CheckAddress(invoice.address)) {
			showError(InvoiceField::Address);
		} else if (invoice.amount > _state.current().account.balance) {
			showError(InvoiceField::Amount);
		} else {
			askSendPassword(invoice, showError);
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

void Window::askSendPassword(
		const PreparedInvoice &invoice,
		Fn<void(InvoiceField)> showInvoiceError) {
	const auto checking = std::make_shared<bool>();
	const auto ready = [=](
			const QByteArray &passcode,
			const PreparedInvoice &invoice,
			Fn<void(QString)> showError) {
		if (*checking) {
			return;
		}
		*checking = true;
		auto done = [=](Ton::Result<Ton::TransactionCheckResult> result) {
			if (!result && IsIncorrectPasswordError(result.error())) {
				*checking = false;
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
					// #TODO fatal?..
				}
				return;
			}
			showSendConfirmation(
				invoice,
				passcode,
				*result,
				showInvoiceError);
		};
		_wallet->checkSendGrams(
			_wallet->publicKeys().front(),
			passcode,
			TransactionFromInvoice(invoice),
			done);
	};
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
		const QByteArray &passcode,
		const Ton::TransactionCheckResult &checkResult,
		Fn<void(InvoiceField)> showInvoiceError) {
	const auto balance = _state.current().account.balance;
	if (invoice.amount + checkResult.sourceFees.sum() > balance) {
		showInvoiceError(InvoiceField::Amount);
		return;
	}
	const auto sending = std::make_shared<bool>();
	const auto confirmed = [=] {
		if (*sending) {
			return;
		}
		const auto confirmations = std::make_shared<rpl::event_stream<>>();
		*sending = true;
		auto ready = [=](Ton::Result<Ton::PendingTransaction> result) {
			*sending = false;
			if (!result) {
				if (const auto field = ErrorInvoiceField(result.error())) {
					showInvoiceError(*field);
				} else {
					// #TODO fatal?..
				}
				return;
			}
			showSendingTransaction(*result, confirmations->events());
		};
		const auto sent = [=](Ton::Result<> result) {
			if (!result) {
				// #TODO fatal?..
				return;
			}
			confirmations->fire({});
		};
		_wallet->sendGrams(
			_wallet->publicKeys().front(),
			passcode,
			TransactionFromInvoice(invoice),
			ready,
			sent);
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
	_viewer->setRefreshEach(kRefreshWhileSendingDelay);
	_sendBox->lifetime().add(crl::guard(_viewer.get(), [=] {
		_viewer->setRefreshEach(kRefreshEachDelay);
	}));
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
		// #TODO fatal?..
	}

	if (_sendBox) {
		_sendBox->closeBox();
	}
}

void Window::receiveGrams() {
	const auto share = [=](const QString &address) {
		QGuiApplication::clipboard()->setText(TransferLink(address));
		auto toast = Ui::Toast::Config();
		toast.text = ph::lng_wallet_receive_copied(ph::now);
		Ui::Toast::Show(_window.get(), toast);
	};
	_layers->showBox(Box(ReceiveGramsBox, _address, share));
}

void Window::changePassword() {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Change password")));
		const auto oldWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto old = Ui::CreateChild<Ui::PasswordInput>(
			oldWrap,
			st::defaultInputField,
			rpl::single(QString("Old password")));
		oldWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			old->resize(width, old->height());
		}, old->lifetime());
		old->heightValue(
		) | rpl::start_with_next([=](int height) {
			oldWrap->resize(oldWrap->width(), height);
		}, old->lifetime());
		old->move(0, 0);
		const auto passwordWrap = box->addRow(object_ptr<Ui::RpWidget>(box));
		const auto password = Ui::CreateChild<Ui::PasswordInput>(
			passwordWrap,
			st::defaultInputField,
			rpl::single(QString("New password")));
		passwordWrap->widthValue(
		) | rpl::start_with_next([=](int width) {
			password->resize(width, password->height());
		}, password->lifetime());
		password->heightValue(
		) | rpl::start_with_next([=](int height) {
			passwordWrap->resize(passwordWrap->width(), height);
		}, password->lifetime());
		password->move(0, 0);
		const auto saving = box->lifetime().make_state<bool>(false);
		box->addButton(rpl::single(QString("Change")), [=] {
			if (*saving) {
				return;
			}
			auto from = old->getLastText().trimmed();
			if (from.isEmpty()) {
				old->showError();
				return;
			}
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*saving = true;
			auto done = [=](Ton::Result<> result) {
				*saving = false;
				if (result) {
					box->closeBox();
				} else {
					old->showError();
					return;
				}
			};
			_wallet->changePassword(from.toUtf8(), pwd.toUtf8(), done);
		});
		box->addButton(rpl::single(QString("Cancel")), [=] {
			box->closeBox();
		});
	}));
}

void Window::logout() {
	_layers->showBox(Box(DeleteWalletBox, [=] {
		_wallet->deleteAllKeys([=](Ton::Result<> result) {
			if (result) {
				showIntro();
			}
		});
	}));
}

} // namespace Wallet
