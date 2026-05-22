// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_guide_renderer.cpp

#include "guides/guide_export_options.h"
#include "guides/guide_renderer.h"

#include <QFile>
#include <QGuiApplication>
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

}

int
main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	test_export_options_defaults();
	test_render_empty_diagram_document();

	return 0;
}
