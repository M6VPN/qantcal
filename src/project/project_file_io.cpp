// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/project/project_file_io.cpp

#include "project_file_io.h"

#include "project/project_serializer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace qantcal::project {

bool
load_project(const QString &path, AntennaProject &project, QString &error_message)
{
	QFile file(path);

	if (!file.open(QIODevice::ReadOnly)) {
		error_message = file.errorString();
		return false;
	}

	QJsonParseError parse_error;
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parse_error);

	if (parse_error.error != QJsonParseError::NoError) {
		error_message = parse_error.errorString();
		return false;
	}
	if (!document.isObject()) {
		error_message = QStringLiteral("Project file must contain a JSON object.");
		return false;
	}

	return from_json(document.object(), project, error_message);
}

bool
save_project(const AntennaProject &project, const QString &path, QString &error_message)
{
	QFile file(path);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		error_message = file.errorString();
		return false;
	}

	const QJsonDocument document(to_json(project));
	if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
		error_message = file.errorString();
		return false;
	}

	return true;
}

}
