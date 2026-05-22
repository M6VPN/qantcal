// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_section.cpp

#include "guide_section.h"

namespace qantcal::guides {

GuideSection
make_text_section(const QString &title, const QString &body_text, bool warning)
{
	GuideSection section;

	section.body_text = body_text;
	section.title = title;
	section.warning = warning;

	return section;
}

}
