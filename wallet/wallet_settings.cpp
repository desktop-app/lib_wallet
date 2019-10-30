// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_settings.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_update_info.h"
#include "wallet/wallet_common.h"
#include "ton/ton_settings.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/box_content_divider.h"
#include "ui/widgets/input_fields.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/text/text_utilities.h"
#include "styles/style_wallet.h"
#include "styles/style_layers.h"
#include "styles/style_widgets.h"

namespace Wallet {
namespace {

[[nodiscard]] QString FormatVersion(int version) {
	const auto major = version / 1000000;
	const auto minor = (version % 1000000) / 1000;
	const auto patch = version % 1000;
	return patch
		? QString("%1.%2.%3").arg(major).arg(minor).arg(patch)
		: QString("%1.%2").arg(major).arg(minor);
}

[[nodiscard]] QString FormatDownloadProgress(int64 ready, int64 total) {
	QString readyStr, totalStr, mb;
	if (total >= 1024 * 1024) { // more than 1 mb
		qint64 readyTenthMb = (ready * 10 / (1024 * 1024)), totalTenthMb = (total * 10 / (1024 * 1024));
		readyStr = QString::number(readyTenthMb / 10) + '.' + QString::number(readyTenthMb % 10);
		totalStr = QString::number(totalTenthMb / 10) + '.' + QString::number(totalTenthMb % 10);
		mb = "MB";
	} else if (total >= 1024) {
		qint64 readyKb = (ready / 1024), totalKb = (total / 1024);
		readyStr = QString::number(readyKb);
		totalStr = QString::number(totalKb);
		mb = "KB";
	} else {
		readyStr = QString::number(ready);
		totalStr = QString::number(total);
		mb = "B";
	}
	return ph::lng_wallet_downloaded(
		ph::now
	).replace("{ready}", readyStr
	).replace("{total}", totalStr
	).replace("{mb}", mb);
}

void SetupUpdate(not_null<Ui::GenericBox*> box, not_null<UpdateInfo*> info) {
	AddBoxSubtitle(box, ph::lng_wallet_settings_version_title());

	const auto texts = Ui::CreateChild<rpl::event_stream<QString>>(
		box.get());
	const auto downloading = Ui::CreateChild<rpl::event_stream<bool>>(
		box.get());
	const auto version = ph::lng_wallet_settings_version(
		ph::now
	).replace("{version}", FormatVersion(info->currentVersion()));
	const auto toggle = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			ph::lng_wallet_settings_autoupdate(),
			st::walletSettingsUpdateToggle),
		QMargins());
	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		toggle,
		texts->events(),
		st::walletSettingsUpdateState);

	const auto options = box->addRow(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			box,
			object_ptr<Ui::VerticalLayout>(box)),
		QMargins());
	const auto inner = options->entity();

	const auto check = inner->add(
		object_ptr<Ui::SettingsButton>(
			inner,
			ph::lng_wallet_settings_check(),
			st::defaultSettingsButton),
		QMargins());
	const auto update = Ui::CreateChild<Ui::SettingsButton>(
		check,
		ph::lng_wallet_settings_update() | Ui::Text::ToUpper(),
		st::walletSettingsUpdate);
	update->hide();
	check->widthValue() | rpl::start_with_next([=](int width) {
		update->resizeToWidth(width);
		update->moveToLeft(0, 0);
	}, update->lifetime());

	rpl::combine(
		toggle->widthValue(),
		label->widthValue()
	) | rpl::start_with_next([=] {
		label->moveToLeft(
			st::walletSettingsUpdateStatePosition.x(),
			st::walletSettingsUpdateStatePosition.y());
	}, label->lifetime());
	label->setAttribute(Qt::WA_TransparentForMouseEvents);

	const auto showDownloadProgress = [=](int64 ready, int64 total) {
		auto text = ph::lng_wallet_settings_downloading(
			ph::now
		).replace("{progress}", FormatDownloadProgress(ready, total));
		texts->fire(std::move(text));
		downloading->fire(true);
	};
	const auto setDefaultStatus = [=] {
		const auto state = info->state();
		switch (state) {
		case UpdateState::Download:
			showDownloadProgress(info->already(), info->size());
			break;
		case UpdateState::Ready:
			texts->fire(ph::lng_wallet_settings_ready(ph::now));
			update->show();
			break;
		default:
			texts->fire_copy(version);
			break;
		}
	};

	toggle->toggleOn(rpl::single(info->toggled()));
	toggle->toggledValue(
	) | rpl::start_with_next([=](bool toggled) {
		info->toggle(toggled);
		if (!toggled) {
			setDefaultStatus();
			update->hide();
		}
	}, toggle->lifetime());

	const auto downloadingNow = (info->state() == UpdateState::Download);
	options->toggleOn(rpl::combine(
		toggle->toggledValue(),
		downloading->events_starting_with_copy(downloadingNow)
	) | rpl::map([](bool check, bool downloading) {
		return check && !downloading;
	}));

	info->checking() | rpl::start_with_next([=] {
		options->setAttribute(Qt::WA_TransparentForMouseEvents);
		texts->fire(ph::lng_wallet_settings_checking(ph::now));
		downloading->fire(false);
	}, options->lifetime());
	info->isLatest() | rpl::start_with_next([=] {
		options->setAttribute(Qt::WA_TransparentForMouseEvents, false);
		texts->fire(ph::lng_wallet_settings_latest(ph::now));
		downloading->fire(false);
	}, options->lifetime());
	info->progress(
	) | rpl::start_with_next([=](UpdateProgress progress) {
		showDownloadProgress(progress.already, progress.size);
	}, options->lifetime());
	info->failed() | rpl::start_with_next([=] {
		options->setAttribute(Qt::WA_TransparentForMouseEvents, false);
		texts->fire(ph::lng_wallet_settings_fail(ph::now));
		downloading->fire(false);
	}, options->lifetime());
	info->ready() | rpl::start_with_next([=] {
		options->setAttribute(Qt::WA_TransparentForMouseEvents, false);
		texts->fire(ph::lng_wallet_settings_ready(ph::now));
		update->show();
		downloading->fire(false);
	}, options->lifetime());

	setDefaultStatus();

	check->addClickHandler([=] {
		info->toggle(true);
	});
	update->addClickHandler([=] {
		info->install();
	});
}

} // namespace

void SettingsBox(
		not_null<Ui::GenericBox*> box,
		const Ton::Settings &settings,
		UpdateInfo *updateInfo,
		Fn<void(Ton::Settings)> save) {
	using namespace rpl::mappers;

	box->setTitle(ph::lng_wallet_settings_title());

	if (updateInfo) {
		SetupUpdate(box, updateInfo);
		box->addRow(
			object_ptr<Ui::BoxContentDivider>(box),
			st::walletSettingsDividerMargin);
	}

	AddBoxSubtitle(box, ph::lng_wallet_settings_configuration());
	const auto download = box->addRow(
		object_ptr<Ui::SettingsButton>(
			box,
			ph::lng_wallet_settings_update_config(),
			st::defaultSettingsButton),
		QMargins()
	)->toggleOn(rpl::single(!settings.useCustomConfig));

	const auto custom = box->addRow(
		object_ptr<Ui::SlideWrap<Ui::SettingsButton>>(
			box,
			object_ptr<Ui::SettingsButton>(
				box,
				ph::lng_wallet_settings_config_from_file(),
				st::defaultSettingsButton)),
		QMargins()
	)->toggleOn(download->toggledValue() | rpl::map(!_1))->setDuration(0);

	// Make field the same height as the button.
	const auto heightDelta = st::defaultSettingsButton.padding.top()
		+ st::defaultSettingsButton.height
		+ st::defaultSettingsButton.padding.bottom()
		- st::walletInput.heightMin;
	const auto url = box->addRow(
		object_ptr<Ui::SlideWrap<Ui::InputField>>(
			box,
			object_ptr<Ui::InputField>(
				box,
				st::walletInput,
				ph::lng_wallet_settings_config_url(),
				settings.configUrl),
			QMargins(0, 0, 0, heightDelta)),
		(st::boxRowPadding
			+ QMargins(0, 0, 0, st::walletSettingsBlockchainNameSkip))
	)->toggleOn(download->toggledValue())->setDuration(0)->entity();

	AddBoxSubtitle(box, ph::lng_wallet_settings_blockchain_name());
	const auto name = box->addRow(object_ptr<Ui::InputField>(
		box,
		st::walletInput,
		rpl::single(QString()),
		settings.blockchainName));

	const auto collectSettings = [=] {
		auto result = settings;
		result.blockchainName = name->getLastText();
		result.useCustomConfig = custom->toggled();
		if (!result.useCustomConfig) {
			result.configUrl = url->getLastText();
		}
		return result;
	};

	box->addButton(ph::lng_wallet_save(), [=] { save(collectSettings()); });
	box->addButton(ph::lng_wallet_cancel(), [=] { box->closeBox(); });
}

} // namespace Wallet
