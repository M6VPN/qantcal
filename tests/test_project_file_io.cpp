// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// tests/test_project_file_io.cpp

#include "project/project_file_io.h"

#include <QFile>
#include <QTemporaryFile>

#include <cassert>

void
test_malformed_json_fails()
{
	QTemporaryFile file;
	qantcal::project::AntennaProject project;
	QString error;

	assert(file.open());
	file.write("{ malformed json");
	file.close();

	assert(!qantcal::project::load_project(file.fileName(), project, error));
	assert(!error.isEmpty());
}

void
test_save_then_load()
{
	QTemporaryFile file;
	qantcal::project::AntennaProject project = qantcal::project::default_project();
	qantcal::project::AntennaProject loaded;
	QString error;

	project.title = QStringLiteral("Saved project");
	assert(file.open());
	file.close();

	assert(qantcal::project::save_project(project, file.fileName(), error));
	assert(qantcal::project::load_project(file.fileName(), loaded, error));
	assert(loaded.title == project.title);
}

int
main()
{
	test_malformed_json_fails();
	test_save_then_load();

	return 0;
}
