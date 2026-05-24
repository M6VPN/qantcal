// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_guide_renderer.cpp

#include "guides/guide_export_options.h"
#include "guides/guide_document.h"
#include "guides/guide_renderer.h"

#include "project/antenna_project.h"

#include <QColor>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTemporaryFile>

#include <cassert>

namespace {

qantcal::guides::GuideDocument
sample_document()
{
	qantcal::guides::GuideDocument document;

	document.antenna_type = QStringLiteral("Half-wave dipole");
	document.generated_utc = QStringLiteral("2026-05-22T00:00:00Z");
	document.length_unit = qantcal::calculators::LengthUnit::Metres;
	document.project_title = QStringLiteral("Renderer test");
	document.title = QStringLiteral("Renderer test");
	document.sections.append(qantcal::guides::make_text_section(QStringLiteral("Title / project summary"), QStringLiteral("Project: Renderer test")));
	document.sections.append(qantcal::guides::make_text_section(QStringLiteral("Diagram"), QStringLiteral("No scene required for test.")));

	return document;
}

qantcal::guides::GuideDocument
sample_vertical_diagram_document()
{
	qantcal::guides::GuideDocument document;
	qantcal::project::DiagramItemDescriptor item;

	document.antenna_type = QStringLiteral("Quarter-wave vertical");
	document.generated_utc = QStringLiteral("2026-05-22T00:00:00Z");
	document.length_unit = qantcal::calculators::LengthUnit::Metres;
	document.project_title = QStringLiteral("Vertical renderer test");
	document.title = QStringLiteral("Vertical renderer test");
	document.sections.append(qantcal::guides::make_text_section(QStringLiteral("Diagram"), QString()));
	item.id = QStringLiteral("vertical-1");
	item.kind = QStringLiteral("vertical");
	item.label = QStringLiteral("Vertical radiator with radials");
	item.length_metres = 5.05;
	item.points.append(QPointF(0.0, 70.0));
	item.points.append(QPointF(0.0, -80.0));
	item.points.append(QPointF(0.0, 70.0));
	item.points.append(QPointF(-120.0, 105.0));
	item.points.append(QPointF(0.0, 70.0));
	item.points.append(QPointF(120.0, 105.0));
	document.diagram_items.append(item);

	return document;
}

qantcal::guides::GuideDocument
sample_folded_dipole_document()
{
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::FoldedDipole;
	input.frequency_mhz = 7.1;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);

	return qantcal::guides::create_guide_document(result, qantcal::calculators::LengthUnit::Metres, QStringLiteral("40m"));
}

qantcal::guides::GuideDocument
sample_line_diagram_document()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();
	qantcal::project::AntennaTarget target;
	qantcal::project::AntennaElement element;
	qantcal::project::DiagramItemDescriptor item;

	project.title = QStringLiteral("PDF dipole");
	target.band_name = QStringLiteral("40m");
	target.enabled = true;
	target.frequency_mhz = 7.1;
	project.targets.append(target);
	element.frequency_mhz = 7.1;
	element.label = QStringLiteral("40m dipole");
	element.length_metres = 20.057;
	element.role = QStringLiteral("calculated_element");
	project.elements.append(element);
	item.id = QStringLiteral("line-1");
	item.kind = QStringLiteral("line");
	item.label = QStringLiteral("40m dipole");
	item.length_metres = 20.057;
	item.points.append(QPointF(-220.0, 0.0));
	item.points.append(QPointF(220.0, 0.0));
	project.diagram_items.append(item);

	return qantcal::guides::create_project_guide_document(project, qantcal::calculators::LengthUnit::Metres);
}

qantcal::guides::GuideDocument
sample_yagi_document()
{
	qantcal::project::AntennaProject project = qantcal::project::default_project();
	qantcal::project::AntennaTarget target;
	qantcal::project::AntennaElement element;

	project.antenna_type = qantcal::calculators::AntennaType::Yagi;
	project.title = QStringLiteral("PDF Yagi");
	project.yagi_design.enabled = true;
	project.yagi_design.element_count = 3;
	project.yagi_design.frequency_mhz = 144.3;
	target.band_name = QStringLiteral("2m");
	target.enabled = true;
	target.frequency_mhz = 144.3;
	project.targets.append(target);
	element.frequency_mhz = 144.3;
	element.label = QStringLiteral("Reflector");
	element.length_metres = 1.036;
	element.role = QStringLiteral("reflector");
	project.elements.append(element);
	qantcal::project::DiagramItemDescriptor item;
	item.id = QStringLiteral("reflector");
	item.kind = QStringLiteral("yagi_element");
	item.label = QStringLiteral("Reflector");
	item.length_metres = 1.036;
	item.position = QPointF(-170.0, 0.0);
	item.points.append(QPointF(0.0, -85.0));
	item.points.append(QPointF(0.0, 85.0));
	project.diagram_items.append(item);

	return qantcal::guides::create_project_guide_document(project, qantcal::calculators::LengthUnit::Centimetres);
}

QRect
blue_pixel_bounds(const QImage &image)
{
	int left = image.width();
	int top = image.height();
	int right = -1;
	int bottom = -1;

	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			const QColor colour = QColor::fromRgb(image.pixel(x, y));
			if (colour.blue() > 100 && colour.red() < 120 && colour.green() < 150) {
				left = qMin(left, x);
				top = qMin(top, y);
				right = qMax(right, x);
				bottom = qMax(bottom, y);
			}
		}
	}

	if (right < left || bottom < top)
		return QRect();

	return QRect(QPoint(left, top), QPoint(right, bottom));
}

int
dark_pixel_count(const QImage &image, const QRect &area)
{
	int count = 0;
	const QRect bounded = area.intersected(image.rect());

	for (int y = bounded.top(); y <= bounded.bottom(); ++y) {
		for (int x = bounded.left(); x <= bounded.right(); ++x) {
			const QColor colour = QColor::fromRgb(image.pixel(x, y));
			if (colour.red() < 80 && colour.green() < 80 && colour.blue() < 80)
				count++;
		}
	}

	return count;
}

QImage
render_document_image(const qantcal::guides::GuideDocument &document)
{
	QImage image(900, 1200, QImage::Format_ARGB32_Premultiplied);
	qantcal::guides::GuideRenderer renderer;

	image.fill(Qt::white);
	QPainter painter(&image);
	assert(renderer.render_to_painter(document, painter, QRectF(0.0, 0.0, image.width(), image.height()), qantcal::guides::default_export_options()));
	painter.end();

	return image;
}

void
test_export_options_defaults()
{
	const qantcal::guides::GuideExportOptions options = qantcal::guides::default_export_options();

	assert(options.include_assumptions);
	assert(options.include_diagram);
	assert(options.include_notes);
	assert(options.include_safety_notes);
	assert(options.page_size_name == QStringLiteral("A4"));
	assert(options.margin_mm > 0.0);
}

void
test_render_empty_diagram_document()
{
	QTemporaryFile file;
	qantcal::guides::GuideRenderer renderer;

	assert(file.open());
	const QString path = file.fileName();
	file.close();

	assert(renderer.render_to_pdf(sample_document(), path, qantcal::guides::default_export_options()));
	assert(QFile::exists(path));
	assert(QFile(path).size() > 0);
}

void
test_render_folded_dipole_pdf()
{
	QTemporaryFile file;
	qantcal::guides::GuideRenderer renderer;
	const qantcal::guides::GuideDocument document = sample_folded_dipole_document();

	assert(document.diagram_items.size() == 1);
	assert(document.diagram_items[0].kind == QStringLiteral("folded_dipole"));
	assert(file.open());
	const QString path = file.fileName();
	file.close();

	assert(renderer.render_to_pdf(document, path, qantcal::guides::default_export_options()));
	assert(QFile::exists(path));
	assert(QFile(path).size() > 0);
}

void
test_render_line_diagram_pdf()
{
	QTemporaryFile file;
	qantcal::guides::GuideRenderer renderer;
	const qantcal::guides::GuideDocument document = sample_line_diagram_document();

	assert(document.diagram_items.size() == 1);
	assert(file.open());
	const QString path = file.fileName();
	file.close();

	assert(renderer.render_to_pdf(document, path, qantcal::guides::default_export_options()));
	assert(QFile::exists(path));
	assert(QFile(path).size() > 0);
}

void
test_render_vertical_diagram_image_is_not_flattened()
{
	const QImage image = render_document_image(sample_vertical_diagram_document());
	const QRect wire_bounds = blue_pixel_bounds(image);

	assert(wire_bounds.isValid());
	assert(wire_bounds.height() > 160);
	assert(wire_bounds.width() > 160);
	assert(dark_pixel_count(image, QRect(0, wire_bounds.top(), image.width(), wire_bounds.height() + 80)) > 20);
}

void
test_render_vertical_diagram_pdf()
{
	QTemporaryFile file;
	qantcal::guides::GuideRenderer renderer;
	const qantcal::guides::GuideDocument document = sample_vertical_diagram_document();

	assert(document.diagram_items.size() == 1);
	assert(document.diagram_items[0].kind == QStringLiteral("vertical"));
	assert(file.open());
	const QString path = file.fileName();
	file.close();

	assert(renderer.render_to_pdf(document, path, qantcal::guides::default_export_options()));
	assert(QFile::exists(path));
	assert(QFile(path).size() > 0);
}

void
test_render_yagi_pdf()
{
	QTemporaryFile file;
	qantcal::guides::GuideRenderer renderer;
	const qantcal::guides::GuideDocument document = sample_yagi_document();

	assert(document.diagram_items.size() == 1);
	assert(file.open());
	const QString path = file.fileName();
	file.close();

	assert(renderer.render_to_pdf(document, path, qantcal::guides::default_export_options()));
	assert(QFile::exists(path));
	assert(QFile(path).size() > 0);
}

}

int
main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	test_export_options_defaults();
	test_render_empty_diagram_document();
	test_render_folded_dipole_pdf();
	test_render_line_diagram_pdf();
	test_render_vertical_diagram_image_is_not_flattened();
	test_render_vertical_diagram_pdf();
	test_render_yagi_pdf();

	return 0;
}
