// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_export_options.h

#ifndef QANTCAL_GUIDES_GUIDE_EXPORT_OPTIONS_H
#define QANTCAL_GUIDES_GUIDE_EXPORT_OPTIONS_H

#include <QString>

namespace qantcal::guides {

struct GuideExportOptions {
	bool include_assumptions = true;
	bool include_diagram = true;
	bool include_notes = true;
	bool include_safety_notes = true;
	double margin_mm = 15.0;
	QString page_size_name = "A4";
};

GuideExportOptions default_export_options();

}

#endif
