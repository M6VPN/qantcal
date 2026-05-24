// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main.cpp

#include "main_window.h"

#include "settings/app_settings.h"
#include "settings/translation_manager.h"

#include <QApplication>

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	const qantcal::settings::AppSettings app_settings;

	QCoreApplication::setOrganizationName(QStringLiteral("M6VPN"));
	QCoreApplication::setApplicationName(QStringLiteral("qantcal"));
	qantcal::settings::apply_language(app_settings.language_code());

	qantcal::MainWindow window;

	window.show();

	return app.exec();
}
