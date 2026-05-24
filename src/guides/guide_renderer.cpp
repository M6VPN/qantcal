// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_renderer.cpp

#include "guide_renderer.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPageSize>
#include <QPdfWriter>
#include <QPen>
#include <QPolygonF>
#include <QTextOption>

#include <limits>

namespace qantcal::guides {

namespace {

struct RenderState {
	QRectF content_rect;
	QRectF page_rect;
	double bottom = 0.0;
	double y = 0.0;
	int page_number = 1;
};

struct DiagramLegendEntry {
	QColor colour;
	QString label;
	bool feedpoint = false;
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

QVector<QPointF>
diagram_points(const project::DiagramItemDescriptor &item)
{
	QVector<QPointF> points;

	if (item.points.isEmpty()) {
		points.append(item.position);
		return points;
	}
	for (const QPointF &point : item.points)
		points.append(item.position + point);

	return points;
}

bool
is_feedpoint_kind(const QString &kind)
{
	return kind == QStringLiteral("dipole")
		|| kind == QStringLiteral("folded_dipole")
		|| kind == QStringLiteral("halo")
		|| kind == QStringLiteral("end_fed")
		|| kind == QStringLiteral("vertical")
		|| kind == QStringLiteral("yagi_driven_element");
}

bool
is_yagi_kind(const QString &kind)
{
	return kind == QStringLiteral("yagi_element")
		|| kind == QStringLiteral("yagi_driven_element");
}

QRectF
diagram_bounds(const QVector<project::DiagramItemDescriptor> &items)
{
	double left = std::numeric_limits<double>::max();
	double top = std::numeric_limits<double>::max();
	double right = std::numeric_limits<double>::lowest();
	double bottom = std::numeric_limits<double>::lowest();

	for (const project::DiagramItemDescriptor &item : items) {
		for (const QPointF &point : diagram_points(item)) {
			left = qMin(left, point.x());
			top = qMin(top, point.y());
			right = qMax(right, point.x());
			bottom = qMax(bottom, point.y());
		}
	}

	if (left == std::numeric_limits<double>::max())
		return QRectF(-1.0, -1.0, 2.0, 2.0);
	if (qFuzzyCompare(left + 1.0, right + 1.0)) {
		left -= 1.0;
		right += 1.0;
	}
	if (qFuzzyCompare(top + 1.0, bottom + 1.0)) {
		top -= 1.0;
		bottom += 1.0;
	}

	QRectF bounds(QPointF(left, top), QPointF(right, bottom));
	const double minimum_dimension = qMax(20.0, qMax(bounds.width(), bounds.height()) * 0.08);
	if (bounds.width() < minimum_dimension) {
		const double delta = (minimum_dimension - bounds.width()) / 2.0;
		bounds.adjust(-delta, 0.0, delta, 0.0);
	}
	if (bounds.height() < minimum_dimension) {
		const double delta = (minimum_dimension - bounds.height()) / 2.0;
		bounds.adjust(0.0, -delta, 0.0, delta);
	}

	const double x_padding = qMax(24.0, bounds.width() * 0.12);
	const double y_padding = qMax(24.0, bounds.height() * 0.12);
	bounds.adjust(-x_padding, -y_padding, x_padding, y_padding);

	return bounds;
}

QPointF
map_diagram_point(const QPointF &point, const QRectF &bounds, const QRectF &target)
{
	const double padding = 4.0;
	const QRectF draw_rect = target.adjusted(padding, padding, -padding, -padding);
	const double scale = qMin(draw_rect.width() / bounds.width(), draw_rect.height() / bounds.height());
	const QSizeF scaled_size(bounds.width() * scale, bounds.height() * scale);
	const QPointF origin(
		draw_rect.left() + (draw_rect.width() - scaled_size.width()) / 2.0,
		draw_rect.top() + (draw_rect.height() - scaled_size.height()) / 2.0
	);

	return QPointF(
		origin.x() + (point.x() - bounds.left()) * scale,
		origin.y() + (point.y() - bounds.top()) * scale
	);
}

QPointF
feedpoint_for_item(const QString &kind, const QPolygonF &mapped_points, const QRectF &diagram_rect)
{
	if (mapped_points.isEmpty())
		return diagram_rect.center();

	const QRectF item_bounds = mapped_points.boundingRect();
	if (kind == QStringLiteral("end_fed") || kind == QStringLiteral("vertical"))
		return mapped_points.first();
	if (kind == QStringLiteral("halo") || kind == QStringLiteral("loop"))
		return QPointF(item_bounds.center().x(), item_bounds.bottom());

	return item_bounds.center();
}

QString
legend_label_for_item(const project::DiagramItemDescriptor &item, calculators::LengthUnit length_unit)
{
	QString label = item.label;
	if (label.isEmpty())
		label = item.kind;
	if (item.length_metres > 0.0) {
		label += QStringLiteral(" / %1")
			.arg(QString::fromStdString(calculators::format_length(item.length_metres, length_unit)));
	}

	return label;
}

void
draw_diagram_legend(QPainter &painter, const QRectF &legend_rect, const QVector<DiagramLegendEntry> &entries, const QFont &font)
{
	if (entries.isEmpty())
		return;

	const int columns = entries.size() > 3 ? 2 : 1;
	const int rows = (entries.size() + columns - 1) / columns;
	const double row_height = qMax(13.0, qMin(18.0, (legend_rect.height() - 8.0) / qMax(1, rows)));
	const double column_width = legend_rect.width() / columns;
	const QFontMetricsF metrics(font);

	painter.save();
	painter.setFont(font);

	for (int i = 0; i < entries.size(); ++i) {
		const int column = i % columns;
		const int row = i / columns;
		const QRectF row_rect(
			legend_rect.left() + column * column_width,
			legend_rect.top() + 4.0 + row * row_height,
			column_width,
			row_height
		);
		const QPointF sample_start(row_rect.left() + 4.0, row_rect.center().y());
		const QPointF sample_end(sample_start.x() + 20.0, row_rect.center().y());
		const QRectF text_rect(row_rect.left() + 32.0, row_rect.top(), row_rect.width() - 36.0, row_rect.height());
		const QString text = metrics.elidedText(entries[i].label, Qt::ElideRight, text_rect.width());

		painter.setPen(QPen(entries[i].colour, 2.0));
		painter.drawLine(sample_start, sample_end);
		if (entries[i].feedpoint) {
			painter.setPen(QPen(QColor(170, 60, 40), 1.4));
			painter.drawEllipse(sample_start + QPointF(10.0, 0.0), 3.0, 3.0);
		}

		painter.setPen(Qt::black);
		painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);
	}

	painter.restore();
}

void
draw_diagram_fallback(QPainter &painter, RenderState &state)
{
	const double diagram_height = 150.0;
	ensure_space(painter, state, diagram_height + 12.0);
	const QRectF diagram_rect(state.content_rect.left(), state.y, state.content_rect.width(), diagram_height);

	painter.setPen(QPen(QColor(60, 90, 140), 2.0));
	painter.drawRect(diagram_rect);
	painter.setPen(Qt::black);
	painter.drawText(diagram_rect.adjusted(10.0, 10.0, -10.0, -10.0), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("No diagram available"));
	state.y += diagram_rect.height() + 12.0;
}

void
draw_diagram(QPainter &painter, RenderState &state, const GuideDocument &document, const QFont &font)
{
	if (document.diagram_items.isEmpty()) {
		draw_diagram_fallback(painter, state);
		return;
	}

	const int item_count = qMax(1, document.diagram_items.size());
	const int legend_columns = item_count > 3 ? 2 : 1;
	const int legend_rows = (item_count + legend_columns - 1) / legend_columns;
	const double legend_height = qMin(82.0, qMax(44.0, 18.0 * legend_rows + 12.0));
	const double diagram_height = qMin(350.0, qMax(235.0, state.content_rect.width() * 0.48 + legend_height));
	ensure_space(painter, state, diagram_height + 12.0);
	const QRectF diagram_rect(state.content_rect.left(), state.y, state.content_rect.width(), diagram_height);

	const QRectF bounds = diagram_bounds(document.diagram_items);
	const QPen frame_pen(QColor(130, 130, 130), 1.0);
	const QPen wire_pen(QColor(30, 90, 150), 2.2);
	const QPen yagi_pen(QColor(30, 90, 150), 2.0);
	const QPen feed_pen(QColor(170, 60, 40), 1.6);
	const QPen separator_pen(QColor(190, 190, 190), 0.8);
	const QRectF legend_rect(
		diagram_rect.left() + 12.0,
		diagram_rect.bottom() - legend_height - 8.0,
		diagram_rect.width() - 24.0,
		legend_height
	);
	const QRectF plot_rect = diagram_rect.adjusted(12.0, 12.0, -12.0, -(legend_height + 18.0));
	QVector<DiagramLegendEntry> legend_entries;
	QFont label_font = font;
	label_font.setPointSize(8);

	painter.save();
	painter.setFont(label_font);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(frame_pen);
	painter.drawRect(diagram_rect);
	painter.setPen(separator_pen);
	painter.drawLine(QPointF(legend_rect.left(), legend_rect.top() - 5.0), QPointF(legend_rect.right(), legend_rect.top() - 5.0));

	for (const project::DiagramItemDescriptor &item : document.diagram_items) {
		const QVector<QPointF> points = diagram_points(item);
		QPolygonF mapped_points;
		const bool yagi_kind = is_yagi_kind(item.kind);
		const QPen item_pen = yagi_kind ? yagi_pen : wire_pen;

		for (const QPointF &point : points)
			mapped_points << map_diagram_point(point, bounds, plot_rect);
		if (mapped_points.size() >= 2) {
			painter.setPen(item_pen);
			if (item.kind == QStringLiteral("loop")) {
				painter.drawPolygon(mapped_points);
			} else if (item.kind == QStringLiteral("folded_dipole") && mapped_points.size() >= 7) {
				painter.drawLine(mapped_points[0], mapped_points[1]);
				painter.drawLine(mapped_points[1], mapped_points[2]);
				painter.drawLine(mapped_points[2], mapped_points[3]);
				painter.drawLine(mapped_points[4], mapped_points[5]);
				painter.drawLine(mapped_points[5], mapped_points[6]);
			} else {
				for (int i = 1; i < mapped_points.size(); ++i)
					painter.drawLine(mapped_points[i - 1], mapped_points[i]);
			}
		}

		if (is_feedpoint_kind(item.kind)) {
			const QPointF feedpoint = feedpoint_for_item(item.kind, mapped_points, plot_rect);
			painter.setPen(feed_pen);
			painter.drawEllipse(feedpoint, 3.8, 3.8);
		}

		legend_entries.append(DiagramLegendEntry{
			item_pen.color(),
			legend_label_for_item(item, document.length_unit),
			is_feedpoint_kind(item.kind)
		});
	}

	draw_diagram_legend(painter, legend_rect, legend_entries, label_font);
	painter.restore();
	state.y += diagram_rect.height() + 12.0;
}

void
draw_section(QPainter &painter, RenderState &state, const GuideDocument &document, const GuideSection &section, const GuideExportOptions &options, const QFont &heading_font, const QFont &body_font)
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
		draw_diagram(painter, state, document, body_font);
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
		draw_section(painter, state, document, section, options, heading_font, body_font);

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
