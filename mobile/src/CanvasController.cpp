/*
 * This file is part of the KDE project
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2010-2011 Jarosław Staniek <staniek@kde.org>
 * Copyright (C) 2011 Shantanu Tushar <jhahoneyk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */


#include "CanvasController.h"

#include <KoDocument.h>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <KoView.h>
#include <KoCanvasBase.h>
#include <KWCanvasItem.h>
#include <KDebug>
#include <KoZoomController.h>
#include <KoZoomHandler.h>
#include <KActionCollection>
#include <KoToolManager.h>
#include <tables/part/CanvasItem.h>
#include <libs/kopageapp/KoPACanvasItem.h>

#include <QPoint>
#include <QSize>
#include <QGraphicsWidget>

/*!
* extensions
*/
const QString EXT_PPS("pps");
const QString EXT_PPSX("ppsx");
const QString EXT_PPT("ppt");
const QString EXT_PPTX("pptx");
const QString EXT_ODP("odp");
const QString EXT_DOC("doc");
const QString EXT_DOCX("docx");
const QString EXT_ODT("odt");
const QString EXT_TXT("txt");
const QString EXT_RTF("rtf");
const QString EXT_ODS("ods");
const QString EXT_XLS("xls");
const QString EXT_XLSX("xlsx");

CanvasController::CanvasController(QDeclarativeItem* parent)
    : KoCanvasController(0), QDeclarativeItem(parent)
{
    KActionCollection *actionCollection = new KActionCollection(this);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    m_zoomHandler = new KoZoomHandler();
    m_zoomController = new KoZoomController(this, m_zoomHandler, actionCollection);
}

void CanvasController::openDocument(const QString& path)
{
    QString error;
    QString mimetype = KMimeType::findByPath(path)->name();
    KoDocument *doc = KMimeTypeTrader::createPartInstanceFromQuery<KoDocument>(mimetype, 0, 0, QString(),
                                                                               QVariantList(), &error);
    doc->openUrl(KUrl(path));
    // get the one canvas item for this document
    m_canvas = dynamic_cast<KoCanvasBase*>(doc->canvasItem());
    KoToolManager::instance()->addController(this);

    QString fname(path);
    QString ext = KMimeType::extractKnownExtension(fname);
    if (!ext.isEmpty()) {
        fname.chop(ext.length() + 1);
    }

    if (isPresentationDocumentExtension(ext)) {
        //FIXME: Doesn't work, crashes
        KoPACanvasItem *canvas = dynamic_cast<KoPACanvasItem*>(m_canvas);

        if (canvas) {
            // update the canvas whenever we scroll, the canvas controller must emit this signal on scrolling/panning
            connect(proxyObject, SIGNAL(moveDocumentOffset(const QPoint&)), canvas, SLOT(slotSetDocumentOffset(QPoint)));
            canvas->update();
        }
    } else if (isSpreadsheetDocumentExtension(ext)) {
        //TODO: Sheet tabs
        Calligra::Tables::CanvasItem *canvas = dynamic_cast<Calligra::Tables::CanvasItem*>(m_canvas);

        if (canvas) {
            // update the canvas whenever we scroll, the canvas controller must emit this signal on scrolling/panning
            connect(proxyObject, SIGNAL(moveDocumentOffset(const QPoint&)), canvas, SLOT(setDocumentOffset(QPoint)));
            canvas->update();
        }
    } else {
        KWCanvasItem *canvas = dynamic_cast<KWCanvasItem*>(m_canvas);

        if (canvas) {
            // update the canvas whenever we scroll, the canvas controller must emit this signal on scrolling/panning
            connect(proxyObject, SIGNAL(moveDocumentOffset(const QPoint&)), canvas, SLOT(setDocumentOffset(QPoint)));
            // whenever the size of the document viewed in the canvas changes, inform the zoom controller
            connect(canvas, SIGNAL(documentSize(QSizeF)), m_zoomController, SLOT(setDocumentSize(QSizeF)));
            canvas->updateSize();
        }
    }

    setCanvas(m_canvas);
}

void CanvasController::setVastScrolling(qreal factor)
{
    kDebug() << factor;
}

void CanvasController::setZoomWithWheel(bool zoom)
{
    kDebug() << zoom;
}

void CanvasController::updateDocumentSize(const QSize& sz, bool recalculateCenter)
{
    kDebug() << sz << recalculateCenter;
}

void CanvasController::setScrollBarValue(const QPoint& value)
{
    kDebug() << value;
}

QPoint CanvasController::scrollBarValue() const
{
    return QPoint();
}

void CanvasController::pan(const QPoint& distance)
{
    kDebug() << distance;
}

QPoint CanvasController::preferredCenter() const
{
    return QPoint();
}

void CanvasController::setPreferredCenter(const QPoint& viewPoint)
{
    kDebug() << viewPoint;
}

void CanvasController::recenterPreferred()
{
    kDebug() << "BLEH";
}

void CanvasController::zoomTo(const QRect& rect)
{
    kDebug() << rect;
}

void CanvasController::zoomBy(const QPoint& center, qreal zoom)
{
    kDebug() << center << zoom;
}

void CanvasController::zoomOut(const QPoint& center)
{
    kDebug() << center;
}

void CanvasController::zoomIn(const QPoint& center)
{
    kDebug() << center;
}

void CanvasController::ensureVisible(KoShape* shape)
{
    kDebug() << shape;
}

void CanvasController::ensureVisible(const QRectF& rect, bool smooth)
{
    kDebug() << rect << smooth;
}

int CanvasController::canvasOffsetY() const
{
    kDebug() << "ASKING";
    return 0;
}

int CanvasController::canvasOffsetX() const
{
    return 0;
}

int CanvasController::visibleWidth() const
{
    kDebug() << "ASKING";
    return 0;
}

int CanvasController::visibleHeight() const
{
    kDebug() << "ASKING";
    return 0;
}

KoCanvasBase* CanvasController::canvas() const
{
    kDebug() << "ASKING";
    return m_canvas;
}

void CanvasController::setCanvas(KoCanvasBase* canvas)
{
    canvas->setCanvasController(this);
    QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget*>(canvas);
    widget->setParentItem(this);
    widget->setVisible(true);
    widget->setGeometry(0,0,width(),height());
}

void CanvasController::setDrawShadow(bool drawShadow)
{
    kDebug() << "ASKING";
    kDebug() << drawShadow;
}

QSize CanvasController::viewportSize() const
{
    kDebug() << "ASKING";
    return QSize();
}

void CanvasController::scrollContentsBy(int dx, int dy)
{
    kDebug() << dx << dy;
}

void CanvasController::scrollDown()
{
    m_currentPoint.ry()+=50;
    proxyObject->emitMoveDocumentOffset(m_currentPoint);
    dynamic_cast<KWCanvasItem*>(m_canvas)->update();
}

void CanvasController::scrollUp()
{
    m_currentPoint.ry()-=50;
    proxyObject->emitMoveDocumentOffset(m_currentPoint);
    dynamic_cast<KWCanvasItem*>(m_canvas)->update();

}

bool CanvasController::isPresentationDocumentExtension(const QString& extension) const
{
    return 0 == QString::compare(extension, EXT_ODP, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_PPS, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_PPSX, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_PPT, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_PPTX, Qt::CaseInsensitive);
}

bool CanvasController::isSpreadsheetDocumentExtension(const QString& extension) const
{
    return 0 == QString::compare(extension, EXT_ODS, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_XLS, Qt::CaseInsensitive)
       ||  0 == QString::compare(extension, EXT_XLSX, Qt::CaseInsensitive);
}

#include "CanvasController.moc"
