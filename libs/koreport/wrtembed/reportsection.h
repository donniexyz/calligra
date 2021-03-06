/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2014 Jarosław Staniek <staniek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __REPORTSECTION_H__
#define __REPORTSECTION_H__

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <koproperty/Set.h>
#include <koproperty/Property.h>
#include <krsectiondata.h>
#include <QGraphicsScene>

#include "koreport_export.h"
#include "reportscene.h"

typedef QList<QGraphicsItem*> QGraphicsItemList;

// forward declarations
class QLabel;
class QDomNode;
class QDomDocument;
class QDomElement;

class KoReportDesigner;
class ReportSceneView;
class QVBoxLayout;
class ReportResizeBar;
class KoRuler;
class ReportSectionTitle;

//
// Class ReportSection
//
//     This class is the base to all Report Section's visual representation.
// It contains the basic data and interface that all the sections need to work.
//
class KOREPORT_EXPORT ReportSection : public QWidget
{
    Q_OBJECT
public:
    explicit ReportSection(KoReportDesigner * rptdes);
    virtual ~ReportSection();

    void setTitle(const QString & s);
    void buildXML(QDomDocument & doc, QDomElement & section);
    void initFromXML(QDomNode & section);
    virtual QSize sizeHint() const;

    /**
     * @brief Return the items in the section
     * Only return top-level items ... ie, items with no parent item
     * because child items are not full report-items, they are implementation
     * details of a report item and do not need to be counted individually
     *
     * @return QGraphicsItemList
     */
    QGraphicsItemList items() const;

    void setSectionCursor(const QCursor&);
    void unsetSectionCursor();

protected Q_SLOTS:
    void slotResizeBarDragged(int delta);

private Q_SLOTS:
    void slotPageOptionsChanged(KoProperty::Set &);
    void slotSceneClicked();
    void slotPropertyChanged(KoProperty::Set &, KoProperty::Property &);

protected:
    ReportSectionTitle * m_title;
    ReportScene * m_scene;
    ReportResizeBar * m_resizeBar;
    ReportSceneView * m_sceneView;
    KoReportDesigner* m_reportDesigner;
    KoRuler* m_sectionRuler;

private:
    KRSectionData *m_sectionData;

    friend class ReportSectionTitle;
};

class ReportResizeBar : public QFrame
{
    Q_OBJECT
public:
    explicit ReportResizeBar(QWidget * parent = 0, Qt::WFlags f = 0);

Q_SIGNALS:
    void barDragged(int delta);

protected:
    void mouseMoveEvent(QMouseEvent * e);
};

class ReportSectionTitle : public QLabel
{
    Q_OBJECT
public:
    explicit ReportSectionTitle(QWidget *parent = 0);
    ~ReportSectionTitle();

Q_SIGNALS:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif

