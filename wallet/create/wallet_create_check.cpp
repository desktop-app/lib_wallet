// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_check.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "ui/ton_word_input.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {
namespace {

using TonWordInput = Ui::TonWordInput;

} // namespace

Check::Check(
	Fn<std::vector<QString>(QString)> wordsByPrefix,
	const std::vector<int> &indices)
: Step(Type::Default) {
	Expects(indices.size() == 3);

	setTitle(ph::lng_wallet_check_title(Ui::Text::RichLangValue));
	setDescription(ph::lng_wallet_check_description(
	) | rpl::map([=](QString text) {
		return text.replace(
			"{index1}",
			QString::number(indices[0] + 1)
		).replace(
			"{index2}",
			QString::number(indices[1] + 1)
		).replace(
			"{index3}",
			QString::number(indices[2] + 1));
	}) | Ui::Text::ToRichLangValue());
	initControls(std::move(wordsByPrefix), indices);
}

std::vector<QString> Check::words() const {
	return _words();
}

rpl::producer<> Check::submitRequests() const {
	return _submitRequests.events();
}

void Check::setFocus() {
	_setFocus();
}

bool Check::checkAll() {
	return _checkAll();
}

bool Check::allowEscapeBack() const {
	return false;
}

int Check::desiredHeight() const {
	return st::walletChecksHeight;
}

void Check::initControls(
		Fn<std::vector<QString>(QString)> wordsByPrefix,
		const std::vector<int> &indices) {
	showLottie(
		"test",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	stopLottieOnLoop();

	const auto count = indices.size();
	auto inputs = std::make_shared<std::vector<
		std::unique_ptr<TonWordInput>>>();
	const auto wordsTop = st::walletChecksTop;
	const auto rowsBottom = wordsTop + count * st::walletWordHeight;
	const auto isValid = [=](int index) {
		Expects(index < count);

		const auto word = (*inputs)[index]->word();
		const auto words = wordsByPrefix(word);
		return !words.empty() && (words.front() == word);
	};
	const auto showError = [=](int index) {
		Expects(index < count);

		if (isValid(index)) {
			return false;
		}
		(*inputs)[index]->showError();
		return true;
	};
	const auto init = [&](const TonWordInput &word, int index) {
		const auto next = [=] {
			return (index + 1 < count)
				? (*inputs)[index + 1].get()
				: nullptr;
		};
		const auto previous = [=] {
			return (index > 0)
				? (*inputs)[index - 1].get()
				: nullptr;
		};

		word.blurred(
		) | rpl::filter([=] {
			return !(*inputs)[index]->word().trimmed().isEmpty()
				&& !isValid(index);
		}) | rpl::start_with_next([=] {
			(*inputs)[index]->showErrorNoFocus();
		}, lifetime());

		word.tabbed(
		) | rpl::start_with_next([=](TonWordInput::TabDirection direction) {
			if (direction == TonWordInput::TabDirection::Forward) {
				if (const auto word = next()) {
					word->setFocus();
				}
			} else {
				if (const auto word = previous()) {
					word->setFocus();
				}
			}
		}, lifetime());

		word.submitted(
		) | rpl::start_with_next([=] {
			if ((*inputs)[index]->word() == TonWordInput::kSkipPassword) {
				_submitRequests.fire({});
			} else if (!showError(index)) {
				if (const auto word = next()) {
					word->setFocus();
				} else {
					_submitRequests.fire({});
				}
			}
		}, lifetime());
	};
	for (auto i = 0; i != count; ++i) {
		inputs->push_back(std::make_unique<TonWordInput>(
			inner(),
			indices[i],
			wordsByPrefix));
		init(*inputs->back(), i);
	}

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto half = size.width() / 2;
		const auto left = half - st::walletWordSkipLeft;
		const auto right = half + st::walletWordSkipRight;
		auto x = left;
		auto y = contentTop() + wordsTop;
		auto index = 0;
		for (const auto &input : *inputs) {
			input->move(x, y);
			y += st::walletWordHeight;
		}
	}, inner()->lifetime());

	showNextButton(ph::lng_wallet_continue());

	_words = [=] {
		return (*inputs) | ranges::view::transform(
			[](const std::unique_ptr<TonWordInput> &p) { return p->word(); }
		) | ranges::to_vector;
	};
	_setFocus = [=] {
		inputs->front()->setFocus();
	};
	_checkAll = [=] {
		if ((*inputs)[0]->word() == TonWordInput::kSkipPassword) {
			return true;
		}
		auto result = true;
		for (auto i = count; i != 0;) {
			result = !showError(--i) && result;
		}
		return result;
	};
}

void Check::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
