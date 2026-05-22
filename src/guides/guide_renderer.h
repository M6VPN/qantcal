// qantcal - Developed by M6VPN (M6VPN@tuta.com)
// src/guides/guide_renderer.h

#ifndef QANTCAL_GUIDES_GUIDE_RENDERER_H
#define QANTCAL_GUIDES_GUIDE_RENDERER_H

#include "guides/guide_document.h"

#include <QPainter>
#include <QRectF>
#include <QString>

namespace qantcal::guides {

class GuideRenderer {
public:
	bool render_to_painter(const GuideDocument &document, QPainter &painter, const QRectF &page_rect) const;
	bool render_to_pdf(const GuideDocument &document, const QString &path) const;
};

}

#endif
