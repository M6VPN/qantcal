// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/project_file_io.h

#ifndef QANTCAL_PROJECT_PROJECT_FILE_IO_H
#define QANTCAL_PROJECT_PROJECT_FILE_IO_H

#include "project/antenna_project.h"

#include <QString>

namespace qantcal::project {

bool load_project(const QString &path, AntennaProject &project, QString &error_message);
bool save_project(const AntennaProject &project, const QString &path, QString &error_message);

}

#endif
