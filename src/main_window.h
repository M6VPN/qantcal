// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.h

#ifndef QANTCAL_MAIN_WINDOW_H
#define QANTCAL_MAIN_WINDOW_H

#include "calculators/antenna_calculator.h"
#include "design/antenna_design_scene.h"

#include <QMainWindow>

class QComboBox;
class QDoubleSpinBox;
class QGraphicsView;
class QLabel;
class QPushButton;
class QTextEdit;

namespace qantcal {

class MainWindow : public QMainWindow {
public:
	explicit MainWindow(QWidget *parent = nullptr);

private:
	void calculate();
	void create_actions();
	void create_central_widget();
	void populate_band_selector();
	void set_frequency_from_band(int index);
	void show_about();
	void show_export_pdf_placeholder();
	void show_print_placeholder();
	calculators::AntennaCalculationInput current_input() const;

	QComboBox *antenna_type_box = nullptr;
	QComboBox *band_box = nullptr;
	QComboBox *design_mode_box = nullptr;
	QDoubleSpinBox *frequency_box = nullptr;
	QDoubleSpinBox *length_box = nullptr;
	QDoubleSpinBox *velocity_factor_box = nullptr;
	QGraphicsView *design_view = nullptr;
	QPushButton *calculate_button = nullptr;
	QTextEdit *result_text = nullptr;
	design::AntennaDesignScene *design_scene = nullptr;
};

}

#endif
