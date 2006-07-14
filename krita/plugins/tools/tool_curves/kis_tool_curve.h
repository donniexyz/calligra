/*
 *  kis_tool_curve.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_TOOL_CURVE_H_
#define KIS_TOOL_CURVE_H_

#include "kis_tool_paint.h"
#include "kis_point.h"

#include "kis_curve_framework.h"

class CurvePoint;
class KisPoint;
class KisPainter;

class KisToolCurve : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:
    KisToolCurve(const QString& UIName);
    virtual ~KisToolCurve();

    //
    // KisCanvasObserver interface
    //

    virtual void update (KisCanvasSubject *subject);

    //
    // KisToolPaint interface
    //
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);
    virtual void doubleClick(KisDoubleClickEvent *);

public slots:

    void deactivate();

protected:

    virtual void paint(KisCanvasPainter&);
    virtual void paint(KisCanvasPainter&, const QRect&);

    //
    // KisToolCurve interface
    //
    virtual KisPoint mouseOnHandle(KisPoint);
    virtual void draw();
    virtual void draw(const KisCurve& curve);
    virtual KisCurve::iterator drawPoint(KisCanvasPainter& gc, KisCurve::iterator point, const KisCurve& curve);
    virtual void paintCurve();
    virtual KisCurve::iterator paintPoint(KisPainter& painter, KisCurve::iterator point);

protected:

    KisCurve *m_curve;
    KisImageSP m_currentImage;
    bool m_drawPivots;
};

#endif //__KIS_TOOL_CURVE_H_
