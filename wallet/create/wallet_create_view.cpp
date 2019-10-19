// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/create/wallet_create_view.h"

#include "wallet/wallet_phrases.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/labels.h"
#include "ui/rp_widget.h"
#include "ui/lottie_widget.h"
#include "styles/style_wallet.h"

namespace Wallet::Create {
namespace {

class Word final {
public:
	Word(not_null<QWidget*> parent, int index, const QString &word);

	void move(int left, int top) const;
	[[nodiscard]] int top() const;

private:
	object_ptr<Ui::FlatLabel> _index;
	object_ptr<Ui::FlatLabel> _word;

};

Word::Word(not_null<QWidget*> parent, int index, const QString &word)
: _index(parent, QString::number(index + 1) + '.', st::walletWordIndexLabel)
, _word(parent, word, st::walletWordLabel) {
}

void Word::move(int left, int top) const {
	_index->move(left - _index->width() - st::walletWordIndexSkip, top);
	_word->move(left, top);
}

int Word::top() const {
	return _index->y();
}

} // namespace

View::View(const std::vector<QString> &words, Layout layout)
: Step(Type::Scroll) {
	setTitle(
		ph::lng_wallet_words_title(Ui::Text::RichLangValue),
		(layout == Layout::Export) ? st::walletExportTitleTop : 0);
	setDescription(
		ph::lng_wallet_words_description(Ui::Text::RichLangValue));
	initControls(words, layout);
}

int View::desiredHeight() const {
	return _desiredHeight;
}

void View::initControls(const std::vector<QString> &words, Layout layout) {
	Expects(words.size() % 2 == 0);

	showLottie(
		"paper",
		st::walletStepIntroLottieTop,
		st::walletStepIntroLottieSize);
	stopLottieOnLoop();

	auto labels = std::make_shared<std::vector<std::pair<Word, Word>>>();
	const auto rows = words.size() / 2;
	for (auto i = 0; i != rows; ++i) {
		labels->emplace_back(
			Word(inner(), i, words[i]),
			Word(inner(), i + rows, words[i + rows]));
	}
	const auto wordsTop = (layout == Layout::Export)
		? st::walletExportWordsTop
		: st::walletWordsTop;
	const auto rowsBottom = wordsTop + rows * st::walletWordHeight;

	inner()->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		const auto half = size.width() / 2;
		const auto left = half - st::walletWordSkipLeft;
		const auto right = half + st::walletWordSkipRight;
		auto top = contentTop() + wordsTop;
		for (const auto &pair : *labels) {
			pair.first.move(left, top);
			pair.second.move(right, top);
			top += st::walletWordHeight;
		}
	}, inner()->lifetime());

	if (layout != Layout::Export) {
		showNextButton(ph::lng_wallet_continue());
		_desiredHeight = rowsBottom
			+ st::walletWordsNextSkip
			+ st::walletWordsNextBottomSkip;
	} else {
		_desiredHeight = rowsBottom + st::walletExportBottomSkip;
	}
}

void View::showFinishedHook() {
	startLottie();
}

} // namespace Wallet::Create
