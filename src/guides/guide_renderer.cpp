// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_renderer.cpp

#include "guide_renderer.h"

#include <QFont>
#include <QPageSize>
#include <QPdfWriter>
#include <QPen>
#include <QTextOption>

namespace qantcal::guides {

namespace {

struct RenderState {
	QRectF content_rect;
	QRectF page_rect;
	double bottom = 0.0;
	double y = 0.0;
	int page_number = 1;
};

double
mm_to_points(double mm)
{
	return mm * 72.0 / 25.4;
}

void
draw_footer(QPainter &painter, const RenderState &state)
{
	QFont font = painter.font();
	font.setPointSize(8);
	painter.setFont(font);
	painter.setPen(QColor(90, 90, 90));
	painter.drawText(
		QRectF(state.content_rect.left(), state.page_rect.bottom() - 30.0, state.content_rect.width(), 20.0),
		Qt::AlignRight,
		QStringLiteral("Page %1").arg(state.page_number)
	);
}

void
new_page(QPainter &painter, RenderState &state)
{
	draw_footer(painter, state);
	QPdfWriter *pdf_writer = dynamic_cast<QPdfWriter *>(painter.device());
	QPrinter *printer = dynamic_cast<QPrinter *>(painter.device());

	if (pdf_writer != nullptr)
		pdf_writer->newPage();
	else if (printer != nullptr)
		printer->newPage();

	state.page_number++;
	state.y = state.content_rect.top();
}

void
ensure_space(QPainter &painter, RenderState &state, double needed)
{
	if (state.y + needed > state.bottom)
		new_page(painter, state);
}

void
draw_wrapped_text(QPainter &painter, RenderState &state, const QString &text, const QFont &font, double spacing = 8.0)
{
	if (text.isEmpty())
		return;

	painter.setFont(font);
	painter.setPen(Qt::black);
	QTextOption option;
	option.setWrapMode(QTextOption::WordWrap);

	const QRectF estimate = painter.boundingRect(
		QRectF(state.content_rect.left(), state.y, state.content_rect.width(), 10000.0),
		text,
		option
	);
	ensure_space(painter, state, estimate.height() + spacing);
	painter.drawText(QRectF(state.content_rect.left(), state.y, state.content_rect.width(), estimate.height()), text, option);
	state.y += estimate.height() + spacing;
}

void
draw_table(QPainter &painter, RenderState &state, const QVector<GuideTableRow> &rows, const QFont &font)
{
	if (rows.isEmpty())
		return;

	painter.setFont(font);
	painter.setPen(Qt::black);

	const double row_height = 22.0;
	const double first_col_width = state.content_rect.width() * 0.45;
	const double second_col_width = state.content_rect.width() - first_col_width;

	for (const GuideTableRow &row : rows) {
		ensure_space(painter, state, row_height + 2.0);
		const QRectF first_rect(state.content_rect.left(), state.y, first_col_width, row_height);
		const QRectF second_rect(first_rect.right(), state.y, second_col_width, row_height);

		painter.drawRect(first_rect);
		painter.drawRect(second_rect);
		painter.drawText(first_rect.adjusted(5.0, 2.0, -5.0, -2.0), Qt::AlignVCenter | Qt::AlignLeft, row.cells.value(0));
		painter.drawText(second_rect.adjusted(5.0, 2.0, -5.0, -2.0), Qt::AlignVCenter | Qt::AlignLeft, row.cells.value(1));
		state.y += row_height;
	}

	state.y += 10.0;
}

void
draw_diagram_placeholder(QPainter &painter, RenderState &state)
{
	const QRectF diagram_rect(state.content_rect.left(), state.y, state.content_rect.width(), 150.0);
	ensure_space(painter, state, diagram_rect.height() + 12.0);

	painter.setPen(QPen(QColor(60, 90, 140), 2.0));
	painter.drawRect(diagram_rect);
	painter.drawLine(diagram_rect.left() + 50.0, diagram_rect.center().y(), diagram_rect.right() - 50.0, diagram_rect.center().y());
	painter.setPen(Qt::black);
	painter.drawText(diagram_rect.adjusted(10.0, 10.0, -10.0, -10.0), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Diagram snapshot placeholder"));
	state.y += diagram_rect.height() + 12.0;
}

void
draw_section(QPainter &painter, RenderState &state, const GuideSection &section, const GuideExportOptions &options, const QFont &heading_font, const QFont &body_font)
{
	if (section.title == QStringLiteral("Diagram") && !options.include_diagram)
		return;
	if (section.title == QStringLiteral("Construction notes") && !options.include_notes)
		return;
	if (section.title == QStringLiteral("Assumptions and limitations") && !options.include_assumptions)
		return;
	if (section.title == QStringLiteral("Safety and licence reminder") && !options.include_safety_notes)
		return;

	ensure_space(painter, state, 40.0);
	painter.setFont(heading_font);
	painter.setPen(section.warning ? QColor(150, 60, 30) : QColor(30, 30, 30));
	painter.drawText(QRectF(state.content_rect.left(), state.y, state.content_rect.width(), 24.0), Qt::AlignLeft | Qt::AlignVCenter, section.title);
	state.y += 28.0;

	if (section.title == QStringLiteral("Diagram")) {
		draw_wrapped_text(painter, state, section.body_text, body_font, 4.0);
		draw_diagram_placeholder(painter, state);
		return;
	}

	draw_wrapped_text(painter, state, section.body_text, body_font);
	draw_table(painter, state, section.table_rows, body_font);
}

}

bool
GuideRenderer::render_to_painter(const GuideDocument &document, QPainter &painter, const QRectF &page_rect, const GuideExportOptions &options) const
{
	if (!painter.isActive())
		return false;

	const double margin = mm_to_points(options.margin_mm);
	RenderState state;
	state.page_rect = page_rect;
	state.content_rect = page_rect.adjusted(margin, margin, -margin, -margin);
	state.bottom = state.content_rect.bottom() - 24.0;
	state.y = state.content_rect.top();

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
	painter.drawText(QRectF(state.content_rect.left(), state.y, state.content_rect.width(), 34.0), Qt::AlignLeft | Qt::AlignTop, document.title);
	state.y += 44.0;

	draw_wrapped_text(
		painter,
		state,
		QStringLiteral("Generated: %1\nProject: %2\nAntenna: %3")
			.arg(document.generated_utc)
			.arg(document.project_title)
			.arg(document.antenna_type),
		body_font
	);

	for (const GuideSection &section : document.sections)
		draw_section(painter, state, section, options, heading_font, body_font);

	draw_footer(painter, state);

	return true;
}

bool
GuideRenderer::render_to_pdf(const GuideDocument &document, const QString &path, const GuideExportOptions &options) const
{
	if (path.isEmpty())
		return false;

	QPdfWriter writer(path);
	writer.setCreator(QStringLiteral("qantcal"));
	writer.setPageSize(QPageSize(QPageSize::A4));
	writer.setResolution(72);
	writer.setTitle(document.title);

	QPainter painter(&writer);
	if (!painter.isActive())
		return false;

	const bool ok = render_to_painter(document, painter, QRectF(0.0, 0.0, writer.width(), writer.height()), options);
	painter.end();

	return ok;
}

bool
GuideRenderer::render_to_printer(const GuideDocument &document, QPrinter &printer, const GuideExportOptions &options) const
{
	QPainter painter(&printer);

	if (!painter.isActive())
		return false;

	const bool ok = render_to_painter(document, painter, QRectF(0.0, 0.0, printer.width(), printer.height()), options);
	painter.end();

	return ok;
}

}
