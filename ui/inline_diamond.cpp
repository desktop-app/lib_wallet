// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "ui/inline_diamond.h"

#include "ui/rp_widget.h"
#include "qr/qr_generate.h"
#include "styles/style_wallet.h"

#include <QtGui/QPainter>

namespace Ui {
namespace {

constexpr auto kShareQrSize = 768;
constexpr auto kShareQrPadding = 16;

const std::vector<std::pair<int, QString>> &Variants() {
	static const auto result = std::vector<std::pair<int, QString>>{
		{ 22, "gem.png" },
		{ 44, "gem@2x.png" },
		{ 88, "gem@4x.png" },
		{ 192, "gem@large.png" },
	};
	return result;
}

QString ChooseVariant(int desiredSize) {
	const auto &variants = Variants();
	for (const auto &[size, name] : Variants()) {
		if (size == desiredSize || size >= desiredSize * 2) {
			return name;
		}
	}
	return Variants().back().second;
}

QImage CreateImage(int size) {
	Expects(size > 0);

	const auto variant = ChooseVariant(size);
	auto result = QImage(":/gui/art/" + variant).scaled(
		size,
		size,
		Qt::IgnoreAspectRatio,
		Qt::SmoothTransformation);
	result.setDevicePixelRatio(1.);

	Ensures(!result.isNull());
	return result;
}

QImage Image() {
	static const auto result = CreateImage(
		st::walletDiamondSize * style::DevicePixelRatio());
	return result;
}

void Paint(QPainter &p, int x, int y) {
	p.drawImage(
		QRect(x, y, st::walletDiamondSize, st::walletDiamondSize),
		Image());
}

} // namespace

void PaintInlineDiamond(QPainter &p, int x, int y, const style::font &font) {
	Paint(p, x, y + font->ascent - st::walletDiamondAscent);
}

QImage InlineDiamondImage(int size) {
	return CreateImage(size);
}

not_null<RpWidget*> CreateInlineDiamond(
		not_null<QWidget*> parent,
		int x,
		int y,
		const style::font &font) {
	auto result = Ui::CreateChild<RpWidget>(parent.get());
	result->setGeometry(
		x,
		y + font->ascent - st::walletDiamondAscent,
		st::walletDiamondSize,
		st::walletDiamondSize);
	result->paintRequest(
	) | rpl::start_with_next([=] {
		auto p = QPainter(result);
		Paint(p, 0, 0);
	}, result->lifetime());
	return result;
}

QImage DiamondQrExact(const Qr::Data &data, int pixel) {
	return Qr::ReplaceCenter(
		Qr::Generate(data, pixel),
		Ui::InlineDiamondImage(Qr::ReplaceSize(data, pixel)));
}

QImage DiamondQr(const Qr::Data &data, int pixel, int max = 0) {
	Expects(data.size > 0);

	if (max > 0 && data.size * pixel > max) {
		pixel = std::max(max / data.size, 1);
	}
	return DiamondQrExact(data, pixel * style::DevicePixelRatio());
}

QImage DiamondQr(const QString &text, int pixel, int max) {
	return DiamondQr(Qr::Encode(text), pixel, max);
}

QImage DiamondQrForShare(const QString &text) {
	const auto data = Qr::Encode(text);
	const auto size = (kShareQrSize - 2 * kShareQrPadding);
	const auto image = DiamondQrExact(data, size / data.size);
	auto result = QImage(
		kShareQrPadding * 2 + image.width(),
		kShareQrPadding * 2 + image.height(),
		QImage::Format_ARGB32_Premultiplied);
	result.fill(Qt::white);
	{
		auto p = QPainter(&result);
		p.drawImage(kShareQrPadding, kShareQrPadding, image);
	}
	return result;
}

} // namespace Ui
