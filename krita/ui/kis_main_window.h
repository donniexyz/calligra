/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_MAIN_WINDOW_H
#define KIS_MAIN_WINDOW_H

#include <KoMainWindow.h>

class QMdiArea;
class QSignalMapper;
class QCloseEvent;

class KAction;

class KoPart;

class KisMainGui;
class KisView2;

class KisMainWindow : public KoMainWindow
{
    Q_OBJECT
public:
    explicit KisMainWindow(KoPart *part, const KComponentData &instance);

    void showView(KoView *view);
    
signals:
    
public slots:

    void slotPreferences();

protected:

    void closeEvent(QCloseEvent *e);

private slots:

    void updateMenus();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);

private:
    KisView2 *activeKisView();

    KisMainGui *m_guiClient;

    QMdiArea *m_mdiArea;
    QSignalMapper *m_windowMapper;

    KAction *m_mdiCascade;
    KAction *m_mdiTile;
    KAction *m_mdiNextWindow;
    KAction *m_mdiPreviousWindow;


};

#endif // KIS_MAIN_WINDOW_H