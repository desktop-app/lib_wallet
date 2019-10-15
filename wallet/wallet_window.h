// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

namespace Ton {
class Wallet;
class AccountViewer;
} // namespace Ton

namespace Ui {
class Window;
class LayerManager;
} // namespace Ui

namespace Wallet {

class Intro;
class Info;

class Window final {
public:
	explicit Window(not_null<Ton::Wallet*> wallet);
	~Window();

	void show();
	void setFocus();

	void criticalError(const QString &error);

private:
	void init();
	void updatePalette();
	void showIntro();
	void showAccount(const QByteArray &publicKey);

	void saveKey(const std::vector<QString> &words);
	void sendGrams();
	void receiveGrams();
	void changePassword();
	void logout();

	const not_null<Ton::Wallet*> _wallet;
	const std::unique_ptr<Ui::Window> _window;
	const std::unique_ptr<Ui::LayerManager> _layers;

	std::unique_ptr<Intro> _intro;

	QString _address;
	std::unique_ptr<Ton::AccountViewer> _viewer;
	std::unique_ptr<Info> _info;

};

} // namespace Wallet
