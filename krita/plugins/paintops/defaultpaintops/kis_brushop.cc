/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qrect.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_input_device.h"

#include "kis_brushop.h"

KisPaintOp * KisBrushOpFactory::createOp(KisPainter * painter)
{
    KisPaintOp * op = 0;
    if (m_optionWidget && m_size -> isVisible() /* bah */) {
        op = new KisBrushOp(painter, m_size -> isChecked(), m_opacity -> isChecked(),
                            m_darken -> isChecked());
    } else {
        op = new KisBrushOp(painter);
    }
    Q_CHECK_PTR(op);
    return op;
}

QWidget * KisBrushOpFactory::optionWidget(QWidget * parent, const KisInputDevice& inputDevice)
{
    if (m_optionWidget == 0) {
        m_optionWidget = new QWidget(parent, "brush option widget");
        QHBoxLayout * l = new QHBoxLayout(m_optionWidget);
        l->setAutoAdd(true);
        m_pressureVariation = new QLabel(i18n("Pressure variation: "), m_optionWidget);
        m_size =  new QCheckBox(i18n("size"), m_optionWidget);
        m_size->setChecked(true);
        m_opacity = new QCheckBox(i18n("opacity"), m_optionWidget);
        m_darken = new QCheckBox(i18n("darken"), m_optionWidget);
    }

    if (inputDevice != KisInputDevice::mouse()) {
        m_pressureVariation->show();
        m_size->show();
        m_opacity->show();
        m_darken->show();
    } else {
        m_pressureVariation->hide();
        m_size->hide();
        m_opacity->hide();
        m_darken->hide();
    }

    return m_optionWidget;
}

KisBrushOp::KisBrushOp(KisPainter * painter, bool size, bool opacity, bool darken)
    : super(painter)
    , m_pressureSize(size)
    , m_pressureOpacity(opacity)
    , m_pressureDarken(darken)
{
}

KisBrushOp::~KisBrushOp()
{
}

void KisBrushOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);
    if (!m_pressureSize)
        adjustedInfo.pressure = PRESSURE_DEFAULT;


    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (colour | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (!m_painter -> device()) return;

    KisBrush *brush = m_painter -> brush();
    
    Q_ASSERT(brush);
    if (!brush) return;
    if (! brush -> canPaintFor(adjustedInfo) )
        return;
    
    KisPaintDeviceSP device = m_painter -> device();

    KisPoint hotSpot = brush -> hotSpot(adjustedInfo);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    Q_INT32 x;
    double xFraction;
    Q_INT32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = 0;

    Q_UINT8 origOpacity = m_painter -> opacity();
    KisColor origColor = m_painter -> paintColor();

    if (m_pressureOpacity)
        m_painter -> setOpacity(origOpacity * info.pressure);

    if (m_pressureDarken) {
        KisColor darkened = origColor;
        // Darken docs aren't really clear about what exactly the amount param can have as value...
        darkened.colorSpace() -> darken(origColor.data(), darkened.data(),
                                        255  - 75 * info.pressure, false, 0.0, 1);
        m_painter -> setPaintColor(darkened);
    }

    if (brush -> brushType() == IMAGE || brush -> brushType() == PIPE_IMAGE) {
        dab = brush -> image(device -> colorSpace(), adjustedInfo, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush -> mask(adjustedInfo, xFraction, yFraction);
        dab = computeDab(mask);
    }

    m_painter -> setPressure(adjustedInfo.pressure);

    QRect dabRect = QRect(0, 0, brush -> maskWidth(adjustedInfo),
                          brush -> maskHeight(adjustedInfo));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device -> image();
    
    if (image != 0) {
        dstRect &= image -> bounds();
    }
    
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    Q_INT32 sw = dstRect.width();
    Q_INT32 sh = dstRect.height();

    m_painter -> bltSelection(dstRect.x(), dstRect.y(), m_painter -> compositeOp(), dab.data(), m_painter -> opacity(), sx, sy, sw, sh);
    m_painter -> addDirtyRect(dstRect);

    m_painter -> setOpacity(origOpacity);
    m_painter -> setPaintColor(origColor);
}
