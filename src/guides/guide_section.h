// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_section.h

#ifndef QANTCAL_GUIDES_GUIDE_SECTION_H
#define QANTCAL_GUIDES_GUIDE_SECTION_H

#include <QString>
#include <QStringList>
#include <QVector>

namespace qantcal::guides {

struct GuideTableRow {
	QStringList cells;
};

struct GuideSection {
	QString body_text;
	QString title;
	QVector<GuideTableRow> table_rows;
	bool warning = false;
};

GuideSection make_text_section(const QString &title, const QString &body_text, bool warning = false);

}

#endif
