// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.cpp

#include "main_window.h"

#include "calculators/ham_band.h"
#include "calculators/rf_units.h"
#include "guides/guide_document.h"
#include "guides/guide_renderer.h"

#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

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

}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(QStringLiteral("qantcal"));
	resize(1100, 720);

	create_actions();
	create_central_widget();
	restore_settings();
	calculate();
}

void
MainWindow::calculate()
{
	const calculators::AntennaCalculationInput input = current_input();
	const calculators::AntennaCalculationResult result = calculators::calculate_antenna(input);

	latest_result = result;
	result_text->setPlainText(result_to_text(result, current_length_unit));
	design_scene->show_antenna_diagram(result, current_length_unit);

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

	current_length_unit = new_unit;
	configure_length_input();

	{
		const QSignalBlocker blocker(length_box);
		length_box->setValue(calculators::metres_to_length_unit(length_m, current_length_unit));
	}

	app_settings.set_length_unit(current_length_unit);
	calculate();
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
}

void
MainWindow::create_actions()
{
	QMenu *file_menu = menuBar()->addMenu(QStringLiteral("&File"));
	QAction *print_action = file_menu->addAction(QStringLiteral("Print Guide"));
	QAction *export_pdf_action = file_menu->addAction(QStringLiteral("Export PDF"));
	file_menu->addSeparator();
	QAction *exit_action = file_menu->addAction(QStringLiteral("Exit"));

	QMenu *help_menu = menuBar()->addMenu(QStringLiteral("&Help"));
	QAction *about_action = help_menu->addAction(QStringLiteral("About qantcal"));

	connect(print_action, &QAction::triggered, this, &MainWindow::show_print_placeholder);
	connect(export_pdf_action, &QAction::triggered, this, &MainWindow::export_pdf);
	connect(exit_action, &QAction::triggered, this, &QWidget::close);
	connect(about_action, &QAction::triggered, this, &MainWindow::show_about);
}

void
MainWindow::create_central_widget()
{
	QWidget *central = new QWidget(this);
	QSplitter *splitter = new QSplitter(Qt::Horizontal, central);
	QVBoxLayout *root_layout = new QVBoxLayout(central);
	QGroupBox *input_group = new QGroupBox(QStringLiteral("Calculator"), splitter);
	QFormLayout *input_layout = new QFormLayout(input_group);

	band_box = new QComboBox(input_group);
	antenna_type_box = new QComboBox(input_group);
	design_mode_box = new QComboBox(input_group);
	length_unit_box = new QComboBox(input_group);
	frequency_box = new QDoubleSpinBox(input_group);
	length_box = new QDoubleSpinBox(input_group);
	velocity_factor_box = new QDoubleSpinBox(input_group);
	calculate_button = new QPushButton(QStringLiteral("Calculate"), input_group);

	populate_band_selector();

	antenna_type_box->addItem(QStringLiteral("Half-wave dipole"), static_cast<int>(calculators::AntennaType::HalfWaveDipole));
	antenna_type_box->addItem(QStringLiteral("Quarter-wave vertical"), static_cast<int>(calculators::AntennaType::QuarterWaveVertical));
	antenna_type_box->addItem(QStringLiteral("End-fed half-wave"), static_cast<int>(calculators::AntennaType::EndFedHalfWave));
	antenna_type_box->addItem(QStringLiteral("Full-wave loop"), static_cast<int>(calculators::AntennaType::FullWaveLoop));
	antenna_type_box->addItem(QStringLiteral("Inverted Vee"), static_cast<int>(calculators::AntennaType::InvertedVee));
	antenna_type_box->addItem(QStringLiteral("Random wire"), static_cast<int>(calculators::AntennaType::RandomWire));

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

	input_layout->addRow(QStringLiteral("Band"), band_box);
	input_layout->addRow(QStringLiteral("Antenna type"), antenna_type_box);
	input_layout->addRow(QStringLiteral("Design mode"), design_mode_box);
	input_layout->addRow(QStringLiteral("Length unit"), length_unit_box);
	input_layout->addRow(QStringLiteral("Frequency"), frequency_box);
	input_layout->addRow(QStringLiteral("Wire / element length"), length_box);
	input_layout->addRow(QStringLiteral("Shortening factor"), velocity_factor_box);
	input_layout->addRow(calculate_button);

	QWidget *workspace = new QWidget(splitter);
	QVBoxLayout *workspace_layout = new QVBoxLayout(workspace);
	design_scene = new design::AntennaDesignScene(workspace);
	design_view = new QGraphicsView(design_scene, workspace);
	result_text = new QTextEdit(workspace);

	design_view->setRenderHint(QPainter::Antialiasing);
	design_view->setMinimumHeight(320);
	result_text->setReadOnly(true);
	result_text->setMinimumHeight(160);

	workspace_layout->addWidget(design_view, 3);
	workspace_layout->addWidget(new QLabel(QStringLiteral("Results"), workspace));
	workspace_layout->addWidget(result_text, 1);

	splitter->addWidget(input_group);
	splitter->addWidget(workspace);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);

	root_layout->addWidget(splitter);
	setCentralWidget(central);

	connect(calculate_button, &QPushButton::clicked, this, &MainWindow::calculate);
	connect(band_box, &QComboBox::currentIndexChanged, this, &MainWindow::set_frequency_from_band);
	connect(antenna_type_box, &QComboBox::currentIndexChanged, this, &MainWindow::save_antenna_type);
	connect(design_mode_box, &QComboBox::currentIndexChanged, this, &MainWindow::calculate);
	connect(length_unit_box, &QComboBox::currentIndexChanged, this, &MainWindow::change_length_unit);
	connect(frequency_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(length_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(velocity_factor_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::save_shortening_factor);
}

void
MainWindow::populate_band_selector()
{
	band_box->addItem(QStringLiteral("Custom"), 0.0);

	for (const calculators::HamBand &band : calculators::ham_bands()) {
		const QString label = QStringLiteral("%1 (%2-%3 MHz)")
			.arg(QString::fromStdString(band.display_name))
			.arg(band.lower_frequency_mhz, 0, 'f', 3)
			.arg(band.upper_frequency_mhz, 0, 'f', 3);
		band_box->addItem(label, band.design_frequency_mhz);
	}

	band_box->setCurrentIndex(4);
}

void
MainWindow::set_frequency_from_band(int index)
{
	const double frequency_mhz = band_box->itemData(index).toDouble();

	if (frequency_mhz > 0.0)
		frequency_box->setValue(frequency_mhz);

	app_settings.set_band_index(index);
	calculate();
}

void
MainWindow::export_pdf()
{
	if (!latest_result.ok) {
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

	const guides::GuideDocument document = guides::create_guide_document(
		latest_result,
		current_length_unit,
		band_box->currentText()
	);
	const guides::GuideRenderer renderer;

	if (renderer.render_to_pdf(document, path)) {
		statusBar()->showMessage(QStringLiteral("Exported PDF: %1").arg(path));
		return;
	}

	statusBar()->showMessage(QStringLiteral("Export failed: could not write PDF"));
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

	const int antenna_index = antenna_type_box->findData(static_cast<int>(app_settings.antenna_type()));
	if (antenna_index >= 0) {
		const QSignalBlocker blocker(antenna_type_box);
		antenna_type_box->setCurrentIndex(antenna_index);
	}

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
	calculate();
}

void
MainWindow::save_shortening_factor()
{
	app_settings.set_shortening_factor(velocity_factor_box->value());
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
MainWindow::show_print_placeholder()
{
	/* TODO: Build printable guide rendering through Qt Print Support. */
	QMessageBox::information(this, QStringLiteral("Print Guide"), QStringLiteral("Printable guide output is planned for a later pass."));
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

calculators::LengthUnit
MainWindow::selected_length_unit() const
{
	return current_length_unit;
}

}
