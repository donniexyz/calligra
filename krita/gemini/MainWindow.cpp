/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2013 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "MainWindow.h"
#include "desktopviewproxy.h"
#include "ViewModeSwitchEvent.h"

#include "opengl/kis_opengl.h"

#include <QApplication>
#include <QResizeEvent>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QToolButton>
#include <QFileInfo>
#include <QGLWidget>

#include <kcmdlineargs.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kaboutdata.h>
#include <ktoolbar.h>
#include <kmessagebox.h>
#include <kmenubar.h>

#include <KoCanvasBase.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoMainWindow.h>
#include <KoGlobal.h>
#include <KoDocumentInfo.h>
#include <KoAbstractGradient.h>
#include <KoZoomController.h>

#include <kis_paintop_preset.h>
#include <kis_pattern.h>
#include <kis_config.h>
#include <kis_factory2.h>
#include <kis_doc2.h>
#include <kis_view2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_canvas_controller.h>

#include "sketch/SketchDeclarativeView.h"
#include "sketch/RecentFileManager.h"
#include "sketch/DocumentManager.h"
#include "sketch/KisSketchPart.h"

#ifdef Q_OS_WIN
// Slate mode/docked detection stuff
#include <shellapi.h>
#define SM_CONVERTIBLESLATEMODE 0x2003
#define SM_SYSTEMDOCKED         0x2004
#endif

class MainWindow::Private
{
public:
    Private(MainWindow* qq)
        : q(qq)
        , allowClose(true)
        , sketchView(0)
        , desktopView(0)
        , currentView(0)
        , slateMode(false)
        , docked(false)
        , sketchKisView(0)
        , desktopViewProxy(0)
        , forceDesktop(false)
        , forceSketch(false)
        , wasMaximized(true)
        , temporaryFile(false)
    {
#ifdef Q_OS_WIN
        slateMode = (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);
        docked = (GetSystemMetrics(SM_SYSTEMDOCKED) != 0);
#endif
}
    MainWindow* q;
    bool allowClose;
    SketchDeclarativeView* sketchView;
    KoMainWindow* desktopView;
    QObject* currentView;

    bool slateMode;
    bool docked;
    QString currentSketchPage;
    KisView2* sketchKisView;
    DesktopViewProxy* desktopViewProxy;

    bool forceDesktop;
    bool forceSketch;
    bool wasMaximized;
    bool temporaryFile;

    void initSketchView(QObject* parent)
    {
        sketchView = new SketchDeclarativeView();
        sketchView->engine()->rootContext()->setContextProperty("mainWindow", parent);

        QDir appdir(qApp->applicationDirPath());
        // for now, the app in bin/ and we still use the env.bat script
        appdir.cdUp();

        sketchView->engine()->addImportPath(appdir.canonicalPath() + "/lib/calligra/imports");
        sketchView->engine()->addImportPath(appdir.canonicalPath() + "/lib64/calligra/imports");
        QString mainqml = appdir.canonicalPath() + "/share/apps/kritagemini/kritagemini.qml";

        Q_ASSERT(QFile::exists(mainqml));
        if (!QFile::exists(mainqml)) {
            QMessageBox::warning(0, "No QML found", mainqml + " doesn't exist.");
        }
        QFileInfo fi(mainqml);

        sketchView->setSource(QUrl::fromLocalFile(fi.canonicalFilePath()));
        sketchView->setResizeMode( QDeclarativeView::SizeRootObjectToView );

        KAction* toDesktop = new KAction(q);
        toDesktop->setText(tr("Switch to Desktop"));
        connect(toDesktop, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchDesktopForced()));
        connect(toDesktop, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchToDesktop()));
        sketchView->engine()->rootContext()->setContextProperty("switchToDesktopAction", toDesktop);
    }

    void initDesktopView()
    {
        // Tell the iconloader about share/apps/calligra/icons
        KIconLoader::global()->addAppDir("calligra");
        // Initialize all Calligra directories etc.
        KoGlobal::initialize();

        desktopView = new KoMainWindow(KisFactory2::componentData());
        if (qgetenv("KDE_FULL_SESSION").isEmpty()) {
            // There are two themes that work for Krita, oxygen and plastique. Try to set plastique first, then oxygen
            qobject_cast<QApplication*>(QApplication::instance())->setStyle("Plastique");
            qobject_cast<QApplication*>(QApplication::instance())->setStyle("Oxygen");
        }

        KAction* toSketch = new KAction(desktopView);
        toSketch->setText(tr("Switch to Sketch"));
        toSketch->setIcon(QIcon::fromTheme("system-reboot"));
        toSketch->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        connect(toSketch, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchSketchForced()));
        connect(toSketch, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchToSketch()));
        desktopView->actionCollection()->addAction("SwitchToSketchView", toSketch);
        QToolButton* switcher = new QToolButton();
        switcher->setText(tr("Switch to Sketch"));
        switcher->setIcon(QIcon::fromTheme("system-reboot"));
        switcher->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(switcher, SIGNAL(clicked(bool)), q, SLOT(switchDesktopForced()));
        connect(switcher, SIGNAL(clicked(bool)), q, SLOT(switchToSketch()));
        desktopView->menuBar()->setCornerWidget(switcher);

        // DesktopViewProxy connects itself up to everything appropriate on construction,
        // and destroys itself again when the view is removed
        desktopViewProxy = new DesktopViewProxy(q, desktopView);
    }

    void notifySlateModeChange();
    void notifyDockingModeChange();
    bool queryClose();
};

MainWindow::MainWindow(QStringList fileNames, QWidget* parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags ), d( new Private(this) )
{
    qApp->setActiveWindow( this );

    setWindowTitle(i18n("Krita Gemini"));

    KisConfig cfg;
    cfg.setUseOpenGL(true);
    cfg.setCursorStyle(CURSOR_STYLE_NO_CURSOR);

    foreach(QString fileName, fileNames) {
        DocumentManager::instance()->recentFileManager()->addRecent(fileName);
    }

    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(documentChanged()));

    d->initSketchView(this);

    // Set the initial view to sketch... because reasons.
    // Really, this allows us to show the pleasant welcome screen from Sketch
    switchToSketch();
    d->wasMaximized = true;
}

void MainWindow::switchDesktopForced()
{
    if(d->slateMode)
        d->forceDesktop = true;
    d->forceSketch = false;
}

void MainWindow::switchSketchForced()
{
    if(!d->slateMode)
        d->forceSketch = true;
    d->forceDesktop = false;
}

void MainWindow::switchToSketch()
{
    QTime timer;
    timer.start();

    ViewModeSynchronisationObject* syncObject = new ViewModeSynchronisationObject;
    KisView2* view = 0;

    if(d->desktopView && centralWidget() == d->desktopView) {
        view = qobject_cast<KisView2*>(d->desktopView->rootView());

        //Notify the view we are switching away from that we are about to switch away from it
        //giving it the possibility to set up the synchronisation object.
        ViewModeSwitchEvent aboutToSwitchEvent(ViewModeSwitchEvent::AboutToSwitchViewModeEvent, view, d->sketchView, syncObject);
        QApplication::sendEvent(view, &aboutToSwitchEvent);

        d->desktopView->setParent(0);
        d->wasMaximized = isMaximized();

        //Notify the new view that we just switched to it, passing our synchronisation object
        //so it can use those values to sync with the old view.
        ViewModeSwitchEvent switchedEvent(ViewModeSwitchEvent::SwitchedToSketchModeEvent, view, d->sketchView, syncObject);
        QApplication::sendEvent(d->sketchView, &switchedEvent);
    }

    setCentralWidget(d->sketchView);
    if(d->slateMode)
        showFullScreen();
    emit switchedToSketch();
    qApp->processEvents();

    qDebug() << "milliseconds to switch to sketch:" << timer.elapsed();

    delete syncObject;
}

void MainWindow::switchToDesktop(bool justLoaded)
{
    QTime timer;
    timer.start();

    if(d->currentSketchPage == "MainPage")
    {
        d->sketchView->setParent(0);
        setCentralWidget(d->desktopView);
    }

    if(d->wasMaximized)
        showMaximized();
    else
        showNormal();

    ViewModeSynchronisationObject* syncObject = new ViewModeSynchronisationObject;

    KisView2* view = 0;
    if(d->desktopView) {
        view = qobject_cast<KisView2*>(d->desktopView->rootView());
    }

    if(!justLoaded)
    {
        //Notify the view we are switching away from that we are about to switch away from it
        //giving it the possibility to set up the synchronisation object.
        ViewModeSwitchEvent aboutToSwitchEvent(ViewModeSwitchEvent::AboutToSwitchViewModeEvent, d->sketchView, view, syncObject);
        QApplication::sendEvent(d->sketchView, &aboutToSwitchEvent);
    }

    qApp->processEvents();

    if(view && !justLoaded) {
        QPoint center = view->rect().center();
        view->canvasController()->zoomIn(center);
        view->canvasController()->zoomOut(center);

        //Notify the new view that we just switched to it, passing our synchronisation object
        //so it can use those values to sync with the old view.
        ViewModeSwitchEvent switchedEvent(ViewModeSwitchEvent::SwitchedToDesktopModeEvent, d->sketchView, view, syncObject);
        QApplication::sendEvent(view, &switchedEvent);
    }
    else if(justLoaded)
    {
        QTimer::singleShot(0, this, SLOT(adjustZoomOnDocumentChangedAndStuff()));
    }

    qDebug() << "milliseconds to switch to desktop:" << timer.elapsed();
}

void MainWindow::adjustZoomOnDocumentChangedAndStuff()
{
    if(d->desktopView && centralWidget() == d->desktopView) {
        KisView2* view = qobject_cast<KisView2*>(d->desktopView->rootView());
        qApp->processEvents();
        view->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
        qApp->processEvents();
        QPoint center = view->rect().center();
        view->canvasControllerWidget()->zoomRelativeToPoint(center, 0.9);
    }
    else if(d->sketchKisView && centralWidget() == d->sketchView) {
        qApp->processEvents();
        d->sketchKisView->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
        qApp->processEvents();
        QPoint center = d->sketchKisView->rect().center();
        d->sketchKisView->canvasControllerWidget()->zoomRelativeToPoint(center, 0.9);
    }
}

void MainWindow::documentChanged()
{
    if(d->desktopView) {
        d->desktopView->setNoCleanup(true);
        d->desktopView->deleteLater();
        d->desktopView = 0;
    }
    d->initDesktopView();
    d->desktopView->setRootDocument(DocumentManager::instance()->document(), DocumentManager::instance()->part(), false);
    qApp->processEvents();
    if(!d->forceSketch && !d->slateMode)
        switchToDesktop(true);
}

bool MainWindow::allowClose() const
{
    return d->allowClose;
}

void MainWindow::setAllowClose(bool allow)
{
    d->allowClose = allow;
}

bool MainWindow::slateMode() const
{
    return d->slateMode;
}

QString MainWindow::currentSketchPage() const
{
    return d->currentSketchPage;
}

void MainWindow::setCurrentSketchPage(QString newPage)
{
    d->currentSketchPage = newPage;
    emit currentSketchPageChanged();

    if(newPage == "MainPage")
    {
        if(!d->forceSketch && !d->slateMode)
        {
            // Just loaded to desktop, do nothing
        }
        else
        {
            QTimer::singleShot(2000, this, SLOT(adjustZoomOnDocumentChangedAndStuff()));
        }
    }
}

bool MainWindow::temporaryFile() const
{
    return d->temporaryFile;
}

void MainWindow::setTemporaryFile(bool newValue)
{
    d->temporaryFile = newValue;
    emit temporaryFileChanged();
}

QObject* MainWindow::sketchKisView() const
{
    return d->sketchKisView;
}

void MainWindow::setSketchKisView(QObject* newView)
{
    if(d->sketchKisView != newView)
    {
        d->sketchKisView = qobject_cast<KisView2*>(newView);
        emit sketchKisViewChanged();
    }
}

void MainWindow::minimize()
{
    setWindowState(windowState() ^ Qt::WindowMinimized);
}

void MainWindow::closeWindow()
{
    d->desktopView->setNoCleanup(true);
    //For some reason, close() does not work even if setAllowClose(true) was called just before this method.
    //So instead just completely quit the application, since we are using a single window anyway.
    DocumentManager::instance()->closeDocument();
    DocumentManager::instance()->part()->deleteLater();
    qApp->processEvents();
    QApplication::instance()->quit();
}

bool MainWindow::Private::queryClose()
{
    desktopView->setNoCleanup(true);
    if (DocumentManager::instance()->document() == 0)
        return true;

    // main doc + internally stored child documents
    if (DocumentManager::instance()->document()->isModified()) {
        QString name;
        if (DocumentManager::instance()->document()->documentInfo()) {
            name = DocumentManager::instance()->document()->documentInfo()->aboutInfo("title");
        }
        if (name.isEmpty())
            name = DocumentManager::instance()->document()->url().fileName();

        if (name.isEmpty())
            name = i18n("Untitled");

        int res = KMessageBox::warningYesNoCancel(q,
                  i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name),
                  QString(),
                  KStandardGuiItem::save(),
                  KStandardGuiItem::discard());

        switch (res) {
        case KMessageBox::Yes : {
            if (temporaryFile && !desktopViewProxy->fileSaveAs())
                return false;
            if (!DocumentManager::instance()->save())
                return false;
            break;
        }
        case KMessageBox::No :
            DocumentManager::instance()->document()->removeAutoSaveFiles();
            DocumentManager::instance()->document()->setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        default : // case KMessageBox::Cancel :
            return false;
        }
    }

    return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if(centralWidget() == d->desktopView)
    {
        if(DocumentManager::instance()->document()->isLoading()) {
            event->ignore();
            return;
        }
        d->allowClose = d->queryClose();
    }

    if(d->allowClose)
    {
        if(d->desktopView)
        {
            d->desktopView->setNoCleanup(true);
            d->desktopView->close();
        }
        event->accept();
    }
    else
    {
        event->ignore();
        emit closeRequested();
    }
}

MainWindow::~MainWindow()
{
    delete d;
}

#ifdef Q_OS_WIN
bool MainWindow::winEvent( MSG * message, long * result )
{
    if(message->message == WM_SETTINGCHANGE)
    {
        if(wcscmp(TEXT("ConvertibleSlateMode"), (TCHAR *) message->lParam) == 0)
            d->notifySlateModeChange();
        else if(wcscmp(TEXT("SystemDockMode"), (TCHAR *) message->lParam) == 0)
            d->notifyDockingModeChange();
        *result = 0;
        return true;
    }
    return false;
}
#endif

void MainWindow::Private::notifySlateModeChange()
{
#ifdef Q_OS_WIN
    bool bSlateMode = (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);

    if (slateMode != bSlateMode)
    {
        slateMode = bSlateMode;
        emit q->slateModeChanged();
        if(forceSketch || (slateMode && !forceDesktop))
            q->switchToSketch();
        else
            q->switchToDesktop();
        qDebug() << "Slate mode is now" << slateMode;
    } 
#endif
}

void MainWindow::Private::notifyDockingModeChange()
{
#ifdef Q_OS_WIN
    bool bDocked = (GetSystemMetrics(SM_SYSTEMDOCKED) != 0);

    if (docked != bDocked)
    {
        docked = bDocked;
        qDebug() << "Docking mode is now" << docked;
    }
#endif
}

void MainWindow::cloneResources(KisCanvasResourceProvider *from, KisCanvasResourceProvider *to)
{
    to->setBGColor(from->bgColor());
    to->setFGColor(from->fgColor());
    to->setHDRExposure(from->HDRExposure());
    to->setHDRGamma(from->HDRGamma());
    to->setCurrentCompositeOp(from->currentCompositeOp());
    to->slotPatternActivated(from->currentPattern());
    to->slotGradientActivated(from->currentGradient());
    to->slotNodeActivated(from->currentNode());
    to->setPaintOpPreset(from->currentPreset());
    to->setOpacity(from->opacity());
    to->setGlobalAlphaLock(from->globalAlphaLock());

}

#include "MainWindow.moc"