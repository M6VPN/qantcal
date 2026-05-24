// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_main_window_layout.cpp

#include "main_window.h"

#include "calculators/antenna_calculator.h"
#include "design/antenna_design_scene.h"
#include "project/antenna_project.h"
#include "settings/translation_manager.h"

#include <QApplication>
#include <QComboBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QTabWidget>
#include <QTextEdit>

#include <cassert>

namespace {

void
process_resize(qantcal::MainWindow &window, int width, int height)
{
	window.resize(width, height);
	window.show();
	QApplication::processEvents();
}

void
test_antenna_selector_contains_halo(const qantcal::MainWindow &window)
{
	const QList<QComboBox *> combo_boxes = window.findChildren<QComboBox *>();
	bool found = false;

	for (const QComboBox *combo_box : combo_boxes)
		found = found || combo_box->findText(QStringLiteral("Halo")) >= 0;

	assert(found);
}

void
test_antenna_selector_contains_folded_dipole(const qantcal::MainWindow &window)
{
	const QList<QComboBox *> combo_boxes = window.findChildren<QComboBox *>();
	bool found = false;

	for (const QComboBox *combo_box : combo_boxes)
		found = found || combo_box->findText(QStringLiteral("Folded dipole")) >= 0;

	assert(found);
}

void
test_folded_dipole_scene_uses_folded_path()
{
	qantcal::design::AntennaDesignScene scene;
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::FoldedDipole;
	input.frequency_mhz = 7.1;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);
	scene.show_antenna_diagram(result, qantcal::calculators::LengthUnit::Metres);

	int path_count = 0;
	for (QGraphicsItem *item : scene.items()) {
		if (dynamic_cast<QGraphicsPathItem *>(item) != nullptr)
			++path_count;
	}

	assert(result.ok);
	assert(path_count >= 1);
}

void
test_halo_scene_uses_path()
{
	qantcal::design::AntennaDesignScene scene;
	qantcal::calculators::AntennaCalculationInput input;

	input.antenna_type = qantcal::calculators::AntennaType::Halo;
	input.frequency_mhz = 144.2;

	const qantcal::calculators::AntennaCalculationResult result =
		qantcal::calculators::calculate_antenna(input);
	scene.show_antenna_diagram(result, qantcal::calculators::LengthUnit::Metres);

	int path_count = 0;
	for (QGraphicsItem *item : scene.items()) {
		if (dynamic_cast<QGraphicsPathItem *>(item) != nullptr)
			++path_count;
	}

	assert(result.ok);
	assert(path_count >= 1);
}

void
test_help_menu_is_final(const qantcal::MainWindow &window)
{
	const QList<QAction *> actions = window.menuBar()->actions();

	assert(actions.size() >= 5);
	assert(actions.last()->menu()->objectName() == QStringLiteral("help_menu"));
	assert(actions[actions.size() - 2]->menu()->objectName() == QStringLiteral("language_menu"));
}

void
test_project_feedpoint_uses_descriptor_position()
{
	qantcal::design::AntennaDesignScene scene;
	qantcal::project::AntennaProject project;
	qantcal::project::AntennaElement element;
	qantcal::project::DiagramItemDescriptor descriptor;

	element.frequency_mhz = 7.1;
	element.label = QStringLiteral("40m");
	element.length_metres = 20.0;
	project.elements.append(element);
	descriptor.id = QStringLiteral("wire");
	descriptor.kind = QStringLiteral("dipole");
	descriptor.label = QStringLiteral("40m");
	descriptor.length_metres = 20.0;
	descriptor.points = { QPointF(-100.0, 0.0), QPointF(100.0, 0.0) };
	descriptor.position = QPointF(42.0, 33.0);
	project.diagram_items.append(descriptor);

	scene.show_project_diagram(project, qantcal::calculators::LengthUnit::Metres);

	bool found_feedpoint = false;
	for (QGraphicsItem *item : scene.items()) {
		QGraphicsEllipseItem *ellipse = dynamic_cast<QGraphicsEllipseItem *>(item);
		if (ellipse == nullptr)
			continue;
		found_feedpoint = found_feedpoint || ellipse->rect().center() == descriptor.position;
	}

	assert(found_feedpoint);
}

void
test_read_only_outputs_have_accessible_names(const qantcal::MainWindow &window)
{
	const QList<QTextEdit *> text_edits = window.findChildren<QTextEdit *>();
	int output_count = 0;

	for (const QTextEdit *text_edit : text_edits) {
		if (!text_edit->isReadOnly())
			continue;
		++output_count;
		assert(!text_edit->accessibleName().isEmpty());
	}

	assert(output_count >= 9);
}

void
test_russian_menu_translation_loads()
{
	assert(qantcal::settings::apply_language(QStringLiteral("ru_RU")));
	assert(QCoreApplication::translate("qantcal::MainWindow", "&File") == QStringLiteral("&Файл"));
	qantcal::settings::apply_language(QStringLiteral("en"));
}

void
test_scroll_areas_are_present(const qantcal::MainWindow &window)
{
	const QList<QScrollArea *> scroll_areas = window.findChildren<QScrollArea *>();

	assert(scroll_areas.size() >= 9);
	for (const QScrollArea *scroll_area : scroll_areas) {
		assert(scroll_area->widgetResizable());
		assert(!scroll_area->accessibleName().isEmpty());
	}
}

void
test_tabs_are_present(const qantcal::MainWindow &window)
{
	const QList<QTabWidget *> tab_widgets = window.findChildren<QTabWidget *>();
	bool has_main_tabs = false;
	bool has_rf_tabs = false;

	for (const QTabWidget *tab_widget : tab_widgets) {
		has_main_tabs = has_main_tabs || tab_widget->count() == 4;
		has_rf_tabs = has_rf_tabs || tab_widget->count() == 10;
	}

	assert(has_main_tabs);
	assert(has_rf_tabs);
}

}

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	qantcal::MainWindow window;

	test_folded_dipole_scene_uses_folded_path();
	test_halo_scene_uses_path();
	test_project_feedpoint_uses_descriptor_position();
	test_russian_menu_translation_loads();
	process_resize(window, 800, 600);
	test_antenna_selector_contains_folded_dipole(window);
	test_antenna_selector_contains_halo(window);
	test_help_menu_is_final(window);
	test_scroll_areas_are_present(window);
	test_read_only_outputs_have_accessible_names(window);
	test_tabs_are_present(window);

	process_resize(window, 480, 720);
	test_antenna_selector_contains_folded_dipole(window);
	test_antenna_selector_contains_halo(window);
	test_help_menu_is_final(window);
	test_scroll_areas_are_present(window);
	test_read_only_outputs_have_accessible_names(window);
	test_tabs_are_present(window);

	return 0;
}
