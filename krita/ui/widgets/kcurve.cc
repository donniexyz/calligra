/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "widgets/kcurve.h"

// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QEvent>
#include <QRect>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QList>

#include <QSpinBox>

// KDE includes.

#include <kis_debug.h>
#include <kcursor.h>
#include <klocale.h>

// Local includes.

#define bounds(x,a,b) (x<a ? a : (x>b ? b :x))
#define MOUSE_AWAY_THRES 15
#define POINT_AREA       1E-4
#define CURVE_AREA       1E-4


static
bool pointLessThan(const QPointF &a, const QPointF &b);


#include "kcurve_p.h"


KCurve::KCurve(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f), d(new KCurve::Private(this))
{
    setObjectName("KCurve");
    d->m_grab_point_index = -1;
    d->m_readOnlyMode   = false;
    d->m_guideVisible   = false;
    d->m_pixmapDirty=true;
    d->m_pixmapCache=NULL;
    d->setState(ST_NORMAL);


    d->m_intIn=NULL;
    d->m_intOut=NULL;

    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(150, 50);
    QPointF p;
    p.rx() = 0.0; p.ry() = 0.0;
    d->m_points.append(p);
    p.rx() = 1.0; p.ry() = 1.0;
    d->m_points.append(p);

    d->setCurveModified();

    setFocusPolicy(Qt::StrongFocus);
}

KCurve::~KCurve()
{
    if(d->m_pixmapCache)
	delete d->m_pixmapCache;
    delete d;
}

void KCurve::setupInOutControls(QSpinBox *in, QSpinBox *out, int min, int max)
{
    d->m_intIn=in;
    d->m_intOut=out;
  
    if(!d->m_intIn || !d->m_intOut)
	return;

    d->m_inOutMin=min;
    d->m_inOutMax=max;

    d->m_intIn->setRange(d->m_inOutMin, d->m_inOutMax);
    d->m_intOut->setRange(d->m_inOutMin, d->m_inOutMax);
  

    connect(d->m_intIn, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged(int)));
    connect(d->m_intOut, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged(int)));
    d->syncIOControls();
  
}
void KCurve::dropInOutControls()
{
    if(!d->m_intIn || !d->m_intOut)
	return;

    disconnect(d->m_intIn, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged(int)));
    disconnect(d->m_intOut, SIGNAL(valueChanged(int)), this, SLOT(inOutChanged(int)));

    d->m_intIn=d->m_intOut=NULL;
  
}

void KCurve::inOutChanged(int)
{
    QPointF pt;

    Q_ASSERT(d->m_grab_point_index>=0);

    pt.rx()=d->io2sp(d->m_intIn->value());
    pt.ry()=d->io2sp(d->m_intOut->value());
  
    if(d->jumpOverExistingPoints(pt, d->m_grab_point_index)) {
	d->m_points[d->m_grab_point_index]=pt;
	qSort(d->m_points.begin(), d->m_points.end(), pointLessThan);
	d->m_grab_point_index = d->m_points.indexOf(pt);
    }
    else
	pt=d->m_points[d->m_grab_point_index];
   

    d->m_intIn->blockSignals(true);
    d->m_intOut->blockSignals(true);

    d->m_intIn->setValue(d->sp2io(pt.rx()));
    d->m_intOut->setValue(d->sp2io(pt.ry()));

    d->m_intIn->blockSignals(false);
    d->m_intOut->blockSignals(false);

    d->setCurveModified();
}


void KCurve::reset(void)
{
    d->m_grab_point_index = -1;
    d->m_guideVisible = false;
    
    d->setCurveModified();
}

void KCurve::setCurveGuide(const QColor & color)
{
    d->m_guideVisible = true;
    d->m_colorGuide   = color;

}

void KCurve::setPixmap(const QPixmap & pix)
{
    d->m_pix = pix;
    d->m_pixmapDirty=true;
    d->setCurveRepaint();
}

void KCurve::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        if (d->m_grab_point_index > 0 && d->m_grab_point_index < d->m_points.count() - 1) {
            //rx() find closest point to get focus afterwards
            double grab_point_x = d->m_points[d->m_grab_point_index].rx();

            int left_of_grab_point_index = d->m_grab_point_index - 1;
            int right_of_grab_point_index = d->m_grab_point_index + 1;
            int new_grab_point_index;

            if (fabs(d->m_points[left_of_grab_point_index].rx() - grab_point_x) <
		fabs(d->m_points[right_of_grab_point_index].rx() - grab_point_x)) {
                new_grab_point_index = left_of_grab_point_index;
            } else {
                new_grab_point_index = d->m_grab_point_index;
            }
            d->m_points.removeAt(d->m_grab_point_index);
            d->m_grab_point_index = new_grab_point_index;
	    setCursor(Qt::ArrowCursor);
	    d->setState(ST_NORMAL);
        }
	d->setCurveModified();
    } else if (e->key() == Qt::Key_Escape && d->state()!=ST_NORMAL) {
        d->m_points[d->m_grab_point_index].rx() = d->m_grabOriginalX;
        d->m_points[d->m_grab_point_index].ry() = d->m_grabOriginalY;
        setCursor(Qt::ArrowCursor);
	d->setState(ST_NORMAL);

	d->setCurveModified();
    } else if ((e->key() == Qt::Key_A || e->key()==Qt::Key_Insert) && d->state()==ST_NORMAL) {
	/* FIXME: Lets user choose the hotkeys */
        addPointInTheMiddle();
    } else
        QWidget::keyPressEvent(e);
}

void KCurve::addPointInTheMiddle()
{
    QPointF pt;
    pt.rx()=0.5;
    pt.ry()=getCurveValue(pt.rx());
  
    if(!d->jumpOverExistingPoints(pt, -1))
	return;

    d->m_points.append(pt);
    qSort(d->m_points.begin(), d->m_points.end(), pointLessThan);
    d->m_grab_point_index = d->m_points.indexOf(pt);
  
    if(d->m_intIn)
	d->m_intIn->setFocus(Qt::TabFocusReason);
    d->setCurveModified();
}

void KCurve::resizeEvent(QResizeEvent *e)
{
    d->m_pixmapDirty=true;
    QWidget::resizeEvent(e);
}

void KCurve::paintEvent(QPaintEvent *)
{
    int    wWidth = width()-1;
    int    wHeight = height()-1;

    QPainter p(this);

    // Antialiasing is not a good idea here, because 
    // the grid will drift one pixel to any side due to rounding of int
    // FIXME: let's user tell the last word (in config)
    //p.setRenderHint(QPainter::Antialiasing);


    //  draw background
    if (!d->m_pix.isNull()) {
	if(d->m_pixmapDirty || !d->m_pixmapCache) {
	    if(d->m_pixmapCache)
		delete d->m_pixmapCache;
	    d->m_pixmapCache = new QPixmap(width(), height());
	    QPainter cachePainter(d->m_pixmapCache);

	    cachePainter.scale(1.0*width() / d->m_pix.width(), 1.0*height() / d->m_pix.height());
	    cachePainter.drawPixmap(0, 0, d->m_pix);
	    d->m_pixmapDirty=false;
	}
	p.drawPixmap(0,0,*d->m_pixmapCache);
    } else
        p.fillRect(rect(), palette().background());

    
    d->drawGrid(p, wWidth, wHeight);


    // Draw curve.
    double prevY = wHeight-getCurveValue(0.)*wHeight;
    double prevX = 0.;
    double curY;
    double normalizedX;
    int x;

    p.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    for (x = 0 ; x < wWidth ; x++) {
        normalizedX = double(x) / wWidth;
        curY = wHeight-getCurveValue(normalizedX)*wHeight;

	/**
	 * Keep in mind that QLineF rounds doubles 
	 * to ints matematically, not just rounds down
	 * like in C
	 */
 	p.drawLine(QLineF(prevX, prevY,
                          x, curY ));
	prevX = x;
        prevY = curY;
    }
    p.drawLine(QLineF(prevX, prevY ,
                      x, wHeight - getCurveValue(1.0) * wHeight));

    // Drawing curve handles.
    double curveX;
    double curveY;
    if (!d->m_readOnlyMode) {
        for (int i = 0; i < d->m_points.count(); ++i) {
            curveX = d->m_points.at(i).x();
            curveY = d->m_points.at(i).y();

            if (i == d->m_grab_point_index) {
                p.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
                p.drawEllipse(QRectF(curveX * wWidth - 2,
                                     wHeight - 2 - curveY * wHeight, 4, 4));
            } else {
                p.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                p.drawEllipse(QRectF(curveX * wWidth - 3,
                                     wHeight - 3 - curveY * wHeight, 6, 6));
            }
        }
    }
}

static bool pointLessThan(const QPointF &a, const QPointF &b)
{
    return a.x() < b.x();
}

void KCurve::mousePressEvent(QMouseEvent * e)
{
    if (d->m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    double x = e->pos().x() / (double)(width()-1);
    double y = 1.0 - e->pos().y() / (double)(height()-1);


        
    int closest_point_index = d->nearestPointInRange(QPointF(x, y), width(), height());
    if (closest_point_index < 0) {
	QPointF newPoint(x, y);
	if(!d->jumpOverExistingPoints(newPoint, -1))
	  return;
	d->m_points.append(newPoint);
	qSort(d->m_points.begin(), d->m_points.end(), pointLessThan);
	d->m_grab_point_index = d->m_points.indexOf(newPoint);
    } else {
	d->m_grab_point_index = closest_point_index;
    }
    
    d->m_grabOriginalX = d->m_points[d->m_grab_point_index].x();
    d->m_grabOriginalY = d->m_points[d->m_grab_point_index].y();
    d->m_grabOffsetX = d->m_points[d->m_grab_point_index].x() - x;
    d->m_grabOffsetY = d->m_points[d->m_grab_point_index].y() - y;
    d->m_points[d->m_grab_point_index].rx() = x + d->m_grabOffsetX;
    d->m_points[d->m_grab_point_index].ry() = y + d->m_grabOffsetY;
    
    d->m_draggedAwayPointIndex = -1;
    d->setState(ST_DRAG);   

	
    d->setCurveModified();
}


void KCurve::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    setCursor(Qt::ArrowCursor);
    d->setState(ST_NORMAL);
    
    d->setCurveModified();
}


void KCurve::mouseMoveEvent(QMouseEvent * e)
{
    if (d->m_readOnlyMode) return;

    double x = e->pos().x() / (double)(width()-1);
    double y = 1.0 - e->pos().y() / (double)(height()-1);

    if (d->state() == ST_NORMAL) 
    { // If no point is selected set the the cursor shape if on top
        int nearestPointIndex = d->nearestPointInRange(QPointF(x, y), width(), height());
	
        if (nearestPointIndex < 0)
	    setCursor(Qt::ArrowCursor);
        else
	    setCursor(Qt::CrossCursor);
    } else { // Else, drag the selected point
	bool crossedHoriz = e->pos().x() - width() > MOUSE_AWAY_THRES ||
	                    e->pos().x() < -MOUSE_AWAY_THRES;
	bool crossedVert =  e->pos().y() - height() > MOUSE_AWAY_THRES ||
	                    e->pos().y() < -MOUSE_AWAY_THRES;
      
	bool removePoint =  (crossedHoriz || crossedVert);

        if (!removePoint && d->m_draggedAwayPointIndex>=0) {
            // point is no longer dragged away so reinsert it
            QPointF newPoint(d->m_draggedAwayPoint);
            d->m_points.insert(d->m_draggedAwayPointIndex, newPoint);
            d->m_grab_point_index = d->m_draggedAwayPointIndex;
            d->m_draggedAwayPointIndex = -1;
        }

        if (removePoint && 
	    (d->m_draggedAwayPointIndex>=0))
	    return;


        setCursor(Qt::CrossCursor);

        x += d->m_grabOffsetX;
        y += d->m_grabOffsetY;

        double leftX;
        double rightX;
        if (d->m_grab_point_index == 0) {
            leftX = 0.0;
            if (d->m_points.count() > 1)
                rightX = d->m_points[d->m_grab_point_index + 1].rx() - POINT_AREA;
            else
                rightX = 1.0;
        } else if (d->m_grab_point_index == d->m_points.count() - 1) {
            leftX = d->m_points[d->m_grab_point_index - 1].rx() + POINT_AREA;
            rightX = 1.0;
        } else {
            Q_ASSERT(d->m_grab_point_index > 0 && d->m_grab_point_index < d->m_points.count() - 1);

            // the 1E-4 addition so we can grab the dot later.
            leftX = d->m_points[d->m_grab_point_index - 1].rx() + POINT_AREA;
            rightX = d->m_points[d->m_grab_point_index + 1].rx() - POINT_AREA;
        }

	x=bounds(x, leftX, rightX);
	y=bounds(y, 0., 1.);

        d->m_points[d->m_grab_point_index].rx() = x;
	d->m_points[d->m_grab_point_index].ry() = y;


        if (removePoint && d->m_points.count()>2) {
            d->m_draggedAwayPoint.rx() = d->m_points[d->m_grab_point_index].rx();
            d->m_draggedAwayPoint.ry() = d->m_points[d->m_grab_point_index].ry();
            d->m_draggedAwayPointIndex = d->m_grab_point_index;
            d->m_points.removeAt(d->m_grab_point_index);
	    d->m_grab_point_index=bounds(d->m_grab_point_index, 0, d->m_points.count()-1);
        }

	d->setCurveModified();
    }
}

double KCurve::getCurveValue(double x)
{
    if(d->m_splineDirty) {
	d->m_spline.createSpline(QVector<QPointF>::fromList(d->m_points));
	d->m_splineDirty=false;
    }
    return Private::checkBounds(d->m_spline, x);
}

double KCurve::getCurveValue(const QList<QPointF>& curve, double x)
{
    KisCubicSpline<QPointF, double> spline(QVector<QPointF>::fromList(curve));
    return Private::checkBounds(spline, x);
}

QList<QPointF> KCurve::getCurve()
{
    return d->m_points;
}

void KCurve::setCurve(QList<QPointF >inlist)
{
    d->m_points = inlist;
    d->m_grab_point_index=bounds(d->m_grab_point_index, 0,d->m_points.count()-1);
    d->setCurveModified();
}

void KCurve::leaveEvent(QEvent *)
{
}

#include "kcurve.moc"
