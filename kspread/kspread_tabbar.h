/* This file is part of the KDE project
   Copyright (C) 2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright (C) 1998-2000 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KSPREAD_TABBAR
#define KSPREAD_TABBAR

#include <qwidget.h>
#include <qstringlist.h>

class KSpreadSheet;
class KSpreadView;

namespace KSpread
{

class TabBarPrivate;

/**
 * This tab bar is used by @ref KSpreadView. It is used to choose between all
 * available tables.
 *
 * Adding, removing or renaming of tabs does not automatically add, rename or
 * remove KSpreadSheet objects. The tabbar is just a GUI element.
 *
 * But activating a tab emits a signal which in turn will show this table
 * in the associated KSpreadView.
 *
 * @short A bar with tabs and scroll buttons.
 */
class TabBar : public QWidget
{
    Q_OBJECT
public:

    /**
     * Creates a new tabbar.
     */
    TabBar( KSpreadView *view, const char *name = 0 );

    /**
     * Destroy the tabbar.
     */
    virtual ~TabBar();

    /**
     * Adds a tab to the bar and paints it. The tab does not become active.
     * call @ref #setActiveTab to do so.
     */
    void addTab( const QString& _text );

    /**
     * Adds a hidden tab.
     *
     * @see KSpreadView::setActiveTable
     */
    void addHiddenTab( const QString & text );

    /**
     * Removes a hidden tab.
     *
     * @see KSpreadView::setActiveTable
     */
    void removeHiddenTab(const QString & text);

    /**
     * Removes the tab from the bar. If the tab was the active one then the one
     * left of it ( or if not available ) the one right of it will become active.
     * It is recommended to call @ref #setActiveTab after a call to this function.
     */
    void removeTab( const QString& _text );

    /**
     * Renames a tab.
     */
    void renameTab( const QString& old_name, const QString& new_name );

    /**
     * Removes all tabs from the bar and repaints the widget.
     */
    void removeAllTabs();

    bool canScrollLeft() const;

    bool canScrollRight() const;

    /**
     * Highlights this tab.
     */
    void setActiveTab( const QString& _text );

    /**
    * Remove table name from tabList and
    * put tablename in tablehide
    * and highlights first name in tabList
    */
    void hideTable();
    void hideTable(const QString& tableName );


    /**
     * Shows the table. This makes only sense if
     * the table was hiddem before.
     *
     * The table does not become automatically the active one.
     */
    void showTable(const QString& _text);
    void showTable(QStringList list);
    void displayTable(const QString& _text);

    /**
     * Sets the tab bar to be read only. This means, no dragging
     * for reordering the tabs is possible.
     * Signal contextMenu and doubleClicked would not be emitted.
     */
    void setReadOnly( bool ro );

    /**
     * Returns true if the tab bar is read only.
     */
    bool readOnly() const;

public slots:
    void scrollLeft();
    void scrollRight();
    void scrollFirst();
    void scrollLast();

signals:
    /**
     * Emitted if the active tab changed. This will cause the
     * KSpreadView to change its active table, too.
     */
    void tabChanged( const QString& _text );

    /**
     * Emitted upon mouse right click. This is typically used
     * to popup a context menu.
     */
    void contextMenu( const QPoint& pos );

    /**
     * This signal is emitted whenever the tab bar is double-clicked.
     */
    void doubleClicked();

protected slots:
    void autoScrollLeft();
    void autoScrollRight();

protected:
    virtual void paintEvent ( QPaintEvent* ev );
    virtual void resizeEvent( QResizeEvent* ev );
    virtual void mousePressEvent ( QMouseEvent* ev );
    virtual void mouseReleaseEvent ( QMouseEvent* ev );
    virtual void mouseDoubleClickEvent ( QMouseEvent* ev );
    virtual void mouseMoveEvent( QMouseEvent* ev );

private:
    TabBarPrivate *d;

    // don't allow copy or assignment
    TabBar( const TabBar& );
    TabBar& operator=( const TabBar& );
};

};

// for source compatibility only, remove in the future
typedef KSpread::TabBar KSpreadTabBar;

#endif // KSPREAD_TABBAR
