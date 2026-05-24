// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main.cpp

#include "main_window.h"

#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QTranslator>

namespace {

QStringList
translation_search_paths()
{
	QStringList paths;
	const QByteArray override_path = qgetenv("QANTCAL_TRANSLATION_DIR");
	const QString app_dir = QCoreApplication::applicationDirPath();

	if (!override_path.isEmpty())
		paths << QString::fromLocal8Bit(override_path);
	paths << app_dir + QStringLiteral("/translations");
	paths << app_dir + QStringLiteral("/../translations");
	paths << app_dir + QStringLiteral("/../share/qantcal/translations");
	paths << QDir::currentPath() + QStringLiteral("/translations");

	return paths;
}

bool
load_application_translation(QTranslator &translator)
{
	const QLocale locale;

	for (const QString &path : translation_search_paths()) {
		if (translator.load(locale, QStringLiteral("qantcal"), QStringLiteral("_"), path))
			return QCoreApplication::installTranslator(&translator);
	}

	return false;
}

}

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QTranslator app_translator;

	QCoreApplication::setOrganizationName(QStringLiteral("M6VPN"));
	QCoreApplication::setApplicationName(QStringLiteral("qantcal"));
	load_application_translation(app_translator);

	qantcal::MainWindow window;

	window.show();

	return app.exec();
}
