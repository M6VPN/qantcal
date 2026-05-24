// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.cpp

#include "main_window.h"

#include "calculators/choke_calculator.h"
#include "calculators/coil_calculator.h"
#include "calculators/coax_loss_calculator.h"
#include "calculators/impedance_calculator.h"
#include "calculators/lf_mf_antenna_calculator.h"
#include "calculators/lc_resonance_calculator.h"
#include "calculators/matching_network_calculator.h"
#include "calculators/radio_horizon_calculator.h"
#include "calculators/rf_units.h"
#include "calculators/swr_calculator.h"
#include "calculators/trap_calculator.h"
#include "calculators/yagi_calculator.h"
#include "design/antenna_design_view.h"
#include "guides/guide_document.h"
#include "guides/guide_renderer.h"
#include "project/multi_band_guidance.h"
#include "project/project_file_io.h"
#include "reference/band_reference.h"
#include "reference/lf_mf_reference.h"
#include "reference/mode_reference.h"
#include "reference/propagation_notes.h"
#include "reference/reach_estimator.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QValidator>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileInfo>
#include <QUndoStack>

#include <cmath>

namespace qantcal {

namespace {

class LengthSpinBox : public QDoubleSpinBox {
public:
	explicit LengthSpinBox(QWidget *parent = nullptr)
		: QDoubleSpinBox(parent)
	{
	}

	void set_length_unit(calculators::LengthUnit unit)
	{
		length_unit = unit;
	}

protected:
	QString textFromValue(double value) const override
	{
		if (length_unit == calculators::LengthUnit::FeetInches)
			return QString::number(value, 'f', decimals());

		return QDoubleSpinBox::textFromValue(value);
	}

	QValidator::State validate(QString &text, int &pos) const override
	{
		if (length_unit != calculators::LengthUnit::FeetInches)
			return QDoubleSpinBox::validate(text, pos);

		const QString cleaned = cleaned_feet_inches_text(text);
		double feet = 0.0;

		if (cleaned.isEmpty())
			return QValidator::Intermediate;
		if (calculators::parse_feet_inches(cleaned.toStdString(), feet) && feet >= minimum() && feet <= maximum())
			return QValidator::Acceptable;
		if (has_only_feet_inches_characters(cleaned))
			return QValidator::Intermediate;

		return QValidator::Invalid;
	}

	double valueFromText(const QString &text) const override
	{
		if (length_unit == calculators::LengthUnit::FeetInches) {
			double feet = 0.0;

			if (calculators::parse_feet_inches(cleaned_feet_inches_text(text).toStdString(), feet))
				return feet;
		}

		return QDoubleSpinBox::valueFromText(text);
	}

private:
	static QString cleaned_feet_inches_text(const QString &text)
	{
		QString cleaned = text.trimmed();
		const QString suffix_text = QStringLiteral("ft");

		if (cleaned.endsWith(suffix_text, Qt::CaseInsensitive))
			cleaned.chop(suffix_text.size());
		if (cleaned.endsWith(QStringLiteral("feet"), Qt::CaseInsensitive))
			cleaned.chop(4);

		return cleaned.trimmed();
	}

	static bool has_only_feet_inches_characters(const QString &text)
	{
		for (const QChar &character : text) {
			if (character.isDigit() || character.isSpace())
				continue;
			if (character == QChar('.') || character == QChar(':') || character == QChar('\'') || character == QChar('"'))
				continue;
			if (QStringLiteral("efhintoc").contains(character, Qt::CaseInsensitive))
				continue;
			return false;
		}

		return true;
	}

	calculators::LengthUnit length_unit = calculators::LengthUnit::Metres;
};

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
	if (!result.warnings.empty()) {
		text += QStringLiteral("\nWarnings:\n");
		for (const std::string &warning : result.warnings)
			text += QStringLiteral("%1\n").arg(QString::fromStdString(warning));
	}

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
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\nWarnings:\n%1\n").arg(result.warnings.join(QStringLiteral("\n")));

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

QString
format_finite_or_infinite(double value, int decimals, const QString &infinite_text)
{
	if (std::isinf(value))
		return infinite_text;

	return QString::number(value, 'f', decimals);
}

void
configure_form_layout(QFormLayout *layout)
{
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setRowWrapPolicy(QFormLayout::WrapLongRows);
	layout->setLabelAlignment(Qt::AlignLeft);
}

void
configure_result_text(QTextEdit *text, const QString &name, const QString &description)
{
	text->setReadOnly(true);
	text->setFontFamily(QStringLiteral("monospace"));
	text->setMinimumHeight(120);
	text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	text->setAccessibleName(name);
	text->setAccessibleDescription(description);
}

QScrollArea *
create_scroll_area(QWidget *content, QWidget *parent, const QString &name)
{
	QScrollArea *area = new QScrollArea(parent);

	area->setWidget(content);
	area->setWidgetResizable(true);
	area->setFrameShape(QFrame::NoFrame);
	area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	area->setAccessibleName(name);

	return area;
}

void
set_widget_hint(QWidget *widget, const QString &name, const QString &description)
{
	widget->setAccessibleName(name);
	widget->setAccessibleDescription(description);
	widget->setToolTip(description);
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
	if (length_box != nullptr)
		length_box->interpretText();

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
	if (length_box != nullptr)
		length_box->interpretText();

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
	const double lf_mf_vertical_m = lf_mf_vertical_box != nullptr
		? calculators::length_unit_to_metres(lf_mf_vertical_box->value(), current_length_unit)
		: 0.0;
	const double lf_mf_horizontal_m = lf_mf_horizontal_box != nullptr
		? calculators::length_unit_to_metres(lf_mf_horizontal_box->value(), current_length_unit)
		: 0.0;
	const double coax_length_m = coax_length_box != nullptr
		? calculators::length_unit_to_metres(coax_length_box->value(), current_length_unit)
		: 0.0;

	current_length_unit = new_unit;
	configure_length_input();
	update_lf_mf_length_inputs();

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
	if (lf_mf_vertical_box != nullptr && lf_mf_horizontal_box != nullptr) {
		{
			const QSignalBlocker blocker(lf_mf_vertical_box);
			lf_mf_vertical_box->setValue(calculators::metres_to_length_unit(lf_mf_vertical_m, current_length_unit));
		}
		{
			const QSignalBlocker blocker(lf_mf_horizontal_box);
			lf_mf_horizontal_box->setValue(calculators::metres_to_length_unit(lf_mf_horizontal_m, current_length_unit));
		}
	}
	if (coax_length_box != nullptr) {
		const QSignalBlocker blocker(coax_length_box);
		coax_length_box->setValue(calculators::metres_to_length_unit(coax_length_m, current_length_unit));
	}

	app_settings.set_length_unit(current_length_unit);
	current_project.preferred_length_unit = current_length_unit;
	mark_project_dirty();
	calculate();
	update_reference_panel();
	calculate_lf_mf();
	calculate_coax_loss();
	statusBar()->showMessage(
		QStringLiteral("Length unit set to %1")
			.arg(QString::fromStdString(calculators::length_unit_label(current_length_unit)))
	);
}

void
MainWindow::configure_length_input()
{
	static_cast<LengthSpinBox *>(length_box)->set_length_unit(current_length_unit);

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
	update_lf_mf_length_inputs();
	if (coax_length_box != nullptr) {
		coax_length_box->setSuffix(length_box->suffix());
		coax_length_box->setDecimals(length_box->decimals());
		coax_length_box->setSingleStep(length_box->singleStep());
	}
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

	new_action->setShortcut(QKeySequence::New);
	open_action->setShortcut(QKeySequence::Open);
	save_action->setShortcut(QKeySequence::Save);
	save_as_action->setShortcut(QKeySequence::SaveAs);
	print_action->setShortcut(QKeySequence::Print);
	export_pdf_action->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
	exit_action->setShortcut(QKeySequence::Quit);
	undo_action->setShortcut(QKeySequence::Undo);
	redo_action->setShortcut(QKeySequence::Redo);
	zoom_in_action->setShortcut(QKeySequence::ZoomIn);
	zoom_out_action->setShortcut(QKeySequence::ZoomOut);
	reset_zoom_action->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
	fit_design_action->setShortcut(QKeySequence(QStringLiteral("Ctrl+F")));
	pan_action->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+P")));

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

	tabs->setAccessibleName(QStringLiteral("qantcal calculator tabs"));
	tabs->setAccessibleDescription(QStringLiteral("Switches between antenna, LF/MF, RF, and propagation calculators."));

	create_antenna_tab(tabs);
	create_lf_mf_tab(tabs);
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
	QGroupBox *input_group = new QGroupBox(QStringLiteral("Calculator"), central);
	QFormLayout *input_layout = new QFormLayout(input_group);

	configure_form_layout(input_layout);

	band_box = new QComboBox(input_group);
	band_filter_box = new QComboBox(input_group);
	antenna_type_box = new QComboBox(input_group);
	design_mode_box = new QComboBox(input_group);
	length_unit_box = new QComboBox(input_group);
	project_title_box = new QLineEdit(input_group);
	frequency_box = new QDoubleSpinBox(input_group);
	length_box = new LengthSpinBox(input_group);
	velocity_factor_box = new QDoubleSpinBox(input_group);
	project_notes_edit = new QTextEdit(input_group);
	calculate_button = new QPushButton(QStringLiteral("Calculate"), input_group);

	set_widget_hint(project_title_box, QStringLiteral("Project title"), QStringLiteral("Names the current antenna project."));
	set_widget_hint(band_filter_box, QStringLiteral("Band filter"), QStringLiteral("Limits the band selector to amateur, broadcast, or all references."));
	set_widget_hint(band_box, QStringLiteral("Band"), QStringLiteral("Selects a reference band and sets the design frequency."));
	set_widget_hint(antenna_type_box, QStringLiteral("Antenna type"), QStringLiteral("Selects the antenna design to calculate."));
	set_widget_hint(design_mode_box, QStringLiteral("Design mode"), QStringLiteral("Chooses whether to calculate length from frequency or frequency from length."));
	set_widget_hint(length_unit_box, QStringLiteral("Length unit"), QStringLiteral("Sets the displayed unit for antenna dimensions."));
	set_widget_hint(frequency_box, QStringLiteral("Frequency"), QStringLiteral("Design frequency in MHz."));
	set_widget_hint(length_box, QStringLiteral("Wire or element length"), QStringLiteral("Existing wire or element length used for length to frequency mode. Feet/inches accepts formats such as 33 ft 6 in, 33' 6\", and 33:6."));
	set_widget_hint(velocity_factor_box, QStringLiteral("Shortening factor"), QStringLiteral("Velocity or shortening factor applied to calculated wire length."));
	set_widget_hint(project_notes_edit, QStringLiteral("Project notes"), QStringLiteral("Optional notes saved with the antenna project."));
	set_widget_hint(calculate_button, QStringLiteral("Calculate antenna"), QStringLiteral("Updates the antenna calculation and diagram."));

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
	project_notes_edit->setMinimumHeight(72);
	yagi_group = new QGroupBox(QStringLiteral("Yagi design"), input_group);
	QFormLayout *yagi_layout = new QFormLayout(yagi_group);
	yagi_element_count_box = new QSpinBox(yagi_group);
	yagi_preset_box = new QComboBox(yagi_group);
	yagi_element_diameter_box = new QDoubleSpinBox(yagi_group);
	yagi_boom_correction_box = new QDoubleSpinBox(yagi_group);

	configure_form_layout(yagi_layout);
	set_widget_hint(yagi_element_count_box, QStringLiteral("Yagi element count"), QStringLiteral("Number of Yagi elements to calculate."));
	set_widget_hint(yagi_preset_box, QStringLiteral("Yagi preset"), QStringLiteral("Spacing and boom-length preset for the Yagi design."));
	set_widget_hint(yagi_element_diameter_box, QStringLiteral("Yagi element diameter"), QStringLiteral("Element diameter used for Yagi correction notes."));
	set_widget_hint(yagi_boom_correction_box, QStringLiteral("Yagi boom correction"), QStringLiteral("Optional boom correction applied to Yagi element lengths."));

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

	set_widget_hint(add_target_button, QStringLiteral("Add current target"), QStringLiteral("Adds the current band and frequency to the target band list."));
	set_widget_hint(remove_target_button, QStringLiteral("Remove selected target"), QStringLiteral("Removes the selected target band."));
	set_widget_hint(recalculate_targets_button, QStringLiteral("Recalculate target bands"), QStringLiteral("Updates all enabled target band calculations."));
	set_widget_hint(target_list, QStringLiteral("Target band list"), QStringLiteral("Lists enabled target bands for the current project."));
	design_view->setAccessibleName(QStringLiteral("Antenna design diagram"));
	design_view->setAccessibleDescription(QStringLiteral("Shows the calculated antenna diagram and editable project layout."));

	target_button_layout->addWidget(add_target_button);
	target_button_layout->addWidget(remove_target_button);
	target_button_layout->addWidget(recalculate_targets_button);
	targets_layout->addWidget(target_list);
	targets_layout->addLayout(target_button_layout);

	design_view->setMinimumHeight(220);
	configure_result_text(result_text, QStringLiteral("Antenna calculation results"), QStringLiteral("Shows antenna dimensions, notes, and warnings."));
	result_text->setMinimumHeight(140);

	workspace_layout->addWidget(design_view, 3);
	workspace_layout->addWidget(targets_group, 1);
	workspace_layout->addWidget(new QLabel(QStringLiteral("Results"), workspace));
	workspace_layout->addWidget(result_text, 1);

	QScrollArea *input_area = create_scroll_area(input_group, splitter, QStringLiteral("Antenna calculator input area"));
	splitter->addWidget(input_area);
	splitter->addWidget(workspace);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	splitter->setChildrenCollapsible(false);
	splitter->setSizes({360, 740});

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
MainWindow::create_lf_mf_tab(QTabWidget *tabs)
{
	QWidget *tab = new QWidget(tabs);
	QVBoxLayout *root_layout = new QVBoxLayout(tab);
	QGroupBox *input_group = new QGroupBox(QStringLiteral("Inputs"), tab);
	QFormLayout *input_layout = new QFormLayout(input_group);
	QPushButton *calculate_lf_mf_button = new QPushButton(QStringLiteral("Calculate"), input_group);

	configure_form_layout(input_layout);

	lf_mf_band_box = new QComboBox(input_group);
	lf_mf_design_type_box = new QComboBox(input_group);
	lf_mf_frequency_box = create_positive_spin_box(input_group, 30.0, 6, QStringLiteral(" MHz"), 0.475);
	lf_mf_vertical_box = create_positive_spin_box(input_group, 10000000.0, 3, QString(), 10.0);
	lf_mf_horizontal_box = create_positive_spin_box(input_group, 10000000.0, 3, QString(), 20.0);
	lf_mf_capacitance_box = create_positive_spin_box(input_group, 1000000.0, 3, QStringLiteral(" pF"), 0.0);
	lf_mf_result_text = new QTextEdit(tab);

	set_widget_hint(lf_mf_band_box, QStringLiteral("LF/MF band"), QStringLiteral("Selects the LF or MF reference band."));
	set_widget_hint(lf_mf_design_type_box, QStringLiteral("LF/MF design type"), QStringLiteral("Selects the LF or MF antenna style to estimate."));
	set_widget_hint(lf_mf_frequency_box, QStringLiteral("LF/MF frequency"), QStringLiteral("Design frequency in MHz."));
	set_widget_hint(lf_mf_vertical_box, QStringLiteral("LF/MF vertical height"), QStringLiteral("Available vertical height in the selected length unit."));
	set_widget_hint(lf_mf_horizontal_box, QStringLiteral("LF/MF horizontal or top wire"), QStringLiteral("Available horizontal or top-loading wire length."));
	set_widget_hint(lf_mf_capacitance_box, QStringLiteral("LF/MF estimated capacitance"), QStringLiteral("Optional capacitance estimate used for loading coil calculations."));
	set_widget_hint(calculate_lf_mf_button, QStringLiteral("Calculate LF/MF antenna"), QStringLiteral("Updates the LF/MF antenna estimate."));

	for (const reference::BandReference &band : reference::lf_mf_band_references()) {
		lf_mf_band_box->addItem(
			QStringLiteral("%1 (%2, %3-%4 kHz)")
				.arg(band.name)
				.arg(reference::band_service_label(band.service))
				.arg(band.lower_frequency_mhz * 1000.0, 0, 'f', 1)
				.arg(band.upper_frequency_mhz * 1000.0, 0, 'f', 1),
			band.design_frequency_mhz
		);
		lf_mf_band_box->setItemData(lf_mf_band_box->count() - 1, band.name, Qt::UserRole + 1);
	}

	lf_mf_design_type_box->addItem(calculators::lf_mf_design_type_label(calculators::LfMfDesignType::FullSizeReference), calculators::lf_mf_design_type_key(calculators::LfMfDesignType::FullSizeReference));
	lf_mf_design_type_box->addItem(calculators::lf_mf_design_type_label(calculators::LfMfDesignType::ShortLoadedVertical), calculators::lf_mf_design_type_key(calculators::LfMfDesignType::ShortLoadedVertical));
	lf_mf_design_type_box->addItem(calculators::lf_mf_design_type_label(calculators::LfMfDesignType::InvertedL), calculators::lf_mf_design_type_key(calculators::LfMfDesignType::InvertedL));
	lf_mf_design_type_box->addItem(calculators::lf_mf_design_type_label(calculators::LfMfDesignType::TopLoadedT), calculators::lf_mf_design_type_key(calculators::LfMfDesignType::TopLoadedT));
	lf_mf_design_type_box->addItem(calculators::lf_mf_design_type_label(calculators::LfMfDesignType::ReceiveOnlyCompact), calculators::lf_mf_design_type_key(calculators::LfMfDesignType::ReceiveOnlyCompact));

	configure_result_text(lf_mf_result_text, QStringLiteral("LF/MF antenna results"), QStringLiteral("Shows LF/MF antenna estimates, loading notes, and warnings."));
	update_lf_mf_length_inputs();

	input_layout->addRow(QStringLiteral("Band"), lf_mf_band_box);
	input_layout->addRow(QStringLiteral("Design type"), lf_mf_design_type_box);
	input_layout->addRow(QStringLiteral("Frequency"), lf_mf_frequency_box);
	input_layout->addRow(QStringLiteral("Vertical height"), lf_mf_vertical_box);
	input_layout->addRow(QStringLiteral("Horizontal/top wire"), lf_mf_horizontal_box);
	input_layout->addRow(QStringLiteral("Estimated capacitance"), lf_mf_capacitance_box);
	input_layout->addRow(calculate_lf_mf_button);
	root_layout->addWidget(create_scroll_area(input_group, tab, QStringLiteral("LF/MF antenna input area")), 0);
	root_layout->addWidget(lf_mf_result_text, 1);
	tabs->addTab(tab, QStringLiteral("LF/MF Antennas"));

	connect(calculate_lf_mf_button, &QPushButton::clicked, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);
	connect(lf_mf_band_box, &QComboBox::currentIndexChanged, this, [this](int index) {
		const double frequency_mhz = lf_mf_band_box->itemData(index).toDouble();
		if (frequency_mhz > 0.0)
			lf_mf_frequency_box->setValue(frequency_mhz);
		mark_project_dirty_and_recalculate_lf_mf();
	});
	connect(lf_mf_design_type_box, &QComboBox::currentIndexChanged, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);
	connect(lf_mf_frequency_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);
	connect(lf_mf_vertical_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);
	connect(lf_mf_horizontal_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);
	connect(lf_mf_capacitance_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::mark_project_dirty_and_recalculate_lf_mf);

	calculate_lf_mf(false);
}

void
MainWindow::create_rf_calculators_tab(QTabWidget *tabs)
{
	QWidget *rf_tab = new QWidget(tabs);
	QVBoxLayout *root_layout = new QVBoxLayout(rf_tab);
	QTabWidget *rf_tabs = new QTabWidget(rf_tab);

	rf_tabs->setAccessibleName(QStringLiteral("RF calculator tabs"));
	rf_tabs->setAccessibleDescription(QStringLiteral("Switches between coil, coax, choke, matching, impedance, LC, trap, SWR, and radio horizon calculators."));

	QWidget *coil_tab = new QWidget(rf_tabs);
	QFormLayout *coil_layout = new QFormLayout(coil_tab);
	QPushButton *coil_button = new QPushButton(QStringLiteral("Calculate"), coil_tab);
	coil_diameter_box = create_positive_spin_box(coil_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 0.05);
	coil_length_box = create_positive_spin_box(coil_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 0.10);
	coil_turns_box = create_positive_spin_box(coil_tab, 10000.0, 2, QString(), 10.0);
	coil_result_text = new QTextEdit(coil_tab);
	configure_form_layout(coil_layout);
	set_widget_hint(coil_diameter_box, QStringLiteral("Coil diameter"), QStringLiteral("Air-core coil diameter in the selected length unit."));
	set_widget_hint(coil_length_box, QStringLiteral("Coil length"), QStringLiteral("Air-core coil length in the selected length unit."));
	set_widget_hint(coil_turns_box, QStringLiteral("Coil turns"), QStringLiteral("Number of coil turns."));
	set_widget_hint(coil_button, QStringLiteral("Calculate air-core coil"), QStringLiteral("Updates the air-core coil inductance estimate."));
	configure_result_text(coil_result_text, QStringLiteral("Air-core coil results"), QStringLiteral("Shows calculated coil inductance and notes."));
	coil_layout->addRow(QStringLiteral("Diameter"), coil_diameter_box);
	coil_layout->addRow(QStringLiteral("Coil length"), coil_length_box);
	coil_layout->addRow(QStringLiteral("Turns"), coil_turns_box);
	coil_layout->addRow(coil_button);
	coil_layout->addRow(coil_result_text);
	rf_tabs->addTab(create_scroll_area(coil_tab, rf_tabs, QStringLiteral("Air-core coil input area")), QStringLiteral("Air-core coil"));

	QWidget *coax_tab = new QWidget(rf_tabs);
	QFormLayout *coax_layout = new QFormLayout(coax_tab);
	QPushButton *coax_button = new QPushButton(QStringLiteral("Calculate"), coax_tab);
	coax_frequency_box = create_positive_spin_box(coax_tab, 300000.0, 6, QStringLiteral(" MHz"), 144.3);
	coax_length_box = create_positive_spin_box(coax_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 30.0);
	coax_loss_box = create_positive_spin_box(coax_tab, 1000.0, 3, QStringLiteral(" dB / 100 m"), 4.5);
	coax_power_box = create_positive_spin_box(coax_tab, 1000000.0, 3, QStringLiteral(" W"), 100.0);
	coax_swr_box = create_positive_spin_box(coax_tab, 1000.0, 3, QString(), 1.0);
	coax_swr_box->setMinimum(1.0);
	coax_result_text = new QTextEdit(coax_tab);
	configure_form_layout(coax_layout);
	set_widget_hint(coax_frequency_box, QStringLiteral("Coax frequency"), QStringLiteral("Operating frequency in MHz."));
	set_widget_hint(coax_length_box, QStringLiteral("Coax length"), QStringLiteral("Feedline length in the selected length unit."));
	set_widget_hint(coax_loss_box, QStringLiteral("Matched coax loss"), QStringLiteral("Matched feedline loss in dB per 100 metres at the entered frequency."));
	set_widget_hint(coax_power_box, QStringLiteral("Input power"), QStringLiteral("Transmitter power entering the feedline."));
	set_widget_hint(coax_swr_box, QStringLiteral("Load SWR"), QStringLiteral("SWR at the load end of the feedline."));
	set_widget_hint(coax_button, QStringLiteral("Calculate coax loss"), QStringLiteral("Updates feedline loss and delivered power estimates."));
	configure_result_text(coax_result_text, QStringLiteral("Coax loss results"), QStringLiteral("Shows feedline loss, delivered power, notes, and warnings."));
	coax_layout->addRow(QStringLiteral("Frequency"), coax_frequency_box);
	coax_layout->addRow(QStringLiteral("Coax length"), coax_length_box);
	coax_layout->addRow(QStringLiteral("Matched loss"), coax_loss_box);
	coax_layout->addRow(QStringLiteral("Input power"), coax_power_box);
	coax_layout->addRow(QStringLiteral("Load SWR"), coax_swr_box);
	coax_layout->addRow(coax_button);
	coax_layout->addRow(coax_result_text);
	rf_tabs->addTab(create_scroll_area(coax_tab, rf_tabs, QStringLiteral("Coax loss input area")), QStringLiteral("Coax loss"));

	QWidget *choke_tab = new QWidget(rf_tabs);
	QFormLayout *choke_layout = new QFormLayout(choke_tab);
	QPushButton *choke_button = new QPushButton(QStringLiteral("Calculate"), choke_tab);
	choke_mix_box = new QComboBox(choke_tab);
	choke_mix_box->addItem(calculators::choke_mix_label(calculators::ChokeCoreMix::Custom), static_cast<int>(calculators::ChokeCoreMix::Custom));
	choke_mix_box->addItem(calculators::choke_mix_label(calculators::ChokeCoreMix::Mix31), static_cast<int>(calculators::ChokeCoreMix::Mix31));
	choke_mix_box->addItem(calculators::choke_mix_label(calculators::ChokeCoreMix::Mix43), static_cast<int>(calculators::ChokeCoreMix::Mix43));
	choke_mix_box->addItem(calculators::choke_mix_label(calculators::ChokeCoreMix::Mix61), static_cast<int>(calculators::ChokeCoreMix::Mix61));
	choke_mix_box->addItem(calculators::choke_mix_label(calculators::ChokeCoreMix::AirCore), static_cast<int>(calculators::ChokeCoreMix::AirCore));
	choke_frequency_box = create_positive_spin_box(choke_tab, 300000.0, 6, QStringLiteral(" MHz"), 14.2);
	choke_reference_impedance_box = create_positive_spin_box(choke_tab, 1000000.0, 3, QStringLiteral(" ohms"), 100.0);
	choke_turns_box = create_positive_spin_box(choke_tab, 1000.0, 2, QString(), 4.0);
	choke_target_impedance_box = create_positive_spin_box(choke_tab, 1000000.0, 3, QStringLiteral(" ohms"), 1000.0);
	choke_result_text = new QTextEdit(choke_tab);
	configure_form_layout(choke_layout);
	set_widget_hint(choke_mix_box, QStringLiteral("Choke ferrite mix"), QStringLiteral("Ferrite mix or air-core style for notes only; impedance still needs measured or datasheet data."));
	set_widget_hint(choke_frequency_box, QStringLiteral("Choke frequency"), QStringLiteral("Operating frequency in MHz for the reference impedance."));
	set_widget_hint(choke_reference_impedance_box, QStringLiteral("Reference impedance"), QStringLiteral("Measured or datasheet impedance for one pass or turn at the entered frequency."));
	set_widget_hint(choke_turns_box, QStringLiteral("Choke turns"), QStringLiteral("Number of turns used for the turns-squared estimate."));
	set_widget_hint(choke_target_impedance_box, QStringLiteral("Target choking impedance"), QStringLiteral("Desired common-mode choking impedance."));
	set_widget_hint(choke_button, QStringLiteral("Calculate RF choke"), QStringLiteral("Updates RF choke impedance estimates and warnings."));
	configure_result_text(choke_result_text, QStringLiteral("RF choke results"), QStringLiteral("Shows estimated choke impedance, suggested turns, notes, and warnings."));
	choke_layout->addRow(QStringLiteral("Mix"), choke_mix_box);
	choke_layout->addRow(QStringLiteral("Frequency"), choke_frequency_box);
	choke_layout->addRow(QStringLiteral("Reference impedance"), choke_reference_impedance_box);
	choke_layout->addRow(QStringLiteral("Turns"), choke_turns_box);
	choke_layout->addRow(QStringLiteral("Target impedance"), choke_target_impedance_box);
	choke_layout->addRow(choke_button);
	choke_layout->addRow(choke_result_text);
	rf_tabs->addTab(create_scroll_area(choke_tab, rf_tabs, QStringLiteral("RF choke input area")), QStringLiteral("RF choke"));

	QWidget *matching_tab = new QWidget(rf_tabs);
	QFormLayout *matching_layout = new QFormLayout(matching_tab);
	QPushButton *matching_button = new QPushButton(QStringLiteral("Calculate"), matching_tab);
	matching_frequency_box = create_positive_spin_box(matching_tab, 300000.0, 6, QStringLiteral(" MHz"), 14.2);
	matching_source_resistance_box = create_positive_spin_box(matching_tab, 1000000.0, 3, QStringLiteral(" ohms"), 50.0);
	matching_load_resistance_box = create_positive_spin_box(matching_tab, 1000000.0, 3, QStringLiteral(" ohms"), 200.0);
	matching_result_text = new QTextEdit(matching_tab);
	configure_form_layout(matching_layout);
	set_widget_hint(matching_frequency_box, QStringLiteral("Matching frequency"), QStringLiteral("Operating frequency in MHz."));
	set_widget_hint(matching_source_resistance_box, QStringLiteral("Source resistance"), QStringLiteral("Purely resistive source resistance in ohms."));
	set_widget_hint(matching_load_resistance_box, QStringLiteral("Load resistance"), QStringLiteral("Purely resistive load resistance in ohms."));
	set_widget_hint(matching_button, QStringLiteral("Calculate matching network"), QStringLiteral("Updates first-pass L-network matching values."));
	configure_result_text(matching_result_text, QStringLiteral("Matching network results"), QStringLiteral("Shows L-network values, notes, and warnings."));
	matching_layout->addRow(QStringLiteral("Frequency"), matching_frequency_box);
	matching_layout->addRow(QStringLiteral("Source resistance"), matching_source_resistance_box);
	matching_layout->addRow(QStringLiteral("Load resistance"), matching_load_resistance_box);
	matching_layout->addRow(matching_button);
	matching_layout->addRow(matching_result_text);
	rf_tabs->addTab(create_scroll_area(matching_tab, rf_tabs, QStringLiteral("Matching network input area")), QStringLiteral("Matching"));

	QWidget *impedance_tab = new QWidget(rf_tabs);
	QFormLayout *impedance_layout = new QFormLayout(impedance_tab);
	QPushButton *impedance_button = new QPushButton(QStringLiteral("Calculate"), impedance_tab);
	impedance_frequency_box = create_positive_spin_box(impedance_tab, 300000.0, 6, QStringLiteral(" MHz"), 14.2);
	impedance_system_box = create_positive_spin_box(impedance_tab, 1000000.0, 3, QStringLiteral(" ohms"), 50.0);
	impedance_resistance_box = create_positive_spin_box(impedance_tab, 1000000.0, 3, QStringLiteral(" ohms"), 50.0);
	impedance_reactance_box = new QDoubleSpinBox(impedance_tab);
	impedance_reactance_box->setRange(-1000000.0, 1000000.0);
	impedance_reactance_box->setDecimals(3);
	impedance_reactance_box->setSuffix(QStringLiteral(" ohms"));
	impedance_reactance_box->setValue(0.0);
	impedance_result_text = new QTextEdit(impedance_tab);
	configure_form_layout(impedance_layout);
	set_widget_hint(impedance_frequency_box, QStringLiteral("Impedance frequency"), QStringLiteral("Operating frequency in MHz."));
	set_widget_hint(impedance_system_box, QStringLiteral("System impedance"), QStringLiteral("Reference system impedance, usually 50 ohms."));
	set_widget_hint(impedance_resistance_box, QStringLiteral("Measured resistance"), QStringLiteral("Measured resistance part of the impedance."));
	set_widget_hint(impedance_reactance_box, QStringLiteral("Measured reactance"), QStringLiteral("Measured reactance part of the impedance; positive is inductive and negative is capacitive."));
	set_widget_hint(impedance_button, QStringLiteral("Calculate impedance"), QStringLiteral("Updates impedance, SWR, return loss, and equivalent component values."));
	configure_result_text(impedance_result_text, QStringLiteral("Impedance results"), QStringLiteral("Shows complex impedance analysis, match quality, notes, and warnings."));
	impedance_layout->addRow(QStringLiteral("Frequency"), impedance_frequency_box);
	impedance_layout->addRow(QStringLiteral("System impedance"), impedance_system_box);
	impedance_layout->addRow(QStringLiteral("Resistance R"), impedance_resistance_box);
	impedance_layout->addRow(QStringLiteral("Reactance X"), impedance_reactance_box);
	impedance_layout->addRow(impedance_button);
	impedance_layout->addRow(impedance_result_text);
	rf_tabs->addTab(create_scroll_area(impedance_tab, rf_tabs, QStringLiteral("Impedance helper input area")), QStringLiteral("Impedance"));

	QWidget *lc_tab = new QWidget(rf_tabs);
	QFormLayout *lc_layout = new QFormLayout(lc_tab);
	QPushButton *lc_button = new QPushButton(QStringLiteral("Calculate"), lc_tab);
	lc_inductance_box = create_positive_spin_box(lc_tab, 1000000.0, 6, QStringLiteral(" uH"), 1.0);
	lc_capacitance_box = create_positive_spin_box(lc_tab, 10000000.0, 3, QStringLiteral(" pF"), 100.0);
	lc_frequency_box = create_positive_spin_box(lc_tab, 300000.0, 6, QStringLiteral(" MHz"), 0.0);
	lc_result_text = new QTextEdit(lc_tab);
	configure_form_layout(lc_layout);
	set_widget_hint(lc_inductance_box, QStringLiteral("LC inductance"), QStringLiteral("Inductance in microhenries."));
	set_widget_hint(lc_capacitance_box, QStringLiteral("LC capacitance"), QStringLiteral("Capacitance in picofarads."));
	set_widget_hint(lc_frequency_box, QStringLiteral("LC reverse frequency"), QStringLiteral("Optional frequency used to solve for missing inductance or capacitance."));
	set_widget_hint(lc_button, QStringLiteral("Calculate LC resonance"), QStringLiteral("Updates LC resonance calculations."));
	configure_result_text(lc_result_text, QStringLiteral("LC resonance results"), QStringLiteral("Shows resonant frequency, inductance, and capacitance."));
	lc_layout->addRow(QStringLiteral("Inductance"), lc_inductance_box);
	lc_layout->addRow(QStringLiteral("Capacitance"), lc_capacitance_box);
	lc_layout->addRow(QStringLiteral("Frequency for reverse"), lc_frequency_box);
	lc_layout->addRow(lc_button);
	lc_layout->addRow(lc_result_text);
	rf_tabs->addTab(create_scroll_area(lc_tab, rf_tabs, QStringLiteral("LC resonance input area")), QStringLiteral("LC resonance"));

	QWidget *trap_tab = new QWidget(rf_tabs);
	QFormLayout *trap_layout = new QFormLayout(trap_tab);
	QPushButton *trap_button = new QPushButton(QStringLiteral("Calculate"), trap_tab);
	trap_frequency_box = create_positive_spin_box(trap_tab, 300000.0, 6, QStringLiteral(" MHz"), 0.0);
	trap_inductance_box = create_positive_spin_box(trap_tab, 1000000.0, 6, QStringLiteral(" uH"), 1.0);
	trap_capacitance_box = create_positive_spin_box(trap_tab, 10000000.0, 3, QStringLiteral(" pF"), 100.0);
	trap_operating_frequency_box = create_positive_spin_box(trap_tab, 300000.0, 6, QStringLiteral(" MHz"), 7.1);
	trap_result_text = new QTextEdit(trap_tab);
	configure_form_layout(trap_layout);
	set_widget_hint(trap_frequency_box, QStringLiteral("Trap reverse frequency"), QStringLiteral("Optional trap resonance frequency used to solve for missing inductance or capacitance."));
	set_widget_hint(trap_inductance_box, QStringLiteral("Trap inductance"), QStringLiteral("Trap coil inductance in microhenries."));
	set_widget_hint(trap_capacitance_box, QStringLiteral("Trap capacitance"), QStringLiteral("Trap capacitance in picofarads."));
	set_widget_hint(trap_operating_frequency_box, QStringLiteral("Trap operating frequency"), QStringLiteral("Optional frequency used to show component reactance away from resonance."));
	set_widget_hint(trap_button, QStringLiteral("Calculate trap"), QStringLiteral("Updates ideal parallel LC trap calculations."));
	configure_result_text(trap_result_text, QStringLiteral("Trap calculator results"), QStringLiteral("Shows trap resonance, component values, reactance, notes, and warnings."));
	trap_layout->addRow(QStringLiteral("Frequency for reverse"), trap_frequency_box);
	trap_layout->addRow(QStringLiteral("Inductance"), trap_inductance_box);
	trap_layout->addRow(QStringLiteral("Capacitance"), trap_capacitance_box);
	trap_layout->addRow(QStringLiteral("Operating frequency"), trap_operating_frequency_box);
	trap_layout->addRow(trap_button);
	trap_layout->addRow(trap_result_text);
	rf_tabs->addTab(create_scroll_area(trap_tab, rf_tabs, QStringLiteral("Trap calculator input area")), QStringLiteral("Trap"));

	QWidget *swr_tab = new QWidget(rf_tabs);
	QFormLayout *swr_layout = new QFormLayout(swr_tab);
	QPushButton *swr_button = new QPushButton(QStringLiteral("Calculate"), swr_tab);
	swr_forward_power_box = create_positive_spin_box(swr_tab, 1000000.0, 3, QStringLiteral(" W"), 100.0);
	swr_value_box = create_positive_spin_box(swr_tab, 1000.0, 3, QString(), 2.0);
	swr_value_box->setMinimum(1.0);
	swr_result_text = new QTextEdit(swr_tab);
	configure_form_layout(swr_layout);
	set_widget_hint(swr_forward_power_box, QStringLiteral("SWR forward power"), QStringLiteral("Forward power in watts."));
	set_widget_hint(swr_value_box, QStringLiteral("SWR value"), QStringLiteral("Standing wave ratio."));
	set_widget_hint(swr_button, QStringLiteral("Calculate SWR"), QStringLiteral("Updates reflected power and delivered power estimates."));
	configure_result_text(swr_result_text, QStringLiteral("SWR results"), QStringLiteral("Shows reflection coefficient, reflected power, and delivered estimate."));
	swr_layout->addRow(QStringLiteral("Forward power"), swr_forward_power_box);
	swr_layout->addRow(QStringLiteral("SWR"), swr_value_box);
	swr_layout->addRow(swr_button);
	swr_layout->addRow(swr_result_text);
	rf_tabs->addTab(create_scroll_area(swr_tab, rf_tabs, QStringLiteral("SWR input area")), QStringLiteral("SWR / reflected power"));

	QWidget *horizon_tab = new QWidget(rf_tabs);
	QFormLayout *horizon_layout = new QFormLayout(horizon_tab);
	QPushButton *horizon_button = new QPushButton(QStringLiteral("Calculate"), horizon_tab);
	horizon_tx_height_box = create_positive_spin_box(horizon_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 10.0);
	horizon_rx_height_box = create_positive_spin_box(horizon_tab, 10000000.0, 3, QStringLiteral(" selected unit"), 10.0);
	horizon_result_text = new QTextEdit(horizon_tab);
	configure_form_layout(horizon_layout);
	set_widget_hint(horizon_tx_height_box, QStringLiteral("Radio horizon TX height"), QStringLiteral("Transmit antenna height in the selected length unit."));
	set_widget_hint(horizon_rx_height_box, QStringLiteral("Radio horizon RX height"), QStringLiteral("Receive antenna height in the selected length unit."));
	set_widget_hint(horizon_button, QStringLiteral("Calculate radio horizon"), QStringLiteral("Updates radio line-of-sight horizon estimates."));
	configure_result_text(horizon_result_text, QStringLiteral("Radio horizon results"), QStringLiteral("Shows TX, RX, and combined radio horizon distances."));
	horizon_layout->addRow(QStringLiteral("TX antenna height"), horizon_tx_height_box);
	horizon_layout->addRow(QStringLiteral("RX antenna height"), horizon_rx_height_box);
	horizon_layout->addRow(horizon_button);
	horizon_layout->addRow(horizon_result_text);
	rf_tabs->addTab(create_scroll_area(horizon_tab, rf_tabs, QStringLiteral("Radio horizon input area")), QStringLiteral("Radio horizon"));

	root_layout->addWidget(rf_tabs);
	tabs->addTab(rf_tab, QStringLiteral("RF Calculators"));

	connect(coil_button, &QPushButton::clicked, this, &MainWindow::calculate_coil);
	connect(choke_button, &QPushButton::clicked, this, &MainWindow::calculate_choke);
	connect(coax_button, &QPushButton::clicked, this, &MainWindow::calculate_coax_loss);
	connect(matching_button, &QPushButton::clicked, this, &MainWindow::calculate_matching_network);
	connect(impedance_button, &QPushButton::clicked, this, &MainWindow::calculate_impedance);
	connect(lc_button, &QPushButton::clicked, this, &MainWindow::calculate_lc);
	connect(trap_button, &QPushButton::clicked, this, &MainWindow::calculate_trap);
	connect(swr_button, &QPushButton::clicked, this, &MainWindow::calculate_swr);
	connect(horizon_button, &QPushButton::clicked, this, &MainWindow::calculate_horizon);

	calculate_coil();
	calculate_choke();
	calculate_coax_loss();
	calculate_matching_network();
	calculate_impedance();
	calculate_lc();
	calculate_trap();
	calculate_swr();
	calculate_horizon();
}

void
MainWindow::create_band_propagation_tab(QTabWidget *tabs)
{
	QWidget *tab = new QWidget(tabs);
	QVBoxLayout *root_layout = new QVBoxLayout(tab);
	QSplitter *splitter = new QSplitter(Qt::Vertical, tab);
	QGroupBox *input_group = new QGroupBox(QStringLiteral("Inputs"), tab);
	QFormLayout *input_layout = new QFormLayout(input_group);
	QGroupBox *reference_group = new QGroupBox(QStringLiteral("Band reference"), tab);
	QVBoxLayout *reference_layout = new QVBoxLayout(reference_group);
	QGroupBox *reach_group = new QGroupBox(QStringLiteral("Reach guidance"), tab);
	QVBoxLayout *reach_layout = new QVBoxLayout(reach_group);
	QPushButton *update_button = new QPushButton(QStringLiteral("Update guidance"), input_group);

	configure_form_layout(input_layout);

	propagation_band_box = new QComboBox(input_group);
	propagation_band_filter_box = new QComboBox(input_group);
	propagation_mode_box = new QComboBox(input_group);
	propagation_environment_box = new QComboBox(input_group);
	propagation_tx_height_box = create_positive_spin_box(input_group, 10000000.0, 3, QString(), 10.0);
	propagation_rx_height_box = create_positive_spin_box(input_group, 10000000.0, 3, QString(), 10.0);
	propagation_power_box = create_positive_spin_box(input_group, 1000000.0, 3, QStringLiteral(" W"), 0.0);
	reference_text = new QTextEdit(reference_group);
	reach_text = new QTextEdit(reach_group);

	set_widget_hint(propagation_band_filter_box, QStringLiteral("Propagation band filter"), QStringLiteral("Limits propagation band choices to amateur, broadcast, or all references."));
	set_widget_hint(propagation_band_box, QStringLiteral("Propagation band"), QStringLiteral("Selects the band used for reference and reach guidance."));
	set_widget_hint(propagation_mode_box, QStringLiteral("Propagation mode"), QStringLiteral("Selects the operating mode used for reach guidance."));
	set_widget_hint(propagation_environment_box, QStringLiteral("Propagation environment"), QStringLiteral("Selects the operating environment for reach guidance."));
	set_widget_hint(propagation_tx_height_box, QStringLiteral("Propagation TX height"), QStringLiteral("Transmit antenna height in the selected length unit."));
	set_widget_hint(propagation_rx_height_box, QStringLiteral("Propagation RX height"), QStringLiteral("Receive antenna height in the selected length unit."));
	set_widget_hint(propagation_power_box, QStringLiteral("Propagation power"), QStringLiteral("Optional transmit power in watts."));
	set_widget_hint(update_button, QStringLiteral("Update propagation guidance"), QStringLiteral("Updates band reference and reach guidance output."));

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
	reference_text->setAccessibleName(QStringLiteral("Band reference output"));
	reference_text->setAccessibleDescription(QStringLiteral("Shows band, antenna, mode, propagation, and service notes."));
	reference_text->setMinimumHeight(150);
	reach_text->setReadOnly(true);
	reach_text->setAccessibleName(QStringLiteral("Reach guidance output"));
	reach_text->setAccessibleDescription(QStringLiteral("Shows practical reach estimate, radio horizon values, and warnings."));
	reach_text->setMinimumHeight(150);
	update_reference_height_inputs();

	input_layout->addRow(QStringLiteral("Band filter"), propagation_band_filter_box);
	input_layout->addRow(QStringLiteral("Band"), propagation_band_box);
	input_layout->addRow(QStringLiteral("Mode"), propagation_mode_box);
	input_layout->addRow(QStringLiteral("Environment"), propagation_environment_box);
	input_layout->addRow(QStringLiteral("TX antenna height"), propagation_tx_height_box);
	input_layout->addRow(QStringLiteral("RX antenna height"), propagation_rx_height_box);
	input_layout->addRow(QStringLiteral("Power"), propagation_power_box);
	input_layout->addRow(update_button);

	reference_layout->addWidget(reference_text);
	reach_layout->addWidget(reach_text);
	splitter->addWidget(create_scroll_area(input_group, splitter, QStringLiteral("Propagation input area")));
	splitter->addWidget(reference_group);
	splitter->addWidget(reach_group);
	splitter->setChildrenCollapsible(false);
	splitter->setSizes({210, 260, 230});

	root_layout->addWidget(splitter);
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

	band_box->setCurrentIndex(selected_index >= 0 ? selected_index : qMin(1, band_box->count() - 1));
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
	if (current_project.lf_mf_design.enabled && lf_mf_band_box != nullptr) {
		const int band_index = lf_mf_band_box->findText(current_project.lf_mf_design.band_name, Qt::MatchStartsWith);
		const int design_type_index = lf_mf_design_type_box->findData(calculators::lf_mf_design_type_key(current_project.lf_mf_design.design_type));
		if (band_index >= 0) {
			const QSignalBlocker blocker(lf_mf_band_box);
			lf_mf_band_box->setCurrentIndex(band_index);
		}
		if (design_type_index >= 0) {
			const QSignalBlocker blocker(lf_mf_design_type_box);
			lf_mf_design_type_box->setCurrentIndex(design_type_index);
		}
		{
			const QSignalBlocker blocker(lf_mf_frequency_box);
			lf_mf_frequency_box->setValue(current_project.lf_mf_design.frequency_mhz);
		}
		{
			const QSignalBlocker blocker(lf_mf_vertical_box);
			lf_mf_vertical_box->setValue(calculators::metres_to_length_unit(current_project.lf_mf_design.vertical_height_metres, current_project.preferred_length_unit));
		}
		{
			const QSignalBlocker blocker(lf_mf_horizontal_box);
			lf_mf_horizontal_box->setValue(calculators::metres_to_length_unit(current_project.lf_mf_design.horizontal_or_top_length_metres, current_project.preferred_length_unit));
		}
		{
			const QSignalBlocker blocker(lf_mf_capacitance_box);
			lf_mf_capacitance_box->setValue(current_project.lf_mf_design.has_estimated_capacitance ? current_project.lf_mf_design.estimated_capacitance_pf : 0.0);
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
	calculate_lf_mf(false);
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
MainWindow::mark_project_dirty_and_recalculate_lf_mf()
{
	calculate_lf_mf();
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
	const project::MultiBandGuidance guidance = project::create_multi_band_guidance(current_project);
	result_text->append(QStringLiteral("\nMulti-band guidance:\n%1").arg(project::multi_band_guidance_text(guidance)));
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
MainWindow::calculate_lf_mf()
{
	calculate_lf_mf(true);
}

void
MainWindow::calculate_lf_mf(bool update_project)
{
	if (lf_mf_result_text == nullptr)
		return;

	reference::BandReference band;
	const QString band_name = lf_mf_band_box->itemData(lf_mf_band_box->currentIndex(), Qt::UserRole + 1).toString();
	const bool has_band = reference::band_reference_by_name(band_name, band);
	calculators::LfMfAntennaInput input;

	input.design_type = calculators::lf_mf_design_type_from_key(lf_mf_design_type_box->currentData().toString());
	input.frequency_mhz = lf_mf_frequency_box->value();
	input.vertical_height_metres = calculators::length_unit_to_metres(lf_mf_vertical_box->value(), current_length_unit);
	input.horizontal_or_top_length_metres = calculators::length_unit_to_metres(lf_mf_horizontal_box->value(), current_length_unit);
	input.has_estimated_capacitance = lf_mf_capacitance_box->value() > 0.0;
	input.estimated_capacitance_pf = lf_mf_capacitance_box->value();
	input.preferred_length_unit = current_length_unit;

	const calculators::LfMfAntennaResult result = calculators::calculate_lf_mf_antenna(input);
	QString text;

	if (!result.ok) {
		lf_mf_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	text += QStringLiteral("Band: %1\n").arg(has_band ? band.name : QStringLiteral("Custom LF/MF"));
	text += QStringLiteral("Service: %1\n").arg(has_band ? reference::band_service_label(band.service) : QStringLiteral("Unknown"));
	text += QStringLiteral("Frequency: %1 kHz (%2 MHz)\n")
		.arg(result.frequency_khz, 0, 'f', 3)
		.arg(result.frequency_mhz, 0, 'f', 6);
	text += QStringLiteral("Design type: %1\n\n")
		.arg(calculators::lf_mf_design_type_label(input.design_type));
	text += result.dimensions.join(QStringLiteral("\n"));
	text += QStringLiteral("\n\nNotes:\n%1\n").arg(result.notes.join(QStringLiteral("\n")));
	if (result.loading_likely_required)
		text += QStringLiteral("\nLoading: loading coil or matching/tuning network likely required.\n");
	if (result.loading_coil.has_inductance) {
		text += QStringLiteral("Approximate inductance for supplied capacitance: %1 uH (%2 mH)\n")
			.arg(result.loading_coil.inductance_uh, 0, 'f', 2)
			.arg(result.loading_coil.inductance_mh, 0, 'f', 4);
	}
	text += QStringLiteral("\nWarnings:\n%1\n%2")
		.arg(result.warnings.join(QStringLiteral("\n")))
		.arg(result.loading_coil.warnings.join(QStringLiteral("\n")));
	if (has_band) {
		text += QStringLiteral("\n\nBand warning: %1\nPropagation notes: %2")
			.arg(band.warning)
			.arg(band.propagation_notes);
	}

	lf_mf_result_text->setPlainText(text);
	if (design_scene != nullptr && (update_project || current_project.lf_mf_design.enabled))
		design_scene->show_lf_mf_diagram(result, input.design_type, current_length_unit);

	if (!update_project)
		return;

	current_project.lf_mf_design.enabled = true;
	current_project.lf_mf_design.band_name = has_band ? band.name : QStringLiteral("Custom LF/MF");
	current_project.lf_mf_design.band_service = has_band ? band.service : reference::BandService::Unknown;
	current_project.lf_mf_design.category = has_band ? band.category : QStringLiteral("LF/MF");
	current_project.lf_mf_design.design_type = input.design_type;
	current_project.lf_mf_design.frequency_mhz = input.frequency_mhz;
	current_project.lf_mf_design.vertical_height_metres = input.vertical_height_metres;
	current_project.lf_mf_design.horizontal_or_top_length_metres = input.horizontal_or_top_length_metres;
	current_project.lf_mf_design.estimated_capacitance_pf = input.estimated_capacitance_pf;
	current_project.lf_mf_design.has_estimated_capacitance = input.has_estimated_capacitance;
	current_project.lf_mf_design.receive_only = result.receive_only;
	current_project.lf_mf_design.has_calculated_loading_inductance = result.loading_coil.has_inductance;
	current_project.lf_mf_design.calculated_loading_inductance_uh = result.loading_coil.inductance_uh;
}

void
MainWindow::calculate_choke()
{
	if (choke_result_text == nullptr)
		return;

	calculators::ChokeCalculationInput input;

	input.mix = static_cast<calculators::ChokeCoreMix>(choke_mix_box->currentData().toInt());
	input.frequency_mhz = choke_frequency_box->value();
	input.reference_impedance_ohms = choke_reference_impedance_box->value();
	input.target_impedance_ohms = choke_target_impedance_box->value();
	input.turns = choke_turns_box->value();

	const calculators::ChokeCalculationResult result = calculators::calculate_choke(input);
	if (!result.ok) {
		choke_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	QString text;
	text += QStringLiteral("Estimated choking impedance: %1 ohms\n").arg(result.estimated_impedance_ohms, 0, 'f', 3);
	text += QStringLiteral("Target ratio: %1x\n").arg(result.target_ratio, 0, 'f', 2);
	text += QStringLiteral("Suggested turns for target: %1\n").arg(result.suggested_turns);
	text += QStringLiteral("\n%1\n\n%2").arg(result.mix_note, result.note);
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\n\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));

	choke_result_text->setPlainText(text);
}

void
MainWindow::calculate_impedance()
{
	if (impedance_result_text == nullptr)
		return;

	calculators::ImpedanceCalculationInput input;

	input.frequency_mhz = impedance_frequency_box->value();
	input.reactance_ohms = impedance_reactance_box->value();
	input.resistance_ohms = impedance_resistance_box->value();
	input.system_impedance_ohms = impedance_system_box->value();

	const calculators::ImpedanceCalculationResult result = calculators::calculate_impedance(input);
	if (!result.ok) {
		impedance_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	QString text;
	text += QStringLiteral("Impedance magnitude: %1 ohms\n").arg(result.impedance_magnitude_ohms, 0, 'f', 3);
	text += QStringLiteral("Phase angle: %1 degrees\n").arg(result.phase_degrees, 0, 'f', 3);
	text += QStringLiteral("Admittance magnitude: %1 S\n").arg(result.admittance_magnitude_siemens, 0, 'g', 6);
	text += QStringLiteral("Conductance: %1 S\n").arg(result.conductance_siemens, 0, 'g', 6);
	text += QStringLiteral("Susceptance: %1 S\n").arg(result.susceptance_siemens, 0, 'g', 6);
	text += QStringLiteral("Reflection coefficient magnitude: %1\n").arg(result.reflection_coefficient_magnitude, 0, 'f', 6);
	text += QStringLiteral("SWR: %1\n").arg(format_finite_or_infinite(result.swr, 3, QStringLiteral("infinite")));
	text += QStringLiteral("Return loss: %1 dB\n").arg(format_finite_or_infinite(result.return_loss_db, 3, QStringLiteral("infinite")));
	text += QStringLiteral("Mismatch loss: %1 dB\n").arg(format_finite_or_infinite(result.mismatch_loss_db, 3, QStringLiteral("infinite")));
	if (!result.has_reactive_component)
		text += QStringLiteral("Reactive component: none at the entered frequency\n");
	else if (result.reactance_is_inductive)
		text += QStringLiteral("Equivalent series inductance: %1 uH\n").arg(result.equivalent_inductance_uh, 0, 'f', 6);
	else
		text += QStringLiteral("Equivalent series capacitance: %1 pF\n").arg(result.equivalent_capacitance_pf, 0, 'f', 3);
	text += QStringLiteral("\n%1").arg(result.note);
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\n\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));

	impedance_result_text->setPlainText(text);
}

void
MainWindow::calculate_matching_network()
{
	if (matching_result_text == nullptr)
		return;

	calculators::MatchingNetworkInput input;

	input.frequency_mhz = matching_frequency_box->value();
	input.source_resistance_ohms = matching_source_resistance_box->value();
	input.load_resistance_ohms = matching_load_resistance_box->value();

	const calculators::MatchingNetworkResult result = calculators::calculate_matching_network(input);
	if (!result.ok) {
		matching_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	if (result.no_network_needed) {
		matching_result_text->setPlainText(result.note);
		return;
	}

	QString text;
	text += QStringLiteral("Resistance ratio: %1:1\n").arg(result.ratio, 0, 'f', 3);
	text += QStringLiteral("Loaded Q: %1\n").arg(result.q, 0, 'f', 3);
	text += QStringLiteral("Low-resistance side: %1 (%2 ohms)\n").arg(result.low_side_label).arg(result.low_resistance_ohms, 0, 'f', 3);
	text += QStringLiteral("High-resistance side: %1 (%2 ohms)\n").arg(result.high_side_label).arg(result.high_resistance_ohms, 0, 'f', 3);
	text += QStringLiteral("Series reactance: %1 ohms\n").arg(result.series_reactance_ohms, 0, 'f', 3);
	text += QStringLiteral("Shunt reactance: %1 ohms\n").arg(result.shunt_reactance_ohms, 0, 'f', 3);
	text += QStringLiteral("\nLow-pass L-network:\n");
	text += QStringLiteral("Series inductor on low side: %1 uH\n").arg(result.low_pass_series_inductance_uh, 0, 'f', 6);
	text += QStringLiteral("Shunt capacitor across high side: %1 pF\n").arg(result.low_pass_shunt_capacitance_pf, 0, 'f', 3);
	text += QStringLiteral("\nHigh-pass L-network:\n");
	text += QStringLiteral("Series capacitor on low side: %1 pF\n").arg(result.high_pass_series_capacitance_pf, 0, 'f', 3);
	text += QStringLiteral("Shunt inductor across high side: %1 uH\n").arg(result.high_pass_shunt_inductance_uh, 0, 'f', 6);
	text += QStringLiteral("\n%1").arg(result.note);
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\n\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));

	matching_result_text->setPlainText(text);
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
MainWindow::calculate_coax_loss()
{
	if (coax_result_text == nullptr)
		return;

	calculators::CoaxLossInput input;

	input.frequency_mhz = coax_frequency_box->value();
	input.length_metres = calculators::length_unit_to_metres(coax_length_box->value(), current_length_unit);
	input.loss_db_per_100m = coax_loss_box->value();
	input.input_power_watts = coax_power_box->value();
	input.swr = coax_swr_box->value();

	const calculators::CoaxLossResult result = calculators::calculate_coax_loss(input);
	if (!result.ok) {
		coax_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	QString text;
	text += QStringLiteral("Matched coax loss: %1 dB\n").arg(result.matched_loss_db, 0, 'f', 3);
	text += QStringLiteral("Additional SWR loss: %1 dB\n").arg(result.additional_swr_loss_db, 0, 'f', 3);
	text += QStringLiteral("Total estimated loss: %1 dB\n").arg(result.total_loss_db, 0, 'f', 3);
	text += QStringLiteral("Delivered power: %1 W\n").arg(result.delivered_power_watts, 0, 'f', 3);
	text += QStringLiteral("Power lost: %1%\n\n").arg(result.loss_percent, 0, 'f', 2);
	text += result.note;
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\n\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));

	coax_result_text->setPlainText(text);
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
MainWindow::calculate_trap()
{
	calculators::TrapCalculationInput input;

	input.inductance_uh = trap_inductance_box->value();
	input.capacitance_pf = trap_capacitance_box->value();
	input.frequency_mhz = trap_frequency_box->value();
	input.operating_frequency_mhz = trap_operating_frequency_box->value();

	if (input.frequency_mhz > 0.0 && input.capacitance_pf > 0.0 && input.inductance_uh <= 0.0)
		input.mode = calculators::TrapCalculationMode::InductanceFromFrequencyCapacitance;
	else if (input.frequency_mhz > 0.0 && input.inductance_uh > 0.0 && input.capacitance_pf <= 0.0)
		input.mode = calculators::TrapCalculationMode::CapacitanceFromFrequencyInductance;
	else
		input.mode = calculators::TrapCalculationMode::FrequencyFromLC;

	const calculators::TrapCalculationResult result = calculators::calculate_trap(input);
	if (!result.ok) {
		trap_result_text->setPlainText(QStringLiteral("Input error: %1").arg(result.error_message));
		return;
	}

	QString text;
	text += QStringLiteral("Trap resonance: %1 MHz\n").arg(result.frequency_mhz, 0, 'f', 6);
	text += QStringLiteral("Trap resonance: %1 kHz\n").arg(result.frequency_khz, 0, 'f', 3);
	text += QStringLiteral("Inductance: %1 uH\n").arg(result.inductance_uh, 0, 'f', 6);
	text += QStringLiteral("Capacitance: %1 pF\n").arg(result.capacitance_pf, 0, 'f', 3);
	text += QStringLiteral("Component reactance at resonance: %1 ohms\n").arg(result.reactance_ohms, 0, 'f', 3);
	if (result.has_operating_reactance) {
		text += QStringLiteral("\nAt %1 MHz:\n").arg(result.operating_frequency_mhz, 0, 'f', 6);
		text += QStringLiteral("Inductive reactance: %1 ohms\n").arg(result.operating_inductive_reactance_ohms, 0, 'f', 3);
		text += QStringLiteral("Capacitive reactance: %1 ohms\n").arg(result.operating_capacitive_reactance_ohms, 0, 'f', 3);
	}
	text += QStringLiteral("\n%1").arg(result.note);
	if (!result.warnings.isEmpty())
		text += QStringLiteral("\n\nWarnings:\n%1").arg(result.warnings.join(QStringLiteral("\n")));

	trap_result_text->setPlainText(text);
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
MainWindow::update_lf_mf_length_inputs()
{
	if (lf_mf_vertical_box == nullptr || lf_mf_horizontal_box == nullptr || length_box == nullptr)
		return;

	lf_mf_vertical_box->setSuffix(length_box->suffix());
	lf_mf_horizontal_box->setSuffix(length_box->suffix());
	lf_mf_vertical_box->setDecimals(length_box->decimals());
	lf_mf_horizontal_box->setDecimals(length_box->decimals());
	lf_mf_vertical_box->setSingleStep(length_box->singleStep());
	lf_mf_horizontal_box->setSingleStep(length_box->singleStep());
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
