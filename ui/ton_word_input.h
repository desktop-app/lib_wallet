// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#pragma once

#include "base/object_ptr.h"

namespace style {
struct InputField;
} // namespace style

namespace Ui {

class FlatLabel;
class InputField;
class TonWordSuggestions;

class TonWordInput final {
public:
	enum class TabDirection {
		Forward,
		Backward,
	};

	static const QString kSkipPassword;

	TonWordInput(
		not_null<QWidget*> parent,
		const style::InputField &st,
		int index,
		Fn<std::vector<QString>(QString)> wordsByPrefix);
	TonWordInput(const TonWordInput &other) = delete;
	TonWordInput &operator=(const TonWordInput &other) = delete;
	~TonWordInput();

	void move(int left, int top) const;
	int top() const;
	QString word() const;
	void setFocus() const;
	void showError() const;
	void showErrorNoFocus() const;

	[[nodiscard]] rpl::producer<> focused() const;
	[[nodiscard]] rpl::producer<> blurred() const;
	[[nodiscard]] rpl::producer<TabDirection> tabbed() const;
	[[nodiscard]] rpl::producer<> submitted() const;

private:
	void setupSuggestions();
	void createSuggestionsWidget();
	void showSuggestions(const QString &word);

	object_ptr<FlatLabel> _index;
	object_ptr<InputField> _word;
	const Fn<std::vector<QString>(QString)> _wordsByPrefix;
	std::unique_ptr<TonWordSuggestions> _suggestions;
	rpl::event_stream<TabDirection> _wordTabbed;
	bool _chosen = false;

};

} // namespace Ui
