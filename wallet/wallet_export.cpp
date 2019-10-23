// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "wallet/wallet_export.h"

#include "wallet/wallet_phrases.h"
#include "wallet/create/wallet_create_view.h"
#include "ui/widgets/buttons.h"
#include "styles/style_layers.h"
#include "styles/style_wallet.h"

namespace Wallet {

void ExportedBox(
		not_null<Ui::GenericBox*> box,
		const std::vector<QString> &words) {
	box->setWidth(st::boxWideWidth);
	box->setStyle(st::walletBox);
	box->setNoContentMargin(true);

	const auto view = box->lifetime().make_state<Create::View>(
		words,
		Create::View::Layout::Export);
	view->widget()->resize(st::boxWideWidth, view->desiredHeight());
	box->addRow(
		object_ptr<Ui::RpWidget>::fromRaw(view->widget()),
		QMargins());
	view->showFast();

	box->addButton(
		ph::lng_wallet_done(),
		[=] { box->closeBox(); },
		st::walletWideBottomButton
	)->setTextTransform(Ui::RoundButton::TextTransform::NoTransform);
}

} // namespace Wallet
