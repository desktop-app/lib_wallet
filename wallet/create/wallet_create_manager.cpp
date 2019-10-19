// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_manager.h"

#include "wallet/create/wallet_create_intro.h"
#include "wallet/create/wallet_creatE_import.h"
#include "wallet/create/wallet_create_created.h"
#include "wallet/create/wallet_create_view.h"
#include "wallet/create/wallet_create_check.h"
#include "wallet/create/wallet_create_passcode.h"
#include "wallet/create/wallet_create_ready.h"
#include "wallet/wallet_phrases.h"
#include "ton/ton_wallet.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "ui/toast/toast.h"
#include "ui/ton_word_input.h"
#include "ui/rp_widget.h"
#include "base/call_delayed.h"
#include "styles/style_wallet.h"

#include <random>

namespace Wallet::Create {
namespace {

constexpr auto kCheckWordCount = 3;
constexpr auto kWaitForWordsDelay = 30 * crl::time(1000);

[[nodiscard]] std::vector<int> SelectRandomIndices(int select, int count) {
	Expects(select <= count);
	Expects(select >= 0);

	auto generator = std::mt19937(std::random_device()());
	auto result = base::flat_set<int>();
	for (auto i = 0; i != select; ++i) {
		auto distribution = std::uniform_int_distribution<int>(
			0,
			count - i - 1);
		auto value = distribution(generator);
		for (const auto already : result) {
			if (already <= value) {
				++value;
			} else {
				break;
			}
		}
		result.emplace(value);
	}

	for (const auto value : result) {
		Ensures(value >= 0 && value < count);
	}
	return result | ranges::to_vector;
}

[[nodiscard]] bool CheckWords(
		const std::vector<QString> &original,
		const std::vector<int> &indices,
		const std::vector<QString> &offered) {
	Expects(indices.size() == offered.size());
	for (auto i = 0; i != indices.size(); ++i) {
		Expects(indices[i] >= 0 && indices[i] < original.size());
	}

	if (!offered.empty()
		&& offered.front() == Ui::TonWordInput::kSkipPassword) {
		return true;
	}
	for (auto i = 0; i != indices.size(); ++i) {
		const auto different = offered[i].trimmed().compare(
			original[indices[i]].trimmed(),
			Qt::CaseInsensitive);
		if (different) {
			return false;
		}
	}
	return true;
}

} // namespace

Manager::Manager(not_null<QWidget*> parent)
: _content(std::make_unique<Ui::RpWidget>(parent))
, _backButton(
	std::in_place,
	_content.get(),
	object_ptr<Ui::IconButton>(_content.get(), st::walletStepBackButton))
, _validWords(Ton::Wallet::GetValidWords())
, _waitForWords([=] { _wordsShouldBeReady = true; }) {
	_content->show();
	initButtons();
	showIntro();
}

void Manager::initButtons() {
	_backButton->entity()->setClickedCallback([=] { back(); });
	_backButton->toggle(false, anim::type::instant);
	_backButton->setDuration(st::slideDuration);
	_backButton->move(0, 0);
}

Manager::~Manager() = default;

not_null<Ui::RpWidget*> Manager::content() const {
	return _content.get();
}

void Manager::next() {
	if (_next) {
		_next();
	}
}

void Manager::back() {
	if (_back) {
		_back();
	}
}

void Manager::backByEscape() {
	if (_step->allowEscapeBack()) {
		back();
	}
}

void Manager::setFocus() {
	_step->setFocus();
}

void Manager::showIntro() {
	showStep(std::make_unique<Intro>(), Direction::Forward, [=] {
		_actionRequests.fire(Action::CreateKey);
	});
}

void Manager::showCreated(std::vector<QString> &&words) {
	Expects(_words.empty());
	Expects(!words.empty());

	_words = std::move(words);
	showStep(std::make_unique<Created>(), Direction::Forward, [=] {
		showWords(Direction::Forward);
	});
}

void Manager::showWords(Direction direction) {
	if (!_waitForWords.isActive() && !_wordsShouldBeReady) {
		_waitForWords.callOnce(kWaitForWordsDelay);
	}
	showStep(std::make_unique<View>(_words), direction, [=] {
		if (!_wordsShouldBeReady) {
			_actionRequests.fire(Action::ShowCheckTooSoon);
		} else {
			showCheck();
		}
	});
}

void Manager::showCheck() {
	const auto indices = SelectRandomIndices(kCheckWordCount, _words.size());

	auto check = std::make_unique<Check>([=](const QString &prefix) {
		return wordsByPrefix(prefix);
	}, indices);

	const auto raw = check.get();

	raw->submitRequests(
	) | rpl::start_with_next([=] {
		next();
	}, raw->lifetime());

	showStep(std::move(check), Direction::Forward, [=] {
		if (raw->checkAll()) {
			if (CheckWords(_words, indices, raw->words())) {
				showPasscode();
			} else {
				_actionRequests.fire(Action::ShowCheckIncorrect);
			}
		}
	}, [=] {
		showWords(Direction::Backward);
	});
}

void Manager::showPasscode() {
	auto passcode = std::make_unique<Passcode>();

	const auto raw = passcode.get();

	//raw->submitRequests( // #TODO keyboard
	//) | rpl::start_with_next([=] {
	//	next();
	//}, raw->lifetime());

	showStep(std::move(passcode), Direction::Forward, [=] {
		if (auto passcode = raw->passcode(); !passcode.isEmpty()) {
			_passcodeChosen.fire(std::move(passcode));
		}
	});
}

void Manager::showReady(const QByteArray &publicKey) {
	Expects(!publicKey.isEmpty());

	_publicKey = publicKey;
	showStep(std::make_unique<Ready>(), Direction::Forward, [=] {
		_actionRequests.fire(Action::ShowAccount);
	});
}

void Manager::showStep(
		std::unique_ptr<Step> step,
		Direction direction,
		FnMut<void()> next,
		FnMut<void()> back) {
	std::swap(_step, step);
	_next = std::move(next);
	_back = std::move(back);

	const auto inner = _step->widget();
	inner->setParent(_content.get());
	_content->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		inner->setGeometry({ QPoint(), size });
	}, inner->lifetime());
	inner->show();

	_backButton->toggle(_back != nullptr, anim::type::normal);
	_backButton->raise();

	_step->nextClicks(
	) | rpl::start_with_next([=](Qt::KeyboardModifiers modifiers) {
		acceptWordsDelayByModifiers(modifiers);
		this->next();
	}, _step->lifetime());

	_step->importClicks(
	) | rpl::start_with_next([=] {
		showImport();
	}, _step->lifetime());

	if (step) {
		_step->showAnimated(step.get(), direction);
	} else {
		_step->showFast();
	}
}

void Manager::showImport() {
	auto step = std::make_unique<Import>([=](const QString &prefix) {
		return wordsByPrefix(prefix);
	});

	const auto raw = step.get();

	raw->actionRequests(
	) | rpl::start_with_next([=](Import::Action action) {
		switch (action) {
		case Import::Action::Submit: next(); return;
		case Import::Action::NoWords: showImportFail(); return;
		}
		Unexpected("Action in Manager::showImport.");
	}, raw->lifetime());

	showStep(std::move(step), Direction::Forward, [=] {
		if (raw->checkAll()) {
			_importRequests.fire(raw->words());
		}
	}, [=] {
		showIntro();
	});
}

void Manager::showImportFail() {
	_actionRequests.fire(Action::ShowImportFail);
}

void Manager::acceptWordsDelayByModifiers(Qt::KeyboardModifiers modifiers) {
	const auto kRequired = Qt::ControlModifier | Qt::AltModifier;
	if (!_waitForWords.isActive()) {
		return;
	} else if ((modifiers & kRequired) != kRequired) {
		return;
	}
	_wordsShouldBeReady = true;
}

void Manager::setGeometry(QRect geometry) {
	_content->setGeometry(geometry);
}

rpl::producer<Manager::Action> Manager::actionRequests() const {
	return _actionRequests.events();
}

rpl::producer<std::vector<QString>> Manager::importRequests() const {
	return _importRequests.events();
}

rpl::producer<QByteArray> Manager::passcodeChosen() const {
	return _passcodeChosen.events();
}

QByteArray Manager::publicKey() const {
	Expects(!_publicKey.isEmpty());

	return _publicKey;
}

rpl::lifetime &Manager::lifetime() {
	return _content->lifetime();
}

std::vector<QString> Manager::wordsByPrefix(const QString &word) const {
	const auto adjusted = word.trimmed().toLower();
	if (adjusted.isEmpty()) {
		return {};
	} else if (_validWords.empty()) {
		return { word };
	}
	auto prefix = QString();
	auto count = 0;
	auto maxCount = 0;
	for (const auto &word : _validWords) {
		if (word.midRef(0, 3) != prefix) {
			prefix = word.mid(0, 3);
			count = 1;
		} else {
			++count;
		}
		if (maxCount < count) {
			maxCount = count;
		}
	}
	auto result = std::vector<QString>();
	const auto from = ranges::lower_bound(_validWords, adjusted);
	const auto end = _validWords.end();
	for (auto i = from; i != end && i->startsWith(adjusted); ++i) {
		result.push_back(*i);
	}
	return result;
}

} // namespace Wallet::Create
