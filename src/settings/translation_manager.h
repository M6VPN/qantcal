// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/settings/translation_manager.h

#ifndef QANTCAL_SETTINGS_TRANSLATION_MANAGER_H
#define QANTCAL_SETTINGS_TRANSLATION_MANAGER_H

#include <QString>
#include <QStringList>

namespace qantcal::settings {

bool apply_language(const QString &language_code);
QString normalised_language_code(const QString &language_code);
QStringList translation_search_paths();

}

#endif
