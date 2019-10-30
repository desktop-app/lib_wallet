// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_top_bar.h"

#include "wallet/wallet_phrases.h"
#include "wallet/wallet_common.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/dropdown_menu.h"
#include "base/timer.h"
#include "ton/ton_state.h"
#include "styles/style_widgets.h"
#include "styles/style_wallet.h"
#include "styles/palette.h"

namespace Wallet {
namespace {

constexpr auto kMsInMinute = 60 * crl::time(1000);

[[nodiscard]] auto ToTopBarState(bool refreshing = false) {
	return rpl::map([=](QString &&text) {
		return TopBarState{ std::move(text), refreshing };
	});
}

[[nodiscard]] rpl::producer<TopBarState> MakeTopBarStateRefreshing() {
	return ph::lng_wallet_refreshing() | ToTopBarState(true);
}

[[nodiscard]] rpl::producer<int> MakeRefreshedMinutesAgo(crl::time when) {
	return rpl::make_producer<int>([=](const auto &consumer) {
		auto result = rpl::lifetime();
		const auto timer = result.make_state<base::Timer>();
		const auto putNext = [=] {
			const auto elapsed = crl::now() - when;
			const auto minutes = int(elapsed / kMsInMinute);
			const auto next = kMsInMinute * (minutes + 1) - elapsed;
			if (consumer.put_next_copy(minutes)) {
				timer->callOnce(next);
			}
		};
		timer->setCallback(putNext);
		putNext();
		return result;
	});
}

[[nodiscard]] rpl::producer<TopBarState> MakeTopBarStateRefreshed(
		crl::time when) {
	return MakeRefreshedMinutesAgo(
		when
	) | rpl::map([](int minutes) {
		return minutes
			? ph::lng_wallet_refreshed_minutes_ago(minutes)()
			: ph::lng_wallet_refreshed_just_now();
	}) | rpl::flatten_latest(
	) | ToTopBarState();
}

[[nodiscard]] rpl::producer<TopBarState> MakeNonSyncTopBarState(
		const Ton::WalletViewerState &state) {
	if (state.refreshing || !state.lastRefresh) {
		return MakeTopBarStateRefreshing();
	}
	return MakeTopBarStateRefreshed(state.lastRefresh);
}

} // namespace

TopBar::TopBar(
	not_null<Ui::RpWidget*> parent,
	rpl::producer<TopBarState> state)
: _widgetParent(parent)
, _widget(parent) {
	parent->widthValue(
	) | rpl::start_with_next([=](int width) {
		_widget.setGeometry(0, 0, width, st::walletTopBarHeight);
	}, lifetime());

	setupControls(std::move(state));
}

rpl::producer<Action> TopBar::actionRequests() const {
	return _actionRequests.events();
}

rpl::lifetime &TopBar::lifetime() {
	return _widget.lifetime();
}

void TopBar::setupControls(rpl::producer<TopBarState> &&state) {
	auto text = rpl::duplicate(
		state
	) | rpl::map([](const TopBarState &state) {
		return state.text;
	});
	const auto refresh = Ui::CreateChild<Ui::IconButton>(
		&_widget,
		st::walletTopRefreshButton);
	refresh->clicks(
	) | rpl::map([] {
		return Action::Refresh;
	}) | rpl::start_to_stream(_actionRequests, refresh->lifetime());

	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		&_widget,
		std::move(text),
		st::walletTopLabel);

	const auto menu = Ui::CreateChild<Ui::IconButton>(
		&_widget,
		st::walletTopMenuButton);
	menu->setClickedCallback([=] { showMenu(menu); });

	_widget.setAttribute(Qt::WA_OpaquePaintEvent);
	_widget.paintRequest(
	) | rpl::start_with_next([=](QRect clip) {
		QPainter(&_widget).fillRect(clip, st::walletTopBg);
	}, lifetime());

	label->show();
	menu->show();

	rpl::combine(
		_widget.widthValue(),
		std::move(state)
	) | rpl::start_with_next([=](int width, const TopBarState&) {
		const auto height = _widget.height();
		refresh->moveToLeft(0, (height - refresh->height()) / 2, width);
		menu->moveToRight(0, (height - menu->height()) / 2, width);
		label->moveToLeft(
			(width - label->width()) / 2,
			(height - label->height()) / 2,
			width);
	}, lifetime());
}

void TopBar::showMenu(not_null<Ui::IconButton*> toggle) {
	if (_menu) {
		return;
	}
	_menu.emplace(_widgetParent);

	const auto menu = _menu.get();
	toggle->installEventFilter(menu);

	const auto weak = Ui::MakeWeak(toggle);
	menu->setHiddenCallback([=] {
		menu->deleteLater();
		if (weak && _menu.get() == menu) {
			_menu = nullptr;
			toggle->setForceRippled(false);
		}
	});
	menu->setShowStartCallback(crl::guard(weak, [=] {
		if (_menu == menu) {
			toggle->setForceRippled(true);
		}
	}));
	menu->setHideStartCallback(crl::guard(weak, [=] {
		if (_menu == menu) {
			toggle->setForceRippled(false);
		}
	}));

	menu->addAction(ph::lng_wallet_menu_settings(ph::now), [=] {
		_actionRequests.fire(Action::ShowSettings);
	});
	menu->addAction(ph::lng_wallet_menu_change_passcode(ph::now), [=] {
		_actionRequests.fire(Action::ChangePassword);
	});
	menu->addAction(ph::lng_wallet_menu_export(ph::now), [=] {
		_actionRequests.fire(Action::Export);
	});
	menu->addAction(ph::lng_wallet_menu_delete(ph::now), [=] {
		_actionRequests.fire(Action::LogOut);
	});

	_widgetParent->widthValue(
	) | rpl::start_with_next([=](int width) {
		menu->moveToRight(
			st::walletMenuPosition.x(),
			st::walletMenuPosition.y(),
			width);
	}, menu->lifetime());

	menu->showAnimated(Ui::PanelAnimation::Origin::TopRight);
}

rpl::producer<TopBarState> MakeTopBarState(
		rpl::producer<Ton::WalletViewerState> &&state,
		rpl::producer<Ton::Update> &&updates,
		rpl::lifetime &alive) {
	auto syncs = rpl::single(
		Ton::SyncState()
	) | rpl::then(std::move(
		updates
	) | rpl::filter([](const Ton::Update &update) {
		return update.data.is<Ton::SyncState>();
	}) | rpl::map([](const Ton::Update &update) {
		return update.data.get<Ton::SyncState>();
	}));
	return rpl::combine(
		std::move(state),
		std::move(syncs)
	) | rpl::map([=](
			const Ton::WalletViewerState &state,
			const Ton::SyncState &sync) -> rpl::producer<TopBarState> {
		if (!sync.valid() || sync.current == sync.to) {
			return MakeNonSyncTopBarState(state);
		} else if (sync.current == sync.from) {
			return ph::lng_wallet_sync() | ToTopBarState();
		} else {
			const auto percent = QString::number(
				(100 * (sync.current - sync.from)
					/ (sync.to - sync.from)));
			return ph::lng_wallet_sync_percent(
			) | rpl::map([=](QString &&text) {
				return TopBarState{
					text.replace("{percent}", percent),
					false
				};
			});
		}
	}) | rpl::flatten_latest() | rpl::start_spawning(alive);
}

} // namespace Wallet
