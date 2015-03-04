/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include "timeline_view.h"

#include <QHeaderView>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

#include "kis_timeline_model.h"

TimelineView::TimelineView(QWidget *parent)
    : QTreeView(parent)
    , m_channelDelegate(new KeyframeChannelDelegate(this, this))
{
    setItemDelegateForColumn(1, m_channelDelegate);

    setVerticalScrollMode(ScrollPerPixel);
    //setSelectionMode(ExtendedSelection); // TODO
    setSelectionMode(SingleSelection);
    setSelectionBehavior(SelectItems);
    header()->hide();
}

QModelIndex TimelineView::indexAt(const QPoint &point) const
{
    QModelIndex cell = QTreeView::indexAt(point);
    if (cell.column() == 0) return cell;
    if (!cell.isValid()) return QModelIndex();

    int column = getKeyframeAt(cell, point.x());
    if (column < 0) return QModelIndex();

    return cell.child(0, column);
}

void TimelineView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QRect rectN = rect.normalized();
    QModelIndex cell = QTreeView::indexAt(rectN.topLeft());

    if (cell.column() == 1) {
        selectKeyframesBetween(cell, rectN.left(), rectN.right(), command);
    }

    viewport()->update();
}

bool TimelineView::viewportEvent(QEvent *e)
{
    if (model()) {
        switch(e->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);

            if (mouseEvent->button() == Qt::LeftButton) {
                QModelIndex index = indexAt(mouseEvent->pos());

                if (selectionModel()->isSelected(index)) {
                    m_dragStart = mouseEvent->pos().x();
                    m_canStartDrag = true;
                }
            }
        } break;

        case QEvent::MouseMove: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);

            if (m_canStartDrag && abs(mouseEvent->pos().x() - m_dragStart) > 5) {
                m_isDragging = true;
                m_canStartDrag = false;
            }

            if (m_isDragging) {
                m_dragOffset = mouseEvent->pos().x() - m_dragStart;
                viewport()->update();
                return true;
            }
        } break;

        case QEvent::MouseButtonRelease: {
            if (m_isDragging) {
                QModelIndexList selected = selectionModel()->selectedIndexes();

                for (int i=0; i<selected.size(); i++) {
                    QModelIndex item = selected[i];

                    int oldPos = timeToPosition(item.data(KisTimelineModel::TimeRole).toInt());
                    int newTime = positionToTime(oldPos + m_dragOffset);

                    if (newTime >= 0) {
                        model()->setData(item, newTime, KisTimelineModel::TimeRole);
                    }
                }
            }

            m_canStartDrag = false;
            m_isDragging = false;
        }

        default: break;
        }
    }

    return QTreeView::viewportEvent(e);
}


int TimelineView::getKeyframeAt(const QModelIndex &channelIndex, int x) const
{
    return findKeyframe(channelIndex, x, true);
}

void TimelineView::selectKeyframesBetween(const QModelIndex &channelIndex, int fromX, int toX, QItemSelectionModel::SelectionFlags command)
{
    QItemSelectionModel *selection = selectionModel();

    int first = findKeyframe(channelIndex, fromX, false);
    int last = findKeyframe(channelIndex, toX, false);

    if (getTime(channelIndex, first) < positionToTime(fromX)) first++;

    for (int i=first; i <= last; i++) {
        selection->select(channelIndex.child(0, i), command);
    }
}

int TimelineView::findKeyframe(const QModelIndex &channelIndex, int x, bool exact) const
{
    int min = 0;
    int max = channelIndex.model()->columnCount(channelIndex) - 1;

    while (max >= min) {
        int i = (max + min) / 2;
        int time = getTime(channelIndex, i);
        int x_here = timeToPosition(time);
        int x_next = timeToPosition(time + 1);

        if (x_here <= x && x < x_next) {
            return i;
        } else if (x_here < x) {
            min = i + 1;
        } else {
            max = i - 1;
        }
    }

    return (exact) ? -1 : max;
}

int TimelineView::getTime(const QModelIndex &channelIndex, int index) const
{
    return channelIndex.child(0, index).data(KisTimelineModel::TimeRole).toInt();
}

int TimelineView::timeToPosition(int time) const
{
    return time * 8 + columnViewportPosition(1);
}

int TimelineView::positionToTime(int x) const
{
    return (x - columnViewportPosition(1)) / 8;
}

bool TimelineView::isWithingView(int x) const
{
    return x >= columnViewportPosition(1) && x < viewport()->width();
}

bool TimelineView::isDragging()
{
    return m_isDragging;
}

float TimelineView::dragOffset()
{
    return m_dragOffset;
}

#include "timeline_view.moc"