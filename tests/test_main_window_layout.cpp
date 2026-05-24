// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_main_window_layout.cpp

#include "main_window.h"

#include <QApplication>
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

	process_resize(window, 800, 600);
	test_scroll_areas_are_present(window);
	test_read_only_outputs_have_accessible_names(window);
	test_tabs_are_present(window);

	process_resize(window, 480, 720);
	test_scroll_areas_are_present(window);
	test_read_only_outputs_have_accessible_names(window);
	test_tabs_are_present(window);

	return 0;
}
