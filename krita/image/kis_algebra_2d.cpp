/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_algebra_2d.h"

#include <QPainterPath>
#include <kis_debug.h>


#define SANITY_CHECKS

namespace KisAlgebra2D {

void adjustIfOnPolygonBoundary(const QPolygonF &poly, int polygonDirection, QPointF *pt)
{
    const int numPoints = poly.size();
    for (int i = 0; i < numPoints; i++) {
        int nextI = i + 1;
        if (nextI >= numPoints) {
            nextI = 0;
        }

        const QPointF &p0 = poly[i];
        const QPointF &p1 = poly[nextI];

        QPointF edge = p1 - p0;

        qreal cross = crossProduct(edge, *pt - p0)
            / (0.5 * edge.manhattanLength());

        if (cross < 1.0 &&
            isInRange(pt->x(), p0.x(), p1.x()) &&
            isInRange(pt->y(), p0.y(), p1.y())) {

            QPointF salt = 1.0e-3 * inwardUnitNormal(edge, polygonDirection);

            QPointF adjustedPoint = *pt + salt;

            // in case the polygon is self-intersecting, polygon direction
            // might not help
            if (kisDistanceToLine(adjustedPoint, QLineF(p0, p1)) < 1e-4) {
                adjustedPoint = *pt - salt;

#ifdef SANITY_CHECKS
                if (kisDistanceToLine(adjustedPoint, QLineF(p0, p1)) < 1e-4) {
                    qDebug() << ppVar(*pt);
                    qDebug() << ppVar(adjustedPoint);
                    qDebug() << ppVar(QLineF(p0, p1));
                    qDebug() << ppVar(salt);

                    qDebug() << ppVar(poly.containsPoint(*pt, Qt::OddEvenFill));

                    qDebug() << ppVar(kisDistanceToLine(*pt, QLineF(p0, p1)));
                    qDebug() << ppVar(kisDistanceToLine(adjustedPoint, QLineF(p0, p1)));
                }

                *pt = adjustedPoint;

                KIS_ASSERT_RECOVER_NOOP(kisDistanceToLine(*pt, QLineF(p0, p1)) > 1e-4);
#endif /* SANITY_CHECKS */
            }
        }
    }
}

QPointF transformAsBase(const QPointF &pt, const QPointF &base1, const QPointF &base2) {
    qreal len1 = norm(base1);
    if (len1 < 1e-5) return pt;
    qreal sin1 = base1.y() / len1;
    qreal cos1 = base1.x() / len1;

    qreal len2 = norm(base2);
    if (len2 < 1e-5) return QPointF();
    qreal sin2 = base2.y() / len2;
    qreal cos2 = base2.x() / len2;

    qreal sinD = sin2 * cos1 - cos2 * sin1;
    qreal cosD = cos1 * cos2 + sin1 * sin2;
    qreal scaleD = len2 / len1;

    QPointF result;
    result.rx() = scaleD * (pt.x() * cosD - pt.y() * sinD);
    result.ry() = scaleD * (pt.x() * sinD + pt.y() * cosD);

    return result;
}

qreal angleBetweenVectors(const QPointF &v1, const QPointF &v2)
{
    qreal a1 = std::atan2(v1.y(), v1.x());
    qreal a2 = std::atan2(v2.y(), v2.x());

    return a2 - a1;
}

QPainterPath smallArrow()
{
    QPainterPath p;

    p.moveTo(5, 2);
    p.lineTo(-3, 8);
    p.lineTo(-5, 5);
    p.lineTo( 2, 0);
    p.lineTo(-5,-5);
    p.lineTo(-3,-8);
    p.lineTo( 5,-2);
    p.arcTo(QRectF(3, -2, 4, 4), 90, -180);

    return p;
}

QRect blowRect(const QRect &rect, qreal coeff)
{
    int w = rect.width() * coeff;
    int h = rect.height() * coeff;

    return rect.adjusted(-w, -h, w, h);
}

template <class Point, class Rect>
inline Point ensureInRectImpl(Point pt, const Rect &bounds)
{
    if (pt.x() > bounds.right()) {
        pt.rx() = bounds.right();
    } else if (pt.x() < bounds.left()) {
        pt.rx() = bounds.left();
    }

    if (pt.y() > bounds.bottom()) {
        pt.ry() = bounds.bottom();
    } else if (pt.y() < bounds.top()) {
        pt.ry() = bounds.top();
    }

    return pt;
}

QPoint ensureInRect(QPoint pt, const QRect &bounds)
{
    return ensureInRectImpl(pt, bounds);
}

QPointF ensureInRect(QPointF pt, const QRectF &bounds)
{
    return ensureInRectImpl(pt, bounds);
}

QRect ensureRectNotSmaller(QRect rc, const QSize &size)
{
    if (rc.width() < size.width() ||
        rc.height() < size.height()) {

        int width = qMax(rc.width(), size.width());
        int height = qMax(rc.height(), size.height());

        rc = QRect(rc.topLeft(), QSize(width, height));
    }

    return rc;
}

bool intersectLineRect(QLineF &line, const QRect rect)
{
    QPointF pt1 = QPointF(), pt2 = QPointF();
    QPointF tmp;

    if (line.intersect(QLineF(rect.topLeft(), rect.topRight()), &tmp) != QLineF::NoIntersection) {
        if (tmp.x() >= rect.left() && tmp.x() <= rect.right()) {
            pt1 = tmp;
        }
    }

    if (line.intersect(QLineF(rect.topRight(), rect.bottomRight()), &tmp) != QLineF::NoIntersection) {
        if (tmp.y() >= rect.top() && tmp.y() <= rect.bottom()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }
    if (line.intersect(QLineF(rect.bottomRight(), rect.bottomLeft()), &tmp) != QLineF::NoIntersection) {
        if (tmp.x() >= rect.left() && tmp.x() <= rect.right()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }
    if (line.intersect(QLineF(rect.bottomLeft(), rect.topLeft()), &tmp) != QLineF::NoIntersection) {
        if (tmp.y() >= rect.top() && tmp.y() <= rect.bottom()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }

    if (pt1.isNull() || pt2.isNull()) return false;

    // Attempt to retain polarity of end points
    if ((line.x1() < line.x2()) != (pt1.x() > pt2.x()) || (line.y1() < line.y2()) != (pt1.y() > pt2.y())) {
        tmp = pt1;
        pt1 = pt2;
        pt2 = tmp;
    }

    line.setP1(pt1);
    line.setP2(pt2);

    return true;
}

}
