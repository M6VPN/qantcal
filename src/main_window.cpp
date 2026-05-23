// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.cpp

#include "main_window.h"

#include "calculators/coil_calculator.h"
#include "calculators/lc_resonance_calculator.h"
#include "calculators/radio_horizon_calculator.h"
#include "calculators/rf_units.h"
#include "calculators/swr_calculator.h"
#include "calculators/yagi_calculator.h"
#include "design/antenna_design_view.h"
#include "guides/guide_document.h"
#include "guides/guide_renderer.h"
#include "project/project_file_io.h"
#include "reference/band_reference.h"
#include "reference/mode_reference.h"
#include "reference/propagation_notes.h"
#include "reference/reach_estimator.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileInfo>
#include <QUndoStack>

namespace qantcal {

namespace {

QString
result_to_text(const calculators::AntennaCalculationResult &result, calculators::LengthUnit length_unit)
{
	if (!result.ok)
		return QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error));

	QString text;
	text += QStringLiteral("Frequency: %1\n")
		.arg(QString::fromStdString(calculators::format_mhz(result.frequency_mhz)));
	text += QStringLiteral("Wavelength: %1\n")
		.arg(QString::fromStdString(calculators::format_length(result.wavelength_m, length_unit)));
	text += QStringLiteral("Length unit: %1\n")
		.arg(QString::fromStdString(calculators::length_unit_label(length_unit)));
	text += QStringLiteral("Velocity / shortening factor: %1\n")
		.arg(result.shortening_factor, 0, 'f', 3);

	if (result.total_length_m > 0.0) {
		text += QStringLiteral("Total length: %1\n")
			.arg(QString::fromStdString(calculators::format_length(result.total_length_m, length_unit)));
	}

	if (result.leg_length_m > 0.0) {
		text += QStringLiteral("Per-leg length: %1\n")
			.arg(QString::fromStdString(calculators::format_length(result.leg_length_m, length_unit)));
	}

	if (result.radiator_length_m > 0.0) {
		text += QStringLiteral("Radiator length: %1\n")
			.arg(QString::fromStdString(calculators::format_length(result.radiator_length_m, length_unit)));
	}

	text += QStringLiteral("\nCounterpoise/radial note: %1\n")
		.arg(QString::fromStdString(result.counterpoise_note));
	text += QStringLiteral("Matching note: %1\n")
		.arg(QString::fromStdString(result.matching_note));
	text += QStringLiteral("Trimming note: %1\n")
		.arg(QString::fromStdString(result.trimming_note));

	return text;
}

QString
yagi_result_to_text(const calculators::YagiDesignResult &result, calculators::LengthUnit length_unit)
{
	if (!result.ok)
		return QStringLiteral("Input error: %1").arg(result.error_message);

	QString text;
	text += QStringLiteral("Frequency: %1\n")
		.arg(QString::fromStdString(calculators::format_mhz(result.frequency_mhz)));
	text += QStringLiteral("Wavelength: %1\n")
		.arg(QString::fromStdString(calculators::format_length(result.wavelength_metres, length_unit)));
	text += QStringLiteral("Boom length: %1\n")
		.arg(QString::fromStdString(calculators::format_length(result.boom_length_metres, length_unit)));
	text += QStringLiteral("Length unit: %1\n\n")
		.arg(QString::fromStdString(calculators::length_unit_label(length_unit)));
	text += QStringLiteral("%1  %2  %3  %4  %5  %6  %7\n")
		.arg(QStringLiteral("No."), -4)
		.arg(QStringLiteral("Element"), -12)
		.arg(QStringLiteral("Role"), -10)
		.arg(QStringLiteral("Length"), -15)
		.arg(QStringLiteral("Half-length"), -15)
		.arg(QStringLiteral("Spacing"), -15)
		.arg(QStringLiteral("Position"), -15);

	for (int i = 0; i < result.elements.size(); ++i) {
		const calculators::YagiElement &element = result.elements[i];
		text += QStringLiteral("%1  %2  %3  %4  %5  %6  %7\n")
			.arg(i + 1)
			.arg(element.label, -12)
			.arg(calculators::yagi_element_role_label(element.role), -10)
			.arg(QString::fromStdString(calculators::format_length(element.length_metres, length_unit)), -15)
			.arg(QString::fromStdString(calculators::format_length(element.half_length_metres, length_unit)), -15)
			.arg(QString::fromStdString(calculators::format_length(element.spacing_from_previous_metres, length_unit)), -15)
			.arg(QString::fromStdString(calculators::format_length(element.position_from_reflector_metres, length_unit)), -15)
			.arg(element.notes);
	}

	text += QStringLiteral("\nAssumptions:\n%1\n").arg(result.assumptions.join(QStringLiteral("\n")));
	text += QStringLiteral("\nConstruction notes:\n%1\n").arg(result.construction_notes.join(QStringLiteral("\n")));
	text += QStringLiteral("\nTuning notes:\n%1\n").arg(result.tuning_notes.join(QStringLiteral("\n")));

	return text;
}

QDoubleSpinBox *
create_positive_spin_box(QWidget *parent, double maximum, int decimals, const QString &suffix, double value)
{
	QDoubleSpinBox *box = new QDoubleSpinBox(parent);

	box->setRange(0.0, maximum);
	box->setDecimals(decimals);
	box->setSuffix(suffix);
	box->setValue(value);

	return box;
}

reference::BandReferenceFilter
band_reference_filter_from_combo(const QComboBox *box)
{
	if (box == nullptr)
		return reference::BandReferenceFilter::Amateur;

	return static_cast<reference::BandReferenceFilter>(box->currentData().toInt());
}

}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	current_project = project::default_project();
	undo_stack = new QUndoStack(this);
	update_project_title();
	resize(1100, 720);

	create_actions();
	create_central_widget();
	restore_settings();
	calculate();
}

void
MainWindow::calculate()
{
	if (static_cast<calculators::AntennaType>(antenna_type_box->currentData().toInt()) == calculators::AntennaType::Yagi) {
		calculate_yagi();
		return;
	}

	const calculators::AntennaCalculationInput input = current_input();
	const calculators::AntennaCalculationResult result = calculators::calculate_antenna(input);

	latest_result = result;
	latest_yagi_result = calculators::YagiDesignResult();
	result_text->setPlainText(result_to_text(result, current_length_unit));
	if (result.ok) {
		reference::BandReference band;
		const QString band_name = band_box->itemData(band_box->currentIndex(), Qt::UserRole + 1).toString();
		if (reference::band_reference_by_name(band_name, band) && band.service != reference::BandService::Amateur)
			result_text->append(QStringLiteral("\nService warning: %1").arg(band.warning));
	}
	build_project_from_ui();
	if (current_project.targets.isEmpty())
		design_scene->show_antenna_diagram(result, current_length_unit);
	else
		design_scene->show_project_diagram(current_project, current_length_unit);

	if (result.ok) {
		statusBar()->showMessage(QStringLiteral("Calculation updated"));
		return;
	}

	statusBar()->showMessage(QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error)));
}

void
MainWindow::change_length_unit(int index)
{
	const calculators::LengthUnit new_unit = static_cast<calculators::LengthUnit>(length_unit_box->itemData(index).toInt());
	const double length_m = calculators::length_unit_to_metres(length_box->value(), current_length_unit);
	const double yagi_diameter_m = calculators::length_unit_to_metres(yagi_element_diameter_box->value(), current_length_unit);
	const double yagi_boom_correction_m = calculators::length_unit_to_metres(yagi_boom_correction_box->value(), current_length_unit);
	const double propagation_tx_height_m = propagation_tx_height_box != nullptr
		? calculators::length_unit_to_metres(propagation_tx_height_box->value(), current_length_unit)
		: 0.0;
	const double propagation_rx_height_m = propagation_rx_height_box != nullptr
		? calculators::length_unit_to_metres(propagation_rx_height_box->value(), current_length_unit)
		: 0.0;

	current_length_unit = new_unit;
	configure_length_input();

	{
		const QSignalBlocker blocker(length_box);
		length_box->setValue(calculators::metres_to_length_unit(length_m, current_length_unit));
	}
	{
		const QSignalBlocker blocker(yagi_element_diameter_box);
		yagi_element_diameter_box->setValue(calculators::metres_to_length_unit(yagi_diameter_m, current_length_unit));
	}
	{
		const QSignalBlocker blocker(yagi_boom_correction_box);
		yagi_boom_correction_box->setValue(calculators::metres_to_length_unit(yagi_boom_correction_m, current_length_unit));
	}
	if (propagation_tx_height_box != nullptr && propagation_rx_height_box != nullptr) {
		update_reference_height_inputs();
		{
			const QSignalBlocker blocker(propagation_tx_height_box);
			propagation_tx_height_box->setValue(calculators::metres_to_length_unit(propagation_tx_height_m, current_length_unit));
		}
		{
			const QSignalBlocker blocker(propagation_rx_height_box);
			propagation_rx_height_box->setValue(calculators::metres_to_length_unit(propagation_rx_height_m, current_length_unit));
		}
	}

	app_settings.set_length_unit(current_length_unit);
	current_project.preferred_length_unit = current_length_unit;
	mark_project_dirty();
	calculate();
	update_reference_panel();
	statusBar()->showMessage(
		QStringLiteral("Length unit set to %1")
			.arg(QString::fromStdString(calculators::length_unit_label(current_length_unit)))
	);
}

void
MainWindow::configure_length_input()
{
	switch (current_length_unit) {
	case calculators::LengthUnit::Millimetres:
		length_box->setRange(1.0, 10000000.0);
		length_box->setDecimals(1);
		length_box->setSingleStep(10.0);
		length_box->setSuffix(QStringLiteral(" mm"));
		break;
	case calculators::LengthUnit::Centimetres:
		length_box->setRange(0.1, 1000000.0);
		length_box->setDecimals(2);
		length_box->setSingleStep(1.0);
		length_box->setSuffix(QStringLiteral(" cm"));
		break;
	case calculators::LengthUnit::Metres:
		length_box->setRange(0.001, 10000.0);
		length_box->setDecimals(3);
		length_box->setSingleStep(0.1);
		length_box->setSuffix(QStringLiteral(" m"));
		break;
	case calculators::LengthUnit::FeetInches:
		/* TODO: Add richer parsing for values such as 33 ft 6 in, 33' 6", or 33:6. */
		length_box->setRange(0.001, calculators::metres_to_feet(10000.0));
		length_box->setDecimals(3);
		length_box->setSingleStep(1.0);
		length_box->setSuffix(QStringLiteral(" ft"));
		break;
	}

	if (yagi_element_diameter_box == nullptr || yagi_boom_correction_box == nullptr)
		return;

	yagi_element_diameter_box->setSuffix(length_box->suffix());
	yagi_boom_correction_box->setSuffix(length_box->suffix());
	yagi_element_diameter_box->setDecimals(length_box->decimals());
	yagi_boom_correction_box->setDecimals(length_box->decimals());
	yagi_element_diameter_box->setSingleStep(length_box->singleStep());
	yagi_boom_correction_box->setSingleStep(length_box->singleStep());
	update_reference_height_inputs();
}

void
MainWindow::closeEvent(QCloseEvent *event)
{
	if (confirm_discard_changes()) {
		event->accept();
		return;
	}

	event->ignore();
}

void
MainWindow::create_actions()
{
	QMenu *file_menu = menuBar()->addMenu(QStringLiteral("&File"));
	QAction *new_action = file_menu->addAction(QStringLiteral("New Project"));
	QAction *open_action = file_menu->addAction(QStringLiteral("Open Project"));
	QAction *save_action = file_menu->addAction(QStringLiteral("Save Project"));
	QAction *save_as_action = file_menu->addAction(QStringLiteral("Save Project As"));
	file_menu->addSeparator();
	QAction *print_action = file_menu->addAction(QStringLiteral("Print Guide"));
	QAction *export_pdf_action = file_menu->addAction(QStringLiteral("Export PDF"));
	file_menu->addSeparator();
	QAction *exit_action = file_menu->addAction(QStringLiteral("Exit"));

	QMenu *help_menu = menuBar()->addMenu(QStringLiteral("&Help"));
	QAction *about_action = help_menu->addAction(QStringLiteral("About qantcal"));
	QMenu *edit_menu = menuBar()->addMenu(QStringLiteral("&Edit"));
	QAction *undo_action = undo_stack->createUndoAction(this, QStringLiteral("Undo"));
	QAction *redo_action = undo_stack->createRedoAction(this, QStringLiteral("Redo"));
	QMenu *view_menu = menuBar()->addMenu(QStringLiteral("&View"));
	QAction *zoom_in_action = view_menu->addAction(QStringLiteral("Zoom In"));
	QAction *zoom_out_action = view_menu->addAction(QStringLiteral("Zoom Out"));
	QAction *reset_zoom_action = view_menu->addAction(QStringLiteral("Reset Zoom"));
	QAction *fit_design_action = view_menu->addAction(QStringLiteral("Fit Design"));
	QAction *pan_action = view_menu->addAction(QStringLiteral("Pan Mode"));
	pan_action->setCheckable(true);

	edit_menu->addAction(undo_action);
	edit_menu->addAction(redo_action);

	connect(new_action, &QAction::triggered, this, &MainWindow::new_project);
	connect(open_action, &QAction::triggered, this, &MainWindow::open_project);
	connect(save_action, &QAction::triggered, this, &MainWindow::save_project);
	connect(save_as_action, &QAction::triggered, this, &MainWindow::save_project_as);
	connect(print_action, &QAction::triggered, this, &MainWindow::print_guide);
	connect(export_pdf_action, &QAction::triggered, this, &MainWindow::export_pdf);
	connect(exit_action, &QAction::triggered, this, &QWidget::close);
	connect(about_action, &QAction::triggered, this, &MainWindow::show_about);
	connect(zoom_in_action, &QAction::triggered, this, [this]() { design_view->zoom_in(); });
	connect(zoom_out_action, &QAction::triggered, this, [this]() { design_view->zoom_out(); });
	connect(reset_zoom_action, &QAction::triggered, this, [this]() { design_view->reset_zoom(); });
	connect(fit_design_action, &QAction::triggered, this, [this]() { design_view->fit_design(); });
	connect(pan_action, &QAction::toggled, this, [this](bool enabled) { design_view->set_pan_mode(enabled); });
}

void
MainWindow::create_central_widget()
{
	QWidget *central = new QWidget(this);
	QVBoxLayout *root_layout = new QVBoxLayout(central);
	QTabWidget *tabs = new QTabWidget(central);

	create_antenna_tab(tabs);
	create_rf_calculators_tab(tabs);
	create_band_propagation_tab(tabs);

	root_layout->addWidget(tabs);
	setCentralWidget(central);
}

void
MainWindow::create_antenna_tab(QTabWidget *tabs)
{
	QWidget *central = new QWidget(tabs);
	QSplitter *splitter = new QSplitter(Qt::Horizontal, central);
	QVBoxLayout *root_layout = new QVBoxLayout(central);
	QGroupBox *input_group = new QGroupBox(QStringLiteral("Calculator"), splitter);
	QFormLayout *input_layout = new QFormLayout(input_group);

	band_box = new QComboBox(input_group);
	band_filter_box = new QComboBox(input_group);
	antenna_type_box = new QComboBox(input_group);
	design_mode_box = new QComboBox(input_group);
	length_unit_box = new QComboBox(input_group);
	project_title_box = new QLineEdit(input_group);
	frequency_box = new QDoubleSpinBox(input_group);
	length_box = new QDoubleSpinBox(input_group);
	velocity_factor_box = new QDoubleSpinBox(input_group);
	project_notes_edit = new QTextEdit(input_group);
	calculate_button = new QPushButton(QStringLiteral("Calculate"), input_group);

	band_filter_box->addItem(QStringLiteral("Amateur"), static_cast<int>(reference::BandReferenceFilter::Amateur));
	band_filter_box->addItem(QStringLiteral("Broadcast/Reference"), static_cast<int>(reference::BandReferenceFilter::BroadcastReference));
	band_filter_box->addItem(QStringLiteral("All"), static_cast<int>(reference::BandReferenceFilter::All));
	populate_band_selector();

	antenna_type_box->addItem(QStringLiteral("Half-wave dipole"), static_cast<int>(calculators::AntennaType::HalfWaveDipole));
	antenna_type_box->addItem(QStringLiteral("Quarter-wave vertical"), static_cast<int>(calculators::AntennaType::QuarterWaveVertical));
	antenna_type_box->addItem(QStringLiteral("End-fed half-wave"), static_cast<int>(calculators::AntennaType::EndFedHalfWave));
	antenna_type_box->addItem(QStringLiteral("Full-wave loop"), static_cast<int>(calculators::AntennaType::FullWaveLoop));
	antenna_type_box->addItem(QStringLiteral("Inverted Vee"), static_cast<int>(calculators::AntennaType::InvertedVee));
	antenna_type_box->addItem(QStringLiteral("Random wire"), static_cast<int>(calculators::AntennaType::RandomWire));
	antenna_type_box->addItem(QStringLiteral("Yagi"), static_cast<int>(calculators::AntennaType::Yagi));

	design_mode_box->addItem(QStringLiteral("Frequency to length"), static_cast<int>(calculators::DesignMode::FrequencyToLength));
	design_mode_box->addItem(QStringLiteral("Length to frequency"), static_cast<int>(calculators::DesignMode::LengthToFrequency));

	length_unit_box->addItem(QStringLiteral("millimetres"), static_cast<int>(calculators::LengthUnit::Millimetres));
	length_unit_box->addItem(QStringLiteral("centimetres"), static_cast<int>(calculators::LengthUnit::Centimetres));
	length_unit_box->addItem(QStringLiteral("metres"), static_cast<int>(calculators::LengthUnit::Metres));
	length_unit_box->addItem(QStringLiteral("feet / inches"), static_cast<int>(calculators::LengthUnit::FeetInches));

	frequency_box->setRange(0.001, 300000.0);
	frequency_box->setDecimals(6);
	frequency_box->setSuffix(QStringLiteral(" MHz"));
	frequency_box->setValue(7.1);

	configure_length_input();
	length_box->setValue(20.0);

	velocity_factor_box->setRange(calculators::MIN_WIRE_FACTOR, calculators::MAX_WIRE_FACTOR);
	velocity_factor_box->setDecimals(3);
	velocity_factor_box->setSingleStep(0.005);
	velocity_factor_box->setValue(calculators::DEFAULT_WIRE_FACTOR);
	project_title_box->setText(current_project.title);
	project_notes_edit->setMaximumHeight(80);
	yagi_group = new QGroupBox(QStringLiteral("Yagi design"), input_group);
	QFormLayout *yagi_layout = new QFormLayout(yagi_group);
	yagi_element_count_box = new QSpinBox(yagi_group);
	yagi_preset_box = new QComboBox(yagi_group);
	yagi_element_diameter_box = new QDoubleSpinBox(yagi_group);
	yagi_boom_correction_box = new QDoubleSpinBox(yagi_group);

	yagi_element_count_box->setRange(2, 10);
	yagi_element_count_box->setValue(3);
	yagi_preset_box->addItem(calculators::yagi_preset_label(calculators::YagiPreset::Conservative), static_cast<int>(calculators::YagiPreset::Conservative));
	yagi_preset_box->addItem(calculators::yagi_preset_label(calculators::YagiPreset::Compact), static_cast<int>(calculators::YagiPreset::Compact));
	yagi_preset_box->addItem(calculators::yagi_preset_label(calculators::YagiPreset::LongBoom), static_cast<int>(calculators::YagiPreset::LongBoom));
	yagi_element_diameter_box->setRange(0.0, 10000.0);
	yagi_element_diameter_box->setDecimals(3);
	yagi_element_diameter_box->setValue(0.010);
	yagi_boom_correction_box->setRange(0.0, 10000.0);
	yagi_boom_correction_box->setDecimals(3);
	yagi_boom_correction_box->setValue(0.0);
	yagi_layout->addRow(QStringLiteral("Elements"), yagi_element_count_box);
	yagi_layout->addRow(QStringLiteral("Preset"), yagi_preset_box);
	yagi_layout->addRow(QStringLiteral("Element diameter"), yagi_element_diameter_box);
	yagi_layout->addRow(QStringLiteral("Boom correction"), yagi_boom_correction_box);
	configure_length_input();

	input_layout->addRow(QStringLiteral("Project title"), project_title_box);
	input_layout->addRow(QStringLiteral("Band filter"), band_filter_box);
	input_layout->addRow(QStringLiteral("Band"), band_box);
	input_layout->addRow(QStringLiteral("Antenna type"), antenna_type_box);
	input_layout->addRow(QStringLiteral("Design mode"), design_mode_box);
	input_layout->addRow(QStringLiteral("Length unit"), length_unit_box);
	input_layout->addRow(QStringLiteral("Frequency"), frequency_box);
	input_layout->addRow(QStringLiteral("Wire / element length"), length_box);
	input_layout->addRow(QStringLiteral("Shortening factor"), velocity_factor_box);
	input_layout->addRow(yagi_group);
	input_layout->addRow(QStringLiteral("Project notes"), project_notes_edit);
	input_layout->addRow(calculate_button);

	QWidget *workspace = new QWidget(splitter);
	QVBoxLayout *workspace_layout = new QVBoxLayout(workspace);
	QGroupBox *targets_group = new QGroupBox(QStringLiteral("Target bands"), workspace);
	QHBoxLayout *target_button_layout = new QHBoxLayout();
	QPushButton *add_target_button = new QPushButton(QStringLiteral("Add current"), targets_group);
	QPushButton *remove_target_button = new QPushButton(QStringLiteral("Remove selected"), targets_group);
	QPushButton *recalculate_targets_button = new QPushButton(QStringLiteral("Recalculate all"), targets_group);
	QVBoxLayout *targets_layout = new QVBoxLayout(targets_group);
	design_scene = new design::AntennaDesignScene(workspace);
	design_scene->set_undo_stack(undo_stack);
	design_scene->set_item_moved_callback([this](const project::DiagramItemDescriptor &descriptor) {
		update_diagram_item_descriptor(descriptor);
	});
	design_view = new design::AntennaDesignView(design_scene, workspace);
	result_text = new QTextEdit(workspace);
	target_list = new QListWidget(targets_group);

	target_button_layout->addWidget(add_target_button);
	target_button_layout->addWidget(remove_target_button);
	target_button_layout->addWidget(recalculate_targets_button);
	targets_layout->addWidget(target_list);
	targets_layout->addLayout(target_button_layout);

	design_view->setMinimumHeight(320);
	result_text->setReadOnly(true);
	result_text->setFontFamily(QStringLiteral("monospace"));
	result_text->setMinimumHeight(160);

	workspace_layout->addWidget(design_view, 3);
	workspace_layout->addWidget(targets_group, 1);
	workspace_layout->addWidget(new QLabel(QStringLiteral("Results"), workspace));
	workspace_layout->addWidget(result_text, 1);

	splitter->addWidget(input_group);
	splitter->addWidget(workspace);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);

	root_layout->addWidget(splitter);
	tabs->addTab(central, QStringLiteral("Antenna Calculator"));

	connect(calculate_button, &QPushButton::clicked, this, &MainWindow::calculate);
	connect(add_target_button, &QPushButton::clicked, this, &MainWindow::add_current_target);
	connect(band_filter_box, &QComboBox::currentIndexChanged, this, [this]() {
		populate_band_selector();
		set_frequency_from_band(band_box->currentIndex());
	});
	connect(band_box, &QComboBox::currentIndexChanged, this, &MainWindow::set_frequency_from_band);
	connect(antenna_type_box, &QComboBox::currentIndexChanged, this, &MainWindow::save_antenna_type);
	connect(design_mode_box, &QComboBox::currentIndexChanged, this, &MainWindow::calculate);
	connect(length_unit_box, &QComboBox::currentIndexChanged, this, &MainWindow::change_length_unit);
	connect(frequency_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(length_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(project_notes_edit, &QTextEdit::textChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	connect(project_title_box, &QLineEdit::textChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	connect(recalculate_targets_button, &QPushButton::clicked, this, &MainWindow::recalculate_targets);
	connect(remove_target_button, &QPushButton::clicked, this, &MainWindow::remove_selected_target);
	connect(target_list, &QListWidget::itemChanged, this, &MainWindow::target_item_changed);
	connect(velocity_factor_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::save_shortening_factor);
	connect(yagi_boom_correction_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	connect(yagi_element_count_box, &QSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	connect(yagi_element_diameter_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	connect(yagi_preset_box, &QComboBox::currentIndexChanged, this, &MainWindow::mark_project_dirty_and_recalculate);
	update_yagi_controls();
}

void
MainWindow::create_rf_calculators_tab(QTabWidget *tabs)
{
	QWidget *rf_tab = new QWidget(tabs);
	QVBoxLayout *root_layout = new QVBoxLayout(rf_tab);
	QTabWidget *rf_tabs = new QTabWidget(rf_tab);

	QWidget *coil_tab = new QWidget(rf_tabs);
	QFormLayout *coil_layout = new QFormLayout(coil_tab);
	QPushButton *coil_button = new QPushButton(QStringLiteral("Calculate"), coil_tab);
	coil_diameter_box = create_positive_spin_box(coil_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 0.05);
	coil_length_box = create_positive_spin_box(coil_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 0.10);
	coil_turns_box = create_positive_spin_box(coil_tab, 10000.0, 2, QString(), 10.0);
	coil_result_text = new QTextEdit(coil_tab);
	coil_result_text->setReadOnly(true);
	coil_layout->addRow(QStringLiteral("Diameter"), coil_diameter_box);
	coil_layout->addRow(QStringLiteral("Coil length"), coil_length_box);
	coil_layout->addRow(QStringLiteral("Turns"), coil_turns_box);
	coil_layout->addRow(coil_button);
	coil_layout->addRow(coil_result_text);
	rf_tabs->addTab(coil_tab, QStringLiteral("Air-core coil"));

	QWidget *lc_tab = new QWidget(rf_tabs);
	QFormLayout *lc_layout = new QFormLayout(lc_tab);
	QPushButton *lc_button = new QPushButton(QStringLiteral("Calculate"), lc_tab);
	lc_inductance_box = create_positive_spin_box(lc_tab, 1000000.0, 6, QStringLiteral(" uH"), 1.0);
	lc_capacitance_box = create_positive_spin_box(lc_tab, 10000000.0, 3, QStringLiteral(" pF"), 100.0);
	lc_frequency_box = create_positive_spin_box(lc_tab, 300000.0, 6, QStringLiteral(" MHz"), 0.0);
	lc_result_text = new QTextEdit(lc_tab);
	lc_result_text->setReadOnly(true);
	lc_layout->addRow(QStringLiteral("Inductance"), lc_inductance_box);
	lc_layout->addRow(QStringLiteral("Capacitance"), lc_capacitance_box);
	lc_layout->addRow(QStringLiteral("Frequency for reverse"), lc_frequency_box);
	lc_layout->addRow(lc_button);
	lc_layout->addRow(lc_result_text);
	rf_tabs->addTab(lc_tab, QStringLiteral("LC resonance"));

	QWidget *swr_tab = new QWidget(rf_tabs);
	QFormLayout *swr_layout = new QFormLayout(swr_tab);
	QPushButton *swr_button = new QPushButton(QStringLiteral("Calculate"), swr_tab);
	swr_forward_power_box = create_positive_spin_box(swr_tab, 1000000.0, 3, QStringLiteral(" W"), 100.0);
	swr_value_box = create_positive_spin_box(swr_tab, 1000.0, 3, QString(), 2.0);
	swr_value_box->setMinimum(1.0);
	swr_result_text = new QTextEdit(swr_tab);
	swr_result_text->setReadOnly(true);
	swr_layout->addRow(QStringLiteral("Forward power"), swr_forward_power_box);
	swr_layout->addRow(QStringLiteral("SWR"), swr_value_box);
	swr_layout->addRow(swr_button);
	swr_layout->addRow(swr_result_text);
	rf_tabs->addTab(swr_tab, QStringLiteral("SWR / reflected power"));

	QWidget *horizon_tab = new QWidget(rf_tabs);
	QFormLayout *horizon_layout = new QFormLayout(horizon_tab);
	QPushButton *horizon_button = new QPushButton(QStringLiteral("Calculate"), horizon_tab);
	horizon_tx_height_box = create_positive_spin_box(horizon_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 10.0);
	horizon_rx_height_box = create_positive_spin_box(horizon_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 10.0);
	horizon_result_text = new QTextEdit(horizon_tab);
	horizon_result_text->setReadOnly(true);
	horizon_layout->addRow(QStringLiteral("TX antenna height"), horizon_tx_height_box);
	horizon_layout->addRow(QStringLiteral("RX antenna height"), horizon_rx_height_box);
	horizon_layout->addRow(horizon_button);
	horizon_layout->addRow(horizon_result_text);
	rf_tabs->addTab(horizon_tab, QStringLiteral("Radio horizon"));

	root_layout->addWidget(rf_tabs);
	tabs->addTab(rf_tab, QStringLiteral("RF Calculators"));

	connect(coil_button, &QPushButton::clicked, this, &MainWindow::calculate_coil);
	connect(lc_button, &QPushButton::clicked, this, &MainWindow::calculate_lc);
	connect(swr_button, &QPushButton::clicked, this, &MainWindow::calculate_swr);
	connect(horizon_button, &QPushButton::clicked, this, &MainWindow::calculate_horizon);

	calculate_coil();
	calculate_lc();
	calculate_swr();
	calculate_horizon();
}

void
MainWindow::create_band_propagation_tab(QTabWidget *tabs)
{
	QWidget *tab = new QWidget(tabs);
	QVBoxLayout *root_layout = new QVBoxLayout(tab);
	QHBoxLayout *top_layout = new QHBoxLayout();
	QFormLayout *input_layout = new QFormLayout();
	QPushButton *update_button = new QPushButton(QStringLiteral("Update guidance"), tab);

	propagation_band_box = new QComboBox(tab);
	propagation_band_filter_box = new QComboBox(tab);
	propagation_mode_box = new QComboBox(tab);
	propagation_environment_box = new QComboBox(tab);
	propagation_tx_height_box = create_positive_spin_box(tab, 10000000.0, 3, QString(), 10.0);
	propagation_rx_height_box = create_positive_spin_box(tab, 10000000.0, 3, QString(), 10.0);
	propagation_power_box = create_positive_spin_box(tab, 1000000.0, 3, QStringLiteral(" W"), 0.0);
	reference_text = new QTextEdit(tab);
	reach_text = new QTextEdit(tab);

	propagation_band_filter_box->addItem(QStringLiteral("Amateur"), static_cast<int>(reference::BandReferenceFilter::Amateur));
	propagation_band_filter_box->addItem(QStringLiteral("Broadcast/Reference"), static_cast<int>(reference::BandReferenceFilter::BroadcastReference));
	propagation_band_filter_box->addItem(QStringLiteral("All"), static_cast<int>(reference::BandReferenceFilter::All));
	populate_propagation_band_selector();
	for (const reference::ModeReference &mode : reference::mode_references())
		propagation_mode_box->addItem(mode.name, mode.key);

	propagation_environment_box->addItem(reference::environment_profile_label(reference::EnvironmentProfile::Indoor), reference::environment_profile_key(reference::EnvironmentProfile::Indoor));
	propagation_environment_box->addItem(reference::environment_profile_label(reference::EnvironmentProfile::Urban), reference::environment_profile_key(reference::EnvironmentProfile::Urban));
	propagation_environment_box->addItem(reference::environment_profile_label(reference::EnvironmentProfile::Suburban), reference::environment_profile_key(reference::EnvironmentProfile::Suburban));
	propagation_environment_box->addItem(reference::environment_profile_label(reference::EnvironmentProfile::Rural), reference::environment_profile_key(reference::EnvironmentProfile::Rural));
	propagation_environment_box->addItem(reference::environment_profile_label(reference::EnvironmentProfile::HilltopOpen), reference::environment_profile_key(reference::EnvironmentProfile::HilltopOpen));

	reference_text->setReadOnly(true);
	reach_text->setReadOnly(true);
	update_reference_height_inputs();

	input_layout->addRow(QStringLiteral("Band filter"), propagation_band_filter_box);
	input_layout->addRow(QStringLiteral("Band"), propagation_band_box);
	input_layout->addRow(QStringLiteral("Mode"), propagation_mode_box);
	input_layout->addRow(QStringLiteral("Environment"), propagation_environment_box);
	input_layout->addRow(QStringLiteral("TX antenna height"), propagation_tx_height_box);
	input_layout->addRow(QStringLiteral("RX antenna height"), propagation_rx_height_box);
	input_layout->addRow(QStringLiteral("Power"), propagation_power_box);
	input_layout->addRow(update_button);

	top_layout->addLayout(input_layout, 0);
	top_layout->addWidget(reference_text, 1);

	root_layout->addLayout(top_layout, 1);
	root_layout->addWidget(new QLabel(QStringLiteral("Reach guidance"), tab));
	root_layout->addWidget(reach_text, 1);
	tabs->addTab(tab, QStringLiteral("Band & Propagation"));

	connect(propagation_band_filter_box, &QComboBox::currentIndexChanged, this, &MainWindow::populate_propagation_band_selector);
	connect(propagation_band_box, &QComboBox::currentIndexChanged, this, &MainWindow::update_reference_panel);
	connect(propagation_mode_box, &QComboBox::currentIndexChanged, this, &MainWindow::update_reference_panel);
	connect(propagation_environment_box, &QComboBox::currentIndexChanged, this, &MainWindow::update_reference_panel);
	connect(propagation_power_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::update_reference_panel);
	connect(propagation_rx_height_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::update_reference_panel);
	connect(propagation_tx_height_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::update_reference_panel);
	connect(update_button, &QPushButton::clicked, this, &MainWindow::update_reference_panel);

	update_reference_panel();
}

void
MainWindow::populate_band_selector()
{
	const QSignalBlocker blocker(band_box);
	const double previous_frequency_mhz = band_box->currentData().toDouble();
	int selected_index = -1;

	band_box->clear();
	band_box->addItem(QStringLiteral("Custom"), 0.0);

	for (const reference::BandReference &band : reference::band_references(band_reference_filter_from_combo(band_filter_box))) {
		const QString label = QStringLiteral("%1 (%2, %3-%4 MHz)")
			.arg(band.name)
			.arg(reference::band_service_label(band.service))
			.arg(band.lower_frequency_mhz, 0, 'f', 3)
			.arg(band.upper_frequency_mhz, 0, 'f', 3);
		band_box->addItem(label, band.design_frequency_mhz);
		band_box->setItemData(band_box->count() - 1, band.name, Qt::UserRole + 1);
		band_box->setItemData(band_box->count() - 1, reference::band_service_key(band.service), Qt::UserRole + 2);
		if (qFuzzyCompare(previous_frequency_mhz + 1.0, band.design_frequency_mhz + 1.0))
			selected_index = band_box->count() - 1;
	}

	band_box->setCurrentIndex(selected_index >= 0 ? selected_index : qMin(4, band_box->count() - 1));
}

void
MainWindow::populate_propagation_band_selector()
{
	if (propagation_band_box == nullptr)
		return;

	const QSignalBlocker blocker(propagation_band_box);
	const double previous_frequency_mhz = propagation_band_box->currentData().toDouble();
	int selected_index = -1;

	propagation_band_box->clear();
	for (const reference::BandReference &band : reference::band_references(band_reference_filter_from_combo(propagation_band_filter_box))) {
		propagation_band_box->addItem(band.name, band.design_frequency_mhz);
		propagation_band_box->setItemData(propagation_band_box->count() - 1, reference::band_service_key(band.service), Qt::UserRole + 1);
		if (qFuzzyCompare(previous_frequency_mhz + 1.0, band.design_frequency_mhz + 1.0))
			selected_index = propagation_band_box->count() - 1;
	}

	if (propagation_band_box->count() > 0)
		propagation_band_box->setCurrentIndex(selected_index >= 0 ? selected_index : 0);
	update_reference_panel();
}

void
MainWindow::set_frequency_from_band(int index)
{
	const double frequency_mhz = band_box->itemData(index).toDouble();
	const QString band_name = band_box->itemData(index, Qt::UserRole + 1).toString();

	if (frequency_mhz > 0.0)
		frequency_box->setValue(frequency_mhz);
	if (frequency_mhz > 0.0 && propagation_band_box != nullptr) {
		reference::BandReference reference;
		if (reference::band_reference_by_name(band_name, reference) || reference::band_reference_by_frequency(frequency_mhz, reference)) {
			if (propagation_band_filter_box != nullptr) {
				const reference::BandReferenceFilter filter = reference.service == reference::BandService::Amateur
					? reference::BandReferenceFilter::Amateur
					: reference::BandReferenceFilter::BroadcastReference;
				const int filter_index = propagation_band_filter_box->findData(static_cast<int>(filter));
				if (filter_index >= 0 && propagation_band_filter_box->currentIndex() != filter_index) {
					const QSignalBlocker blocker(propagation_band_filter_box);
					propagation_band_filter_box->setCurrentIndex(filter_index);
					populate_propagation_band_selector();
				}
			}
			const int reference_index = propagation_band_box->findText(reference.name);
			if (reference_index >= 0)
				propagation_band_box->setCurrentIndex(reference_index);
		}
	}

	app_settings.set_band_index(index);
	mark_project_dirty();
	calculate();
	update_reference_panel();
}

void
MainWindow::add_current_target()
{
	project::AntennaTarget target;

	target.band_name = band_box->itemData(band_box->currentIndex(), Qt::UserRole + 1).toString();
	if (target.band_name.isEmpty())
		target.band_name = band_box->currentText();
	target.band_service = reference::band_service_from_key(band_box->itemData(band_box->currentIndex(), Qt::UserRole + 2).toString());
	target.enabled = true;
	target.frequency_mhz = frequency_box->value();
	current_project.targets.append(target);

	recalculate_targets();
	mark_project_dirty();
}

void
MainWindow::apply_project_to_ui()
{
	const int antenna_index = antenna_type_box->findData(static_cast<int>(current_project.antenna_type));
	const int unit_index = length_unit_box->findData(static_cast<int>(current_project.preferred_length_unit));
	const int preset_index = yagi_preset_box->findData(static_cast<int>(current_project.yagi_design.preset));

	if (antenna_index >= 0) {
		const QSignalBlocker blocker(antenna_type_box);
		antenna_type_box->setCurrentIndex(antenna_index);
	}
	if (unit_index >= 0) {
		const QSignalBlocker blocker(length_unit_box);
		length_unit_box->setCurrentIndex(unit_index);
	}

	{
		const QSignalBlocker blocker(project_title_box);
		project_title_box->setText(current_project.title);
	}
	{
		const QSignalBlocker blocker(project_notes_edit);
		project_notes_edit->setPlainText(current_project.notes);
	}
	{
		const QSignalBlocker blocker(velocity_factor_box);
		velocity_factor_box->setValue(current_project.yagi_design.enabled
			? current_project.yagi_design.element_shortening_factor
			: current_project.velocity_factor);
	}
	if (current_project.yagi_design.enabled) {
		{
			const QSignalBlocker blocker(yagi_element_count_box);
			yagi_element_count_box->setValue(current_project.yagi_design.element_count);
		}
		if (preset_index >= 0) {
			const QSignalBlocker blocker(yagi_preset_box);
			yagi_preset_box->setCurrentIndex(preset_index);
		}
		{
			const QSignalBlocker blocker(yagi_element_diameter_box);
			yagi_element_diameter_box->setValue(calculators::metres_to_length_unit(current_project.yagi_design.element_diameter_metres, current_project.preferred_length_unit));
		}
		{
			const QSignalBlocker blocker(yagi_boom_correction_box);
			yagi_boom_correction_box->setValue(calculators::metres_to_length_unit(current_project.yagi_design.boom_correction_metres, current_project.preferred_length_unit));
		}
	}
	if (current_project.propagation_settings.enabled && propagation_mode_box != nullptr) {
		const int mode_index = propagation_mode_box->findData(reference::mode_type_key(current_project.propagation_settings.mode));
		const int environment_index = propagation_environment_box->findData(reference::environment_profile_key(current_project.propagation_settings.environment));
		if (mode_index >= 0) {
			const QSignalBlocker blocker(propagation_mode_box);
			propagation_mode_box->setCurrentIndex(mode_index);
		}
		if (environment_index >= 0) {
			const QSignalBlocker blocker(propagation_environment_box);
			propagation_environment_box->setCurrentIndex(environment_index);
		}
		{
			const QSignalBlocker blocker(propagation_tx_height_box);
			propagation_tx_height_box->setValue(calculators::metres_to_length_unit(current_project.propagation_settings.tx_height_metres, current_project.preferred_length_unit));
		}
		{
			const QSignalBlocker blocker(propagation_rx_height_box);
			propagation_rx_height_box->setValue(calculators::metres_to_length_unit(current_project.propagation_settings.rx_height_metres, current_project.preferred_length_unit));
		}
		{
			const QSignalBlocker blocker(propagation_power_box);
			propagation_power_box->setValue(current_project.propagation_settings.has_power_watts ? current_project.propagation_settings.power_watts : 0.0);
		}
	}
	current_length_unit = current_project.preferred_length_unit;
	configure_length_input();
	update_yagi_controls();
	{
		const double frequency_mhz = project_restore_frequency_mhz();
		if (frequency_mhz > 0.0) {
			const QSignalBlocker blocker(frequency_box);
			frequency_box->setValue(frequency_mhz);
		}
	}
	update_target_list();
	update_reference_panel();
	update_project_title();
	calculate();
}

void
MainWindow::build_project_from_ui()
{
	current_project.antenna_type = static_cast<calculators::AntennaType>(antenna_type_box->currentData().toInt());
	current_project.notes = project_notes_edit->toPlainText();
	current_project.preferred_length_unit = current_length_unit;
	current_project.title = project_title_box->text().isEmpty() ? QStringLiteral("Untitled Project") : project_title_box->text();
	current_project.updated_utc = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	current_project.velocity_factor = velocity_factor_box->value();
	current_project.yagi_design.enabled = current_project.antenna_type == calculators::AntennaType::Yagi;
	current_project.yagi_design.element_count = yagi_element_count_box->value();
	current_project.yagi_design.preset = static_cast<calculators::YagiPreset>(yagi_preset_box->currentData().toInt());
	current_project.yagi_design.frequency_mhz = current_project.yagi_design.enabled ? frequency_box->value() : 0.0;
	current_project.yagi_design.element_shortening_factor = velocity_factor_box->value();
	current_project.yagi_design.element_diameter_metres = calculators::length_unit_to_metres(yagi_element_diameter_box->value(), current_length_unit);
	current_project.yagi_design.boom_correction_metres = calculators::length_unit_to_metres(yagi_boom_correction_box->value(), current_length_unit);
	if (propagation_mode_box != nullptr) {
		current_project.propagation_settings.enabled = true;
		current_project.propagation_settings.mode = reference::mode_type_from_key(propagation_mode_box->currentData().toString());
		current_project.propagation_settings.environment = reference::environment_profile_from_key(propagation_environment_box->currentData().toString());
		current_project.propagation_settings.tx_height_metres = calculators::length_unit_to_metres(propagation_tx_height_box->value(), current_length_unit);
		current_project.propagation_settings.rx_height_metres = calculators::length_unit_to_metres(propagation_rx_height_box->value(), current_length_unit);
		current_project.propagation_settings.power_watts = propagation_power_box->value();
		current_project.propagation_settings.has_power_watts = propagation_power_box->value() > 0.0;
		current_project.propagation_settings.include_in_guides = true;
	}
}

bool
MainWindow::confirm_discard_changes()
{
	if (!project_dirty)
		return true;

	const QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		QStringLiteral("Unsaved changes"),
		QStringLiteral("Discard unsaved project changes?"),
		QMessageBox::Discard | QMessageBox::Cancel,
		QMessageBox::Cancel
	);

	return answer == QMessageBox::Discard;
}

void
MainWindow::mark_project_dirty()
{
	project_dirty = true;
	update_project_title();
}

void
MainWindow::mark_project_dirty_and_recalculate()
{
	calculate();
	mark_project_dirty();
}

void
MainWindow::new_project()
{
	if (!confirm_discard_changes())
		return;

	current_project = project::default_project();
	current_project_path.clear();
	project_dirty = false;
	apply_project_to_ui();
	statusBar()->showMessage(QStringLiteral("New project"));
}

void
MainWindow::open_project()
{
	if (!confirm_discard_changes())
		return;

	const QString path = QFileDialog::getOpenFileName(
		this,
		QStringLiteral("Open qantcal Project"),
		QString(),
		QStringLiteral("qantcal project (*.qantcal.json)")
	);
	QString error;
	project::AntennaProject loaded;

	if (path.isEmpty())
		return;

	if (!project::load_project(path, loaded, error)) {
		statusBar()->showMessage(QStringLiteral("Open failed: %1").arg(error));
		return;
	}

	current_project = loaded;
	current_project_path = path;
	project_dirty = false;
	apply_project_to_ui();
	statusBar()->showMessage(QStringLiteral("Opened project: %1").arg(path));
}

void
MainWindow::recalculate_targets()
{
	current_project.elements.clear();
	current_project.diagram_items.clear();
	build_project_from_ui();

	for (const project::AntennaTarget &target : current_project.targets) {
		if (!target.enabled)
			continue;

		if (current_project.antenna_type == calculators::AntennaType::Yagi) {
			calculators::YagiDesignInput input = current_yagi_input();
			input.frequency_mhz = target.frequency_mhz;
			const calculators::YagiDesignResult result = calculators::calculate_yagi(input);
			if (!result.ok)
				continue;
			for (const calculators::YagiElement &yagi_element : result.elements) {
				project::AntennaElement element;
				element.frequency_mhz = target.frequency_mhz;
				element.label = QStringLiteral("%1 %2").arg(target.band_name).arg(yagi_element.label);
				element.length_metres = yagi_element.length_metres;
				element.notes = yagi_element.notes;
				element.role = calculators::yagi_element_role_label(yagi_element.role).toLower();
				current_project.elements.append(element);

				project::DiagramItemDescriptor item;
				item.id = QStringLiteral("target-%1-%2").arg(target.frequency_mhz, 0, 'f', 3).arg(yagi_element.label);
				item.kind = QStringLiteral("yagi_element");
				item.label = element.label;
				item.length_metres = element.length_metres;
				item.position = QPointF(yagi_element.position_from_reflector_metres * 120.0, 0.0);
				item.points.append(QPointF(0.0, -80.0));
				item.points.append(QPointF(0.0, 80.0));
				current_project.diagram_items.append(item);
			}
			continue;
		}

		calculators::AntennaCalculationInput input;
		input.antenna_type = current_project.antenna_type;
		input.design_mode = calculators::DesignMode::FrequencyToLength;
		input.frequency_mhz = target.frequency_mhz;
		input.shortening_factor = current_project.velocity_factor;

		const calculators::AntennaCalculationResult result = calculators::calculate_antenna(input);
		if (!result.ok)
			continue;

		project::AntennaElement element;
		element.frequency_mhz = target.frequency_mhz;
		element.label = target.band_name;
		element.length_metres = result.total_length_m > 0.0 ? result.total_length_m : result.radiator_length_m;
		element.notes = QStringLiteral("Independent target calculation. Multi-band physical interaction is future work.");
		element.role = QStringLiteral("calculated_element");
		current_project.elements.append(element);

		project::DiagramItemDescriptor item;
		item.id = QStringLiteral("target-%1").arg(target.frequency_mhz, 0, 'f', 3);
		item.kind = QStringLiteral("line");
		item.label = element.label;
		item.length_metres = element.length_metres;
		item.points.append(QPointF(-220.0, 0.0));
		item.points.append(QPointF(220.0, 0.0));
		current_project.diagram_items.append(item);
	}

	update_target_list();
	design_scene->show_project_diagram(current_project, current_length_unit);
	result_text->append(QStringLiteral("\nMulti-band note: each target is calculated independently. Fan dipole spacing, traps, common feedpoint impedance, interaction, and NEC modelling are future work."));
	mark_project_dirty();
}

void
MainWindow::remove_selected_target()
{
	const int row = target_list->currentRow();

	if (row < 0 || row >= current_project.targets.size())
		return;

	current_project.targets.removeAt(row);
	recalculate_targets();
	mark_project_dirty();
}

void
MainWindow::export_pdf()
{
	if (!latest_result.ok && !latest_yagi_result.ok && current_project.elements.isEmpty()) {
		statusBar()->showMessage(QStringLiteral("Export failed: calculate a valid antenna first"));
		return;
	}

	const QString path = QFileDialog::getSaveFileName(
		this,
		QStringLiteral("Export PDF"),
		QStringLiteral("qantcal-guide.pdf"),
		QStringLiteral("PDF files (*.pdf)")
	);

	if (path.isEmpty())
		return;

	build_project_from_ui();
	const QString selected_band_name = band_box->itemData(band_box->currentIndex(), Qt::UserRole + 1).toString();
	const guides::GuideDocument document = current_project.elements.isEmpty()
		? guides::create_guide_document(latest_result, current_length_unit, selected_band_name.isEmpty() ? band_box->currentText() : selected_band_name)
		: guides::create_project_guide_document(current_project, current_length_unit);
	const guides::GuideRenderer renderer;

	if (renderer.render_to_pdf(document, path, guides::default_export_options())) {
		statusBar()->showMessage(QStringLiteral("Exported PDF: %1").arg(path));
		return;
	}

	statusBar()->showMessage(QStringLiteral("Export failed: could not write PDF"));
}

void
MainWindow::calculate_coil()
{
	const calculators::CoilCalculationInput input = {
		calculators::length_unit_to_metres(coil_diameter_box->value(), current_length_unit),
		calculators::length_unit_to_metres(coil_length_box->value(), current_length_unit),
		coil_turns_box->value()
	};
	const calculators::CoilCalculationResult result = calculators::calculate_air_core_solenoid(input);

	if (!result.ok) {
		coil_result_text->setPlainText(QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error)));
		return;
	}

	coil_result_text->setPlainText(
		QStringLiteral("Inductance: %1 uH\n\n%2")
			.arg(result.inductance_uh, 0, 'f', 3)
			.arg(QString::fromStdString(result.note))
	);
}

void
MainWindow::calculate_horizon()
{
	const calculators::RadioHorizonInput input = {
		calculators::length_unit_to_metres(horizon_rx_height_box->value(), current_length_unit),
		calculators::length_unit_to_metres(horizon_tx_height_box->value(), current_length_unit)
	};
	const calculators::RadioHorizonResult result = calculators::calculate_radio_horizon(input);

	if (!result.ok) {
		horizon_result_text->setPlainText(QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error)));
		return;
	}

	horizon_result_text->setPlainText(
		QStringLiteral("TX horizon: %1 km\nRX horizon: %2 km\nCombined line-of-sight estimate: %3 km\n\n%4")
			.arg(result.tx_horizon_km, 0, 'f', 2)
			.arg(result.rx_horizon_km, 0, 'f', 2)
			.arg(result.combined_distance_km, 0, 'f', 2)
			.arg(QString::fromStdString(result.note))
	);
}

void
MainWindow::calculate_yagi()
{
	const calculators::YagiDesignInput input = current_yagi_input();
	const calculators::YagiDesignResult result = calculators::calculate_yagi(input);

	latest_result = calculators::AntennaCalculationResult();
	latest_yagi_result = result;
	result_text->setPlainText(yagi_result_to_text(result, current_length_unit));
	if (result.ok) {
		reference::BandReference band;
		const QString band_name = band_box->itemData(band_box->currentIndex(), Qt::UserRole + 1).toString();
		if (reference::band_reference_by_name(band_name, band) && band.service != reference::BandService::Amateur)
			result_text->append(QStringLiteral("\nService warning: %1\nHF Yagis on these bands are physically large and construction-sensitive starting designs only.").arg(band.warning));
	}
	build_project_from_ui();
	current_project.elements.clear();
	current_project.diagram_items.clear();

	if (result.ok) {
		for (const calculators::YagiElement &yagi_element : result.elements) {
			project::AntennaElement element;
			element.frequency_mhz = result.frequency_mhz;
			element.label = yagi_element.label;
			element.length_metres = yagi_element.length_metres;
			element.notes = yagi_element.notes;
			element.role = calculators::yagi_element_role_label(yagi_element.role).toLower();
			current_project.elements.append(element);

			project::DiagramItemDescriptor item;
			item.id = QStringLiteral("yagi-%1").arg(yagi_element.label);
			item.kind = QStringLiteral("yagi_element");
			item.label = yagi_element.label;
			item.length_metres = yagi_element.length_metres;
			item.position = QPointF(yagi_element.position_from_reflector_metres * 120.0, 0.0);
			item.points.append(QPointF(0.0, -80.0));
			item.points.append(QPointF(0.0, 80.0));
			current_project.diagram_items.append(item);
		}
		design_scene->show_yagi_diagram(result, current_length_unit);
		statusBar()->showMessage(QStringLiteral("Yagi calculation updated"));
		return;
	}

	design_scene->show_project_diagram(current_project, current_length_unit);
	statusBar()->showMessage(QStringLiteral("Input error: %1").arg(result.error_message));
}

void
MainWindow::calculate_lc()
{
	calculators::LcResonanceInput input;

	input.inductance_uh = lc_inductance_box->value();
	input.capacitance_pf = lc_capacitance_box->value();
	input.frequency_mhz = lc_frequency_box->value();

	if (input.frequency_mhz > 0.0 && input.capacitance_pf > 0.0 && input.inductance_uh <= 0.0)
		input.mode = calculators::LcCalculationMode::InductanceFromFrequencyCapacitance;
	else if (input.frequency_mhz > 0.0 && input.inductance_uh > 0.0 && input.capacitance_pf <= 0.0)
		input.mode = calculators::LcCalculationMode::CapacitanceFromFrequencyInductance;
	else
		input.mode = calculators::LcCalculationMode::FrequencyFromLC;

	const calculators::LcResonanceResult result = calculators::calculate_lc_resonance(input);

	if (!result.ok) {
		lc_result_text->setPlainText(QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error)));
		return;
	}

	lc_result_text->setPlainText(
		QStringLiteral("Frequency: %1 Hz\nFrequency: %2 kHz\nFrequency: %3 MHz\nInductance: %4 uH\nCapacitance: %5 pF")
			.arg(result.frequency_hz, 0, 'f', 1)
			.arg(result.frequency_khz, 0, 'f', 3)
			.arg(result.frequency_mhz, 0, 'f', 6)
			.arg(result.inductance_uh, 0, 'f', 6)
			.arg(result.capacitance_pf, 0, 'f', 3)
	);
}

void
MainWindow::calculate_swr()
{
	const calculators::SwrCalculationInput input = {
		swr_forward_power_box->value(),
		swr_value_box->value()
	};
	const calculators::SwrCalculationResult result = calculators::calculate_swr(input);

	if (!result.ok) {
		swr_result_text->setPlainText(QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error)));
		return;
	}

	swr_result_text->setPlainText(
		QStringLiteral("Reflection coefficient: %1\nReflected power: %2 W\nReflected power: %3%\nDelivered estimate: %4 W\n\n%5")
			.arg(result.reflection_coefficient, 0, 'f', 3)
			.arg(result.reflected_power_w, 0, 'f', 3)
			.arg(result.reflected_percent, 0, 'f', 2)
			.arg(result.delivered_power_w, 0, 'f', 3)
			.arg(QString::fromStdString(result.note))
	);
}

void
MainWindow::restore_settings()
{
	current_length_unit = app_settings.length_unit();

	const int unit_index = length_unit_box->findData(static_cast<int>(current_length_unit));
	if (unit_index >= 0) {
		const QSignalBlocker blocker(length_unit_box);
		length_unit_box->setCurrentIndex(unit_index);
	}

	configure_length_input();
	{
		const QSignalBlocker blocker(yagi_element_diameter_box);
		yagi_element_diameter_box->setValue(calculators::metres_to_length_unit(0.010, current_length_unit));
	}

	const int antenna_index = antenna_type_box->findData(static_cast<int>(app_settings.antenna_type()));
	if (antenna_index >= 0) {
		const QSignalBlocker blocker(antenna_type_box);
		antenna_type_box->setCurrentIndex(antenna_index);
	}
	update_yagi_controls();

	const int band_index = app_settings.band_index();
	if (band_index >= 0 && band_index < band_box->count()) {
		const QSignalBlocker blocker(band_box);
		band_box->setCurrentIndex(band_index);
		const double frequency_mhz = band_box->itemData(band_index).toDouble();
		if (frequency_mhz > 0.0)
			frequency_box->setValue(frequency_mhz);
	}

	const double shortening_factor = app_settings.shortening_factor();
	if (shortening_factor >= calculators::MIN_WIRE_FACTOR && shortening_factor <= calculators::MAX_WIRE_FACTOR)
		velocity_factor_box->setValue(shortening_factor);
}

void
MainWindow::save_antenna_type()
{
	app_settings.set_antenna_type(static_cast<calculators::AntennaType>(antenna_type_box->currentData().toInt()));
	update_yagi_controls();
	mark_project_dirty();
	calculate();
}

bool
MainWindow::save_project()
{
	QString error;

	if (current_project_path.isEmpty())
		return save_project_as();

	build_project_from_ui();
	if (!project::save_project(current_project, current_project_path, error)) {
		statusBar()->showMessage(QStringLiteral("Save failed: %1").arg(error));
		return false;
	}

	project_dirty = false;
	update_project_title();
	statusBar()->showMessage(QStringLiteral("Saved project: %1").arg(current_project_path));

	return true;
}

bool
MainWindow::save_project_as()
{
	const QString path = QFileDialog::getSaveFileName(
		this,
		QStringLiteral("Save qantcal Project"),
		current_project_path.isEmpty() ? QStringLiteral("untitled.qantcal.json") : current_project_path,
		QStringLiteral("qantcal project (*.qantcal.json)")
	);

	if (path.isEmpty())
		return false;

	current_project_path = path.endsWith(QStringLiteral(".qantcal.json"))
		? path
		: path + QStringLiteral(".qantcal.json");
	return save_project();
}

void
MainWindow::save_shortening_factor()
{
	app_settings.set_shortening_factor(velocity_factor_box->value());
	mark_project_dirty();
	calculate();
}

void
MainWindow::show_about()
{
	QMessageBox::about(
		this,
		QStringLiteral("About qantcal"),
		QStringLiteral("qantcal\n\nEarly Qt6 scaffold for amateur radio antenna calculation, design diagrams, and printable guides.")
	);
}

void
MainWindow::print_guide()
{
	if (!latest_result.ok && !latest_yagi_result.ok && current_project.elements.isEmpty()) {
		statusBar()->showMessage(QStringLiteral("Print failed: calculate a valid antenna first"));
		return;
	}

	build_project_from_ui();
	const QString selected_band_name = band_box->itemData(band_box->currentIndex(), Qt::UserRole + 1).toString();
	const guides::GuideDocument document = current_project.elements.isEmpty()
		? guides::create_guide_document(latest_result, current_length_unit, selected_band_name.isEmpty() ? band_box->currentText() : selected_band_name)
		: guides::create_project_guide_document(current_project, current_length_unit);
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog dialog(&printer, this);

	if (dialog.exec() != QDialog::Accepted)
		return;

	const guides::GuideRenderer renderer;
	if (renderer.render_to_printer(document, printer, guides::default_export_options())) {
		statusBar()->showMessage(QStringLiteral("Guide sent to printer"));
		return;
	}

	statusBar()->showMessage(QStringLiteral("Print failed"));
}

calculators::AntennaCalculationInput
MainWindow::current_input() const
{
	calculators::AntennaCalculationInput input;

	input.antenna_type = static_cast<calculators::AntennaType>(antenna_type_box->currentData().toInt());
	input.design_mode = static_cast<calculators::DesignMode>(design_mode_box->currentData().toInt());
	input.frequency_mhz = frequency_box->value();
	input.length_m = calculators::length_unit_to_metres(length_box->value(), current_length_unit);
	input.shortening_factor = velocity_factor_box->value();

	return input;
}

calculators::YagiDesignInput
MainWindow::current_yagi_input() const
{
	calculators::YagiDesignInput input;

	input.frequency_mhz = frequency_box->value();
	input.element_count = yagi_element_count_box->value();
	input.element_shortening_factor = velocity_factor_box->value();
	input.element_diameter_metres = calculators::length_unit_to_metres(yagi_element_diameter_box->value(), current_length_unit);
	input.boom_correction_metres = calculators::length_unit_to_metres(yagi_boom_correction_box->value(), current_length_unit);
	input.preset = static_cast<calculators::YagiPreset>(yagi_preset_box->currentData().toInt());
	input.preferred_length_unit = current_length_unit;

	return input;
}

double
MainWindow::project_restore_frequency_mhz() const
{
	if (current_project.yagi_design.enabled && current_project.yagi_design.frequency_mhz > 0.0)
		return current_project.yagi_design.frequency_mhz;

	for (const project::AntennaTarget &target : current_project.targets) {
		if (target.enabled && target.frequency_mhz > 0.0)
			return target.frequency_mhz;
	}

	for (const project::AntennaElement &element : current_project.elements) {
		if (element.frequency_mhz > 0.0)
			return element.frequency_mhz;
	}

	return 0.0;
}

calculators::LengthUnit
MainWindow::selected_length_unit() const
{
	return current_length_unit;
}

void
MainWindow::target_item_changed(QListWidgetItem *item)
{
	const int row = target_list->row(item);

	if (row < 0 || row >= current_project.targets.size())
		return;

	current_project.targets[row].enabled = item->checkState() == Qt::Checked;
	recalculate_targets();
	mark_project_dirty();
}

void
MainWindow::update_diagram_item_descriptor(const project::DiagramItemDescriptor &descriptor)
{
	for (project::DiagramItemDescriptor &item : current_project.diagram_items) {
		if (item.id == descriptor.id) {
			item = descriptor;
			mark_project_dirty();
			statusBar()->showMessage(QStringLiteral("Moved design item"));
			return;
		}
	}

	current_project.diagram_items.append(descriptor);
	mark_project_dirty();
	statusBar()->showMessage(QStringLiteral("Moved design item"));
}

void
MainWindow::update_project_title()
{
	QString title = current_project.title;

	if (!current_project_path.isEmpty())
		title = QFileInfo(current_project_path).fileName();
	if (title.isEmpty())
		title = QStringLiteral("Untitled Project");
	if (project_dirty)
		title += QStringLiteral(" *");

	setWindowTitle(QStringLiteral("qantcal - %1").arg(title));
}

void
MainWindow::update_reference_height_inputs()
{
	if (propagation_tx_height_box == nullptr || propagation_rx_height_box == nullptr)
		return;

	propagation_tx_height_box->setSuffix(length_box != nullptr ? length_box->suffix() : QStringLiteral(" m"));
	propagation_rx_height_box->setSuffix(length_box != nullptr ? length_box->suffix() : QStringLiteral(" m"));
	propagation_tx_height_box->setDecimals(length_box != nullptr ? length_box->decimals() : 3);
	propagation_rx_height_box->setDecimals(length_box != nullptr ? length_box->decimals() : 3);
	propagation_tx_height_box->setSingleStep(length_box != nullptr ? length_box->singleStep() : 0.1);
	propagation_rx_height_box->setSingleStep(length_box != nullptr ? length_box->singleStep() : 0.1);
}

void
MainWindow::update_reference_panel()
{
	if (propagation_band_box == nullptr || reference_text == nullptr || reach_text == nullptr)
		return;

	reference::BandReference band;
	reference::PropagationProfile profile;
	reference::ModeReference mode;
	const QString band_name = propagation_band_box->currentText();
	const double frequency_mhz = propagation_band_box->currentData().toDouble();

	QString reference_output;
	if (reference::band_reference_by_name(band_name, band)) {
		reference_output += QStringLiteral("Band: %1\nService: %2\nFrequency: %3-%4 MHz\nDesign frequency: %5 MHz\nCategory: %6\nWavelength label: %7\n\n")
			.arg(band.name)
			.arg(reference::band_service_label(band.service))
			.arg(band.lower_frequency_mhz, 0, 'f', 3)
			.arg(band.upper_frequency_mhz, 0, 'f', 3)
			.arg(band.design_frequency_mhz, 0, 'f', 3)
			.arg(band.category)
			.arg(band.wavelength_label);
		reference_output += QStringLiteral("Mode notes: %1\n\nAntenna notes: %2\n\nPropagation notes: %3\n\nUse-case notes: %4\n\nWarning: %5\n")
			.arg(band.mode_notes)
			.arg(band.antenna_notes)
			.arg(band.propagation_notes)
			.arg(band.use_case_notes)
			.arg(band.warning);
	}
	if (reference::propagation_profile_by_band_name(band_name, profile)) {
		reference_output += QStringLiteral("\nPropagation categories: %1\nDay/night tendency: %2\nCharacter: %3\nNoise/interference: %4\nAntenna practicality: %5\nVariability: %6\n")
			.arg(profile.categories.join(QStringLiteral(", ")))
			.arg(profile.day_night_tendency)
			.arg(profile.character)
			.arg(profile.noise_tendency)
			.arg(profile.antenna_practicality)
			.arg(profile.variability);
	}
	if (reference::mode_reference_by_key(propagation_mode_box->currentData().toString(), mode)) {
		reference_output += QStringLiteral("\nMode: %1\nBandwidth category: %2\nUse notes: %3\nWeak-signal suitability: %4\nBeginner notes: %5\nCaution: %6\n")
			.arg(mode.name)
			.arg(mode.bandwidth_category)
			.arg(mode.use_notes)
			.arg(mode.weak_signal_notes)
			.arg(mode.beginner_notes)
			.arg(mode.caution);
	}
	reference_text->setPlainText(reference_output);

	reference::ReachEstimateInput input;
	input.frequency_mhz = frequency_mhz;
	input.band_name = band_name;
	input.mode = reference::mode_type_from_key(propagation_mode_box->currentData().toString());
	input.environment = reference::environment_profile_from_key(propagation_environment_box->currentData().toString());
	input.tx_height_metres = calculators::length_unit_to_metres(propagation_tx_height_box->value(), current_length_unit);
	input.rx_height_metres = calculators::length_unit_to_metres(propagation_rx_height_box->value(), current_length_unit);
	input.power_watts = propagation_power_box->value();
	input.has_power_watts = propagation_power_box->value() > 0.0;
	const reference::ReachEstimateResult result = reference::estimate_reach(input);

	if (!result.ok) {
		reach_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	QString reach_output = result.summary;
	reach_output += QStringLiteral("\nCategories: %1\n").arg(result.categories.join(QStringLiteral(", ")));
	if (result.includes_radio_horizon) {
		reach_output += QStringLiteral("TX horizon: %1 km\nRX horizon: %2 km\nCombined radio horizon: %3 km\n")
			.arg(result.tx_horizon_km, 0, 'f', 1)
			.arg(result.rx_horizon_km, 0, 'f', 1)
			.arg(result.combined_horizon_km, 0, 'f', 1);
	}
	reach_output += QStringLiteral("\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));
	reach_text->setPlainText(reach_output);
}

void
MainWindow::update_target_list()
{
	const QSignalBlocker blocker(target_list);
	target_list->clear();

	for (const project::AntennaTarget &target : current_project.targets) {
		QListWidgetItem *item = new QListWidgetItem(
			QStringLiteral("%1 - %2 MHz")
				.arg(target.band_name)
				.arg(target.frequency_mhz, 0, 'f', 3),
			target_list
		);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(target.enabled ? Qt::Checked : Qt::Unchecked);
	}
}

void
MainWindow::update_yagi_controls()
{
	const bool is_yagi = static_cast<calculators::AntennaType>(antenna_type_box->currentData().toInt()) == calculators::AntennaType::Yagi;

	yagi_group->setVisible(is_yagi);
	design_mode_box->setEnabled(!is_yagi);
	length_box->setEnabled(!is_yagi);
	velocity_factor_box->setMinimum(is_yagi ? 0.85 : calculators::MIN_WIRE_FACTOR);
	if (is_yagi && velocity_factor_box->value() < 0.85)
		velocity_factor_box->setValue(0.95);
}

}
