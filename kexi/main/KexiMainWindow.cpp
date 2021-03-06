/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KexiMainWindow.h"
#include <config-kexi.h>
#include <unistd.h>

#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <QToolButton>

#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QFocusEvent>
#include <QTextStream>
#include <QEvent>
#include <QKeyEvent>
#include <QHash>
#include <QDockWidget>
#include <QMenuBar>
#include <QShortcut>
#include <QStylePainter>
#include <QScopedPointer>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardshortcut.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <ktogglefullscreenaction.h>

#include <kiconloader.h>
#include <khelpmenu.h>
#include <kfiledialog.h>
#include <kmultitabbar.h>

#include <db/connection.h>
#include <db/utils.h>
#include <db/cursor.h>
#include <db/admin.h>
#include <db/drivermanager.h>
#include <kexidb/dbobjectnamevalidator.h>
#include <kexiutils/utils.h>
#include <kexiutils/styleproxy.h>
#include <kexiutils/KexiCloseButton.h>
#include <kexi_version.h>

#include <core/KexiWindow.h>
#include <core/KexiRecentProjects.h>

#include "kexiactionproxy.h"
#include "kexipartmanager.h"
#include "kexipart.h"
#include "kexipartinfo.h"
#include "kexipartguiclient.h"
#include "kexiproject.h"
#include "kexiprojectdata.h"
#include "kexi.h"
#include "kexistatusbar.h"
#include "kexiinternalpart.h"
#include "kexiactioncategories.h"
#include "kexifinddialog.h"
#include "kexisearchandreplaceiface.h"
#include "KexiBugReportDialog.h"

#include <kexi_global.h>

#include <widget/properties/KexiPropertyEditorView.h>
#include <widget/utils/kexirecordnavigator.h>
#include <widget/utils/KexiDockableWidget.h>
#include <widget/navigator/KexiProjectNavigator.h>
#include <widget/navigator/KexiProjectModel.h>
#include <widget/KexiFileWidget.h>
#include <widget/KexiNameDialog.h>
#include <widget/KexiNameWidget.h>
#include <migration/migratemanager.h>
#include <widget/KexiDBPasswordDialog.h>
#include <koproperty/EditorView.h>
#include <koproperty/Set.h>

#include "startup/KexiStartup.h"
#include "startup/KexiNewProjectAssistant.h"
#include "startup/KexiOpenProjectAssistant.h"
#include "startup/KexiWelcomeAssistant.h"
#include "startup/KexiImportExportAssistant.h"
#include "startup/KexiStartupDialog.h"

#include <KoIcon.h>

#if !defined(KexiVDebug)
# define KexiVDebug if (0) kDebug()
#endif

#undef HAVE_KNEWSTUFF
#ifdef HAVE_KNEWSTUFF
#include <knewstuff/downloaddialog.h>
#include "kexinewstuff.h"
#endif

//-------------------------------------------------

#include "KexiMainWindow_p.h"

//-------------------------------------------------

class KexiDockWidget::Private
{
public:
    Private() {}
    QSize hint;
    KexiCloseButton *closeButton;
};

KexiDockWidget::KexiDockWidget(const QString & title, QWidget *parent)
        : QDockWidget(title, parent), d(new Private)
{
    // No floatable dockers, Dolphin had problems, we don't want the same...
    // https://bugs.kde.org/show_bug.cgi?id=288629
    // https://bugs.kde.org/show_bug.cgi?id=322299
    setFeatures(QDockWidget::NoDockWidgetFeatures);//DockWidgetClosable);
    setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    setFocusPolicy(Qt::NoFocus);

    QStyleOptionDockWidgetV2 dockOpt;
    dockOpt.init(this);
    const int addSpacing = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, &dockOpt, this);

    QWidget *titleBar = new QWidget;
    titleBar->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *titleLyr = new QHBoxLayout(titleBar);
    titleLyr->setContentsMargins(addSpacing, addSpacing, addSpacing, addSpacing);
    titleLyr->setSpacing(0);
    titleLyr->addStretch(1);

    d->closeButton = new KexiCloseButton;
    d->closeButton->setMarginEnabled(false);
    connect(d->closeButton, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
    titleLyr->addWidget(d->closeButton);

    setTitleBarWidget(titleBar);
}

KexiDockWidget::~KexiDockWidget()
{
    delete d;
}

void KexiDockWidget::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    QStylePainter p(this);
    if (isFloating()) {
        QStyleOptionFrame framOpt;
        framOpt.init(this);
        p.drawPrimitive(QStyle::PE_FrameDockWidget, framOpt);
    }

    // Title must be painted after the frame, since the areas overlap, and
    // the title may wish to extend out to all sides (eg. XP style)
    QStyleOptionDockWidgetV2 titleOpt;
    initStyleOption(&titleOpt);
    p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
}

void KexiDockWidget::setSizeHint(const QSize& hint)
{
    d->hint = hint;
}

QSize KexiDockWidget::sizeHint() const
{
    return d->hint.isValid() ? d->hint : QDockWidget::sizeHint();
}

void KexiDockWidget::slotCloseClicked()
{
    if (isVisible()) {
        close();
    }
    else {
        show();
    }
}

QToolButton* KexiDockWidget::closeButton() const
{
    return d->closeButton;
}

//-------------------------------------------------

KexiMainWindowTabWidget::KexiMainWindowTabWidget(QWidget *parent, KexiMainWidget* mainWidget)
        : KTabWidget(parent)
        , m_mainWidget(mainWidget)
        , m_tabIndex(-1)
{
    m_closeAction = new KAction(koIcon("tab-close"), i18n("&Close Tab"), this);
    m_closeAction->setToolTip(i18n("Close the current tab"));
    m_closeAction->setWhatsThis(i18n("Closes the current tab."));
    m_closeAllTabsAction = new KAction(i18n("Cl&ose All Tabs"), this);
    m_closeAllTabsAction->setToolTip(i18n("Close all tabs"));
    m_closeAllTabsAction->setWhatsThis(i18n("Closes all tabs."));
    connect(m_closeAction, SIGNAL(triggered()), this, SLOT(closeTab()));
    connect(m_closeAllTabsAction, SIGNAL(triggered()), this, SLOT(closeAllTabs()));
//! @todo  insert window list in the corner widget as in firefox
#if 0
    // close-tab button:
    QToolButton* rightWidget = new QToolButton(this);
    rightWidget->setDefaultAction(m_closeAction);
    rightWidget->setText(QString());
    rightWidget->setAutoRaise(true);
    rightWidget->adjustSize();
    setCornerWidget(rightWidget, Qt::TopRightCorner);
#endif
    setMovable(true);
    setDocumentMode(true);
    tabBar()->setExpanding(true);
}

KexiMainWindowTabWidget::~KexiMainWindowTabWidget()
{
}

void KexiMainWindowTabWidget::paintEvent(QPaintEvent * event)
{
    if (count() > 0)
        KTabWidget::paintEvent(event);
    else
        QWidget::paintEvent(event);
}

void KexiMainWindowTabWidget::closeTab()
{
    dynamic_cast<KexiMainWindow*>(KexiMainWindowIface::global())->closeWindowForTab(m_tabIndex);
}

tristate KexiMainWindowTabWidget::closeAllTabs()
{
    tristate alternateResult = true;
    QList<KexiWindow*> windowList;
    for (int i = 0; i < count(); i++) {
        KexiWindow *window = dynamic_cast<KexiMainWindow*>(KexiMainWindowIface::global())->windowForTab(i);
        if (window) {
            windowList.append(window);
        }
    }
    foreach (KexiWindow *window, windowList) {
        tristate result = dynamic_cast<KexiMainWindow*>(KexiMainWindowIface::global())->closeWindow(window);
        if (result != true && result != false) {
            return result;
        }
        if (result == false) {
            alternateResult = false;
        }
    }
    return alternateResult;
}

void KexiMainWindowTabWidget::contextMenu(int index, const QPoint& point)
{
    QMenu menu;
    menu.addAction(m_closeAction);
    menu.addAction(m_closeAllTabsAction);
//! @todo add "&Detach Tab"
    setTabIndexFromContextMenu(index);
    menu.exec(point);
    KTabWidget::contextMenu(index, point);
}

void KexiMainWindowTabWidget::setTabIndexFromContextMenu(int clickedIndex)
{
    if (currentIndex() == -1) {
        m_tabIndex = -1;
        return;
    }
    m_tabIndex = clickedIndex;
}

//-------------------------------------------------

//static
int KexiMainWindow::create(int argc, char *argv[], const KAboutData &aboutData)
{
    Kexi::initCmdLineArgs(argc, argv, aboutData);

    bool GUIenabled = true;
//! @todo switch GUIenabled off when needed
    bool kappExisted = kapp;
    KApplication* app = kapp ? kapp : new KApplication(GUIenabled);

    KGlobal::locale()->insertCatalog("calligra");
    KGlobal::locale()->insertCatalog("koproperty");

    tristate res = Kexi::startupHandler().init(argc, argv);
    if (!res || ~res) {
        if (!kappExisted) {
            delete app;
        }
        return (~res) ? 0 : 1;
    }
    //kDebug() << "startupActions OK";

    /* Exit requested, e.g. after database removing. */
    if (Kexi::startupHandler().action() == KexiStartupData::Exit) {
        if (!kappExisted) {
            delete app;
        }
        return 0;
    }

    KexiMainWindow *win = new KexiMainWindow();
#ifdef KEXI_DEBUG_GUI
    QWidget* debugWindow = 0;
    if (GUIenabled) {
        KConfigGroup generalGroup = KGlobal::config()->group("General");
        if (generalGroup.readEntry("ShowInternalDebugger", false)) {
            debugWindow = KexiUtils::createDebugWindow(win);
            debugWindow->show();
        }
    }
#endif

    if (true != win->startup()) {
        delete win;
        if (!kappExisted) {
            delete app;
        }
        return 1;
    }

    win->restoreSettings();
    win->show();
#ifdef KEXI_DEBUG_GUI
    win->raise();
    static_cast<QWidget*>(win)->activateWindow();
#endif
    /*foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        kDebug() << widget;
    }*/
    return 0;
}

//-------------------------------------------------

KexiMainMenuActionShortcut::KexiMainMenuActionShortcut(const QKeySequence& key,
                                                       QWidget *parent, QAction *action)
    : QShortcut(key, parent)
    , m_action(action)
{
    connect(this, SIGNAL(activated()), this, SLOT(slotActivated()));
}

void KexiMainMenuActionShortcut::slotActivated()
{
    if (!m_action->isEnabled()) {
        return;
    }
    m_action->trigger();
}

//-------------------------------------------------

KexiMainWindow::KexiMainWindow(QWidget *parent)
        : KexiMainWindowSuper(parent)
        , KexiMainWindowIface()
        , KexiGUIMessageHandler(this)
        , d(new KexiMainWindow::Private(this))
{
    setObjectName("KexiMainWindow");
    setAttribute(Qt::WA_DeleteOnClose);
    kexiTester() << KexiTestObject(this);

    if (d->userMode)
        kDebug() << "starting up in the User Mode";

    setAsDefaultHost(); //this is default host now.
    KIconLoader *globalIconLoader = KIconLoader::global();
    globalIconLoader->addAppDir(QLatin1String("kexi"));
    globalIconLoader->addAppDir(QLatin1String("calligra"));

    //get informed
    connect(&Kexi::partManager(), SIGNAL(partLoaded(KexiPart::Part*)),
            this, SLOT(slotPartLoaded(KexiPart::Part*)));
    connect(&Kexi::partManager(), SIGNAL(newObjectRequested(KexiPart::Info*)),
            this, SLOT(newObject(KexiPart::Info*)));

    setAcceptDrops(true);
    setupActions();
    setupMainWidget();
    updateAppCaption();

    (void)KexiUtils::smallFont(this/*init*/);

    if (!d->userMode) {
        setupContextHelp();
        setupPropertyEditor();
    }

    d->tabbedToolBar->hideTab("form");//temporalily until createToolbar is split
    d->tabbedToolBar->hideTab("report");//temporalily until createToolbar is split

    invalidateActions();
    d->timer.singleShot(0, this, SLOT(slotLastActions()));
    if (Kexi::startupHandler().forcedFullScreen()) {
        toggleFullScreen(true);
    }

    // --- global config
    //! @todo move to specialized KexiConfig class
    KConfigGroup tablesGroup(d->config->group("Tables"));
    const int defaultMaxLengthForTextFields = tablesGroup.readEntry("DefaultMaxLengthForTextFields", int(-1));
    if (defaultMaxLengthForTextFields >= 0) {
        KexiDB::Field::setDefaultMaxLength(defaultMaxLengthForTextFields);
    }
    // --- /global config
}

KexiMainWindow::~KexiMainWindow()
{
    d->forceWindowClosing = true;
    closeProject();
    delete d;
    Kexi::deleteGlobalObjects();
}

KexiProject *KexiMainWindow::project()
{
    return d->prj;
}

QList<QAction*> KexiMainWindow::allActions() const
{
    return actionCollection()->actions();
}

KActionCollection *KexiMainWindow::actionCollection() const
{
    return d->actionCollection;
}

KexiWindow* KexiMainWindow::currentWindow() const
{
    return windowForTab(d->mainWidget->tabWidget()->currentIndex());
}

KexiWindow* KexiMainWindow::windowForTab(int tabIndex) const
{
    if (!d->mainWidget->tabWidget())
        return 0;
    KexiWindowContainer *windowContainer
        = dynamic_cast<KexiWindowContainer*>(d->mainWidget->tabWidget()->widget(tabIndex));
    if (!windowContainer)
        return 0;
    return windowContainer->window;
}

void KexiMainWindow::setupMainMenuActionShortcut(KAction* action)
{
    if (!action->shortcut().isEmpty()) {
        if (!action->shortcut().primary().isEmpty()) {
            (void)new KexiMainMenuActionShortcut(action->shortcut().primary(), this, action);
        }
        if (!action->shortcut().alternate().isEmpty()) {
            (void)new KexiMainMenuActionShortcut(action->shortcut().alternate(), this, action);
        }
    }
}

static void addThreeDotsToActionText(QAction* action)
{
    action->setText(i18nc("Action name with three dots...", "%1...", action->text()));
}

KAction* KexiMainWindow::addAction(const char *name, const KIcon &icon, const QString& text,
                                   const char *shortcut)
{
    KAction *action = icon.isNull() ? new KAction(text, this) : new KAction(icon, text, this);
    actionCollection()->addAction(name, action);
    if (shortcut) {
        action->setShortcut(QKeySequence(shortcut));
        QShortcut *s = new QShortcut(action->shortcut().primary(), this);
        connect(s, SIGNAL(activated()), action, SLOT(trigger()));
    }
    return action;
}

KAction* KexiMainWindow::addAction(const char *name, const QString& text, const char *shortcut)
{
    return addAction(name, KIcon(), text, shortcut);
}

void KexiMainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    // PROJECT MENU
    KAction *action;

    ac->addAction("project_new",
        action = new KexiMenuWidgetAction(KStandardAction::New, this));
    addThreeDotsToActionText(action);
    action->setShortcut(KStandardShortcut::openNew());
    action->setToolTip(i18n("Create a new project"));
    action->setWhatsThis(
        i18n("Creates a new project. Currently opened project is not affected."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectNew()));
    setupMainMenuActionShortcut(action);

    ac->addAction("project_open",
            action = new KexiMenuWidgetAction(KStandardAction::Open, this));
    action->setToolTip(i18n("Open an existing project"));
    action->setWhatsThis(
        i18n("Opens an existing project. Currently opened project is not affected."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectOpen()));
    setupMainMenuActionShortcut(action);

#ifdef HAVE_KNEWSTUFF
    action = addAction("project_download_examples", koIcon("go-down"),
                       futureI18n("&Download Example Databases..."));
    action->setToolTip(futureI18n("Download example databases from the Internet"));
    action->setWhatsThis(futureI18n("Downloads example databases from the Internet."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotGetNewStuff()));
#endif

    {
        ac->addAction("project_welcome",
            action = d->action_project_welcome = new KexiMenuWidgetAction(
                KIcon(), i18n("Welcome"), this));
            addThreeDotsToActionText(action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotProjectWelcome()));
        setupMainMenuActionShortcut(action);
        action->setToolTip(i18n("Show Welcome page"));
        action->setWhatsThis(
            i18n("Shows Welcome page with list of recently opened projects and other information. "));
    }

    ac->addAction("project_save",
                  d->action_save = KStandardAction::save(this, SLOT(slotProjectSave()), this));
    d->action_save->setToolTip(i18n("Save object changes"));
    d->action_save->setWhatsThis(i18n("Saves object changes from currently selected window."));
    setupMainMenuActionShortcut(d->action_save);

    d->action_save_as = addAction("project_saveas", koIcon("document-save-as"),
                                  i18n("Save &As..."));
    d->action_save_as->setToolTip(i18n("Save object as"));
    d->action_save_as->setWhatsThis(
        i18n("Saves object from currently selected window under a new name (within the same project)."));
    connect(d->action_save_as, SIGNAL(triggered()), this, SLOT(slotProjectSaveAs()));

#ifdef KEXI_SHOW_UNIMPLEMENTED
    ac->addAction("project_properties",
        action = d->action_project_properties = new KexiMenuWidgetAction(
            koIcon("document-properties"), futureI18n("Project Properties"), this));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectProperties()));
    setupMainMenuActionShortcut(action);
#else
    d->action_project_properties = d->dummy_action;
#endif

    //! @todo replace document-import icon with something other
    ac->addAction("project_import_export_send",
        action = d->action_project_import_export_send = new KexiMenuWidgetAction(
            koIcon("document-import"), i18n("&Import, Export or Send..."), this));
    action->setToolTip(i18n("Import, export or send project"));
    action->setWhatsThis(
        i18n("Imports, exports or sends project."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectImportExportOrSend()));
    setupMainMenuActionShortcut(action);

    ac->addAction("project_close",
        action = d->action_close = new KexiMenuWidgetAction(
            koIcon("window-close"), i18nc("Close Project", "&Close"), this));
    action->setToolTip(i18n("Close the current project"));
    action->setWhatsThis(i18n("Closes the current project."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectClose()));
    setupMainMenuActionShortcut(action);

    ac->addAction("quit",
                  action = new KexiMenuWidgetAction(KStandardAction::Quit, this));
    connect(action, SIGNAL(triggered()), this, SLOT(slotProjectQuit()));
    action->setWhatsThis(i18n("Quits Kexi application."));
    setupMainMenuActionShortcut(action);

#ifdef KEXI_SHOW_UNIMPLEMENTED
    d->action_project_relations = addAction("project_relations", koIcon("relation"),
                                            futureI18n("&Relationships..."), "Ctrl+R");
    d->action_project_relations->setToolTip(futureI18n("Project relationships"));
    d->action_project_relations->setWhatsThis(futureI18n("Shows project relationships."));
    connect(d->action_project_relations, SIGNAL(triggered()),
            this, SLOT(slotProjectRelations()));

#else
    d->action_project_relations = d->dummy_action;
#endif
    d->action_tools_import_project = addAction("tools_import_project", KexiIcon(koIconName("database-import")),
                                               i18n("&Import Database..."));
    d->action_tools_import_project->setToolTip(i18n("Import entire database as a Kexi project"));
    d->action_tools_import_project->setWhatsThis(
        i18n("Imports entire database as a Kexi project."));
    connect(d->action_tools_import_project, SIGNAL(triggered()),
            this, SLOT(slotToolsImportProject()));

    d->action_tools_data_import = addAction("tools_import_tables", koIcon("document-import"),
                                            i18n("Import Tables"));
    d->action_tools_data_import->setToolTip(i18n("Import data from an external source into this project"));
    d->action_tools_data_import->setWhatsThis(i18n("Imports data from an external source into this project."));
    connect(d->action_tools_data_import, SIGNAL(triggered()), this, SLOT(slotToolsImportTables()));

    d->action_tools_compact_database = addAction("tools_compact_database",
//! @todo icon
                                                 koIcon("application-x-compress"),
                                                 i18n("&Compact Database..."));
    d->action_tools_compact_database->setToolTip(i18n("Compact the current database project"));
    d->action_tools_compact_database->setWhatsThis(
        i18n("Compacts the current database project, so it will take less space and work faster."));
    connect(d->action_tools_compact_database, SIGNAL(triggered()),
            this, SLOT(slotToolsCompactDatabase()));

    if (d->userMode)
        d->action_project_import_data_table = 0;
    else {
        d->action_project_import_data_table = addAction("project_import_data_table",
            KexiIcon(koIconName("kexi-document-empty")),
            /*! @todo: change to "file_import" with a table or so */
            i18nc("Import->Table Data From File...", "Import Data From &File..."));
        d->action_project_import_data_table->setToolTip(i18n("Import table data from a file"));
        d->action_project_import_data_table->setWhatsThis(i18n("Imports table data from a file."));
        connect(d->action_project_import_data_table, SIGNAL(triggered()),
                this, SLOT(slotProjectImportDataTable()));
    }

    d->action_project_export_data_table = addAction("project_export_data_table",
        koIcon("table"),
        /*! @todo: change to "file_export" with a table or so */
        i18nc("Export->Table or Query Data to File...", "Export Data to &File..."));
    d->action_project_export_data_table->setToolTip(
        i18n("Export data from the active table or query to a file"));
    d->action_project_export_data_table->setWhatsThis(
        i18n("Exports data from the active table or query to a file."));
    connect(d->action_project_export_data_table, SIGNAL(triggered()),
            this, SLOT(slotProjectExportDataTable()));

//! @todo new KAction(i18n("From File..."), "document-open", 0,
//!          this, SLOT(slotImportFile()), actionCollection(), "project_import_file");
//! @todo new KAction(i18n("From Server..."), "network-server-database", 0,
//!          this, SLOT(slotImportServer()), actionCollection(), "project_import_server");

#ifndef KEXI_NO_QUICK_PRINTING
    ac->addAction("project_print",
                  d->action_project_print = KStandardAction::print(this, SLOT(slotProjectPrint()), this));
    d->action_project_print->setToolTip(futureI18n("Print data from the active table or query"));
    d->action_project_print->setWhatsThis(futureI18n("Prints data from the active table or query."));

    ac->addAction("project_print_preview",
                  d->action_project_print_preview = KStandardAction::printPreview(
                                                        this, SLOT(slotProjectPrintPreview()), this));
    d->action_project_print_preview->setToolTip(
        futureI18n("Show print preview for the active table or query"));
    d->action_project_print_preview->setWhatsThis(
        futureI18n("Shows print preview for the active table or query."));

    d->action_project_print_setup = addAction("project_print_setup",
        koIcon("document-page-setup"), futureI18n("Print Set&up..."));
    d->action_project_print_setup->setToolTip(
        futureI18n("Show print setup for the active table or query"));
    d->action_project_print_setup->setWhatsThis(
        futureI18n("Shows print setup for the active table or query."));
    connect(d->action_project_print_setup, SIGNAL(triggered()),
            this, SLOT(slotProjectPageSetup()));
#endif

    //EDIT MENU
    d->action_edit_cut = createSharedAction(KStandardAction::Cut, "edit_cut");
    d->action_edit_copy = createSharedAction(KStandardAction::Copy, "edit_copy");
    d->action_edit_paste = createSharedAction(KStandardAction::Paste, "edit_paste");

    if (d->userMode)
        d->action_edit_paste_special_data_table = 0;
    else {
        d->action_edit_paste_special_data_table = addAction(
            "edit_paste_special_data_table",
            KIcon(d->action_edit_paste->icon()), i18nc("Paste Special->As Data &Table...", "Paste Special..."));
        d->action_edit_paste_special_data_table->setToolTip(
            i18n("Paste clipboard data as a table"));
        d->action_edit_paste_special_data_table->setWhatsThis(
            i18n("Pastes clipboard data as a table."));
        connect(d->action_edit_paste_special_data_table, SIGNAL(triggered()),
                this, SLOT(slotEditPasteSpecialDataTable()));
    }

    d->action_edit_copy_special_data_table = addAction(
        "edit_copy_special_data_table",
        koIcon("table"), i18nc("Copy Special->Table or Query Data...", "Copy Special..."));
    d->action_edit_copy_special_data_table->setToolTip(
        i18n("Copy selected table or query data to clipboard"));
    d->action_edit_copy_special_data_table->setWhatsThis(
        i18n("Copies selected table or query data to clipboard."));
    connect(d->action_edit_copy_special_data_table, SIGNAL(triggered()),
            this, SLOT(slotEditCopySpecialDataTable()));

    d->action_edit_undo = createSharedAction(KStandardAction::Undo, "edit_undo");
    d->action_edit_undo->setWhatsThis(i18n("Reverts the most recent editing action."));
    d->action_edit_redo = createSharedAction(KStandardAction::Redo, "edit_redo");
    d->action_edit_redo->setWhatsThis(i18n("Reverts the most recent undo action."));

    ac->addAction("edit_find",
                  d->action_edit_find = KStandardAction::find(
                                            this, SLOT(slotEditFind()), this));
    d->action_edit_find->setToolTip(i18n("Find text"));
    d->action_edit_find->setWhatsThis(i18n("Looks up the first occurrence of a piece of text."));
    ac->addAction("edit_findnext",
                  d->action_edit_findnext = KStandardAction::findNext(
                                                this, SLOT(slotEditFindNext()), this));
    ac->addAction("edit_findprevious",
                  d->action_edit_findprev = KStandardAction::findPrev(
                                                this, SLOT(slotEditFindPrevious()), this));
    d->action_edit_replace = 0;
//! @todo d->action_edit_replace = KStandardAction::replace(
//!  this, SLOT(slotEditReplace()), actionCollection(), "project_print_preview" );
    d->action_edit_replace_all = 0;
//! @todo d->action_edit_replace_all = new KAction( i18n("Replace All"), "", 0,
//!   this, SLOT(slotEditReplaceAll()), actionCollection(), "edit_replaceall");

    d->action_edit_select_all =  createSharedAction(KStandardAction::SelectAll,
                                 "edit_select_all");

    d->action_edit_delete = createSharedAction(i18n("&Delete"), koIconName("edit-delete"),
                            KShortcut(), "edit_delete");
    d->action_edit_delete->setToolTip(i18n("Delete selected object"));
    d->action_edit_delete->setWhatsThis(i18n("Deletes currently selected object."));

    d->action_edit_delete_row = createSharedAction(i18n("Delete Record"), koIconName("delete_table_row"),
                                KShortcut(Qt::CTRL + Qt::Key_Delete), "edit_delete_row");
    d->action_edit_delete_row->setToolTip(i18n("Delete the current record"));
    d->action_edit_delete_row->setWhatsThis(i18n("Deletes the current record."));

    d->action_edit_clear_table = createSharedAction(i18n("Clear Table Contents"),
                                 koIconName("clear_table_contents"), KShortcut(), "edit_clear_table");
    d->action_edit_clear_table->setToolTip(i18n("Clear table contents"));
    d->action_edit_clear_table->setWhatsThis(i18n("Clears table contents."));
    setActionVolatile(d->action_edit_clear_table, true);

    d->action_edit_edititem = createSharedAction(i18n("Edit Item"), QString(),
                              KShortcut(), /* CONFLICT in TV: Qt::Key_F2,  */
                              "edit_edititem");
    d->action_edit_edititem->setToolTip(i18n("Edit currently selected item"));
    d->action_edit_edititem->setWhatsThis(i18n("Edits currently selected item."));

    d->action_edit_insert_empty_row = createSharedAction(i18n("&Insert Empty Row"),
                                      koIconName("insert_table_row"), KShortcut(Qt::SHIFT | Qt::CTRL | Qt::Key_Insert),
                                      "edit_insert_empty_row");
    setActionVolatile(d->action_edit_insert_empty_row, true);
    d->action_edit_insert_empty_row->setToolTip(i18n("Insert one empty row above"));
    d->action_edit_insert_empty_row->setWhatsThis(
        i18n("Inserts one empty row above currently selected table row."));

    //VIEW MENU
    /* UNUSED, see KexiToggleViewModeAction
      if (!d->userMode) {
        d->action_view_mode = new QActionGroup(this);
        ac->addAction( "view_data_mode",
          d->action_view_data_mode = new KToggleAction(
            koIcon("state_data"), i18n("&Data View"), d->action_view_mode) );
    //  d->action_view_data_mode->setObjectName("view_data_mode");
        d->action_view_data_mode->setShortcut(QKeySequence("F6"));
        //d->action_view_data_mode->setExclusiveGroup("view_mode");
        d->action_view_data_mode->setToolTip(i18n("Switch to data view"));
        d->action_view_data_mode->setWhatsThis(i18n("Switches to data view."));
        d->actions_for_view_modes.insert( Kexi::DataViewMode, d->action_view_data_mode );
        connect(d->action_view_data_mode, SIGNAL(triggered()),
          this, SLOT(slotViewDataMode()));
      }
      else {
        d->action_view_mode = 0;
        d->action_view_data_mode = 0;
      }

      if (!d->userMode) {
        ac->addAction( "view_design_mode",
          d->action_view_design_mode = new KToggleAction(
            koIcon("state_edit"), i18n("D&esign View"), d->action_view_mode) );
    //  d->action_view_design_mode->setObjectName("view_design_mode");
        d->action_view_design_mode->setShortcut(QKeySequence("F7"));
        //d->action_view_design_mode->setExclusiveGroup("view_mode");
        d->action_view_design_mode->setToolTip(i18n("Switch to design view"));
        d->action_view_design_mode->setWhatsThis(i18n("Switches to design view."));
        d->actions_for_view_modes.insert( Kexi::DesignViewMode, d->action_view_design_mode );
        connect(d->action_view_design_mode, SIGNAL(triggered()),
          this, SLOT(slotViewDesignMode()));
      }
      else
        d->action_view_design_mode = 0;

      if (!d->userMode) {
        ac->addAction( "view_text_mode",
          d->action_view_text_mode = new KToggleAction(
            koIcon("state_sql"), i18n("&Text View"), d->action_view_mode) );
        d->action_view_text_mode->setObjectName("view_text_mode");
        d->action_view_text_mode->setShortcut(QKeySequence("F8"));
        //d->action_view_text_mode->setExclusiveGroup("view_mode");
        d->action_view_text_mode->setToolTip(i18n("Switch to text view"));
        d->action_view_text_mode->setWhatsThis(i18n("Switches to text view."));
        d->actions_for_view_modes.insert( Kexi::TextViewMode, d->action_view_text_mode );
        connect(d->action_view_text_mode, SIGNAL(triggered()),
          this, SLOT(slotViewTextMode()));
      }
      else
        d->action_view_text_mode = 0;
    */
    if (d->isProjectNavigatorVisible) {
        d->action_view_nav = addAction("view_navigator",
                                       i18n("Switch to Project Navigator"),
                                       "Alt+1");
        d->action_view_nav->setToolTip(i18n("Switch to Project Navigator panel"));
        d->action_view_nav->setWhatsThis(i18n("Switches to Project Navigator panel."));
        connect(d->action_view_nav, SIGNAL(triggered()),
                this, SLOT(slotViewNavigator()));
    } else
        d->action_view_nav = 0;

    d->action_view_mainarea = addAction("view_mainarea",
                                        i18n("Switch to Main Area"), "Alt+2");
    d->action_view_mainarea->setToolTip(i18n("Switch to main area"));
    d->action_view_mainarea->setWhatsThis(i18n("Switches to main area."));
    connect(d->action_view_mainarea, SIGNAL(triggered()),
            this, SLOT(slotViewMainArea()));

    if (!d->userMode) {
        d->action_view_propeditor = addAction("view_propeditor",
                                              i18n("Switch to Property Editor"), "Alt+3");
        d->action_view_propeditor->setToolTip(i18n("Switch to Property Editor panel"));
        d->action_view_propeditor->setWhatsThis(i18n("Switches to Property Editor panel."));
        connect(d->action_view_propeditor, SIGNAL(triggered()),
                this, SLOT(slotViewPropertyEditor()));
    } else
        d->action_view_propeditor = 0;

    d->action_view_global_search = addAction("view_global_search",
                                             i18n("Switch to Global Search"), "Ctrl+K");
    d->action_view_global_search->setToolTip(i18n("Switch to Global Search box"));
    d->action_view_global_search->setWhatsThis(i18n("Switches to Global Search box."));
    // (connection is added elsewhere)

    //DATA MENU
    d->action_data_save_row = createSharedAction(i18n("&Save Record"), koIconName("dialog-ok"),
                              KShortcut(Qt::SHIFT + Qt::Key_Return), "data_save_row");
    d->action_data_save_row->setToolTip(i18n("Save changes made to the current record"));
    d->action_data_save_row->setWhatsThis(i18n("Saves changes made to the current record."));
//temp. disable because of problems with volatile actions setActionVolatile( d->action_data_save_row, true );

    d->action_data_cancel_row_changes = createSharedAction(i18n("&Cancel Record Changes"),
                                        koIconName("dialog-cancel"), KShortcut(Qt::Key_Escape), "data_cancel_row_changes");
    d->action_data_cancel_row_changes->setToolTip(
        i18n("Cancel changes made to the current record"));
    d->action_data_cancel_row_changes->setWhatsThis(
        i18n("Cancels changes made to the current record."));
//temp. disable because of problems with volatile actions setActionVolatile( d->action_data_cancel_row_changes, true );

    d->action_data_execute = createSharedAction(
                                 i18n("&Execute"), koIconName("media-playback-start"), KShortcut(), "data_execute");
    //! @todo d->action_data_execute->setToolTip(i18n(""));
    //! @todo d->action_data_execute->setWhatsThis(i18n(""));

#ifdef KEXI_SHOW_UNIMPLEMENTED
    action = createSharedAction(futureI18n("&Filter"), koIconName("view-filter"), KShortcut(), "data_filter");
    setActionVolatile(action, true);
#endif
//! @todo action->setToolTip(i18n(""));
//! @todo action->setWhatsThis(i18n(""));

    // - record-navigation related actions
    createSharedAction(KexiRecordNavigator::Actions::moveToFirstRecord(), KShortcut(), "data_go_to_first_record");
    createSharedAction(KexiRecordNavigator::Actions::moveToPreviousRecord(), KShortcut(), "data_go_to_previous_record");
    createSharedAction(KexiRecordNavigator::Actions::moveToNextRecord(), KShortcut(), "data_go_to_next_record");
    createSharedAction(KexiRecordNavigator::Actions::moveToLastRecord(), KShortcut(), "data_go_to_last_record");
    createSharedAction(KexiRecordNavigator::Actions::moveToNewRecord(), KShortcut(), "data_go_to_new_record");

    //FORMAT MENU
    d->action_format_font = createSharedAction(i18n("&Font..."), koIconNameWanted("change font of selected object","fonts"),
                            KShortcut(), "format_font");
    d->action_format_font->setToolTip(i18n("Change font for selected object"));
    d->action_format_font->setWhatsThis(i18n("Changes font for selected object."));

    //TOOLS MENU

    //WINDOW MENU
    //additional 'Window' menu items
    d->action_window_next = addAction("window_next",
                                      i18n("&Next Window"),
#ifdef Q_WS_WIN
        "Ctrl+Tab"
#else
        "Alt+Right"
#endif
    );
    d->action_window_next->setToolTip(i18n("Next window"));
    d->action_window_next->setWhatsThis(i18n("Switches to the next window."));
    connect(d->action_window_next, SIGNAL(triggered()),
            this, SLOT(activateNextWindow()));

    d->action_window_previous = addAction("window_previous",
                                          i18n("&Previous Window"),
#ifdef Q_WS_WIN
        "Ctrl+Shift+Tab"
#else
        "Alt+Left"
#endif
    );
    d->action_window_previous->setToolTip(i18n("Previous window"));
    d->action_window_previous->setWhatsThis(i18n("Switches to the previous window."));
    connect(d->action_window_previous, SIGNAL(triggered()),
            this, SLOT(activatePreviousWindow()));

    d->action_window_fullscreen = KStandardAction::fullScreen(this, SLOT(toggleFullScreen(bool)), this, ac);
    ac->addAction("full_screen", d->action_window_fullscreen);
    QList<QKeySequence> shortcuts;
    KShortcut shortcut(d->action_window_fullscreen->shortcut().primary(), QKeySequence("F11"));
    shortcuts = shortcut.toList();
    d->action_window_fullscreen->setShortcuts(shortcuts);
    QShortcut *s = new QShortcut(d->action_window_fullscreen->shortcut().primary(), this);
    connect(s, SIGNAL(activated()), d->action_window_fullscreen, SLOT(trigger()));
    QShortcut *sa = new QShortcut(d->action_window_fullscreen->shortcut().alternate(), this);
    connect(sa, SIGNAL(activated()), d->action_window_fullscreen, SLOT(trigger()));

    //SETTINGS MENU
//! @todo put 'configure keys' into settings view

#ifdef KEXI_SHOW_UNIMPLEMENTED
    //! @todo toolbars configuration will be handled in a special way
#endif

#ifdef KEXI_MACROS_SUPPORT
    Kexi::tempShowMacros() = true;
#else
    Kexi::tempShowMacros() = false;
#endif

#ifdef KEXI_SCRIPTS_SUPPORT
    Kexi::tempShowScripts() = true;
#else
    Kexi::tempShowScripts() = false;
#endif

#ifdef KEXI_SHOW_UNIMPLEMENTED
//! @todo implement settings window in a specific way
    ac->addAction("settings",
                  action = d->action_settings = new KexiMenuWidgetAction(
                    KStandardAction::Preferences, this));
    action->setObjectName("settings");
    action->setText(futureI18n("Settings..."));
    action->setToolTip(futureI18n("Show Kexi settings"));
    action->setWhatsThis(futureI18n("Shows Kexi settings."));
    connect(action, SIGNAL(triggered()), this, SLOT(slotSettings()));
    setupMainMenuActionShortcut(action);
#else
    d->action_settings = d->dummy_action;
#endif

//! @todo reenable 'tip of the day' later
#if 0
    KStandardAction::tipOfDay(this, SLOT(slotTipOfTheDayAction()), actionCollection())
    ->setWhatsThis(i18n("This shows useful tips on the use of this application."));
#endif

    // GLOBAL
    d->action_show_help_menu = addAction("help_show_menu", i18nc("Help Menu", "Help"), "Alt+H");
    d->action_show_help_menu->setToolTip(i18n("Show Help menu"));
    d->action_show_help_menu->setWhatsThis(i18n("Shows Help menu."));
    // (connection is added elsewhere)

    // ----- declare action categories, so form's "assign action to button"
    //       (and macros in the future) will be able to recognize category
    //       of actions and filter them -----------------------------------
//! @todo shouldn't we move this to core?
    Kexi::ActionCategories *acat = Kexi::actionCategories();
    acat->addAction("data_execute", Kexi::PartItemActionCategory);

    //! @todo unused for now
    acat->addWindowAction("data_filter",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addWindowAction("data_save_row",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addWindowAction("data_cancel_row_changes",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addWindowAction("delete_table_row",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    //! @todo support this in KexiPart::FormObjectType as well
    acat->addWindowAction("data_sort_az",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in KexiPart::FormObjectType as well
    acat->addWindowAction("data_sort_za",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in KexiPart::FormObjectType as well
    acat->addWindowAction("edit_clear_table",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in KexiPart::FormObjectType as well
    acat->addWindowAction("edit_copy_special_data_table",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in FormObjectType as well
    acat->addWindowAction("project_export_data_table",
                          KexiPart::TableObjectType, KexiPart::QueryObjectType);

    // GlobalActions, etc.
    acat->addAction("edit_copy", Kexi::GlobalActionCategory | Kexi::PartItemActionCategory);

    acat->addAction("edit_cut", Kexi::GlobalActionCategory | Kexi::PartItemActionCategory);

    acat->addAction("edit_paste", Kexi::GlobalActionCategory | Kexi::PartItemActionCategory);

    acat->addAction("edit_delete", Kexi::GlobalActionCategory | Kexi::PartItemActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_delete_row", Kexi::GlobalActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_edititem", Kexi::PartItemActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType);

    acat->addAction("edit_find", Kexi::GlobalActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_findnext", Kexi::GlobalActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_findprevious", Kexi::GlobalActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_replace", Kexi::GlobalActionCategory | Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    acat->addAction("edit_paste_special_data_table", Kexi::GlobalActionCategory);

    acat->addAction("help_about_app", Kexi::GlobalActionCategory);

    acat->addAction("help_about_kde", Kexi::GlobalActionCategory);

    acat->addAction("help_contents", Kexi::GlobalActionCategory);

    acat->addAction("help_report_bug", Kexi::GlobalActionCategory);

    acat->addAction("help_whats_this", Kexi::GlobalActionCategory);

    acat->addAction("options_configure_keybinding", Kexi::GlobalActionCategory);

    acat->addAction("project_close", Kexi::GlobalActionCategory);

    acat->addAction("project_import_data_table", Kexi::GlobalActionCategory);

    acat->addAction("project_new", Kexi::GlobalActionCategory);

    acat->addAction("project_open", Kexi::GlobalActionCategory);

#ifndef KEXI_NO_QUICK_PRINTING
    //! @todo support this in FormObjectType, ReportObjectType as well as others
    acat->addAction("project_print", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in FormObjectType, ReportObjectType as well as others
    acat->addAction("project_print_preview", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType);

    //! @todo support this in FormObjectType, ReportObjectType as well as others
    acat->addAction("project_print_setup", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType);
#endif

    acat->addAction("quit", Kexi::GlobalActionCategory);

    acat->addAction("tools_compact_database", Kexi::GlobalActionCategory);

    acat->addAction("tools_import_project", Kexi::GlobalActionCategory);

    acat->addAction("tools_import_tables", Kexi::GlobalActionCategory);

    acat->addAction("view_data_mode", Kexi::GlobalActionCategory);

    acat->addAction("view_design_mode", Kexi::GlobalActionCategory);

    acat->addAction("view_text_mode", Kexi::GlobalActionCategory);

    acat->addAction("view_mainarea", Kexi::GlobalActionCategory);

    acat->addAction("view_navigator", Kexi::GlobalActionCategory);

    acat->addAction("view_propeditor", Kexi::GlobalActionCategory);

    acat->addAction("window_close", Kexi::GlobalActionCategory | Kexi::WindowActionCategory);
    acat->setAllObjectTypesSupported("window_close", true);

    acat->addAction("window_next", Kexi::GlobalActionCategory);

    acat->addAction("window_previous", Kexi::GlobalActionCategory);

    acat->addAction("full_screen", Kexi::GlobalActionCategory);

    //skipped - design view only
    acat->addAction("format_font", Kexi::NoActionCategory);
    acat->addAction("project_save", Kexi::NoActionCategory);
    acat->addAction("edit_insert_empty_row", Kexi::NoActionCategory);
    //! @todo support this in KexiPart::TableObjectType, KexiPart::QueryObjectType later
    acat->addAction("edit_select_all", Kexi::NoActionCategory);
    //! @todo support this in KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType later
    acat->addAction("edit_redo", Kexi::NoActionCategory);
    //! @todo support this in KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType later
    acat->addAction("edit_undo", Kexi::NoActionCategory);

    //record-navigation related actions
    acat->addAction("data_go_to_first_record", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);
    acat->addAction("data_go_to_previous_record", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);
    acat->addAction("data_go_to_next_record", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);
    acat->addAction("data_go_to_last_record", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);
    acat->addAction("data_go_to_new_record", Kexi::WindowActionCategory,
                    KexiPart::TableObjectType, KexiPart::QueryObjectType, KexiPart::FormObjectType);

    //skipped - internal:
    acat->addAction("tablepart_create", Kexi::NoActionCategory);
    acat->addAction("querypart_create", Kexi::NoActionCategory);
    acat->addAction("formpart_create", Kexi::NoActionCategory);
    acat->addAction("reportpart_create", Kexi::NoActionCategory);
    acat->addAction("macropart_create", Kexi::NoActionCategory);
    acat->addAction("scriptpart_create", Kexi::NoActionCategory);
}

void KexiMainWindow::invalidateActions()
{
    invalidateProjectWideActions();
    invalidateSharedActions();
}

void KexiMainWindow::invalidateSharedActions(QObject *o)
{
    //! @todo enabling is more complex...
    /* d->action_edit_cut->setEnabled(true);
      d->action_edit_copy->setEnabled(true);
      d->action_edit_paste->setEnabled(true);*/

    if (!o)
        o = focusWindow();
    KexiSharedActionHost::invalidateSharedActions(o);
}

void KexiMainWindow::invalidateSharedActions()
{
    invalidateSharedActions(0);
}

// unused, I think
void KexiMainWindow::invalidateSharedActionsLater()
{
    QTimer::singleShot(1, this, SLOT(invalidateSharedActions()));
}

void KexiMainWindow::invalidateProjectWideActions()
{
    const bool has_window = currentWindow();
    const bool window_dirty = currentWindow() && currentWindow()->isDirty();
    const bool readOnly = d->prj && d->prj->dbConnection() && d->prj->dbConnection()->isReadOnly();

    //PROJECT MENU
    d->action_save->setEnabled(has_window && window_dirty && !readOnly);
    d->action_save_as->setEnabled(has_window && !readOnly);
    d->action_project_properties->setEnabled(d->prj);
    d->action_close->setEnabled(d->prj);
    d->action_project_relations->setEnabled(d->prj);

    //DATA MENU
    if (d->action_project_import_data_table)
        d->action_project_import_data_table->setEnabled(d->prj && !readOnly);
    if (d->action_tools_data_import)
        d->action_tools_data_import->setEnabled(d->prj && !readOnly);
    d->action_project_export_data_table->setEnabled(
        currentWindow() && currentWindow()->part()->info()->isDataExportSupported());
    if (d->action_edit_paste_special_data_table)
        d->action_edit_paste_special_data_table->setEnabled(d->prj && !readOnly);

#ifndef KEXI_NO_QUICK_PRINTING
    const bool printingActionsEnabled =
        currentWindow() && currentWindow()->part()->info()->isPrintingSupported()
        && !currentWindow()->neverSaved();
    d->action_project_print->setEnabled(printingActionsEnabled);
    d->action_project_print_preview->setEnabled(printingActionsEnabled);
    d->action_project_print_setup->setEnabled(printingActionsEnabled);
#endif

    //EDIT MENU
//! @todo "copy special" is currently enabled only for data view mode;
//!  what about allowing it to enable in design view for "kexi/table" ?
    if (currentWindow() && currentWindow()->currentViewMode() == Kexi::DataViewMode) {
        KexiPart::Info *activePartInfo = currentWindow()->part()->info();
        d->action_edit_copy_special_data_table->setEnabled(
            activePartInfo ? activePartInfo->isDataExportSupported() : false);
    } else {
        d->action_edit_copy_special_data_table->setEnabled(false);
    }
    d->action_edit_find->setEnabled(d->prj);

    //VIEW MENU
    if (d->action_view_nav)
        d->action_view_nav->setEnabled(d->prj);
    d->action_view_mainarea->setEnabled(d->prj);
    if (d->action_view_propeditor)
        d->action_view_propeditor->setEnabled(d->prj);
#ifndef KEXI_NO_CTXT_HELP
    d->action_show_helper->setEnabled(d->prj);
#endif

    //CREATE MENU
    if (d->tabbedToolBar && d->tabbedToolBar->createWidgetToolBar())
        d->tabbedToolBar->createWidgetToolBar()->setEnabled(d->prj);

    // DATA MENU

    //TOOLS MENU
    // "compact db" supported if there's no db or the current db supports compacting and is opened r/w:
    d->action_tools_compact_database->setEnabled(
        !d->prj
        || (!readOnly && d->prj && d->prj->dbConnection()
            && (d->prj->dbConnection()->driver()->features() & KexiDB::Driver::CompactingDatabaseSupported))
    );

    //DOCKS
    if (d->navigator) {
        d->navigator->setEnabled(d->prj);
    }
    if (d->propEditor)
        d->propEditorTabWidget->setEnabled(d->prj);
}

tristate KexiMainWindow::startup()
{
    tristate result = true;
    switch (Kexi::startupHandler().action()) {
    case KexiStartupHandler::CreateBlankProject:
        d->updatePropEditorVisibility(Kexi::NoViewMode);
        break;
    case KexiStartupHandler::CreateFromTemplate:
        result = createProjectFromTemplate(*Kexi::startupHandler().projectData());
        break;
    case KexiStartupHandler::OpenProject:
        result = openProject(*Kexi::startupHandler().projectData());
        break;
    case KexiStartupHandler::ImportProject:
        result = showProjectMigrationWizard(
                   Kexi::startupHandler().importActionData().mimeType,
                   Kexi::startupHandler().importActionData().fileName
               );
        break;
    case KexiStartupHandler::ShowWelcomeScreen:
        //! @todo show welcome screen as soon as is available
        QTimer::singleShot(1, this, SLOT(slotProjectWelcome()));
        break;
    default:
        d->updatePropEditorVisibility(Kexi::NoViewMode);
    }
    return result;
}

static QString internalReason(KexiDB::Object *obj)
{
    const QString &s = obj->errorMsg();
    if (s.isEmpty())
        return s;
    return QString("<br>(%1) ").arg(i18n("reason:") + " <i>" + s + "</i>");
}

tristate KexiMainWindow::openProject(const KexiProjectData& projectData)
{
    kDebug() << projectData;
    QScopedPointer<KexiProject> prj(createKexiProjectObject(projectData));
    if (~KexiDBPasswordDialog::getPasswordIfNeeded(prj->data()->connectionData(), this)) {
        return cancelled;
    }
    bool incompatibleWithKexi;
    tristate res = prj->open(&incompatibleWithKexi);

    if (prj->data()->connectionData()->passwordNeeded()) {
        // password was supplied in this session, and shouldn't be stored or reused afterwards,
        // so let's remove it
        prj->data()->connectionData()->password.clear();
    }

    if (~res) {
        return cancelled;
    }
    else if (!res) {
        if (incompatibleWithKexi) {
            if (KMessageBox::Yes == KMessageBox::questionYesNo(this,
                    i18nc("@info",
                         "Database project <resource>%1</resource> does not appear to have been created using Kexi.<nl/>"
                         "Do you want to import it as a new Kexi project?",
                         projectData.infoString()),
                    QString(), KGuiItem(i18nc("Import Database", "&Import..."), koIconName("database_import")),
                    KStandardGuiItem::cancel()))
            {
                const bool anotherProjectAlreadyOpened = prj;
                tristate res = showProjectMigrationWizard("application/x-kexi-connectiondata",
                               projectData.databaseName(), projectData.constConnectionData());

                if (!anotherProjectAlreadyOpened) //the project could have been opened within this instance
                    return res;

                //always return cancelled because even if migration succeeded, new Kexi instance
                //will be started if user wanted to open the imported db
                return cancelled;
            }
            return cancelled;
        }
        return false;
    }

    // success
    d->prj = prj.take();
    setupProjectNavigator();
    d->prj->data()->setLastOpened(QDateTime::currentDateTime());
    Kexi::recentProjects()->addProjectData(*d->prj->data());
    updateReadOnlyState();
    invalidateActions();
    enableMessages(false);

    QTimer::singleShot(1, this, SLOT(slotAutoOpenObjectsLater()));
    d->tabbedToolBar->showTab("create");// not needed since create toolbar already shows toolbar! move when kexi starts
    d->tabbedToolBar->showTab("data");
    d->tabbedToolBar->showTab("external");
    d->tabbedToolBar->showTab("tools");
    d->tabbedToolBar->hideTab("form");//temporalily until createToolbar is split
    d->tabbedToolBar->hideTab("report");//temporalily until createToolbar is split

    // make sure any tab is activated
    d->tabbedToolBar->setCurrentTab(0);
    return true;
}

tristate KexiMainWindow::openProject(const KexiProjectData& data, const QString& shortcutPath,
                                     bool *opened)
{
    if (!shortcutPath.isEmpty() && d->prj) {
        const tristate result = openProjectInExternalKexiInstance(
            shortcutPath, QString(), QString());
        if (result == true) {
            *opened = true;
        }
        return result;
    }
    return openProject(data);
}

tristate KexiMainWindow::createProjectFromTemplate(const KexiProjectData& projectData)
{
    QStringList mimetypes;
    mimetypes.append(KexiDB::defaultFileBasedDriverMimeType());
    QString fname;
    const QString startDir("kfiledialog:///OpenExistingOrCreateNewProject"/*as in KexiNewProjectWizard*/);
    const QString caption(i18n("Select New Project's Location"));

    while (true) {
        if (fname.isEmpty() &&
                !projectData.constConnectionData()->dbFileName().isEmpty()) {
            //propose filename from db template name
            fname = projectData.constConnectionData()->dbFileName();
        }
        const bool specialDir = fname.isEmpty();
        kDebug() << fname << ".............";
        KFileDialog dlg(specialDir ? KUrl(startDir) : KUrl(),
                        QString(), this);
        dlg.setModal(true);
        dlg.setMimeFilter(mimetypes);
        if (!specialDir)
            dlg.setSelection(fname);   // may also be a filename
        dlg.setOperationMode(KFileDialog::Saving);
        dlg.setWindowTitle(caption);
        dlg.exec();
        fname = dlg.selectedFile();
        if (fname.isEmpty())
            return cancelled;
        if (KexiFileWidget::askForOverwriting(fname, this))
            break;
    }

    QFile sourceFile(projectData.constConnectionData()->fileName());
    if (!sourceFile.copy(fname)) {
//! @todo show error from with QFile::FileError
        return false;
    }

    return openProject(fname, 0, QString(), projectData.autoopenObjects/*copy*/);
}

void KexiMainWindow::updateReadOnlyState()
{
    const bool readOnly = d->prj && d->prj->dbConnection() && d->prj->dbConnection()->isReadOnly();
    d->statusBar->setReadOnlyFlag(readOnly);
    if (d->navigator) {
        d->navigator->setReadOnly(readOnly);
    }

    // update "insert ....." actions for every part
    KexiPart::PartInfoList *plist = Kexi::partManager().infoList();
    if (plist) {
        foreach(KexiPart::Info *info, *plist) {
            QAction *a = info->newObjectAction();
            if (a)
                a->setEnabled(!readOnly);
        }
    }
}

void KexiMainWindow::slotAutoOpenObjectsLater()
{
    QString not_found_msg;
    bool openingCancelled;
    //ok, now open "autoopen: objects
    if (d->prj) {
        foreach(KexiProjectData::ObjectInfo* info, d->prj->data()->autoopenObjects) {
            KexiPart::Info *i = Kexi::partManager().infoForClass(info->value("type"));
            if (!i) {
                not_found_msg += "<li>";
                if (!info->value("name").isEmpty())
                    not_found_msg += (QString("\"") + info->value("name") + "\" - ");
                if (info->value("action") == "new")
                    not_found_msg += i18n("cannot create object - unknown object type \"%1\"", info->value("type"));
                else
                    not_found_msg += i18n("unknown object type \"%1\"", info->value("type"));
                not_found_msg += internalReason(&Kexi::partManager()) + "<br></li>";
                continue;
            }
            // * NEW
            if (info->value("action") == "new") {
                if (!newObject(i, openingCancelled) && !openingCancelled) {
                    not_found_msg += "<li>";
                    not_found_msg += (i18n("cannot create object of type \"%1\"", info->value("type")) +
                                      internalReason(d->prj) + "<br></li>");
                } else
                    d->wasAutoOpen = true;
                continue;
            }

            KexiPart::Item *item = d->prj->item(i, info->value("name"));

            if (!item) {
                QString taskName;
                if (info->value("action") == "execute")
                    taskName = i18nc("\"executing object\" action", "executing");
#ifndef KEXI_NO_QUICK_PRINTING
                else if (info->value("action") == "print-preview")
                    taskName = futureI18n("making print preview for");
                else if (info->value("action") == "print")
                    taskName = futureI18n("printing");
#endif
                else
                    taskName = i18n("opening");

                not_found_msg += (QString("<li>") + taskName + " \"" + info->value("name") + "\" - ");
                if ("table" == info->value("type").toLower())
                    not_found_msg += i18n("table not found");
                else if ("query" == info->value("type").toLower())
                    not_found_msg += i18n("query not found");
                else if ("macro" == info->value("type").toLower())
                    not_found_msg += i18n("macro not found");
                else if ("script" == info->value("type").toLower())
                    not_found_msg += i18n("script not found");
                else
                    not_found_msg += i18n("object not found");
                not_found_msg += (internalReason(d->prj) + "<br></li>");
                continue;
            }
            // * EXECUTE, PRINT, PRINT PREVIEW
            if (info->value("action") == "execute") {
                tristate res = executeItem(item);
                if (false == res) {
                    not_found_msg += (QString("<li>\"") + info->value("name") + "\" - " + i18n("cannot execute object") +
                                      internalReason(d->prj) + "<br></li>");
                }
                continue;
            }
#ifndef KEXI_NO_QUICK_PRINTING
            else if (info->value("action") == "print") {
                tristate res = printItem(item);
                if (false == res) {
                    not_found_msg += (QString("<li>\"") + info->value("name") + "\" - " + futureI18n("cannot print object") +
                                      internalReason(d->prj) + "<br></li>");
                }
                continue;
            }
            else if (info->value("action") == "print-preview") {
                tristate res = printPreviewForItem(item);
                if (false == res) {
                    not_found_msg += (QString("<li>\"") + info->value("name") + "\" - " + futureI18n("cannot make print preview of object") +
                                      internalReason(d->prj) + "<br></li>");
                }
                continue;
            }
#endif

            Kexi::ViewMode viewMode;
            if (info->value("action") == "open")
                viewMode = Kexi::DataViewMode;
            else if (info->value("action") == "design")
                viewMode = Kexi::DesignViewMode;
            else if (info->value("action") == "edittext")
                viewMode = Kexi::TextViewMode;
            else
                continue; //sanity

            QString openObjectMessage;
            if (!openObject(item, viewMode, openingCancelled, 0, &openObjectMessage)
                    && (!openingCancelled || !openObjectMessage.isEmpty())) {
                not_found_msg += (QString("<li>\"") + info->value("name") + "\" - ");
                if (openObjectMessage.isEmpty())
                    not_found_msg += i18n("cannot open object");
                else
                    not_found_msg += openObjectMessage;
                not_found_msg += internalReason(d->prj) + "<br></li>";
                continue;
            } else {
                d->wasAutoOpen = true;
            }
        }
    }
    enableMessages(true);

    if (!not_found_msg.isEmpty())
        showErrorMessage(i18n("You have requested selected objects to be automatically opened "
                              "or processed on startup. Several objects cannot be opened or processed."),
                         QString("<ul>%1</ul>").arg(not_found_msg));

    d->updatePropEditorVisibility(currentWindow() ? currentWindow()->currentViewMode() : Kexi::NoViewMode);
#if defined(KDOCKWIDGET_P)
    if (d->propEditor) {
        KDockWidget *dw = (KDockWidget *)d->propEditorTabWidget->parentWidget();
        KDockSplitter *ds = (KDockSplitter *)dw->parentWidget();
        if (ds)
            ds->setSeparatorPosInPercent(d->config->readEntry("RightDockPosition", 80/* % */));
    }
#endif

    updateAppCaption();
    d->tabbedToolBar->hideMainMenu();

    qApp->processEvents();
    emit projectOpened();
}

tristate KexiMainWindow::closeProject()
{
    if (d->tabbedToolBar)
        d->tabbedToolBar->hideMainMenu();

#ifndef KEXI_NO_PENDING_DIALOGS
    if (d->pendingWindowsExist()) {
        kDebug() << "pendingWindowsExist...";
        d->actionToExecuteWhenPendingJobsAreFinished = Private::CloseProjectAction;
        return cancelled;
    }
#endif

    //only save nav. visibility setting if there is project opened
    d->saveSettingsForShowProjectNavigator = d->prj && d->isProjectNavigatorVisible;

    if (!d->prj)
        return true;

    {
        // make sure the project can be closed
        bool cancel = false;
        emit acceptProjectClosingRequested(cancel);
        if (cancel)
            return cancelled;
    }

    d->windowExistedBeforeCloseProject = currentWindow();

#if defined(KDOCKWIDGET_P)
    //remember docks position - will be used on storeSettings()
    if (d->propEditor) {
        KDockWidget *dw = (KDockWidget *)d->propEditorTabWidget->parentWidget();
        KDockSplitter *ds = (KDockSplitter *)dw->parentWidget();
        if (ds)
            d->propEditorDockSeparatorPos = ds->separatorPosInPercent();
    }
    if (d->nav) {
        if (d->propEditor) {
#ifdef __GNUC__
#warning TODO   if (d->openedWindowsCount() == 0)
#else
#pragma WARNING( TODO   if (d->openedWindowsCount() == 0) )
#endif
#ifdef __GNUC__
#warning TODO    makeWidgetDockVisible(d->propEditorTabWidget);
#else
#pragma WARNING( TODO    makeWidgetDockVisible(d->propEditorTabWidget); )
#endif
            KDockWidget *dw = (KDockWidget *)d->propEditorTabWidget->parentWidget();
            KDockSplitter *ds = (KDockSplitter *)dw->parentWidget();
            if (ds)
                ds->setSeparatorPosInPercent(80);
        }

        KDockWidget *dw = (KDockWidget *)d->nav->parentWidget();
        KDockSplitter *ds = (KDockSplitter *)dw->parentWidget();
        int dwWidth = dw->width();
        if (ds) {
            if (d->openedWindowsCount() != 0 && d->propEditorTabWidget && d->propEditorTabWidget->isVisible()) {
                d->navDockSeparatorPos = ds->separatorPosInPercent();
            }
            else {
                d->navDockSeparatorPos = (100 * dwWidth) / width();
            }
        }
    }
#endif

    //close each window, optionally asking if user wants to close (if data changed)
    while (currentWindow()) {
        tristate res = closeWindow(currentWindow());
        if (!res || ~res)
            return res;
    }

    // now we will close for sure
    emit beforeProjectClosing();

    if (!d->prj->closeConnection())
        return false;

    if (d->navigator) {
        d->navWasVisibleBeforeProjectClosing = d->navDockWidget->isVisible();
        d->navDockWidget->hide();
        d->navigator->setProject(0);
        slotProjectNavigatorVisibilityChanged(true); // hide side tab
    }

    if (d->propEditorDockWidget)
        d->propEditorDockWidget->hide();

    d->clearWindows(); //sanity!
    delete d->prj;
    d->prj = 0;


    updateReadOnlyState();
    invalidateActions();
    updateAppCaption();

    emit projectClosed();
    return true;
}

void KexiMainWindow::setupContextHelp()
{
#ifndef KEXI_NO_CTXT_HELP
    d->ctxHelp = new KexiContextHelp(d->mainWidget, this);
    //! @todo
    /*
      d->ctxHelp->setContextHelp(i18n("Welcome"),i18n("The <B>KEXI team</B> wishes you a lot of productive work, "
        "with this product. <BR><HR><BR>If you have found a <B>bug</B> or have a <B>feature</B> request, please don't "
        "hesitate to report it at our <A href=\"http://www.kexi-project.org/cgi-bin/bug.pl\"> issue "
        "tracking system </A>.<BR><HR><BR>If you would like to <B>join</B> our effort, the <B>development</B> documentation "
        "at <A href=\"http://www.kexi-project.org\">www.kexi-project.org</A> is a good starting point."),0);
    */
    addToolWindow(d->ctxHelp, KDockWidget::DockBottom | KDockWidget::DockLeft, getMainDockWidget(), 20);
#endif
}

void KexiMainWindow::setupMainWidget()
{
    QVBoxLayout *vlyr = new QVBoxLayout(this);
    vlyr->setContentsMargins(0, 0, 0, 0);
    vlyr->setSpacing(0);

    if (d->isMainMenuVisible) {
        QWidget *tabbedToolBarContainer = new QWidget(this);
        vlyr->addWidget(tabbedToolBarContainer);
        QVBoxLayout *tabbedToolBarContainerLyr = new QVBoxLayout(tabbedToolBarContainer);
        tabbedToolBarContainerLyr->setSpacing(0);
        tabbedToolBarContainerLyr->setContentsMargins(
            KDialog::marginHint() / 2, KDialog::marginHint() / 2,
            KDialog::marginHint() / 2, KDialog::marginHint() / 2);

        d->tabbedToolBar = new KexiTabbedToolBar(tabbedToolBarContainer);
        Q_ASSERT(d->action_view_global_search);
        connect(d->action_view_global_search, SIGNAL(triggered()),
                d->tabbedToolBar, SLOT(activateSearchLineEdit()));
        tabbedToolBarContainerLyr->addWidget(d->tabbedToolBar);
    }
    else {
        d->tabbedToolBar = 0;
    }

    QWidget *mainWidgetContainer = new QWidget();
    vlyr->addWidget(mainWidgetContainer, 1);
    QHBoxLayout *mainWidgetContainerLyr = new QHBoxLayout(mainWidgetContainer);
    mainWidgetContainerLyr->setContentsMargins(0, 0, 0, 0);
    mainWidgetContainerLyr->setSpacing(0);

    KMultiTabBar *mtbar = new KMultiTabBar(KMultiTabBar::Left);
    mtbar->setStyle(KMultiTabBar::KDEV3ICON);
    mainWidgetContainerLyr->addWidget(mtbar, 0);
    d->multiTabBars.insert(mtbar->position(), mtbar);

    d->mainWidget = new KexiMainWidget();
    d->mainWidget->setParent(this);

    d->mainWidget->tabWidget()->setTabsClosable(true);
    connect(d->mainWidget->tabWidget(), SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeWindowForTab(int)));
    mainWidgetContainerLyr->addWidget(d->mainWidget, 1);

    mtbar = new KMultiTabBar(KMultiTabBar::Right);
    mtbar->setStyle(KMultiTabBar::KDEV3ICON);
    mainWidgetContainerLyr->addWidget(mtbar, 0);
    d->multiTabBars.insert(mtbar->position(), mtbar);

    d->statusBar = new KexiStatusBar(this);
    vlyr->addWidget(d->statusBar);
}

void KexiMainWindow::slotSetProjectNavigatorVisible(bool set)
{
    if (d->navDockWidget)
        d->navDockWidget->setVisible(set);
}

void KexiMainWindow::slotSetPropertyEditorVisible(bool set)
{
    if (d->propEditorDockWidget)
        d->propEditorDockWidget->setVisible(set);
}

void KexiMainWindow::slotProjectNavigatorVisibilityChanged(bool visible)
{
    d->setTabBarVisible(KMultiTabBar::Left, PROJECT_NAVIGATOR_TABBAR_ID,
                        d->navDockWidget, !visible);
}

void KexiMainWindow::slotPropertyEditorVisibilityChanged(bool visible)
{
    if (!d->enable_slotPropertyEditorVisibilityChanged)
        return;
    d->setPropertyEditorTabBarVisible(!visible);
    if (!visible)
        d->propertyEditorCollapsed = true;
}

void KexiMainWindow::slotMultiTabBarTabClicked(int id)
{
    if (id == PROJECT_NAVIGATOR_TABBAR_ID) {
        slotProjectNavigatorVisibilityChanged(true);
        d->navDockWidget->show();
    }
    else if (id == PROPERTY_EDITOR_TABBAR_ID) {
        slotPropertyEditorVisibilityChanged(true);
        d->propEditorDockWidget->show();
        d->propertyEditorCollapsed = false;
    }
}

static Qt::DockWidgetArea applyRightToLeftToDockArea(Qt::DockWidgetArea area)
{
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        if (area == Qt::LeftDockWidgetArea) {
            return Qt::RightDockWidgetArea;
        }
        else if (area == Qt::RightDockWidgetArea) {
            return Qt::LeftDockWidgetArea;
        }
    }
    return area;
}

void KexiMainWindow::setupProjectNavigator()
{
    if (!d->isProjectNavigatorVisible)
        return;

    if (d->navigator) {
        d->navDockWidget->show();
    }
    else {
        KexiDockableWidget* navDockableWidget = new KexiDockableWidget;
        d->navigator = new KexiProjectNavigator(navDockableWidget);
        kexiTester() << KexiTestObject(d->navigator, "KexiProjectNavigator");

        navDockableWidget->setWidget(d->navigator);

        d->navDockWidget = new KexiDockWidget(d->navigator->windowTitle(), d->mainWidget);
        d->navDockWidget->setObjectName("ProjectNavigatorDockWidget");
        d->navDockWidget->closeButton()->setToolTip(i18n("Hide project navigator"));
        d->mainWidget->addDockWidget(
            applyRightToLeftToDockArea(Qt::LeftDockWidgetArea), d->navDockWidget,
            Qt::Vertical);
        navDockableWidget->setParent(d->navDockWidget);
        d->navDockWidget->setWidget(navDockableWidget);

        KConfigGroup mainWindowGroup(d->config->group("MainWindow"));
        const QSize projectNavigatorSize = mainWindowGroup.readEntry<QSize>("ProjectNavigatorSize", QSize());
        if (!projectNavigatorSize.isNull()) {
            navDockableWidget->setSizeHint(projectNavigatorSize);
        }

        connect(d->navDockWidget, SIGNAL(visibilityChanged(bool)),
            this, SLOT(slotProjectNavigatorVisibilityChanged(bool)));

        //Nav2 Signals
        connect(d->navigator, SIGNAL(openItem(KexiPart::Item*,Kexi::ViewMode)),
                this, SLOT(openObject(KexiPart::Item*,Kexi::ViewMode)));
        connect(d->navigator, SIGNAL(openOrActivateItem(KexiPart::Item*,Kexi::ViewMode)),
                this, SLOT(openObjectFromNavigator(KexiPart::Item*,Kexi::ViewMode)));
        connect(d->navigator, SIGNAL(newItem(KexiPart::Info*)),
                this, SLOT(newObject(KexiPart::Info*)));
        connect(d->navigator, SIGNAL(removeItem(KexiPart::Item*)),
                this, SLOT(removeObject(KexiPart::Item*)));
        connect(d->navigator->model(), SIGNAL(renameItem(KexiPart::Item*,QString,bool&)),
                this, SLOT(renameObject(KexiPart::Item*,QString,bool&)));
        connect(d->navigator->model(), SIGNAL(changeItemCaption(KexiPart::Item*,QString,bool&)),
                this, SLOT(setObjectCaption(KexiPart::Item*,QString,bool&)));
        connect(d->navigator, SIGNAL(executeItem(KexiPart::Item*)),
                this, SLOT(executeItem(KexiPart::Item*)));
        connect(d->navigator, SIGNAL(exportItemToClipboardAsDataTable(KexiPart::Item*)),
                this, SLOT(copyItemToClipboardAsDataTable(KexiPart::Item*)));
        connect(d->navigator, SIGNAL(exportItemToFileAsDataTable(KexiPart::Item*)),
                this, SLOT(exportItemAsDataTable(KexiPart::Item*)));
#ifndef KEXI_NO_QUICK_PRINTING
        connect(d->navigator, SIGNAL(printItem(KexiPart::Item*)),
                this, SLOT(printItem(KexiPart::Item*)));
        connect(d->navigator, SIGNAL(pageSetupForItem(KexiPart::Item*)),
                this, SLOT(showPageSetupForItem(KexiPart::Item*)));
#endif
        connect(d->navigator, SIGNAL(selectionChanged(KexiPart::Item*)),
                this, SLOT(slotPartItemSelectedInNavigator(KexiPart::Item*)));
    }
    if (d->prj->isConnected()) {
        QString partManagerErrorMessages;

        if (!partManagerErrorMessages.isEmpty()) {
            showWarningContinueMessage(partManagerErrorMessages, QString(),
                                       "ShowWarningsRelatedToPluginsLoading");
        }
        d->navigator->setProject(d->prj, QString()/*all classes*/, &partManagerErrorMessages);

    }
    connect(d->prj, SIGNAL(newItemStored(KexiPart::Item&)), d->navigator->model(), SLOT(slotAddItem(KexiPart::Item&)));
    connect(d->prj, SIGNAL(itemRemoved(KexiPart::Item)), d->navigator->model(), SLOT(slotRemoveItem(KexiPart::Item)));

    d->navigator->setFocus();

    if (d->forceShowProjectNavigatorOnCreation) {
        slotViewNavigator();
        d->forceShowProjectNavigatorOnCreation = false;
    } else if (d->forceHideProjectNavigatorOnCreation) {
        d->forceHideProjectNavigatorOnCreation = false;
    }

    invalidateActions();
}

void KexiMainWindow::slotLastActions()
{
}

void KexiMainWindow::setupPropertyEditor()
{
    if (!d->propEditor) {
        KConfigGroup mainWindowGroup(d->config->group("MainWindow"));
//! @todo FIX LAYOUT PROBLEMS
        d->propEditorDockWidget = new KexiDockWidget(i18n("Property Editor"), d->mainWidget);
        d->propEditorDockWidget->setObjectName("PropertyEditorDockWidget");
        d->propEditorDockWidget->closeButton()->setToolTip(i18n("Hide property editor"));
        d->mainWidget->addDockWidget(
            applyRightToLeftToDockArea(Qt::RightDockWidgetArea), d->propEditorDockWidget,
            Qt::Vertical
        );
        connect(d->propEditorDockWidget, SIGNAL(visibilityChanged(bool)),
            this, SLOT(slotPropertyEditorVisibilityChanged(bool)));

        d->propEditorDockableWidget = new KexiDockableWidget(d->propEditorDockWidget);
        d->propEditorDockWidget->setWidget(d->propEditorDockableWidget);
        const QSize propertyEditorSize = mainWindowGroup.readEntry<QSize>("PropertyEditorSize", QSize());
        if (!propertyEditorSize.isNull()) {
            d->propEditorDockableWidget->setSizeHint(propertyEditorSize);
        }

        QWidget *propEditorDockWidgetContents = new QWidget(d->propEditorDockableWidget);
        d->propEditorDockableWidget->setWidget(propEditorDockWidgetContents);
        QVBoxLayout *propEditorDockWidgetContentsLyr = new QVBoxLayout(propEditorDockWidgetContents);
        propEditorDockWidgetContentsLyr->setContentsMargins(0, 0, 0, 0);

        d->propEditorTabWidget = new KTabWidget(propEditorDockWidgetContents);
        d->propEditorTabWidget->setDocumentMode(true);
        propEditorDockWidgetContentsLyr->addWidget(d->propEditorTabWidget);
        d->propEditor = new KexiPropertyEditorView(d->propEditorTabWidget);
        d->propEditorTabWidget->setWindowTitle(d->propEditor->windowTitle());
        d->propEditorTabWidget->addTab(d->propEditor, i18n("Properties"));
//! @todo REMOVE? d->propEditor->installEventFilter(this);

        KConfigGroup propertyEditorGroup(d->config->group("PropertyEditor"));
        int size = propertyEditorGroup.readEntry("FontSize", -1);
        QFont f(KexiUtils::smallFont());
        if (size > 0)
            f.setPixelSize(size);
        d->propEditorTabWidget->setFont(f);

        d->enable_slotPropertyEditorVisibilityChanged = false;
        d->propEditorDockWidget->setVisible(false);
        d->enable_slotPropertyEditorVisibilityChanged = true;
    }
}

void KexiMainWindow::slotPartLoaded(KexiPart::Part* p)
{
    if (!p)
        return;
    p->createGUIClients();
}

void KexiMainWindow::updateAppCaption()
{
//! @todo allow to set custom "static" app caption

    d->appCaptionPrefix.clear();
    if (d->prj && d->prj->data()) {//add project name
        d->appCaptionPrefix = d->prj->data()->caption();
        if (d->appCaptionPrefix.isEmpty()) {
            d->appCaptionPrefix = d->prj->data()->databaseName();
        }
        if (d->prj->dbConnection()->isReadOnly()) {
            d->appCaptionPrefix = i18nc("<project-name> (read only)", "%1 (read only)", d->appCaptionPrefix);
        }
    }

    setWindowTitle(KDialog::makeStandardCaption(d->appCaptionPrefix, this));
}

bool KexiMainWindow::queryClose()
{
#ifndef KEXI_NO_PENDING_DIALOGS
    if (d->pendingWindowsExist()) {
        kDebug() << "pendingWindowsExist...";
        d->actionToExecuteWhenPendingJobsAreFinished = Private::QuitAction;
        return false;
    }
#endif
    const tristate res = closeProject();
    if (~res)
        return false;

    if (res == true)
        storeSettings();

    if (! ~res) {
        Kexi::deleteGlobalObjects();
        qApp->quit();
    }
    return ! ~res;
}

void KexiMainWindow::closeEvent(QCloseEvent *ev)
{
    d->mainWidget->closeEvent(ev);
}

void
KexiMainWindow::restoreSettings()
{
    KConfigGroup mainWindowGroup(d->config->group("MainWindow"));
    const bool maximize = mainWindowGroup.readEntry("Maximized", false);
    const QRect geometry(mainWindowGroup.readEntry("Geometry", QRect()));
    if (geometry.isValid())
        setGeometry(geometry);
    else if (maximize)
        setWindowState(windowState() | Qt::WindowMaximized);
    else {
        QRect desk = QApplication::desktop()->screenGeometry(
            QApplication::desktop()->screenNumber(this));
        if (desk.width() <= 1024 || desk.height() < 768)
            setWindowState(windowState() | Qt::WindowMaximized);
        else
            resize(1024, 768);
    }
    // Saved settings
}

void
KexiMainWindow::storeSettings()
{
    //kDebug();
    KConfigGroup mainWindowGroup(d->config->group("MainWindow"));

    if (isMaximized()) {
        mainWindowGroup.writeEntry("Maximized", true);
        mainWindowGroup.deleteEntry("Geometry");
    } else {
        mainWindowGroup.deleteEntry("Maximized");
        mainWindowGroup.writeEntry("Geometry", geometry());
    }

    if (d->navigator)
        mainWindowGroup.writeEntry("ProjectNavigatorSize", d->navigator->parentWidget()->size());

    if (d->propEditorDockableWidget)
        mainWindowGroup.writeEntry("PropertyEditorSize", d->propEditorDockableWidget->size());

    d->config->sync();
}

void
KexiMainWindow::registerChild(KexiWindow *window)
{
    //kDebug();
    connect(window, SIGNAL(dirtyChanged(KexiWindow*)), this, SLOT(slotDirtyFlagChanged(KexiWindow*)));

    if (window->id() != -1) {
        d->insertWindow(window);
    }
    //kDebug() << "ID=" << window->id();
}

void KexiMainWindow::updateCustomPropertyPanelTabs(KexiWindow *prevWindow,
        Kexi::ViewMode prevViewMode)
{
    updateCustomPropertyPanelTabs(
        prevWindow ? prevWindow->part() : 0,
        prevWindow ? prevWindow->currentViewMode() : prevViewMode,
        currentWindow() ? currentWindow()->part() : 0,
        currentWindow() ? currentWindow()->currentViewMode() : Kexi::NoViewMode
    );
}

void KexiMainWindow::updateCustomPropertyPanelTabs(
    KexiPart::Part *prevWindowPart, Kexi::ViewMode prevViewMode,
    KexiPart::Part *curWindowPart, Kexi::ViewMode curViewMode)
{
    if (!d->propEditorTabWidget)
        return;

    if (   !curWindowPart
        || (/*prevWindowPart &&*/ curWindowPart
             && (prevWindowPart != curWindowPart || prevViewMode != curViewMode)
           )
       )
    {
        if (d->partForPreviouslySetupPropertyPanelTabs) {
            //remember current page number for this part
            if ((   prevViewMode == Kexi::DesignViewMode
                 && static_cast<KexiPart::Part*>(d->partForPreviouslySetupPropertyPanelTabs) != curWindowPart) //part changed
                || curViewMode != Kexi::DesignViewMode)
            { //..or switching to other view mode
                d->recentlySelectedPropertyPanelPages.insert(
                        d->partForPreviouslySetupPropertyPanelTabs,
                        d->propEditorTabWidget->currentIndex());
            }
        }

        //delete old custom tabs (other than 'property' tab)
        const uint count = d->propEditorTabWidget->count();
        for (uint i = 1; i < count; i++)
            d->propEditorTabWidget->removeTab(1);
    }

    //don't change anything if part is not switched nor view mode changed
    if ((!prevWindowPart && !curWindowPart)
            || (prevWindowPart == curWindowPart && prevViewMode == curViewMode)
            || (curWindowPart && curViewMode != Kexi::DesignViewMode))
    {
        //new part for 'previously setup tabs'
        d->partForPreviouslySetupPropertyPanelTabs = curWindowPart;
        return;
    }

    if (curWindowPart) {
        //recreate custom tabs
        curWindowPart->setupCustomPropertyPanelTabs(d->propEditorTabWidget);

        //restore current page number for this part
        if (d->recentlySelectedPropertyPanelPages.contains(curWindowPart)) {
            d->propEditorTabWidget->setCurrentIndex(
                d->recentlySelectedPropertyPanelPages[ curWindowPart ]
            );
        }
    }

    //new part for 'previously setup tabs'
    d->partForPreviouslySetupPropertyPanelTabs = curWindowPart;
}

void KexiMainWindow::activeWindowChanged(KexiWindow *window, KexiWindow *prevWindow)
{
    //kDebug() << "to=" << (window ? window->windowTitle() : "<none>");
    bool windowChanged = prevWindow != window;

    if (windowChanged) {
        if (prevWindow) {
            //inform previously activated dialog about deactivation
            prevWindow->deactivate();
        }
    }

    updateCustomPropertyPanelTabs(prevWindow, prevWindow ? prevWindow->currentViewMode() : Kexi::NoViewMode);

    // inform the current view of the new dialog about property switching
    // (this will also call KexiMainWindow::propertySetSwitched() to update the current property editor's set
    if (windowChanged && currentWindow())
        currentWindow()->selectedView()->propertySetSwitched();

    if (windowChanged) {
        if (currentWindow() && currentWindow()->currentViewMode() != 0 && window) {
            //on opening new dialog it can be 0; we don't want this
            d->updatePropEditorVisibility(currentWindow()->currentViewMode());

            restoreDesignTabIfNeeded(window->partItem()->partClass(), window->currentViewMode(),
                                     prevWindow ? prevWindow->partItem()->identifier() : 0);
            activateDesignTabIfNeeded(window->partItem()->partClass(),
                                      window->currentViewMode());
        }
    }

    invalidateActions();
    d->updateFindDialogContents();
    if (window)
        window->setFocus();
}

bool
KexiMainWindow::activateWindow(int id)
{
    kDebug();
#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    return activateWindow(*d->openedWindowFor(id, pendingType));
#else
    return activateWindow(*d->openedWindowFor(id));
#endif
}

bool
KexiMainWindow::activateWindow(KexiWindow& window)
{
    kDebug();

    d->focus_before_popup = &window;
    d->mainWidget->tabWidget()->setCurrentWidget(window.parentWidget()/*container*/);
    window.activate();
    return true;
}

void KexiMainWindow::activateNextWindow()
{
//! @todo activateNextWindow()
}

void KexiMainWindow::activatePreviousWindow()
{
//! @todo activatePreviousWindow()
}

void
KexiMainWindow::slotSettings()
{
    if (d->tabbedToolBar) {
        d->tabbedToolBar->showMainMenu("settings");
        // dummy
        QLabel *dummy = KEXI_UNFINISHED_LABEL(actionCollection()->action("settings")->text());
        d->tabbedToolBar->setMainMenuContent(dummy);
    }
}

void
KexiMainWindow::slotConfigureKeys()
{
    KShortcutsDialog::configure(actionCollection(),
                                KShortcutsEditor::LetterShortcutsDisallowed, this);
}

void
KexiMainWindow::slotConfigureToolbars()
{
    KEditToolBar edit(actionCollection());
    (void) edit.exec();
}

void KexiMainWindow::slotProjectNew()
{
    createNewProject();
}

KexiProject* KexiMainWindow::createKexiProjectObject(const KexiProjectData &data)
{
    KexiProject *prj = new KexiProject(data, this);
    connect(prj, SIGNAL(itemRenamed(KexiPart::Item,QString)), this, SLOT(slotObjectRenamed(KexiPart::Item,QString)));

    if (d->navigator){
        connect(prj, SIGNAL(itemRemoved(KexiPart::Item)), d->navigator->model(), SLOT(slotRemoveItem(KexiPart::Item)));
    }
    return prj;
}

void KexiMainWindow::createNewProject()
{
    if (!d->tabbedToolBar)
        return;
    d->tabbedToolBar->showMainMenu("project_new");
    KexiNewProjectAssistant* assistant = new KexiNewProjectAssistant;
    connect(assistant, SIGNAL(createProject(KexiProjectData)),
            this, SLOT(createNewProject(KexiProjectData)));

    d->tabbedToolBar->setMainMenuContent(assistant);
}

tristate KexiMainWindow::createNewProject(const KexiProjectData &projectData)
{
    QScopedPointer<KexiProject> prj(createKexiProjectObject(projectData));
    tristate res = prj->create(true /*overwrite*/);
    if (res != true) {
        return res;
    }
    //kDebug() << "new project created ---";
    if (d->prj) {
        res = openProjectInExternalKexiInstance(
                prj->data()->connectionData()->fileName(),
                prj->data()->connectionData(),
                prj->data()->databaseName());
        Kexi::recentProjects()->addProjectData(*prj->data());
        d->tabbedToolBar->hideMainMenu();
        return res;
    }
    d->tabbedToolBar->hideMainMenu();
    d->prj = prj.take();
    setupProjectNavigator();
    d->prj->data()->setLastOpened(QDateTime::currentDateTime());
    Kexi::recentProjects()->addProjectData(*d->prj->data());

    invalidateActions();
    updateAppCaption();
    return true;
}

void KexiMainWindow::slotProjectOpen()
{
    if (!d->tabbedToolBar)
        return;
    d->tabbedToolBar->showMainMenu("project_open");
    KexiOpenProjectAssistant* assistant = new KexiOpenProjectAssistant;
    connect(assistant, SIGNAL(openProject(KexiProjectData)),
            this, SLOT(openProject(KexiProjectData)));
    connect(assistant, SIGNAL(openProject(QString)),
            this, SLOT(openProject(QString)));
    d->tabbedToolBar->setMainMenuContent(assistant);
}

tristate KexiMainWindow::openProject(const QString& aFileName)
{
    return openProject(aFileName, QString(), QString());
}

tristate KexiMainWindow::openProject(const QString& aFileName,
                                     const QString& fileNameForConnectionData, const QString& dbName)
{
    if (d->prj)
        return openProjectInExternalKexiInstance(aFileName, fileNameForConnectionData, dbName);

    KexiDB::ConnectionData *cdata = 0;
    if (!fileNameForConnectionData.isEmpty()) {
        cdata = Kexi::connset().connectionDataForFileName(fileNameForConnectionData);
        if (!cdata) {
            kWarning() << "cdata?";
            return false;
        }
    }
    return openProject(aFileName, cdata, dbName);
}

tristate KexiMainWindow::openProject(const QString& aFileName,
                                     KexiDB::ConnectionData *cdata, const QString& dbName,
                                     const KexiProjectData::AutoOpenObjects& autoopenObjects)
{
    if (d->prj) {
        return openProjectInExternalKexiInstance(aFileName, cdata, dbName);
    }

    KexiProjectData* projectData = 0;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs(0);
    bool readOnly = false;
    if (args) {
        readOnly = args->isSet("readonly");
    }
    bool deleteAfterOpen = false;
    if (cdata) {
        //server-based project
        if (dbName.isEmpty()) {//no database name given, ask user
            bool cancel;
            projectData = Kexi::startupHandler().selectProject(cdata, cancel, this);
            if (cancel)
                return cancelled;
        } else {
//! @todo caption arg?
            projectData = new KexiProjectData(*cdata, dbName);
            deleteAfterOpen = true;
        }
    } else {
        if (aFileName.isEmpty()) {
            kWarning() << "aFileName.isEmpty()";
            return false;
        }
        //file-based project
        kDebug() << "Project File: " << aFileName;
        KexiDB::ConnectionData fileConnData;
        fileConnData.setFileName(aFileName);
        QString detectedDriverName;
        int detectOptions = 0;
        if (readOnly) {
            detectOptions |= KexiStartupHandler::OpenReadOnly;
        }
        KexiStartupData::Import importActionData;
        bool forceReadOnly;
        const tristate res = KexiStartupHandler::detectActionForFile(
                                 &importActionData, &detectedDriverName, fileConnData.driverName,
                                 aFileName, this, detectOptions, &forceReadOnly);
        if (forceReadOnly) {
            readOnly = true;
        }
        if (true != res)
            return res;

        if (importActionData) { //importing requested
            return showProjectMigrationWizard(importActionData.mimeType, importActionData.fileName);
        }
        fileConnData.driverName = detectedDriverName;

        if (fileConnData.driverName.isEmpty())
            return false;

        //opening requested
        projectData = new KexiProjectData(fileConnData);
        deleteAfterOpen = true;
    }
    if (!projectData)
        return false;
    projectData->setReadOnly(readOnly);
    projectData->autoopenObjects = autoopenObjects;
    const tristate res = openProject(*projectData);
    if (deleteAfterOpen) //projectData object has been copied
        delete projectData;
    return res;
}

tristate KexiMainWindow::openProjectInExternalKexiInstance(const QString& aFileName,
        KexiDB::ConnectionData *cdata, const QString& dbName)
{
    QString fileNameForConnectionData;
    if (aFileName.isEmpty()) { //try .kexic file
        if (cdata)
            fileNameForConnectionData = Kexi::connset().fileNameForConnectionData(*cdata);
    }
    return openProjectInExternalKexiInstance(aFileName, fileNameForConnectionData, dbName);
}

tristate KexiMainWindow::openProjectInExternalKexiInstance(const QString& aFileName,
        const QString& fileNameForConnectionData, const QString& dbName)
{
    QString fileName(aFileName);
    QStringList args;

    // open a file-based project or a server connection provided as a .kexic file
    // (we have no other simple way to provide the startup data to a new process)
    if (fileName.isEmpty()) { //try .kexic file
        if (!fileNameForConnectionData.isEmpty())
            args << "--skip-conn-dialog"; //user does not expect conn. dialog to be shown here

        if (dbName.isEmpty()) { //use 'kexi --skip-conn-dialog file.kexic'
            fileName = fileNameForConnectionData;
        } else { //use 'kexi --skip-conn-dialog --connection file.kexic dbName'
            if (fileNameForConnectionData.isEmpty()) {
                kWarning() << "fileNameForConnectionData?";
                return false;
            }
            args << "--connection" << fileNameForConnectionData;
            fileName = dbName;
        }
    }
    if (fileName.isEmpty()) {
        kWarning() << "fileName?";
        return false;
    }
//! @todo use KRun
//! @todo untested
//Can arguments be supplied to KRun like is used here? AP
    args << fileName;
    const bool ok = QProcess::startDetached(
        qApp->applicationFilePath(), args,
        QFileInfo(fileName).absoluteDir().absolutePath());
    if (!ok) {
        d->showStartProcessMsg(args);
    }
    d->tabbedToolBar->hideMainMenu();
    return ok;
}

void KexiMainWindow::slotProjectWelcome()
{
    if (!d->tabbedToolBar)
        return;
    d->tabbedToolBar->showMainMenu("project_welcome");
    KexiWelcomeAssistant* assistant = new KexiWelcomeAssistant(
        Kexi::recentProjects(), this);
    connect(assistant, SIGNAL(openProject(KexiProjectData,QString,bool*)),
            this, SLOT(openProject(KexiProjectData,QString,bool*)));
    d->tabbedToolBar->setMainMenuContent(assistant);
}

void
KexiMainWindow::slotProjectSave()
{
    if (!currentWindow() || currentWindow()->currentViewMode() == Kexi::DataViewMode) {
        return;
    }
    saveObject(currentWindow());
    updateAppCaption();
    invalidateActions();
}

void
KexiMainWindow::slotProjectSaveAs()
{
    if (!currentWindow() || currentWindow()->currentViewMode() == Kexi::DataViewMode) {
        return;
    }
    saveObject(currentWindow(), QString(), SaveObjectAs);
    updateAppCaption();
    invalidateActions();
}

void
KexiMainWindow::slotProjectPrint()
{
#ifndef KEXI_NO_QUICK_PRINTING
    if (currentWindow() && currentWindow()->partItem())
        printItem(currentWindow()->partItem());
#endif
}

void
KexiMainWindow::slotProjectPrintPreview()
{
#ifndef KEXI_NO_QUICK_PRINTING
    if (currentWindow() && currentWindow()->partItem())
        printPreviewForItem(currentWindow()->partItem());
#endif
}

void
KexiMainWindow::slotProjectPageSetup()
{
#ifndef KEXI_NO_QUICK_PRINTING
    if (currentWindow() && currentWindow()->partItem())
        showPageSetupForItem(currentWindow()->partItem());
#endif
}

void KexiMainWindow::slotProjectExportDataTable()
{
    if (currentWindow() && currentWindow()->partItem())
        exportItemAsDataTable(currentWindow()->partItem());
}

void KexiMainWindow::slotProjectProperties()
{
    if (!d->tabbedToolBar)
        return;
    d->tabbedToolBar->showMainMenu("project_properties");
    // dummy
    QLabel *dummy = KEXI_UNFINISHED_LABEL(actionCollection()->action("project_properties")->text());
    d->tabbedToolBar->setMainMenuContent(dummy);
    //! @todo load the implementation not the ui :)
// ProjectSettingsUI u(this);
// u.exec();
}

void KexiMainWindow::slotProjectImportExportOrSend()
{
    if (!d->tabbedToolBar)
        return;
    d->tabbedToolBar->showMainMenu("project_import_export_send");
    KexiImportExportAssistant* assistant = new KexiImportExportAssistant(
        d->action_project_import_export_send,
        d->action_tools_import_project);
    connect(assistant, SIGNAL(importProject()), this, SLOT(slotToolsImportProject()));
    d->tabbedToolBar->setMainMenuContent(assistant);
}

void
KexiMainWindow::slotProjectClose()
{
    closeProject();
}

void KexiMainWindow::slotProjectRelations()
{
    if (!d->prj)
        return;
    KexiWindow *w = KexiInternalPart::createKexiWindowInstance("org.kexi-project.relations", this);
    activateWindow(*w);
}

void KexiMainWindow::slotImportFile()
{
    KEXI_UNFINISHED("Import: " + i18n("From File..."));
}

void KexiMainWindow::slotImportServer()
{
    KEXI_UNFINISHED("Import: " + i18n("From Server..."));
}

void
KexiMainWindow::slotProjectQuit()
{
    if (~ closeProject())
        return;
    close();
}

void KexiMainWindow::slotViewNavigator()
{
    if (!d->navigator) {
        return;
    }
    d->navigator->setFocus();
}

void KexiMainWindow::slotViewMainArea()
{
    if (currentWindow())
        currentWindow()->setFocus();
}

void KexiMainWindow::slotViewPropertyEditor()
{
    if (!d->propEditor) {
        return;
    }

    if (d->propEditorTabWidget->currentWidget())
        d->propEditorTabWidget->currentWidget()->setFocus();
}

tristate KexiMainWindow::switchToViewMode(KexiWindow& window, Kexi::ViewMode viewMode)
{
    const Kexi::ViewMode prevViewMode = currentWindow()->currentViewMode();
    if (prevViewMode == viewMode)
        return true;
    if (!activateWindow(window))
        return false;
    if (!currentWindow()) {
        return false;
    }
    if (&window != currentWindow())
        return false;
    if (!currentWindow()->supportsViewMode(viewMode)) {
        showErrorMessage(i18n("Selected view is not supported for \"%1\" object.",
                              currentWindow()->partItem()->name()),
                         i18n("Selected view (%1) is not supported by this object type (%2).",
                              Kexi::nameForViewMode(viewMode),
                              currentWindow()->part()->info()->instanceCaption()));
        return false;
    }
    updateCustomPropertyPanelTabs(currentWindow()->part(), prevViewMode,
                                  currentWindow()->part(), viewMode);
    tristate res = currentWindow()->switchToViewMode(viewMode);
    if (!res) {
        updateCustomPropertyPanelTabs(0, Kexi::NoViewMode); //revert
        showErrorMessage(i18n("Switching to other view failed (%1).",
                              Kexi::nameForViewMode(viewMode)), currentWindow());
        return false;
    }
    if (~res) {
        updateCustomPropertyPanelTabs(0, Kexi::NoViewMode); //revert
        return cancelled;
    }

    activateWindow(window);

    invalidateSharedActions();
    invalidateProjectWideActions();
    d->updateFindDialogContents();
    d->updatePropEditorVisibility(viewMode);
    QString origTabToActivate;
    if (viewMode == Kexi::DesignViewMode) {
        // Save the orig tab: we want to back to design tab
        // when user moved to data view and then immediately to design view.
        origTabToActivate = d->tabsToActivateOnShow.value(currentWindow()->partItem()->identifier());
    }
    restoreDesignTabIfNeeded(currentWindow()->partItem()->partClass(), viewMode,
                             currentWindow()->partItem()->identifier());
    if (viewMode == Kexi::DesignViewMode) {
        activateDesignTab(currentWindow()->partItem()->partClass());
        // Restore the saved tab to the orig one. restoreDesignTabIfNeeded() saved tools tab probably.
        d->tabsToActivateOnShow.insert(currentWindow()->partItem()->identifier(), origTabToActivate);
    }

    return true;
}

void KexiMainWindow::slotViewDataMode()
{
    if (currentWindow())
        switchToViewMode(*currentWindow(), Kexi::DataViewMode);
}

void KexiMainWindow::slotViewDesignMode()
{
    if (currentWindow())
        switchToViewMode(*currentWindow(), Kexi::DesignViewMode);
}

void KexiMainWindow::slotViewTextMode()
{
    if (currentWindow())
        switchToViewMode(*currentWindow(), Kexi::TextViewMode);
}

//! Used to control if we're not Saving-As object under the original name
class SaveAsObjectNameValidator : public KexiNameDialogValidator
{
public:
    SaveAsObjectNameValidator(const QString &originalObjectName)
     : m_originalObjectName(originalObjectName)
    {
    }
    virtual bool validate(KexiNameDialog *dialog) const {
        if (dialog->widget()->nameText() == m_originalObjectName) {
            KMessageBox::information(dialog,
                                     i18nc("Could not save object under the original name.",
                                           "Could not save under the original name."));
            return false;
        }
        return true;
    }
private:
    QString m_originalObjectName;
};

tristate KexiMainWindow::getNewObjectInfo(
    KexiPart::Item *partItem, const QString &originalName, KexiPart::Part *part,
    bool allowOverwriting, bool *overwriteNeeded, const QString& messageWhenAskingForName)
{
    //data was never saved in the past -we need to create a new object at the backend
    KexiPart::Info *info = part->info();
    if (!d->nameDialog) {
        d->nameDialog = new KexiNameDialog(
            messageWhenAskingForName, this);
        //check if that name is allowed
        d->nameDialog->widget()->addNameSubvalidator(
            new KexiDB::ObjectNameValidator(project()->dbConnection()->driver()));
        d->nameDialog->setButtonText(KexiNameDialog::Ok, i18nc("@action:button Save object", "Save"));
    } else {
        d->nameDialog->widget()->setMessageText(messageWhenAskingForName);
    }
    d->nameDialog->widget()->setCaptionText(partItem->caption());
    d->nameDialog->widget()->setNameText(partItem->name());
    d->nameDialog->setWindowTitle(i18nc("@title:window", "Save Object As"));
    d->nameDialog->setDialogIcon(info->itemIconName());
    d->nameDialog->setAllowOverwriting(allowOverwriting);
    if (!originalName.isEmpty()) {
        d->nameDialog->setValidator(new SaveAsObjectNameValidator(originalName));
    }
    if (d->nameDialog->execAndCheckIfObjectExists(*project(), *part, overwriteNeeded)
        != QDialog::Accepted)
    {
        return cancelled;
    }

    // close window of object that will be overwritten
    if (*overwriteNeeded) {
        KexiPart::Item* overwrittenItem = project()->item(info, d->nameDialog->widget()->nameText());
        if (overwrittenItem) {
            KexiWindow * openedWindow = d->openedWindowFor(overwrittenItem->identifier());
            if (openedWindow) {
                const tristate res = closeWindow(openedWindow);
                if (res != true) {
                    return res;
                }
            }
        }
    }

    //update name and caption
    partItem->setName(d->nameDialog->widget()->nameText());
    partItem->setCaption(d->nameDialog->widget()->captionText());
    return true;
}

//! Used to delete part item on exit from block
class PartItemDeleter : public QScopedPointer<KexiPart::Item>
{
    public:
        explicit PartItemDeleter(KexiProject *prj) : m_prj(prj) {}
        ~PartItemDeleter() {
            if (!isNull()) {
                m_prj->deleteUnstoredItem(take());
            }
        }
    private:
        KexiProject *m_prj;
};

static void showSavingObjectFailedErrorMessage(KexiMainWindow *wnd, KexiPart::Item *item)
{
    wnd->showErrorMessage(
        i18nc("@info Saving object failed",
              "Saving <resource>%1</resource> object failed.", item->name()),
              wnd->currentWindow());
}

tristate KexiMainWindow::saveObject(KexiWindow *window, const QString& messageWhenAskingForName,
                                    SaveObjectOptions options)
{
    tristate res;
    bool saveAs = options & SaveObjectAs;
    if (!saveAs && !window->neverSaved()) {
        //data was saved in the past -just save again
        res = window->storeData(options & DoNotAsk);
        if (!res) {
            showSavingObjectFailedErrorMessage(this, window->partItem());
        }
        return res;
    }
    if (saveAs && window->neverSaved()) {
        //if never saved, saveAs == save
        saveAs = false;
    }

    const int oldItemID = window->partItem()->identifier();

    KexiPart::Item *partItem;
    KexiView::StoreNewDataOptions storeNewDataOptions;
    PartItemDeleter itemDeleter(d->prj);
    if (saveAs) {
        partItem = d->prj->createPartItem(window->part());
        if (!partItem) {
            //! @todo error
            return false;
        }
        itemDeleter.reset(partItem);
    }
    else {
        partItem = window->partItem();
    }

    bool overwriteNeeded;
    res = getNewObjectInfo(partItem, saveAs ? window->partItem()->name() : QString(),
                           window->part(), true /*allowOverwriting*/,
                           &overwriteNeeded, messageWhenAskingForName);
    if (res != true)
        return res;
    if (overwriteNeeded) {
        storeNewDataOptions |= KexiView::OverwriteExistingData;
    }

    if (saveAs) {
        res = window->storeDataAs(partItem, storeNewDataOptions);
    }
    else {
        res = window->storeNewData(storeNewDataOptions);
    }

    if (~res)
        return cancelled;
    if (!res) {
        showSavingObjectFailedErrorMessage(this, partItem);
        return false;
    }

    d->updateWindowId(window, oldItemID);
    invalidateProjectWideActions();
    itemDeleter.take();
    return true;
}

tristate KexiMainWindow::closeWindow(KexiWindow *window)
{
    return closeWindow(window ? window : currentWindow(), true);
}

tristate KexiMainWindow::closeCurrentWindow()
{
    return closeWindow(0);
}

tristate KexiMainWindow::closeWindowForTab(int tabIndex)
{
    KexiWindow* window = windowForTab(tabIndex);
    if (!window)
        return false;
    return closeWindow(window);
}

tristate KexiMainWindow::closeWindow(KexiWindow *window, bool layoutTaskBar, bool doNotSaveChanges)
{
#ifdef __GNUC__
#warning TODO KexiMainWindow::closeWindow()
#else
#pragma WARNING( TODO KexiMainWindow::closeWindow() )
#endif
    ///@note Q_UNUSED layoutTaskBar
    Q_UNUSED(layoutTaskBar);

    if (!window)
        return true;
    if (d->insideCloseWindow)
        return true;

    const int previousItemId = window->partItem()->identifier();

#ifndef KEXI_NO_PENDING_DIALOGS
    d->addItemToPendingWindows(window->partItem(), Private::WindowClosingJob);
#endif

    d->insideCloseWindow = true;

    if (window == currentWindow() && !window->isAttached()) {
        if (d->propEditor) {
            // ah, closing detached window - better switch off property buffer right now...
            d->propertySet = 0;
            d->propEditor->editor()->changeSet(0);
        }
    }

    bool remove_on_closing = window->partItem() ? window->partItem()->neverSaved() : false;
    if (window->isDirty() && !d->forceWindowClosing && !doNotSaveChanges) {
        //more accurate tool tips and what's this
        KGuiItem saveChanges(KStandardGuiItem::save());
        saveChanges.setToolTip(i18n("Save changes"));
        saveChanges.setWhatsThis(
            i18n("Saves all recent changes made in \"%1\" object.",
                 window->partItem()->name()));
        KGuiItem discardChanges(KStandardGuiItem::discard());
        discardChanges.setWhatsThis(
            i18n("Discards all recent changes made in \"%1\" object.",
                 window->partItem()->name()));

        //dialog's data is dirty:
        //--adidional message, e.g. table designer will return
        //  "Note: This table is already filled with data which will be removed."
        //  if the window is in design view mode.
        const KLocalizedString additionalMessage(
            window->part()->i18nMessage(":additional message before saving design", window));
        QString additionalMessageString;
        if (!additionalMessage.isEmpty())
            additionalMessageString = additionalMessage.toString();

        if (additionalMessageString.startsWith(':'))
            additionalMessageString.clear();
        if (!additionalMessageString.isEmpty())
            additionalMessageString = "<p>" + additionalMessageString + "</p>";

        const int questionRes = KMessageBox::warningYesNoCancel(this,
                                "<p>"
                                + window->part()->i18nMessage("Design of object <resource>%1</resource> has been modified.", window)
                                .subs(window->partItem()->name()).toString()
                                + "</p><p>" + i18n("Do you want to save changes?") + "</p>"
                                + additionalMessageString /*may be empty*/,
                                QString(),
                                saveChanges,
                                discardChanges);
        if (questionRes == KMessageBox::Cancel) {
#ifndef KEXI_NO_PENDING_DIALOGS
            d->removePendingWindow(window->id());
#endif
            d->insideCloseWindow = false;
            d->windowsToClose.clear(); //give up with 'close all'
            return cancelled;
        }
        if (questionRes == KMessageBox::Yes) {
            //save it
            tristate res = saveObject(window, QString(), DoNotAsk);
            if (!res || ~res) {
//! @todo show error info; (retry/ignore/cancel)
#ifndef KEXI_NO_PENDING_DIALOGS
                d->removePendingWindow(window->id());
#endif
                d->insideCloseWindow = false;
                d->windowsToClose.clear(); //give up with 'close all'
                return res;
            }
            remove_on_closing = false;
        }
    }

    const int window_id = window->id(); //remember now, because removeObject() can destruct partitem object
    const QString window_partClass = window->partItem()->partClass();
    if (remove_on_closing) {
        //we won't save this object, and it was never saved -remove it
        if (!removeObject(window->partItem(), true)) {
#ifndef KEXI_NO_PENDING_DIALOGS
            d->removePendingWindow(window->id());
#endif
            //msg?
            //! @todo ask if we'd continue and return true/false
            d->insideCloseWindow = false;
            d->windowsToClose.clear(); //give up with 'close all'
            return false;
        }
    } else {
        //not dirty now
        if (d->navigator) {
            d->navigator->updateItemName(*window->partItem(), false);
        }
    }

    hideDesignTab(previousItemId, QString());

    d->removeWindow(window_id);
    QWidget *windowContainer = window->parentWidget();
    d->mainWidget->tabWidget()->removeTab(
        d->mainWidget->tabWidget()->indexOf(windowContainer));

#ifndef KEXI_NO_QUICK_PRINTING
    //also remove from 'print setup dialogs' cache, if needed
    int printedObjectID = 0;
    if (d->pageSetupWindowItemID2dataItemID_map.contains(window_id))
        printedObjectID = d->pageSetupWindowItemID2dataItemID_map[ window_id ];
    d->pageSetupWindows.remove(printedObjectID);
#endif

    delete windowContainer;

    //focus navigator if nothing else available
    if (d->openedWindowsCount() == 0) {
        if (d->navigator) {
            d->navigator->setFocus();
        }
        d->updatePropEditorVisibility(Kexi::NoViewMode);
    }

    invalidateActions();
    d->insideCloseWindow = false;
    if (!d->windowsToClose.isEmpty()) {//continue 'close all'
        KexiWindow* w = d->windowsToClose.takeAt(0);
        closeWindow(w, true);
    }

#ifndef KEXI_NO_PENDING_DIALOGS
    d->removePendingWindow(window_id);

    //perform pending global action that was suspended:
    if (!d->pendingWindowsExist()) {
        d->executeActionWhenPendingJobsAreFinished();
    }
#endif
    d->mainWidget->slotCurrentTabIndexChanged(d->mainWidget->tabWidget()->currentIndex());
    showDesignTabIfNeeded(0);

    if (currentWindow()) {
        restoreDesignTabIfNeeded(currentWindow()->partItem()->partClass(),
                                 currentWindow()->currentViewMode(),
                                 0);
    }
    d->tabsToActivateOnShow.remove(previousItemId);
    return true;
}

QWidget* KexiMainWindow::findWindow(QWidget *w)
{
    while (w && !acceptsSharedActions(w)) {
        if (w == d->propEditorDockWidget)
            return currentWindow();
        w = w->parentWidget();
    }
    return w;
}

KexiWindow* KexiMainWindow::openedWindowFor(int identifier)
{
    return d->openedWindowFor(identifier);
}

KexiWindow* KexiMainWindow::openedWindowFor(const KexiPart::Item* item)
{
    return item ? openedWindowFor(item->identifier()) : 0;
}

KexiDB::QuerySchema* KexiMainWindow::unsavedQuery(int queryId)
{
    KexiWindow * queryWindow = openedWindowFor(queryId);
    if (!queryWindow || !queryWindow->isDirty()) {
        return 0;
    }

    return queryWindow->part()->currentQuery(queryWindow->viewForMode(Kexi::DataViewMode));
}

QList<QVariant> KexiMainWindow::currentParametersForQuery(int queryId) const
{
    KexiWindow *queryWindow = d->openedWindowFor(queryId);
    if (!queryWindow) {
        return QList<QVariant>();
    }

    KexiView *view = queryWindow->viewForMode(Kexi::DataViewMode);
    if (!view) {
        return QList<QVariant>();
    }

    return view->currentParameters();
}

bool KexiMainWindow::acceptsSharedActions(QObject *w)
{
    return w->inherits("KexiWindow") || w->inherits("KexiView");
}

bool KexiMainWindow::openingAllowed(KexiPart::Item* item, Kexi::ViewMode viewMode, QString* errorMessage)
{
    kDebug() << viewMode;
    //! @todo this can be more complex once we deliver ACLs...
    if (!d->userMode)
        return true;
    KexiPart::Part * part = Kexi::partManager().partForClass(item->partClass());
    if (!part) {
        if (errorMessage) {
            *errorMessage = Kexi::partManager().errorMsg();
        }
    }
    kDebug() << part << item->partClass();
    if (part)
        kDebug() << item->partClass() << part->info()->supportedUserViewModes();
    return part && (part->info()->supportedUserViewModes() & viewMode);
}

KexiWindow *
KexiMainWindow::openObject(const QString& partClass, const QString& name,
                           Kexi::ViewMode viewMode, bool &openingCancelled, QMap<QString, QVariant>* staticObjectArgs)
{
    KexiPart::Item *item = d->prj->itemForClass(partClass, name);
    if (!item)
        return 0;
    return openObject(item, viewMode, openingCancelled, staticObjectArgs);
}

KexiWindow *
KexiMainWindow::openObject(KexiPart::Item* item, Kexi::ViewMode viewMode, bool &openingCancelled,
                           QMap<QString, QVariant>* staticObjectArgs, QString* errorMessage)
{
    if (!d->prj || !item) {
        return 0;
    }

    if (!openingAllowed(item, viewMode, errorMessage)) {
        if (errorMessage)
            *errorMessage = i18nc(
                                "opening is not allowed in \"data view/design view/text view\" mode",
                                "opening is not allowed in \"%1\" mode", Kexi::nameForViewMode(viewMode));
        openingCancelled = true;
        return 0;
    }
    kDebug() << d->prj << item;

    KexiWindow *prevWindow = currentWindow();

    KexiUtils::WaitCursor wait;
#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(item, pendingType);
    if (pendingType != Private::NoJob) {
        openingCancelled = true;
        return 0;
    }
#else
    KexiWindow *window = openedWindowFor(item);
#endif
    int previousItemId = currentWindow() ? currentWindow()->partItem()->identifier() : 0;
    openingCancelled = false;

    bool alreadyOpened = false;
    KexiWindowContainer *windowContainer = 0;

    if (window) {
        if (viewMode != window->currentViewMode()) {
            if (true != switchToViewMode(*window, viewMode))
                return 0;
        } else
            activateWindow(*window);
        alreadyOpened = true;
    } else {
        KexiPart::Part *part = Kexi::partManager().partForClass(item->partClass());
        d->updatePropEditorVisibility(viewMode, part ? part->info() : 0);
        //update tabs before opening
        updateCustomPropertyPanelTabs(currentWindow() ? currentWindow()->part() : 0,
                                      currentWindow() ? currentWindow()->currentViewMode() : Kexi::NoViewMode,
                                      part, viewMode);

        // open new tab earlier
        windowContainer = new KexiWindowContainer(d->mainWidget->tabWidget());
        const int tabIndex = d->mainWidget->tabWidget()->addTab(
            windowContainer,
            KIcon(part ? part->info()->itemIconName() : QString()),
            KexiWindow::windowTitleForItem(*item));
        d->mainWidget->tabWidget()->setTabToolTip(tabIndex, KexiPart::fullCaptionForItem(item, part));
        QString whatsThisText;
        if (part) {
            whatsThisText = i18n("Tab for \"%1\" (%2).",
                                 item->captionOrName(), part->info()->instanceCaption());
        }
        else {
            whatsThisText = i18n("Tab for \"%1\".", item->captionOrName());
        }
        d->mainWidget->tabWidget()->setTabWhatsThis(tabIndex, whatsThisText);
        d->mainWidget->tabWidget()->setCurrentWidget(windowContainer);

#ifndef KEXI_NO_PENDING_DIALOGS
        d->addItemToPendingWindows(item, Private::WindowOpeningJob);
#endif
        window = d->prj->openObject(windowContainer, *item, viewMode, staticObjectArgs);
        if (window) {
            windowContainer->setWindow(window);
            // update text and icon
            d->mainWidget->tabWidget()->setTabText(
                d->mainWidget->tabWidget()->indexOf(windowContainer),
                window->windowTitle());
            d->mainWidget->tabWidget()->setTabIcon(
                d->mainWidget->tabWidget()->indexOf(windowContainer),
                window->windowIcon());
        }
    }

    if (!window || !activateWindow(*window)) {
#ifndef KEXI_NO_PENDING_DIALOGS
        d->removePendingWindow(item->identifier());
#endif
        d->mainWidget->tabWidget()->removeTab(
            d->mainWidget->tabWidget()->indexOf(windowContainer));
        delete windowContainer;
        updateCustomPropertyPanelTabs(0, Kexi::NoViewMode); //revert
        //! @todo add error msg...
        return 0;
    }

    if (viewMode != window->currentViewMode())
        invalidateSharedActions();

#ifndef KEXI_NO_PENDING_DIALOGS
    d->removePendingWindow(window->id());

    //perform pending global action that was suspended:
    if (!d->pendingWindowsExist()) {
        d->executeActionWhenPendingJobsAreFinished();
    }
#endif
    if (window && !alreadyOpened) {
        // Call switchToViewMode() and propertySetSwitched() again here because
        // this is the time when then new window is the current one - previous call did nothing.
        switchToViewMode(*window, window->currentViewMode());
        currentWindow()->selectedView()->propertySetSwitched();
    }
    invalidateProjectWideActions();
    restoreDesignTabIfNeeded(item->partClass(), viewMode, previousItemId);
    activateDesignTabIfNeeded(item->partClass(), viewMode);
    QString origTabToActivate;
    if (prevWindow) {
        // Save the orig tab for prevWindow that was stored in the restoreDesignTabIfNeeded() call above
        origTabToActivate = d->tabsToActivateOnShow.value(prevWindow->partItem()->identifier());
    }
    activeWindowChanged(window, prevWindow);
    if (prevWindow) {
        // Restore the orig tab
        d->tabsToActivateOnShow.insert(prevWindow->partItem()->identifier(), origTabToActivate);
    }
    return window;
}

KexiWindow *
KexiMainWindow::openObjectFromNavigator(KexiPart::Item* item, Kexi::ViewMode viewMode)
{
    bool openingCancelled;
    return openObjectFromNavigator(item, viewMode, openingCancelled);
}

KexiWindow *
KexiMainWindow::openObjectFromNavigator(KexiPart::Item* item, Kexi::ViewMode viewMode,
                                        bool &openingCancelled)
{
    if (!openingAllowed(item, viewMode)) {
        openingCancelled = true;
        return 0;
    }
    if (!d->prj || !item)
        return 0;
#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(item, pendingType);
    if (pendingType != Private::NoJob) {
        openingCancelled = true;
        return 0;
    }
#else
    KexiWindow *window = openedWindowFor(item);
#endif
    openingCancelled = false;
    if (window) {
        if (activateWindow(*window)) {
            return window;
        }
    }
    //if DataViewMode is not supported, try Design, then Text mode (currently useful for script part)
    KexiPart::Part *part = Kexi::partManager().partForClass(item->partClass());
    if (!part)
        return 0;
    if (viewMode == Kexi::DataViewMode && !(part->info()->supportedViewModes() & Kexi::DataViewMode)) {
        if (part->info()->supportedViewModes() & Kexi::DesignViewMode)
            return openObjectFromNavigator(item, Kexi::DesignViewMode, openingCancelled);
        else if (part->info()->supportedViewModes() & Kexi::TextViewMode)
            return openObjectFromNavigator(item, Kexi::TextViewMode, openingCancelled);
    }
    //do the same as in openObject()
    return openObject(item, viewMode, openingCancelled);
}

tristate KexiMainWindow::closeObject(KexiPart::Item* item)
{
#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(item, pendingType);
    if (pendingType == Private::WindowClosingJob)
        return true;
    else if (pendingType == Private::WindowOpeningJob)
        return cancelled;
#else
    KexiWindow *window = openedWindowFor(item);
#endif
    if (!window)
        return cancelled;
    return closeWindow(window);
}

bool KexiMainWindow::newObject(KexiPart::Info *info, bool& openingCancelled)
{
    if (d->userMode) {
        openingCancelled = true;
        return false;
    }
    openingCancelled = false;
    if (!d->prj || !info)
        return false;
    KexiPart::Part *part = Kexi::partManager().partForClass(info->partClass());
    if (!part)
        return false;

    KexiPart::Item *it = d->prj->createPartItem(info);
    if (!it) {
        //! @todo error
        return false;
    }

    if (!it->neverSaved()) { //only add stored objects to the browser
        d->navigator->model()->slotAddItem(*it);
    }
    return openObject(it, Kexi::DesignViewMode, openingCancelled);
}

tristate KexiMainWindow::removeObject(KexiPart::Item *item, bool dontAsk)
{
    if (d->userMode)
        return cancelled;
    if (!d->prj || !item)
        return false;

    KexiPart::Part *part = Kexi::partManager().partForClass(item->partClass());
    if (!part)
        return false;

    if (!dontAsk) {
        if (KMessageBox::No == KMessageBox::warningYesNo(this,
                i18nc("@info Remove <objecttype> <objectname>?",
                      "Do you want to permanently delete the following object?<nl/>"
                      "<nl/>%1 <resource>%2</resource><nl/>"
                      "<nl/><note>If you click <interface>Delete</interface>, "
                      "you will not be able to undo the deletion.</note>",
                      part->info()->instanceCaption(), item->name()),
                i18nc("@title:window Delete Object %1.",
                      "Delete <resource>%1</resource>?", item->name()),
                KGuiItem(i18n("Delete"), koIconName("edit-delete")),
                KStandardGuiItem::no()))
        {
            return cancelled;
        }
    }

    tristate res = true;
#ifndef KEXI_NO_QUICK_PRINTING
    //also close 'print setup' dialog for this item, if any
    KexiWindow * pageSetupWindow = d->pageSetupWindows[ item->identifier()];
    const bool oldInsideCloseWindow = d->insideCloseWindow;
    {
        d->insideCloseWindow = false;
        if (pageSetupWindow)
            res = closeWindow(pageSetupWindow);
    }
    d->insideCloseWindow = oldInsideCloseWindow;
    if (!res || ~res) {
        return res;
    }
#endif

#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(item, pendingType);
    if (pendingType != Private::NoJob) {
        return cancelled;
    }
#else
    KexiWindow *window = openedWindowFor(item);
#endif

    if (window) {//close existing window
        const bool tmp = d->forceWindowClosing;
        window->partItem()->neverSaved();
        d->forceWindowClosing = true;
        res = closeWindow(window);
        d->forceWindowClosing = tmp; //restore
        if (!res || ~res) {
            return res;
        }
    }

#ifndef KEXI_NO_QUICK_PRINTING
    //in case the dialog is a 'print setup' dialog, also update d->pageSetupWindows
    int dataItemID = d->pageSetupWindowItemID2dataItemID_map[item->identifier()];
    d->pageSetupWindowItemID2dataItemID_map.remove(item->identifier());
    d->pageSetupWindows.remove(dataItemID);
#endif

    if (!d->prj->removeObject(*item)) {
        //! @todo better msg
        showSorryMessage(i18n("Could not remove object."));
        return false;
    }
    return true;
}

void KexiMainWindow::renameObject(KexiPart::Item *item, const QString& _newName, bool &success)
{
    if (d->userMode) {
        success = false;
        return;
    }
    QString newName = _newName.trimmed();
    if (newName.isEmpty()) {
        showSorryMessage(i18n("Could not set empty name for this object."));
        success = false;
        return;
    }

    KexiWindow *window = openedWindowFor(item);
    if (window) {
        QString msg = i18nc("@info",
                            "<para>Before renaming object <resource>%1</resource> it should be closed.</para>"
                            "<para>Do you want to close it?</para>",
                            item->name());
        int r = KMessageBox::questionYesNo(this, msg, QString(),
                                           KGuiItem(i18n("Close Window"), koIconName("window-close")),
                                           KStandardGuiItem::cancel());
        if (r != KMessageBox::Yes) {
            success = false;
            return;
        }
    }
    enableMessages(false); //to avoid double messages
    const bool res = d->prj->renameObject(*item, newName);
    enableMessages(true);
    if (!res) {
        showErrorMessage(d->prj, i18n("Renaming object \"%1\" failed.", newName));
        success = false;
        return;
    }
}

void KexiMainWindow::setObjectCaption(KexiPart::Item *item, const QString& _newCaption, bool &success)
{
    if (d->userMode) {
        success = false;
        return;
    }
    QString newCaption = _newCaption.trimmed();
    enableMessages(false); //to avoid double messages
    const bool res = d->prj->setObjectCaption(*item, newCaption);
    enableMessages(true);
    if (!res) {
        showErrorMessage(d->prj, i18n("Setting caption for object \"%1\" failed.", newCaption));
        success = false;
        return;
    }
}

void KexiMainWindow::slotObjectRenamed(const KexiPart::Item &item, const QString& /*oldName*/)
{
#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(&item, pendingType);
    if (pendingType != Private::NoJob)
        return;
#else
    KexiWindow *window = openedWindowFor(&item);
#endif
    if (!window)
        return;

    //change item
    window->updateCaption();
    if (static_cast<KexiWindow*>(currentWindow()) == window)//optionally, update app. caption
        updateAppCaption();
}

void KexiMainWindow::acceptPropertySetEditing()
{
    if (d->propEditor)
        d->propEditor->editor()->acceptInput();
}

void KexiMainWindow::propertySetSwitched(KexiWindow *window, bool force,
        bool preservePrevSelection, bool sortedProperties, const QByteArray& propertyToSelect)
{
    KexiWindow* _currentWindow = currentWindow();
    //kDebug() << "currentWindow(): "
    //    << (_currentWindow ? _currentWindow->windowTitle() : QString("NULL"))
    //    << " window: " << (window ? window->windowTitle() : QString("NULL"));
    if (_currentWindow && _currentWindow != window) {
        d->propertySet = 0; //we'll need to move to another prop. set
        return;
    }
    if (d->propEditor) {
        KoProperty::Set *newSet = _currentWindow ? _currentWindow->propertySet() : 0;
        if (!newSet || (force || static_cast<KoProperty::Set*>(d->propertySet) != newSet)) {
            d->propertySet = newSet;
            if (preservePrevSelection || force) {
                KoProperty::EditorView::SetOptions options = KoProperty::EditorView::ExpandChildItems;
                if (preservePrevSelection) {
                    options |= KoProperty::EditorView::PreservePreviousSelection;
                }
                if (sortedProperties) {
                    options |= KoProperty::EditorView::AlphabeticalOrder;
                }

                if (propertyToSelect.isEmpty()) {
                    d->propEditor->editor()->changeSet(d->propertySet, options);
                }
                else {
                    d->propEditor->editor()->changeSet(d->propertySet, propertyToSelect, options);
                }
            }
        }
    }
}

void KexiMainWindow::slotDirtyFlagChanged(KexiWindow* window)
{
    KexiPart::Item *item = window->partItem();
    //update text in navigator and app. caption
    if (!d->userMode) {
        d->navigator->updateItemName(*item, window->isDirty());
    }

    invalidateActions();
    updateAppCaption();
    d->mainWidget->tabWidget()->setTabText(
        d->mainWidget->tabWidget()->indexOf(window->parentWidget()),
        window->windowTitle());
}

void KexiMainWindow::slotTipOfTheDay()
{
    //! @todo
}

void KexiMainWindow::slotReportBug()
{
    KexiBugReportDialog bugReport(this);
    bugReport.exec();
}

bool KexiMainWindow::userMode() const
{
    return d->userMode;
}

void
KexiMainWindow::setupUserActions()
{
}

void KexiMainWindow::slotToolsImportProject()
{
    if (d->tabbedToolBar)
        d->tabbedToolBar->hideMainMenu();
    showProjectMigrationWizard(QString(), QString());
}

void KexiMainWindow::slotToolsImportTables()
{
    if (project()) {
        QMap<QString, QString> args;
        QDialog *dlg = KexiInternalPart::createModalDialogInstance("org.kexi-project.migration", "importtable", this, 0, &args);
        if (!dlg)
            return; //error msg has been shown by KexiInternalPart

        const int result = dlg->exec();
        delete dlg;
        if (result != QDialog::Accepted)
            return;

        QString destinationTableName(args["destinationTableName"]);
        if (!destinationTableName.isEmpty()) {
            QString partClass = "org.kexi-project.table";
            bool openingCancelled;
            KexiMainWindow::openObject(partClass, destinationTableName, Kexi::DataViewMode, openingCancelled);
        }
    }
}

void KexiMainWindow::slotToolsCompactDatabase()
{
    KexiProjectData *data = 0;
    KexiDB::Driver *drv = 0;
    const bool projectWasOpened = d->prj;

    if (!d->prj) {
        KexiStartupDialog dlg(
            KexiStartupDialog::OpenExisting, 0, Kexi::connset(), this);

        if (dlg.exec() != QDialog::Accepted)
            return;

        if (dlg.selectedFileName().isEmpty()) {
//! @todo add support for server based if needed?
            return;
        }
        KexiDB::ConnectionData cdata;
        cdata.setFileName(dlg.selectedFileName());

        //detect driver name for the selected file
        KexiStartupData::Import detectedImportAction;
        tristate res = KexiStartupHandler::detectActionForFile(
                           &detectedImportAction, &cdata.driverName,
                           "" /*suggestedDriverName*/, cdata.fileName(), 0,
                           KexiStartupHandler::SkipMessages | KexiStartupHandler::ThisIsAProjectFile
                           | KexiStartupHandler::DontConvert);

        if (true == res && !detectedImportAction)
            drv = Kexi::driverManager().driver(cdata.driverName);
        if (!drv || !(drv->features() & KexiDB::Driver::CompactingDatabaseSupported)) {
            KMessageBox::information(this, "<qt>" +
                                     i18n("Compacting database file <nobr>\"%1\"</nobr> is not supported.",
                                          QDir::convertSeparators(cdata.fileName())));
            return;
        }
        data = new KexiProjectData(cdata);
    } else {
        //sanity
        if (!(d->prj && d->prj->dbConnection()
                && (d->prj->dbConnection()->driver()->features() & KexiDB::Driver::CompactingDatabaseSupported)))
            return;

        if (KMessageBox::Continue != KMessageBox::warningContinueCancel(this,
                i18n("The current project has to be closed before compacting the database. "
                     "It will be open again after compacting.\n\nDo you want to continue?")))
            return;

        data = new KexiProjectData(*d->prj->data()); // a copy
        drv = d->prj->dbConnection()->driver();
        const tristate res = closeProject();
        if (~res || !res) {
            delete data;
            return;
        }
    }

    if (!drv->adminTools().vacuum(*data->connectionData(), data->databaseName())) {
      showErrorMessage(&drv->adminTools());
    }

    if (projectWasOpened)
      openProject(*data);

    delete data;
}

tristate KexiMainWindow::showProjectMigrationWizard(
    const QString& mimeType, const QString& databaseName, const KexiDB::ConnectionData *cdata)
{
    //pass arguments
    QMap<QString, QString> args;
    args.insert("mimeType", mimeType);
    args.insert("databaseName", databaseName);
    if (cdata) { //pass ConnectionData serialized as a string...
        QString str;
        KexiUtils::serializeMap(KexiDB::toMap(*cdata), str);
        args.insert("connectionData", str);
    }

    QDialog *dlg = KexiInternalPart::createModalDialogInstance("org.kexi-project.migration", "migration", this, 0, &args);
    if (!dlg)
        return false; //error msg has been shown by KexiInternalPart

    const int result = dlg->exec();
    delete dlg;
    if (result != QDialog::Accepted)
        return cancelled;

    //open imported project in a new Kexi instance
    QString destinationDatabaseName(args["destinationDatabaseName"]);
    QString fileName, destinationConnectionShortcut, dbName;
    if (!destinationDatabaseName.isEmpty()) {
        if (args.contains("destinationConnectionShortcut")) {
            // server-based
            destinationConnectionShortcut = args["destinationConnectionShortcut"];
        } else {
            // file-based
            fileName = destinationDatabaseName;
            destinationDatabaseName.clear();
        }
        tristate res = openProject(fileName, destinationConnectionShortcut,
                                   destinationDatabaseName);
        raise();
        return res;
    }
    return true;
}

tristate KexiMainWindow::executeItem(KexiPart::Item* item)
{
    KexiPart::Info *info = item ? Kexi::partManager().infoForClass(item->partClass()) : 0;
    if ((! info) || (! info->isExecuteSupported()))
        return false;
    KexiPart::Part *part = Kexi::partManager().part(info);
    if (!part)
        return false;
    return part->execute(item);
}

void KexiMainWindow::slotProjectImportDataTable()
{
//! @todo allow data appending (it is not possible now)
    if (d->userMode)
        return;
    QMap<QString, QString> args;
    args.insert("sourceType", "file");
    QDialog *dlg = KexiInternalPart::createModalDialogInstance(
                       "org.kexi-project.importexport.csv", "KexiCSVImportDialog", this, 0, &args);
    if (!dlg)
        return; //error msg has been shown by KexiInternalPart
    dlg->exec();
    delete dlg;
}

tristate KexiMainWindow::executeCustomActionForObject(KexiPart::Item* item,
        const QString& actionName)
{
    if (actionName == "exportToCSV")
        return exportItemAsDataTable(item);
    else if (actionName == "copyToClipboardAsCSV")
        return copyItemToClipboardAsDataTable(item);

    kWarning() << "no such action:" << actionName;
    return false;
}

tristate KexiMainWindow::exportItemAsDataTable(KexiPart::Item* item)
{
    if (!item)
        return false;

    QMap<QString, QString> args;

    if (!checkForDirtyFlagOnExport(item, &args)) {
            return false;
    }

    //! @todo: accept record changes...
    args.insert("destinationType", "file");
    args.insert("itemId", QString::number(item->identifier()));
    QDialog *dlg = KexiInternalPart::createModalDialogInstance(
                       "org.kexi-project.importexport.csv", "KexiCSVExportWizard", this, 0, &args);
    if (!dlg)
        return false; //error msg has been shown by KexiInternalPart
    int result = dlg->exec();
    delete dlg;
    return result == QDialog::Rejected ? tristate(cancelled) : tristate(true);
}

bool KexiMainWindow::checkForDirtyFlagOnExport(KexiPart::Item *item, QMap<QString, QString> *args)
{
    //! @todo: handle tables
    if (item->partClass() != "org.kexi-project.query") {
        return true;
    }

    KexiWindow * itemWindow = openedWindowFor(item);
    if (itemWindow && itemWindow->isDirty()) {
        tristate result;
        if (item->neverSaved()) {
            result = true;
        } else {
            int prevWindowId = 0;
            if (!itemWindow->isVisible()) {
                prevWindowId = currentWindow()->id();
                activateWindow(itemWindow->id());
            }
            result = askOnExportingChangedQuery(item);

            if (prevWindowId != 0) {
                activateWindow(prevWindowId);
            }
        }

        if (~result) {
            return false;
        } else if (true == result) {
            args->insert("useTempQuery","1");
        }
    }

    return true;
}

tristate KexiMainWindow::askOnExportingChangedQuery(KexiPart::Item *item) const
{
    int result = KMessageBox::warningYesNoCancel(const_cast<KexiMainWindow*>(this),
        i18nc("@info", "Design of query <resource>%1</resource> that you want to export data"
                                         " from is changed and has not yet been saved. Do you want to use data"
                                         " from the changed query for exporting or from its original (saved)"
                                         " version?", item->captionOrName()),
        QString(),
        KGuiItem(i18nc("Export query data", "Use the Changed Query")),
        KGuiItem(i18nc("Export query data", "Use the Original Query")),
        KStandardGuiItem::cancel(),
        QString(),
        KMessageBox::Notify | KMessageBox::Dangerous);
    if (result == KMessageBox::Yes) {
        return true;
    } else if (result == KMessageBox::No) {
        return false;
    }

    return cancelled;
}

bool KexiMainWindow::printItem(KexiPart::Item* item, const QString& titleText)
{
    //! @todo printItem(item, KexiSimplePrintingSettings::load(), titleText);
    Q_UNUSED(item)
    Q_UNUSED(titleText)
    return false;
}

tristate KexiMainWindow::printItem(KexiPart::Item* item)
{
    return printItem(item, QString());
}

bool KexiMainWindow::printPreviewForItem(KexiPart::Item* item, const QString& titleText, bool reload)
{
//! @todo printPreviewForItem(item, KexiSimplePrintingSettings::load(), titleText, reload);
    Q_UNUSED(item)
    Q_UNUSED(titleText)
    Q_UNUSED(reload)
    return false;
}

tristate KexiMainWindow::printPreviewForItem(KexiPart::Item* item)
{
    return printPreviewForItem(item, QString(),
//! @todo store cached record data?
                               true/*reload*/);
}

tristate KexiMainWindow::showPageSetupForItem(KexiPart::Item* item)
{
    Q_UNUSED(item)
//! @todo check if changes to this object's design are saved, if not: ask for saving
//! @todo accept record changes...
//! @todo printActionForItem(item, PageSetupForItem);
    return false;
}

//! @todo reenable printItem() when ported
#if 0
bool KexiMainWindow::printItem(KexiPart::Item* item, const KexiSimplePrintingSettings& settings,
                               const QString& titleText)
{
//! @todo: check if changes to this object's design are saved, if not: ask for saving
//! @todo: accept record changes...
    KexiSimplePrintingCommand cmd(this, item->identifier());
    //modal
    return cmd.print(settings, titleText);
}

bool KexiMainWindow::printPreviewForItem(KexiPart::Item* item,
        const KexiSimplePrintingSettings& settings, const QString& titleText, bool reload)
{
//! @todo: check if changes to this object's design are saved, if not: ask for saving
//! @todo: accept record changes...
    KexiSimplePrintingCommand* cmd = d->openedCustomObjectsForItem<KexiSimplePrintingCommand>(
                                         item, "KexiSimplePrintingCommand");
    if (!cmd) {
        d->addOpenedCustomObjectForItem(
            item,
            cmd = new KexiSimplePrintingCommand(this, item->identifier()),
            "KexiSimplePrintingCommand"
        );
    }
    return cmd->showPrintPreview(settings, titleText, reload);
}

tristate KexiMainWindow::printActionForItem(KexiPart::Item* item, PrintActionType action)
{
    if (!item)
        return false;
    KexiPart::Info *info = Kexi::partManager().infoForClass(item->partClass());
    if (!info->isPrintingSupported())
        return false;

    KexiWindow *printingWindow = d->pageSetupWindows[ item->identifier()];
    if (printingWindow) {
        if (!activateWindow(*printingWindow))
            return false;
        if (action == PreviewItem || action == PrintItem) {
            QTimer::singleShot(0, printingWindow->selectedView(),
                               (action == PreviewItem) ? SLOT(printPreview()) : SLOT(print()));
        }
        return true;
    }

#ifndef KEXI_NO_PENDING_DIALOGS
    Private::PendingJobType pendingType;
    KexiWindow *window = d->openedWindowFor(item, pendingType);
    if (pendingType != Private::NoJob)
        return cancelled;
#else
    KexiWindow *window = openedWindowFor(item);
#endif

    if (window) {
        // accept record changes
        QWidget *prevFocusWidget = focusWidget();
        window->setFocus();
        d->action_data_save_row->activate(QAction::Trigger);
        if (prevFocusWidget)
            prevFocusWidget->setFocus();

        // opened: check if changes made to this dialog are saved, if not: ask for saving
        if (window->neverSaved()) //sanity check
            return false;
        if (window->isDirty()) {
            KGuiItem saveChanges(KStandardGuiItem::save());
            saveChanges.setToolTip(futureI18n("Save changes"));
            saveChanges.setWhatsThis(
                futureI18n("Pressing this button will save all recent changes made in \"%1\" object.",
                     item->name()));
            KGuiItem doNotSave(KStandardGuiItem::no());
            doNotSave.setWhatsThis(
                futureI18n("Pressing this button will ignore all unsaved changes made in \"%1\" object.",
                     window->partItem()->name()));

            QString question;
            if (action == PrintItem)
                question = futureI18n("Do you want to save changes before printing?");
            else if (action == PreviewItem)
                question = futureI18n("Do you want to save changes before making print preview?");
            else if (action == PageSetupForItem)
                question = futureI18n("Do you want to save changes before showing page setup?");
            else
                return false;

            const int questionRes = KMessageBox::warningYesNoCancel(this,
                                    "<p>"
                                    + window->part()->i18nMessage("Design of object <resource>%1</resource> has been modified.", window)
                                    .subs(item->name())
                                    + "</p><p>" + question + "</p>",
                                    QString(),
                                    saveChanges,
                                    doNotSave);
            if (KMessageBox::Cancel == questionRes)
                return cancelled;
            if (KMessageBox::Yes == questionRes) {
                tristate savingRes = saveObject(window, QString(), DoNotAsk);
                if (true != savingRes)
                    return savingRes;
            }
        }
    }
    KexiPart::Part * printingPart = Kexi::partManager().partForClass("org.kexi-project.simpleprinting");
    if (!printingPart)
        printingPart = new KexiSimplePrintingPart(); //hardcoded as there're no .desktop file
    KexiPart::Item* printingPartItem = d->prj->createPartItem(
                                           printingPart, item->name() //<-- this will look like "table1 : printing" on the window list
                                       );
    QMap<QString, QVariant> staticObjectArgs;
    staticObjectArgs["identifier"] = QString::number(item->identifier());
    if (action == PrintItem)
        staticObjectArgs["action"] = "print";
    else if (action == PreviewItem)
        staticObjectArgs["action"] = "printPreview";
    else if (action == PageSetupForItem)
        staticObjectArgs["action"] = "pageSetup";
    else
        return false;
    bool openingCancelled;
    printingWindow = openObject(printingPartItem, Kexi::DesignViewMode,
                                openingCancelled, &staticObjectArgs);
    if (openingCancelled)
        return cancelled;
    if (!printingWindow) //sanity
        return false;
    d->pageSetupWindows.insert(item->identifier(), printingWindow);
    d->pageSetupWindowItemID2dataItemID_map.insert(
        printingWindow->partItem()->identifier(), item->identifier());

    return true;
}
#endif

void KexiMainWindow::slotEditCopySpecialDataTable()
{
    KexiPart::Item* item = d->navigator->selectedPartItem();
    if (item)
        copyItemToClipboardAsDataTable(item);
}

tristate KexiMainWindow::copyItemToClipboardAsDataTable(KexiPart::Item* item)
{
    if (!item)
        return false;

    QMap<QString, QString> args;

    if (!checkForDirtyFlagOnExport(item, &args)) {
            return false;
    }

    args.insert("destinationType", "clipboard");
    args.insert("itemId", QString::number(item->identifier()));
    QDialog *dlg = KexiInternalPart::createModalDialogInstance(
                       "org.kexi-project.importexport.csv", "KexiCSVExportWizard", this, 0, &args);
    if (!dlg)
        return false; //error msg has been shown by KexiInternalPart
    const int result = dlg->exec();
    delete dlg;
    return result == QDialog::Rejected ? tristate(cancelled) : tristate(true);
}

void KexiMainWindow::slotEditPasteSpecialDataTable()
{
//! @todo allow data appending (it is not possible now)
    if (d->userMode)
        return;
    QMap<QString, QString> args;
    args.insert("sourceType", "clipboard");
    QDialog *dlg = KexiInternalPart::createModalDialogInstance(
                       "org.kexi-project.importexport.csv", "KexiCSVImportDialog", this, 0, &args);
    if (!dlg)
        return; //error msg has been shown by KexiInternalPart
    dlg->exec();
    delete dlg;
}

void KexiMainWindow::slotEditFind()
{
    KexiSearchAndReplaceViewInterface* iface = d->currentViewSupportingSearchAndReplaceInterface();
    if (!iface)
        return;
    d->updateFindDialogContents(true/*create if does not exist*/);
    d->findDialog()->setReplaceMode(false);

    d->findDialog()->show();
    d->findDialog()->activateWindow();
    d->findDialog()->raise();
}

void KexiMainWindow::slotEditFind(bool next)
{
    KexiSearchAndReplaceViewInterface* iface = d->currentViewSupportingSearchAndReplaceInterface();
    if (!iface)
        return;
    tristate res = iface->find(
                       d->findDialog()->valueToFind(), d->findDialog()->options(), next);
    if (~res)
        return;
    d->findDialog()->updateMessage(true == res);
//! @todo result
}

void KexiMainWindow::slotEditFindNext()
{
    slotEditFind(true);
}

void KexiMainWindow::slotEditFindPrevious()
{
    slotEditFind(false);
}

void KexiMainWindow::slotEditReplace()
{
    KexiSearchAndReplaceViewInterface* iface = d->currentViewSupportingSearchAndReplaceInterface();
    if (!iface)
        return;
    d->updateFindDialogContents(true/*create if does not exist*/);
    d->findDialog()->setReplaceMode(true);
//! @todo slotEditReplace()
    d->findDialog()->show();
    d->findDialog()->activateWindow();
}

void KexiMainWindow::slotEditReplaceNext()
{
    slotEditReplace(false);
}

void KexiMainWindow::slotEditReplace(bool all)
{
    KexiSearchAndReplaceViewInterface* iface = d->currentViewSupportingSearchAndReplaceInterface();
    if (!iface)
        return;
//! @todo add question: "Do you want to replace every occurrence of \"%1\" with \"%2\"?
//!       You won't be able to undo this." + "Do not ask again".
    tristate res = iface->findNextAndReplace(
                       d->findDialog()->valueToFind(), d->findDialog()->valueToReplaceWith(),
                       d->findDialog()->options(), all);
    d->findDialog()->updateMessage(true == res);
//! @todo result
}

void KexiMainWindow::slotEditReplaceAll()
{
    slotEditReplace(true);
}

/// TMP (until there's true template support)
void KexiMainWindow::slotGetNewStuff()
{
#ifdef HAVE_KNEWSTUFF
    if (!d->newStuff)
        d->newStuff = new KexiNewStuff(this);
    d->newStuff->download();

    //KNS::DownloadDialog::open(newstuff->customEngine(), "kexi/template");
#endif
}

void KexiMainWindow::highlightObject(const QString& partClass, const QString& name)
{
    slotViewNavigator();
    if (!d->prj)
        return;
    KexiPart::Item *item = d->prj->itemForClass(partClass, name);
    if (!item)
        return;
    if (d->navigator) {
        d->navigator->selectItem(*item);
    }
}

void KexiMainWindow::slotPartItemSelectedInNavigator(KexiPart::Item* item)
{
    Q_UNUSED(item);
}

KToolBar *KexiMainWindow::toolBar(const QString& name) const
{
    return d->tabbedToolBar ? d->tabbedToolBar->toolBar(name) : 0;
}

void KexiMainWindow::appendWidgetToToolbar(const QString& name, QWidget* widget)
{
    if (d->tabbedToolBar)
        d->tabbedToolBar->appendWidgetToToolbar(name, widget);
}

void KexiMainWindow::setWidgetVisibleInToolbar(QWidget* widget, bool visible)
{
    if (d->tabbedToolBar)
        d->tabbedToolBar->setWidgetVisibleInToolbar(widget, visible);
}

void KexiMainWindow::addToolBarAction(const QString& toolBarName, QAction *action)
{
    if (d->tabbedToolBar)
        d->tabbedToolBar->addAction(toolBarName, action);
}

void KexiMainWindow::updatePropertyEditorInfoLabel(const QString& textToDisplayForNullSet)
{
    d->propEditor->updateInfoLabelForPropertySet(d->propertySet, textToDisplayForNullSet);
}

void KexiMainWindow::addSearchableModel(KexiSearchableModel *model)
{
    d->tabbedToolBar->addSearchableModel(model);
}

void KexiMainWindow::setReasonableDialogSize(QDialog *dialog)
{
    dialog->setMinimumSize(600, 400);
    dialog->resize(size() * 0.8);
}

void KexiMainWindow::restoreDesignTabAndActivateIfNeeded(const QString &tabName)
{
    d->tabbedToolBar->showTab(tabName);
    if (currentWindow() && currentWindow()->partItem()
        && currentWindow()->partItem()->identifier() != 0) // for unstored items id can be < 0
    {
        const QString tabToActivate = d->tabsToActivateOnShow.value(
                                          currentWindow()->partItem()->identifier());
        //kDebug() << "tabToActivate:" << tabToActivate << "tabName:" << tabName;
        if (tabToActivate == tabName) {
            d->tabbedToolBar->setCurrentTab(tabToActivate);
        }
    }
}

void KexiMainWindow::restoreDesignTabIfNeeded(const QString &partClass, Kexi::ViewMode viewMode,
                                              int previousItemId)
{
    //kDebug() << partClass << viewMode << previousItemId;
    if (viewMode == Kexi::DesignViewMode) {
        switch (d->prj->idForClass(partClass)) {
        case KexiPart::FormObjectType: {
            hideDesignTab(previousItemId, "org.kexi-project.report");
            restoreDesignTabAndActivateIfNeeded("form");
            break;
        }
        case KexiPart::ReportObjectType: {
            hideDesignTab(previousItemId, "org.kexi-project.form");
            restoreDesignTabAndActivateIfNeeded("report");
            break;
        }
        default:
            hideDesignTab(previousItemId);
        }
    }
    else {
        hideDesignTab(previousItemId);
    }
}

void KexiMainWindow::activateDesignTab(const QString &partClass)
{
    switch (d->prj->idForClass(partClass)) {
    case KexiPart::FormObjectType:
        d->tabbedToolBar->setCurrentTab("form");
        break;
    case KexiPart::ReportObjectType:
        d->tabbedToolBar->setCurrentTab("report");
        break;
    default:;
    }
}

void KexiMainWindow::activateDesignTabIfNeeded(const QString &partClass, Kexi::ViewMode viewMode)
{
    const QString tabToActivate = d->tabsToActivateOnShow.value(currentWindow()->partItem()->identifier());
    //kDebug() << partClass << viewMode << tabToActivate;

    if (viewMode == Kexi::DesignViewMode && tabToActivate.isEmpty()) {
        activateDesignTab(partClass);
    }
    else {
        d->tabbedToolBar->setCurrentTab(tabToActivate);
    }
}

void KexiMainWindow::hideDesignTab(int itemId, const QString &partClass)
{
    //kDebug() << itemId << partClass;
    if (   itemId > 0
        && d->tabbedToolBar->currentWidget())
    {
        const QString currentTab = d->tabbedToolBar->currentWidget()->objectName();
        //kDebug() << "d->tabsToActivateOnShow.insert" << itemId << currentTab;
        d->tabsToActivateOnShow.insert(itemId, currentTab);
    }
    switch (d->prj->idForClass(partClass)) {
    case KexiPart::FormObjectType:
        d->tabbedToolBar->hideTab("form");
        break;
    case KexiPart::ReportObjectType:
        d->tabbedToolBar->hideTab("report");
        break;
    default:
        d->tabbedToolBar->hideTab("form");
        d->tabbedToolBar->hideTab("report");
    }
}

void KexiMainWindow::showDesignTabIfNeeded(int previousItemId)
{
    if (d->insideCloseWindow)
        return;
    if (currentWindow()) {
        restoreDesignTabIfNeeded(currentWindow()->partItem()->partClass(),
                                 currentWindow()->currentViewMode(), previousItemId);
    } else {
        hideDesignTab(previousItemId);
    }
}

KexiUserFeedbackAgent* KexiMainWindow::userFeedbackAgent() const
{
    return &d->userFeedback;
}

KexiMigrateManagerInterface* KexiMainWindow::migrateManager()
{
    if (d->migrateManager.isNull()) {
        d->migrateManager.reset(new KexiMigration::MigrateManager());
    }
    return d->migrateManager.data();
}

void KexiMainWindow::toggleFullScreen(bool isFullScreen)
{
    static bool isTabbarRolledDown;

    if (isFullScreen) {
        isTabbarRolledDown = !d->tabbedToolBar->isRolledUp();
        if (isTabbarRolledDown) {
            d->tabbedToolBar->toggleRollDown();
        }
    } else {
        if (isTabbarRolledDown && d->tabbedToolBar->isRolledUp()) {
            d->tabbedToolBar->toggleRollDown();
        }
    }

    KToggleFullScreenAction::setFullScreen(this, isFullScreen);
}

#include "KexiMainWindow.moc"
#include "KexiMainWindow_p.moc"
