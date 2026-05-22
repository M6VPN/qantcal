// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/project_serializer.h

#ifndef QANTCAL_PROJECT_PROJECT_SERIALIZER_H
#define QANTCAL_PROJECT_PROJECT_SERIALIZER_H

#include "project/antenna_project.h"

#include <QJsonObject>
#include <QString>

namespace qantcal::project {

bool from_json(const QJsonObject &object, AntennaProject &project, QString &error_message);
QJsonObject to_json(const AntennaProject &project);

}

#endif
