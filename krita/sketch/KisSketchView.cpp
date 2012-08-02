/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KisSketchView.h"

#include <QTimer>

#include <kdebug.h>
#include <kmimetype.h>

#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoProgressUpdater.h>
#include <KoToolProxy.h>
#include <KoFilterManager.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "ProgressProxy.h"
#include "KisDeclarativeCanvasItem.h"

#include "kis_doc2.h"
#include "kis_canvas2.h"
#include "kis_config.h"
#include "kis_view2.h"

class KisSketchView::Private
{
public:
    Private( KisSketchView* qq)
        : q(qq)
        , doc(0)
        , view(0)
        , canvas(0)
        , canvasItem(0)
    { }
    ~Private() { }

    void update();
    void updateCanvas();
    void updatePanGesture(const QPointF &location);
    void documentOffsetMoved(QPoint newOffset);

    KisSketchView* q;

    KisDoc2* doc;
    KisView2* view;
    KisCanvas2* canvas;
    KisDeclarativeCanvasItem *canvasItem;
};

void KisSketchView::Private::update()
{
    //
}



KisSketchView::KisSketchView(QDeclarativeItem* parent)
    : CanvasControllerDeclarative(parent)
    , d(new Private(this))
{
    KoZoomMode::setMinimumZoom(0.1);
    KoZoomMode::setMaximumZoom(16.0);

    // make sure we use the opengl canvas
    KisConfig cfg;
    cfg.setUseOpenGL(true);
    cfg.setUseOpenGLShaders(true);
    cfg.setUseOpenGLTrilinearFiltering(true);
}

KisSketchView::~KisSketchView()
{
    delete d;
}

QObject* KisSketchView::doc() const
{
    return d->doc;
}

void KisSketchView::createDocument()
{
    KPluginFactory* factory = KLibLoader::self()->factory("kritapart");
    d->doc = static_cast<KisDoc2*>(factory->create(0, "KritaPart"));
    d->doc->newImage("test", 1000, 100, KoColorSpaceRegistry::instance()->rgb8());
    d->view = qobject_cast<KisView2*>(d->doc->createView(0));
    d->canvas = d->view->canvasBase();
    d->canvasItem = new KisDeclarativeCanvasItem(d->view->canvasBase());

    setCanvas(d->canvas);
    connect(d->canvas, SIGNAL(documentSize(QSizeF)), zoomController(), SLOT(setDocumentSize(QSizeF)));
    //d->canvas->updateSize();
    resetDocumentOffset();
    d->canvas->updateCanvas(QRectF(0, 0, width(), height()));

}

void KisSketchView::loadDocument()
{
/*
    emit progress(1);

    KisDoc2* doc = new KisDoc2();
    d->doc = doc;

    ProgressProxy *proxy = new ProgressProxy(this);
    doc->setProgressProxy(proxy);
    connect(proxy, SIGNAL(valueChanged(int)), SIGNAL(progress(int)));

    KMimeType::Ptr type = KMimeType::findByPath(file());
    QString path = file();

    if (type->name() != doc->nativeFormatMimeType()) {
        KoFilterManager *manager = new KoFilterManager(doc,  doc->progressUpdater());
        //manager->setBatchMode(true);
        KoFilter::ConversionStatus status;
        path = manager->importDocument(KUrl(file()).toLocalFile(), type->name(), status);
    }

    doc->openUrl(KUrl(path));

    setMargin(10);
    d->updateCanvas();

    QList<QTextDocument*> texts;
    KoFindText::findTextInShapes(d->canvas->shapeManager()->shapes(), texts);

    d->find = new KoFindText(texts, this);
    connect(d->find, SIGNAL(matchFound(KoFindMatch)), this, SLOT(matchFound(KoFindMatch)));
    connect(d->find, SIGNAL(updateCanvas()), this, SLOT(update()));

    emit progress(100);
    emit completed();
*/
}

void KisSketchView::onSingleTap( const QPointF& location)
{
    Q_UNUSED(location);
}

void KisSketchView::onDoubleTap ( const QPointF& location)
{
    Q_UNUSED(location);
    emit doubleTapped();
}

void KisSketchView::onLongTap ( const QPointF& location)
{
    Q_UNUSED(location);
}

void KisSketchView::onLongTapEnd(const QPointF &location)
{
    Q_UNUSED(location);
}

QPointF KisSketchView::documentToView(const QPointF& point)
{
//    return d->canvas->viewMode()->documentToView(point, d->canvas->viewConverter());
    return QPointF();
}

QPointF KisSketchView::viewToDocument(const QPointF& point)
{
//    return d->canvas->viewMode()->viewToDocument(point, d->canvas->viewConverter());
    return QPointF();
}

#include "KisSketchView.moc"
