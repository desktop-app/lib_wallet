// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_import.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/buttons.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "ui/ton_word_input.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {
namespace {

using TonWordInput = Ui::TonWordInput;

} // namespace

Import::Import(Fn<std::vector<QString>(QString)> wordsByPrefix)
: Step(Type::Scroll) {
	setTitle(
		ph::lng_wallet_import_title(Ui::Text::RichLangValue),
		st::walletImportTitleTop);
	setDescription(
		ph::lng_wallet_import_description(Ui::Text::RichLangValue));
	initControls(std::move(wordsByPrefix));
}

std::vector<QString> Import::words() const {
	return _words();
}

rpl::producer<Import::Action> Import::actionRequests() const {
	return _actionRequests.events();
}

void Import::setFocus() {
	_setFocus();
}

bool Import::checkAll() {
	return _checkAll();
}

bool Import::allowEscapeBack() const {
	return false;
}

int Import::desiredHeight() const {
	return _desiredHeight;
}

void Import::initControls(Fn<std::vector<QString>(QString)> wordsByPrefix) {
	constexpr auto rows = 12;
	constexpr auto count = rows * 2;
	auto inputs = std::make_shared<std::vector<
		std::unique_ptr<TonWordInput>>>();
	const auto wordsTop = st::walletImportWordsTop;
	const auto rowsBottom = wordsTop + rows * st::walletWordHeight;
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

		word.focused(
		) | rpl::start_with_next([=] {
			const auto row = index % rows;
			ensureVisible(
				wordsTop + (row - 1) * st::walletWordHeight,
				2 * st::walletWordHeight + st::walletSuggestionsHeightMax);
		}, lifetime());

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
				_actionRequests.fire(Action::Submit);
			} else if (!showError(index)) {
				if (const auto word = next()) {
					word->setFocus();
				} else {
					_actionRequests.fire(Action::Submit);
				}
			}
		}, lifetime());
	};
	for (auto i = 0; i != count; ++i) {
		inputs->push_back(std::make_unique<TonWordInput>(
			inner(),
			st::walletImportInputField,
			i,
			wordsByPrefix));
		init(*inputs->back(), i);
	}

	const auto noWords = Ui::CreateChild<Ui::LinkButton>(
		inner().get(),
		ph::lng_wallet_import_dont_have(ph::now),
		st::defaultLinkButton);
	noWords->setClickedCallback([=] {
		_actionRequests.fire(Action::NoWords);
	});

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		noWords->move(
			(size.width() - noWords->width()) / 2,
			contentTop() + st::walletImportNoWordsTop);
		const auto half = size.width() / 2;
		const auto left = half - st::walletImportSkipLeft;
		const auto right = half + st::walletImportSkipRight;
		auto x = left;
		auto y = contentTop() + wordsTop;
		auto index = 0;
		for (const auto &input : *inputs) {
			input->move(x, y);
			y += st::walletWordHeight;
			if (++index == rows) {
				x = right;
				y = contentTop() + wordsTop;
			}
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
		auto result = true;
		for (auto i = count; i != 0;) {
			result = !showError(--i) && result;
		}
		return result;
	};

	_desiredHeight = rowsBottom
		+ st::walletWordsNextSkip
		+ st::walletWordsNextBottomSkip;
}

} // namespace Wallet::Create
