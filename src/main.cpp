// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/main.cpp

#include "main_window.h"

#include <QApplication>

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	qantcal::MainWindow window;

	window.show();

	return app.exec();
}
