/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qsqlcursor.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqlindex.h>
#include <qvariant.h>
#include <qlayout.h>
#include <qstatusbar.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kexiapplication.h"

#include "kexidatatable.h" 
 
KexiDataTable::KexiDataTable(QWidget *parent, QString caption, const char *name)
	: KexiWidget(parent, name)
{
	QGridLayout *g = new QGridLayout(this);
	m_tableView = new KexiTableView(this);
	m_tableView->m_editOnDubleClick = true;
	m_statusBar = new QStatusBar(this);

	setCaption(i18n(caption + " - table"));

	g->addWidget(m_tableView,	0,	0);
	g->addWidget(m_statusBar,	1,	0);
	connect(m_tableView, SIGNAL(itemChanged(KexiTableItem *, int)), this, SLOT(slotItemChanged(KexiTableItem *, int)));
}

bool
KexiDataTable::executeQuery(QString queryStatement)
{
	QTime t;
	t.start();
	QSqlDatabase *db = kexi->project()->db();
	QSqlQuery query(queryStatement);

	QSqlCursor cursor;

	QStringList tables = getInvolvedTables(queryStatement);
	for(QStringList::Iterator it = tables.begin(); it != tables.end(); it++)
	{
		cursor = QSqlCursor((*it), true, db);
	}

	QSqlIndex index = cursor.primaryIndex();
	kdDebug() << "index: " << index.fieldName(0) << endl;


	QSqlRecord record = db->record(query);

	QSqlError error = query.lastError();
	if(error.type() != QSqlError::None)
	{
		QString errorText = error.databaseText();
		KMessageBox::sorry(this, i18n("<qt>Error in your sql-statement:<br><br><b>" + errorText + "</b>"), i18n("Query Statement"));
		return false; 
	}

	unsigned int fields = record.count();

	for (unsigned int i = 0; i < record.count(); i++)
	{
		//WARNING: look for the type!!!
		m_tableView->addColumn(record.field(i)->name(), record.field(i)->type(), true);
	}

	while(query.next())
	{
		KexiTableItem *it = new KexiTableItem(m_tableView);
		for(unsigned int i=0; i < fields; i++)
		{
			it->setValue(i, query.value(i));
		}
	}
	
	m_statusBar->message(QString::number(query.numRowsAffected()) + " rows in " + QString::number(t.elapsed()) + "ms");
	
	return true;
}

void
KexiDataTable::slotItemChanged(KexiTableItem *i, int col)
{
	kdDebug() << "CHANGED!!!" << endl;
}

QStringList
KexiDataTable::getInvolvedTables(QString query)
{
	// switching to perl-mode :)
	// for now it's enought to get the main table
	// rest comes later...

	QStringList tableList;

	QStringList queryFrags = QStringList::split(" ", query);

	bool from = false;
	for(QStringList::Iterator it = queryFrags.begin(); it != queryFrags.end(); it++)
	{
		if(from)
		{
			tableList.append(*it);
			kdDebug() << "used table: '" << *it << "'" << endl;
			break;
		}

		if(*it == "from")
			from = true;

	}


	return tableList;
}

KexiDataTable::~KexiDataTable()
{
}

#include "kexidatatable.moc"
