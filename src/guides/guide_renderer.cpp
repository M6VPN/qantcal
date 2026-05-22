// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_renderer.cpp

#include "guide_renderer.h"

#include <QFont>
#include <QPdfWriter>
#include <QPen>
#include <QTextOption>

namespace qantcal::guides {

bool
GuideRenderer::render_to_painter(const GuideDocument &document, QPainter &painter, const QRectF &page_rect) const
{
	if (!painter.isActive())
		return false;

	const QRectF content_rect = page_rect.adjusted(48.0, 48.0, -48.0, -48.0);
	const QRectF title_rect(content_rect.left(), content_rect.top(), content_rect.width(), 42.0);
	const QRectF meta_rect(content_rect.left(), title_rect.bottom() + 20.0, content_rect.width(), 82.0);
	const QRectF dimensions_rect(content_rect.left(), meta_rect.bottom() + 20.0, content_rect.width(), 150.0);
	const QRectF diagram_rect(content_rect.left(), dimensions_rect.bottom() + 24.0, content_rect.width(), 160.0);
	const QRectF notes_rect(content_rect.left(), diagram_rect.bottom() + 24.0, content_rect.width(), content_rect.bottom() - diagram_rect.bottom() - 24.0);

	QFont title_font = painter.font();
	title_font.setPointSize(20);
	title_font.setBold(true);

	QFont heading_font = painter.font();
	heading_font.setPointSize(12);
	heading_font.setBold(true);

	QFont body_font = painter.font();
	body_font.setPointSize(10);

	painter.setPen(Qt::black);
	painter.setFont(title_font);
	painter.drawText(title_rect, Qt::AlignLeft | Qt::AlignTop, document.title);

	painter.setFont(body_font);
	painter.drawText(
		meta_rect,
		Qt::AlignLeft | Qt::AlignTop,
		QStringLiteral("Antenna: %1\nBand: %2\nFrequency: %3\nLength unit: %4")
			.arg(document.antenna_type)
			.arg(document.band_text)
			.arg(document.frequency_text)
			.arg(QString::fromStdString(calculators::length_unit_label(document.length_unit)))
	);

	painter.setFont(heading_font);
	painter.drawText(dimensions_rect, Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Calculated dimensions"));
	painter.setFont(body_font);
	painter.drawText(dimensions_rect.adjusted(0.0, 26.0, 0.0, 0.0), Qt::AlignLeft | Qt::AlignTop, document.dimensions_text);

	painter.setPen(QPen(QColor(60, 90, 140), 3.0));
	painter.drawRect(diagram_rect);
	painter.drawLine(diagram_rect.left() + 60.0, diagram_rect.center().y(), diagram_rect.right() - 60.0, diagram_rect.center().y());
	painter.setPen(Qt::black);
	painter.setFont(body_font);
	painter.drawText(diagram_rect.adjusted(12.0, 12.0, -12.0, -12.0), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Diagram placeholder"));

	painter.setFont(heading_font);
	painter.drawText(notes_rect, Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Practical notes"));
	painter.setFont(body_font);
	painter.drawText(notes_rect.adjusted(0.0, 26.0, 0.0, 0.0), Qt::TextWordWrap, document.notes_text);

	return true;
}

bool
GuideRenderer::render_to_pdf(const GuideDocument &document, const QString &path) const
{
	if (path.isEmpty())
		return false;

	QPdfWriter writer(path);
	writer.setPageSize(QPageSize(QPageSize::A4));
	writer.setResolution(96);

	QPainter painter(&writer);
	if (!painter.isActive())
		return false;

	const bool ok = render_to_painter(document, painter, QRectF(0.0, 0.0, writer.width(), writer.height()));
	painter.end();

	return ok;
}

}
