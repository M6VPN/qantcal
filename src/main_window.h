// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.h

#ifndef QANTCAL_MAIN_WINDOW_H
#define QANTCAL_MAIN_WINDOW_H

#include "calculators/antenna_calculator.h"
#include "design/antenna_design_scene.h"
#include "design/antenna_design_view.h"
#include "project/antenna_project.h"
#include "settings/app_settings.h"

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTabWidget;
class QTextEdit;
class QUndoStack;

namespace qantcal {

class MainWindow : public QMainWindow {
public:
	explicit MainWindow(QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent *event) override;

private:
	void calculate();
	void change_length_unit(int index);
	void configure_length_input();
	void create_actions();
	void create_antenna_tab(QTabWidget *tabs);
	void create_central_widget();
	void create_rf_calculators_tab(QTabWidget *tabs);
	void export_pdf();
	void calculate_coil();
	void calculate_lc();
	void calculate_horizon();
	void calculate_swr();
	void add_current_target();
	void apply_project_to_ui();
	void build_project_from_ui();
	bool confirm_discard_changes();
	void mark_project_dirty();
	void mark_project_dirty_and_recalculate();
	void new_project();
	void open_project();
	void populate_band_selector();
	void recalculate_targets();
	void remove_selected_target();
	void restore_settings();
	void save_antenna_type();
	bool save_project();
	bool save_project_as();
	void save_shortening_factor();
	void set_frequency_from_band(int index);
	void show_about();
	void show_print_placeholder();
	void target_item_changed(QListWidgetItem *item);
	void update_diagram_item_descriptor(const project::DiagramItemDescriptor &descriptor);
	void update_project_title();
	void update_target_list();
	calculators::AntennaCalculationInput current_input() const;
	calculators::LengthUnit selected_length_unit() const;

	QComboBox *antenna_type_box = nullptr;
	QComboBox *band_box = nullptr;
	QComboBox *design_mode_box = nullptr;
	QComboBox *length_unit_box = nullptr;
	QDoubleSpinBox *frequency_box = nullptr;
	QDoubleSpinBox *coil_diameter_box = nullptr;
	QDoubleSpinBox *coil_length_box = nullptr;
	QDoubleSpinBox *coil_turns_box = nullptr;
	QDoubleSpinBox *horizon_rx_height_box = nullptr;
	QDoubleSpinBox *horizon_tx_height_box = nullptr;
	QDoubleSpinBox *lc_capacitance_box = nullptr;
	QDoubleSpinBox *lc_frequency_box = nullptr;
	QDoubleSpinBox *lc_inductance_box = nullptr;
	QDoubleSpinBox *length_box = nullptr;
	QDoubleSpinBox *swr_forward_power_box = nullptr;
	QDoubleSpinBox *swr_value_box = nullptr;
	QDoubleSpinBox *velocity_factor_box = nullptr;
	design::AntennaDesignView *design_view = nullptr;
	QLineEdit *project_title_box = nullptr;
	QPushButton *calculate_button = nullptr;
	QTextEdit *coil_result_text = nullptr;
	QTextEdit *horizon_result_text = nullptr;
	QTextEdit *lc_result_text = nullptr;
	QTextEdit *project_notes_edit = nullptr;
	QTextEdit *result_text = nullptr;
	QTextEdit *swr_result_text = nullptr;
	QListWidget *target_list = nullptr;
	project::AntennaProject current_project;
	QString current_project_path;
	calculators::AntennaCalculationResult latest_result;
	calculators::LengthUnit current_length_unit = calculators::LengthUnit::Metres;
	design::AntennaDesignScene *design_scene = nullptr;
	bool project_dirty = false;
	settings::AppSettings app_settings;
	QUndoStack *undo_stack = nullptr;
};

}

#endif
