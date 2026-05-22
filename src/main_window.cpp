// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main_window.cpp

#include "main_window.h"

#include "calculators/rf_units.h"

#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

namespace qantcal {

namespace {

struct BandPreset {
	const char *label;
	double frequency_mhz;
};

constexpr BandPreset BAND_PRESETS[] = {
	{ "Custom", 0.0 },
	{ "160 m", 1.9 },
	{ "80 m", 3.65 },
	{ "40 m", 7.1 },
	{ "30 m", 10.125 },
	{ "20 m", 14.2 },
	{ "17 m", 18.1 },
	{ "15 m", 21.2 },
	{ "12 m", 24.95 },
	{ "10 m", 28.5 },
	{ "6 m", 50.15 },
	{ "2 m", 145.0 },
	{ "70 cm", 433.0 },
};

QString
result_to_text(const calculators::AntennaCalculationResult &result)
{
	if (!result.ok)
		return QStringLiteral("Input error: %1").arg(QString::fromStdString(result.error));

	QString text;
	text += QStringLiteral("Frequency: %1\n")
		.arg(QString::fromStdString(calculators::format_mhz(result.frequency_mhz)));
	text += QStringLiteral("Wavelength: %1\n")
		.arg(QString::fromStdString(calculators::format_meters(result.wavelength_m)));
	text += QStringLiteral("Velocity / shortening factor: %1\n")
		.arg(result.velocity_factor, 0, 'f', 3);

	if (result.total_length_m > 0.0) {
		text += QStringLiteral("Total length: %1\n")
			.arg(QString::fromStdString(calculators::format_meters(result.total_length_m)));
	}

	if (result.leg_length_m > 0.0) {
		text += QStringLiteral("Per-leg length: %1\n")
			.arg(QString::fromStdString(calculators::format_meters(result.leg_length_m)));
	}

	if (result.radiator_length_m > 0.0) {
		text += QStringLiteral("Radiator length: %1\n")
			.arg(QString::fromStdString(calculators::format_meters(result.radiator_length_m)));
	}

	text += QStringLiteral("\nNote: %1\n")
		.arg(QString::fromStdString(result.note));
	text += QStringLiteral("\nApproximation warning: these are starting dimensions only. Trim, measure, and check the final installation.");

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
	calculate();
}

void
MainWindow::calculate()
{
	const calculators::AntennaCalculationInput input = current_input();
	const calculators::AntennaCalculationResult result = calculators::calculate_antenna(input);

	result_text->setPlainText(result_to_text(result));
	design_scene->show_placeholder_diagram(QString::fromUtf8(calculators::antenna_type_label(input.antenna_type)));
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
	connect(export_pdf_action, &QAction::triggered, this, &MainWindow::show_export_pdf_placeholder);
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
	frequency_box = new QDoubleSpinBox(input_group);
	length_box = new QDoubleSpinBox(input_group);
	velocity_factor_box = new QDoubleSpinBox(input_group);
	calculate_button = new QPushButton(QStringLiteral("Calculate"), input_group);

	populate_band_selector();

	antenna_type_box->addItem(QStringLiteral("Half-wave dipole"), static_cast<int>(calculators::AntennaType::HalfWaveDipole));
	antenna_type_box->addItem(QStringLiteral("Quarter-wave vertical"), static_cast<int>(calculators::AntennaType::QuarterWaveVertical));
	antenna_type_box->addItem(QStringLiteral("End-fed half-wave"), static_cast<int>(calculators::AntennaType::EndFedHalfWave));
	antenna_type_box->addItem(QStringLiteral("Full-wave loop"), static_cast<int>(calculators::AntennaType::FullWaveLoop));
	antenna_type_box->addItem(QStringLiteral("Long wire / random wire placeholder"), static_cast<int>(calculators::AntennaType::LongWirePlaceholder));

	design_mode_box->addItem(QStringLiteral("Frequency to length"), static_cast<int>(calculators::DesignMode::FrequencyToLength));
	design_mode_box->addItem(QStringLiteral("Length to frequency"), static_cast<int>(calculators::DesignMode::LengthToFrequency));

	frequency_box->setRange(0.001, 300000.0);
	frequency_box->setDecimals(6);
	frequency_box->setSuffix(QStringLiteral(" MHz"));
	frequency_box->setValue(7.1);

	length_box->setRange(0.001, 10000.0);
	length_box->setDecimals(3);
	length_box->setSuffix(QStringLiteral(" m"));
	length_box->setValue(20.0);

	velocity_factor_box->setRange(0.1, 1.0);
	velocity_factor_box->setDecimals(3);
	velocity_factor_box->setSingleStep(0.005);
	velocity_factor_box->setValue(calculators::DEFAULT_WIRE_FACTOR);

	input_layout->addRow(QStringLiteral("Band"), band_box);
	input_layout->addRow(QStringLiteral("Antenna type"), antenna_type_box);
	input_layout->addRow(QStringLiteral("Design mode"), design_mode_box);
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
	connect(antenna_type_box, &QComboBox::currentIndexChanged, this, &MainWindow::calculate);
	connect(design_mode_box, &QComboBox::currentIndexChanged, this, &MainWindow::calculate);
	connect(frequency_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(length_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
	connect(velocity_factor_box, &QDoubleSpinBox::valueChanged, this, &MainWindow::calculate);
}

void
MainWindow::populate_band_selector()
{
	for (const BandPreset &band : BAND_PRESETS)
		band_box->addItem(QString::fromUtf8(band.label), band.frequency_mhz);

	band_box->setCurrentIndex(3);
}

void
MainWindow::set_frequency_from_band(int index)
{
	const double frequency_mhz = band_box->itemData(index).toDouble();

	if (frequency_mhz > 0.0)
		frequency_box->setValue(frequency_mhz);

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
MainWindow::show_export_pdf_placeholder()
{
	/* TODO: Render printable antenna guide pages through Qt Print Support PDF output. */
	QMessageBox::information(this, QStringLiteral("Export PDF"), QStringLiteral("PDF export is planned for a later pass."));
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
	input.length_m = length_box->value();
	input.velocity_factor = velocity_factor_box->value();

	return input;
}

}
