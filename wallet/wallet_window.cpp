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

} // namespace

Window::Window(not_null<Ton::Wallet*> wallet)
: _wallet(wallet)
, _window(std::make_unique<Ui::Window>())
, _layers(std::make_unique<Ui::LayerManager>(_window->body())) {
	_layers->setHideByBackgroundClick(true);
	init();
	if (_wallet->publicKeys().empty()) {
		showIntro();
	} else {
		showAccount(_wallet->publicKeys()[0]);
	}
}

Window::~Window() = default;

void Window::init() {
	_window->setTitle(ph::lng_wallet_window_title(ph::now));
	_window->setGeometry(style::centerrect(
		QApplication::desktop()->geometry(),
		QRect(QPoint(), st::walletWindowSize)));
	_window->setTitleStyle(st::walletWindowTitle);
	_window->setFixedSize(st::walletWindowSize);

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
	_info = nullptr;
	_intro = std::make_unique<Intro>(_window->body());

	_layers->raise();
	_layers->hideAll();

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
	) | rpl::start_with_next([=](Info::Action action) {
		switch (action) {
		case Info::Action::Refresh: _viewer->refreshNow(nullptr); break;
		case Info::Action::Send: sendGrams(); break;
		case Info::Action::Receive: receiveGrams(); break;
		case Info::Action::ChangePassword: changePassword(); break;
		case Info::Action::LogOut: logout(); break;
		}
	}, _info->lifetime());

	_info->preloadRequests(
	) | rpl::start_with_next([=](const Ton::TransactionId &id) {
		_viewer->preloadSlice(id);
	}, _info->lifetime());

	_info->viewRequests(
	) | rpl::start_with_next([=](Ton::Transaction &&data) {
		auto view = ViewTransaction(std::move(data));
		std::move(
			view.sendRequests
		) | rpl::start_with_next([=](const QString &address) {
			sendGrams(address);
		}, view.box->lifetime());
		_layers->showBox(std::move(view.box));
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

void Window::sendGrams(const QString &address) {
	_layers->showBox(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(QString("Send grams")));
		const auto recipient = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Recipient")),
			address));
		const auto amount = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Amount"))));
		const auto comment = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			rpl::single(QString("Comment"))));
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
		const auto sending = box->lifetime().make_state<bool>(false);
		box->addButton(rpl::single(QString("Send")), [=] {
			if (*sending) {
				return;
			}
			auto data = Ton::TransactionToSend();
			data.recipient = recipient->getLastText().trimmed();
			if (data.recipient.isEmpty()) {
				recipient->showError();
				return;
			}
			data.amount = int64(amount->getLastText().trimmed().toDouble()
				* 1'000'000'000);
			if (data.amount <= 0) {
				amount->showError();
				return;
			}
			data.comment = comment->getLastText().trimmed();
			data.allowSendToUninited = true;
			auto pwd = password->getLastText().trimmed();
			if (pwd.isEmpty()) {
				password->showError();
				return;
			}
			*sending = true;
			auto done = [=](Ton::Result<Ton::PendingTransaction> result) {
				*sending = false;
				if (result) {
					box->closeBox();
				} else {
					recipient->showError();
					return;
				}
			};
			_wallet->sendGrams(
				_wallet->publicKeys().front(),
				pwd.toUtf8(),
				data,
				done);
		});
		box->addButton(rpl::single(QString("Cancel")), [=] {
			box->closeBox();
		});
	}));
}

void Window::receiveGrams() {
	auto view = ReceiveGrams(_address);
	std::move(
		view.shareRequests
	) | rpl::start_with_next([=](const QString &address) {
		QGuiApplication::clipboard()->setText(TransferLink(address));
		auto toast = Ui::Toast::Config();
		toast.text = ph::lng_wallet_receive_copied(ph::now);
		Ui::Toast::Show(_window.get(), toast);
	}, view.box->lifetime());
	_layers->showBox(std::move(view.box));
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
	_wallet->deleteAllKeys([=](Ton::Result<> result) {
		if (result) {
			showIntro();
		}
	});
}

} // namespace Wallet
