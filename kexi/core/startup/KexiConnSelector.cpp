/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include "KexiConnSelector.h"

#include <kexidb/drivermanager.h>
#include <kexidb/connectiondata.h>

#include "kexi.h"
#include "KexiConnSelectorBase.h"
#include "KexiOpenExistingFile.h"

#include <kapplication.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kurlcombobox.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

ConnectionDataLVItem::ConnectionDataLVItem(KexiDB::ConnectionData *d, 
	const KexiDB::Driver::Info& info, QListView *list)
	: QListViewItem(list)
	, data(d)
{
	setText(0, data->connName+"  ");
	const QString &sfile = i18n("File");
	QString drvname = info.caption.isEmpty() ? data->driverName : info.caption;
	if (info.fileBased)
		setText(1, sfile + " ("+drvname+")  " );
	else
		setText(1, drvname+"  " );
	setText(2, (info.fileBased ? (QString("<")+sfile.lower()+">") : data->serverInfoString())+"  " );
}

ConnectionDataLVItem::~ConnectionDataLVItem() 
{
}

/*================================================================*/

class KexiConnSelectorWidgetPrivate
{
public:
	KexiConnSelectorWidgetPrivate()
	: conn_sel_shown(false)
	, file_sel_shown(false)
	{
	}
	
	bool conn_sel_shown;//! helper
	bool file_sel_shown;
};

/*================================================================*/

KexiConnSelectorWidget::KexiConnSelectorWidget( const KexiDBConnectionSet& conn_set, QWidget* parent,  const char* name )
    : QWidgetStack( parent, name )
	,m_conn_set(&conn_set)
	,d(new KexiConnSelectorWidgetPrivate())
{
	QString none, iconname = KMimeType::mimeType("application/x-kexiproject-sqlite")->icon(none,0);
	const QPixmap &icon = KGlobal::iconLoader()->loadIcon( iconname, KIcon::Desktop );
	setIcon( icon );
	
	m_file = new KexiOpenExistingFile( this, "KexiOpenExistingFile");
	m_file->btn_advanced->setIconSet( SmallIconSet("1downarrow") );
	m_fileDlg = 0;
		
	addWidget(m_file);
	connect(m_file->btn_advanced,SIGNAL(clicked()),this,SLOT(showAdvancedConn()));

	m_remote = new KexiConnSelectorBase(this,"conn_sel");
	m_remote->icon->setPixmap( DesktopIcon("socket") );
	m_remote->btn_back->setIconSet( SmallIconSet("1uparrow") );
	addWidget(m_remote);
	if (m_remote->layout())
		m_remote->layout()->setMargin(0);
	connect(m_remote->btn_back,SIGNAL(clicked()),this,SLOT(showSimpleConn()));
	connect(m_remote->list,SIGNAL(doubleClicked(QListViewItem*)),
		this,SLOT(slotConnectionItemExecuted(QListViewItem*)));
	connect(m_remote->list,SIGNAL(returnPressed(QListViewItem*)),
		this,SLOT(slotConnectionItemExecuted(QListViewItem*)));
}

/*!  
 *  Destroys the object and frees any allocated resources
 */
KexiConnSelectorWidget::~KexiConnSelectorWidget()
{
	delete d;
}

void KexiConnSelectorWidget::disconnectShowSimpleConnButton()
{
	m_remote->btn_back->disconnect(this,SLOT(showSimpleConn()));
}

void KexiConnSelectorWidget::showAdvancedConn()
{
	if (!d->conn_sel_shown) {
		d->conn_sel_shown=true;
		//show connections (on demand):
		KexiDB::DriverManager manager;
		KexiDB::ConnectionData::List connlist = m_conn_set->list();
		KexiDB::ConnectionData *data = connlist.first();
		while (data) {
			KexiDB::Driver::Info info = manager.driverInfo(data->driverName);
			if (!info.name.isEmpty()) {
				new ConnectionDataLVItem(data, info, m_remote->list);
			}
			else {
				kdWarning() << "KexiConnSelector::KexiConnSelector(): no driver found for '" << data->driverName << "'!" << endl;
			}
			data=connlist.next();
		}
		if (m_remote->list->firstChild()) {
			m_remote->list->setSelected(m_remote->list->firstChild(),true);
		}
		m_remote->list->setFocus();
	}
	raiseWidget(m_remote);
}

void KexiConnSelectorWidget::showSimpleConn()
{
	if (!d->file_sel_shown) {
		d->file_sel_shown=true;
		m_fileDlg = new KexiStartupFileDialog( "", KexiStartupFileDialog::Opening,
			m_file, "openExistingFileDlg");
		static_cast<QVBoxLayout*>(m_file->layout())->insertWidget( 2, m_fileDlg );

		for (QWidget *w = parentWidget(true);w;w=w->parentWidget(true)) {
			if (w->isDialog()) {
//#ifndef Q_WS_WIN
				connect(m_fileDlg,SIGNAL(cancelClicked()),static_cast<QDialog*>(w),SLOT(reject()));
//#endif
//				connect(m_fileDlg,SIGNAL(cancelled()),static_cast<QDialog*>(w),SLOT(reject()));
				break;
			}
		}
	}
	raiseWidget(m_file);
}

int KexiConnSelectorWidget::selectedConnectionType() const
{
	return (visibleWidget()==m_file) ? FileBased : ServerBased;
}

KexiDB::ConnectionData* KexiConnSelectorWidget::selectedConnectionData() const
{
	if (selectedConnectionType()==KexiConnSelectorWidget::FileBased)
		return 0;
	ConnectionDataLVItem *item = static_cast<ConnectionDataLVItem*>(m_remote->list->selectedItem());
	if (item)
		return item->data;
	return 0;
}

void KexiConnSelectorWidget::slotConnectionItemExecuted(QListViewItem *item)
{
	emit connectionItemExecuted(static_cast<ConnectionDataLVItem*>(item));
}

QListView* KexiConnSelectorWidget::connectionsList() const
{
	return m_remote->list;
}

void KexiConnSelectorWidget::setFocus()
{
	QWidgetStack::setFocus();
	if (visibleWidget()==m_file)
		m_fileDlg->setFocus(); //m_fileDlg->locationWidget()->setFocus();
	else
		m_remote->list->setFocus();
}

#include "KexiConnSelector.moc"

