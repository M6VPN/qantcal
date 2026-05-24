// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/settings/translation_manager.cpp

#include "translation_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QTranslator>

namespace qantcal::settings {

namespace {

QTranslator *
application_translator()
{
	static QTranslator translator;

	return &translator;
}

bool
load_translator_for_locale(QTranslator &translator, const QLocale &locale)
{
	for (const QString &path : translation_search_paths()) {
		if (translator.load(locale, QStringLiteral("qantcal"), QStringLiteral("_"), path))
			return true;
	}

	return false;
}

}

bool
apply_language(const QString &language_code)
{
	QTranslator *translator = application_translator();
	const QString normalised_code = normalised_language_code(language_code);
	const QLocale locale = normalised_code == QStringLiteral("system")
		? QLocale::system()
		: QLocale(normalised_code);

	QCoreApplication::removeTranslator(translator);
	if (locale.language() == QLocale::English)
		return true;
	if (!load_translator_for_locale(*translator, locale))
		return false;

	return QCoreApplication::installTranslator(translator);
}

QString
normalised_language_code(const QString &language_code)
{
	const QString trimmed = language_code.trimmed();

	if (trimmed.isEmpty())
		return QStringLiteral("system");
	if (trimmed == QStringLiteral("ru"))
		return QStringLiteral("ru_RU");

	return trimmed;
}

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

}
