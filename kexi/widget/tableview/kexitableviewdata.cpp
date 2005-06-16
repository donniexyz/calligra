/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2005 Jaroslaw Staniek <js@iidea.pl>

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
 
   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include "kexitableviewdata.h"

#include <kexiutils/validator.h>

#include <kexidb/field.h>
#include <kexidb/queryschema.h>
#include <kexidb/roweditbuffer.h>
#include <kexidb/cursor.h>
#include <kexidb/utils.h>
#include <kexi.h>

#include <kdebug.h>
#include <klocale.h>

#include <qapplication.h>

unsigned short KexiTableViewData::charTable[]=
{
	#include "chartable.txt"
};

KexiTableViewColumn::KexiTableViewColumn(KexiDB::Field& f, bool owner)
: fieldinfo(0)
, m_field(&f)
{
	isDBAware = false;
	m_fieldOwned = owner;
	m_captionAliasOrName = m_field->captionOrName();
	init();
}

KexiTableViewColumn::KexiTableViewColumn(const QString& name, KexiDB::Field::Type ctype,
	uint cconst,
	uint options,
	uint length, uint precision,
	QVariant defaultValue,
	const QString& caption, const QString& description, uint width
)
: fieldinfo(0)
{
	m_field = new KexiDB::Field(
		name, ctype,
		cconst,
		options,
		length, precision,
		defaultValue,
		caption, description, width);

	isDBAware = false;
	m_fieldOwned = true;
	m_captionAliasOrName = m_field->captionOrName();
	init();
}

KexiTableViewColumn::KexiTableViewColumn(
	const KexiDB::QuerySchema &query, KexiDB::QueryColumnInfo& fi)
//	const KexiDB::QuerySchema &query, KexiDB::Field& f)
: fieldinfo(&fi)
, m_field(fi.field)
{
	isDBAware = true;
	m_fieldOwned = false;

	//setup column's caption:
	if (!fieldinfo->field->caption().isEmpty()) {
		m_captionAliasOrName = fieldinfo->field->caption();
	}
	else {
		//reuse alias if available:
		m_captionAliasOrName = fieldinfo->alias;
		//last hance: use field name
		if (m_captionAliasOrName.isEmpty())
			m_captionAliasOrName = fieldinfo->field->name();
		//todo: compute other auto-name?
	}
	init();
	//setup column's readonly flag: true if this is parent table's field
	m_readOnly = (query.masterTable()!=fieldinfo->field->table());
	kdDebug() << "KexiTableViewColumn: query.masterTable()==" 
		<< (query.masterTable() ? query.masterTable()->name() : "notable") << ", fieldinfo->field->table()=="
		<< (fieldinfo->field->table() ? fieldinfo->field->table()->name()  : "notable") << endl;
//	m_visible = query.isFieldVisible(&f);
}

KexiTableViewColumn::KexiTableViewColumn(bool)
: fieldinfo(0)
, m_field(0)
{
	isDBAware = false;
	init();
}

KexiTableViewColumn::~KexiTableViewColumn()
{
	if (m_fieldOwned)
		delete m_field;
	setValidator( 0 );
	delete m_relatedData;
}

void KexiTableViewColumn::init()
{
	m_relatedData = 0;
	m_readOnly = false;
	m_visible = true;
	m_data = 0;
	m_validator = 0;
	m_relatedDataEditable = false;
}

void KexiTableViewColumn::setValidator( KexiUtils::Validator* v )
{
	if (m_validator) {//remove old one
		if (!m_validator->parent()) //destroy if has no parent
			delete m_validator;
	}
	m_validator = v;
}

void KexiTableViewColumn::setRelatedData(KexiTableViewData *data)
{
	if (isDBAware)
		return;
	if (m_relatedData)
		delete m_relatedData;
	m_relatedData = 0;
	if (!data)
		return;
	//find a primary key
	KexiTableViewColumn::ListIterator it( data->columns );
	for (int id = 0;it.current();++it, id++) {
		if (it.current()->field()->isPrimaryKey()) {
			//found, remember
			m_relatedDataPKeyID = id;
			m_relatedData = data;
			return;
		}
	}
}

void KexiTableViewColumn::setRelatedDataEditable(bool set)
{
	m_relatedDataEditable = set;
}

bool KexiTableViewColumn::acceptsFirstChar(const QChar& ch) const
{
	if (m_field->isNumericType()) {
		if (ch=="-")
			 return !m_field->isUnsigned();
		if (ch=="+" || (ch>="0" && ch<="9"))
			return true;
		return false;
	}

	switch (m_field->type()) {
	case KexiDB::Field::Boolean:
		return false;
	case KexiDB::Field::Date:
	case KexiDB::Field::DateTime:
	case KexiDB::Field::Time:
		return ch>="0" && ch<="9";
	default:;
	}
	return true;
}


//------------------------------------------------------

KexiTableViewData::KexiTableViewData()
	: QObject()
	, KexiTableViewDataBase()
{
	init();
}

KexiTableViewData::KexiTableViewData(KexiDB::Cursor *c)
	: QObject()
	, KexiTableViewDataBase()
{
	init();
	m_cursor = c;
	m_containsROWIDInfo = m_cursor->containsROWIDInfo();

	KexiDB::QueryColumnInfo::Vector vector 
		= m_cursor->query()->fieldsExpanded();
	KexiTableViewColumn* col;
	const uint count = vector.count();
	for (uint i=0;i<count;i++) {
		KexiDB::QueryColumnInfo *fi = vector[i];
		if (fi->visible) {
			col=new KexiTableViewColumn(*m_cursor->query(), *fi);
			//col->setVisible( detailedVisibility[i] );
			addColumn( col );
		}
	}
}

KexiTableViewData::KexiTableViewData(
	const QValueList<QVariant> &keys, const QValueList<QVariant> &values,
	KexiDB::Field::Type keyType, KexiDB::Field::Type valueType)
	: QObject()
	, KexiTableViewDataBase()
{
	init(keys, values, keyType, valueType);
}

KexiTableViewData::KexiTableViewData(
	KexiDB::Field::Type keyType, KexiDB::Field::Type valueType)
{
	const QValueList<QVariant> empty;
	init(empty, empty, keyType, valueType);
}

KexiTableViewData::~KexiTableViewData()
{
	emit destroying();
}

void KexiTableViewData::init(
	const QValueList<QVariant> &keys, const QValueList<QVariant> &values,
	KexiDB::Field::Type keyType, KexiDB::Field::Type valueType)
{
	init();
	KexiDB::Field *keyField = new KexiDB::Field("key", keyType);
	keyField->setPrimaryKey(true);
	KexiTableViewColumn *keyColumn = new KexiTableViewColumn(*keyField, true);
	keyColumn->setVisible(false);
	addColumn(keyColumn);

	KexiDB::Field *valueField = new KexiDB::Field("value", valueType);
	KexiTableViewColumn *valueColumn = new KexiTableViewColumn(*valueField, true);
	addColumn(valueColumn);

	uint cnt = QMIN(keys.count(), values.count());
	QValueList<QVariant>::ConstIterator it_keys = keys.constBegin();
	QValueList<QVariant>::ConstIterator it_values = values.constBegin();
	for (;cnt>0;++it_keys, ++it_values, cnt--) {
		KexiTableItem *item = new KexiTableItem(2);
		(*item)[0] = (*it_keys);
		(*item)[1] = (*it_values);
		append( item );
	}
}

/*
KexiTableViewData::KexiTableViewData(KexiTableViewColumnList& cols) 
	: KexiTableViewDataBase()
	, columns(cols)
	, m_key(0)
	, m_order(1)
	, m_type(1)
	, m_pRowEditBuffer(0)
	, m_readOnly(false)
	, m_insertingEnabled(true)
{
	setAutoDelete(true);
	columns.setAutoDelete(true);
}*/

void KexiTableViewData::init()
{
	m_key = 0;
//	m_order = 1;
	m_order = 0;
	m_type = 1;
	m_pRowEditBuffer = 0;
	m_cursor = 0;
	m_readOnly = false;
	m_insertingEnabled = true;

	setAutoDelete(true);
	columns.setAutoDelete(true);
	m_visibleColumnsCount=0;
	m_visibleColumnsIDs.resize(100);
	m_globalColumnsIDs.resize(100);

	m_autoIncrementedColumn = -2;
	m_containsROWIDInfo = false;
}

void KexiTableViewData::addColumn( KexiTableViewColumn* col )
{
//	if (!col->isDBAware) {
//		if (!m_simpleColumnsByName)
//			m_simpleColumnsByName = new QDict<KexiTableViewColumn>(101);
//		m_simpleColumnsByName->insert(col->caption,col);//for faster lookup
//	}
	columns.append( col );
	col->m_data = this;
	if (m_globalColumnsIDs.size() < columns.count()) {//sanity
		m_globalColumnsIDs.resize( m_globalColumnsIDs.size()*2 );
	}
	if (col->visible()) {
		m_visibleColumnsCount++;
		if (m_visibleColumnsIDs.size() < m_visibleColumnsCount) {//sanity
			m_visibleColumnsIDs.resize( m_visibleColumnsIDs.size()*2 );
		}
		m_visibleColumnsIDs[ columns.count()-1 ] = m_visibleColumnsCount-1;
		m_globalColumnsIDs[ m_visibleColumnsCount-1 ] = columns.count()-1;
	}
	else {
		m_visibleColumnsIDs[ columns.count()-1 ] = -1;
	}
	m_autoIncrementedColumn = -2; //clear cache;
}

/*void KexiTableViewData::addColumns( KexiDB::QuerySchema *query, KexiDB::Field *field )
{
	field->isQueryAsterisk
}*/

QString KexiTableViewData::dbTableName() const
{
	if (m_cursor && m_cursor->query() && m_cursor->query()->masterTable())
		return m_cursor->query()->masterTable()->name();
	return QString::null;
}

void KexiTableViewData::setSorting(int column, bool ascending)
{
//	m_order = (ascending ? 1 : -1);

	if (column>=0 && column<(int)columns.count()) {
		m_order = (ascending ? 1 : -1);
		m_key = column;
	} 
	else {
		m_order = 0;
		m_key = -1;
		return;
	}

	const int t = columns.at(m_key)->field()->type();
	if (t == KexiDB::Field::Boolean || KexiDB::Field::isNumericType(t))
		cmpFunc = &KexiTableViewData::cmpInt;
	else
		cmpFunc = &KexiTableViewData::cmpStr;
}

int KexiTableViewData::compareItems(Item item1, Item item2)
{
	return ((this->*cmpFunc) (item1, item2));
}

int KexiTableViewData::cmpInt(Item item1, Item item2)
{
	return m_order* (((KexiTableItem *)item1)->at(m_key).toInt() - ((KexiTableItem *)item2)->at(m_key).toInt());
}

int KexiTableViewData::cmpStr(Item item1, Item item2)
{
	const QString &as =((KexiTableItem *)item1)->at(m_key).toString();
	const QString &bs =((KexiTableItem *)item2)->at(m_key).toString();

	const QChar *a = as.unicode();
	const QChar *b = bs.unicode();

	if ( a == b )
		return 0;
	if ( a == 0 )
		return 1;
	if ( b == 0 )
		return -1;

	unsigned short au;
	unsigned short bu;

	int l=QMIN(as.length(),bs.length());

	au = a->unicode();
	bu = b->unicode();
	au = (au <= 0x17e ? charTable[au] : 0xffff);
	bu = (bu <= 0x17e ? charTable[bu] : 0xffff);

	while (l-- && au == bu)
	{
		a++,b++;
		au = a->unicode();
		bu = b->unicode();
		au = (au <= 0x17e ? charTable[au] : 0xffff);
		bu = (bu <= 0x17e ? charTable[bu] : 0xffff);
	}

	if ( l==-1 )
		return m_order*(as.length()-bs.length());

	return m_order*(au-bu);
}

void KexiTableViewData::setReadOnly(bool set)
{
	if (m_readOnly == set)
		return;
	m_readOnly = set;
	if (m_readOnly)
		setInsertingEnabled(false);
}

void KexiTableViewData::setInsertingEnabled(bool set)
{
	if (m_insertingEnabled == set)
		return;
	m_insertingEnabled = set;
	if (m_insertingEnabled)
		setReadOnly(false);
}

void KexiTableViewData::clearRowEditBuffer()
{
	//init row edit buffer
	if (!m_pRowEditBuffer)
		m_pRowEditBuffer = new KexiDB::RowEditBuffer(isDBAware());
	else
		m_pRowEditBuffer->clear();
}

bool KexiTableViewData::updateRowEditBufferRef(KexiTableItem *item, 
	int colnum, KexiTableViewColumn* col, QVariant& newval, bool allowSignals)
{
	m_result.clear();
	if (allowSignals)
		emit aboutToChangeCell(item, colnum, newval, &m_result);
	if (!m_result.success)
		return false;

	kdDebug() << "KexiTableViewData::updateRowEditBufferRef() column #" 
		<< colnum << " = " << newval.toString() << endl;
//	KexiTableViewColumn* col = columns.at(colnum);
	if (!col) {
		kdDebug() << "KexiTableViewData::updateRowEditBufferRef(): column #" 
			<< colnum << " not found! col==0" << endl;
		return false;
	}
	if (m_pRowEditBuffer->isDBAware()) {
		if (!(col->fieldinfo)) {
			kdDebug() << "KexiTableViewData::updateRowEditBufferRef(): column #" 
				<< colnum << " not found!" << endl;
			return false;
		}
//		if (!(static_cast<KexiDBTableViewColumn*>(col)->field)) {
		m_pRowEditBuffer->insert( *col->fieldinfo, newval);
		return true;
	}
	if (!(col->field())) {
		kdDebug() << "KexiTableViewData::updateRowEditBufferRef(): column #" << colnum<<" not found!" << endl;
		return false;
	}
	//not db-aware:
	const QString colname = col->field()->name();
	if (colname.isEmpty()) {
		kdDebug() << "KexiTableViewData::updateRowEditBufferRef(): column #" << colnum<<" not found!" << endl;
		return false;
	}
	m_pRowEditBuffer->insert(colname, newval);
	return true;
}

//get a new value (of present in the buffer), or the old one, otherwise
//(taken here for optimization)
#define GET_VALUE if (!val) { \
	val = m_cursor ? rowEditBuffer()->at( *it_f.current()->fieldinfo ) : rowEditBuffer()->at( *f ); \
	if (!val) \
		val = &(*it_r); /* get old value */ \
	}

//js TODO: if there can be multiple views for this data, we need multiple buffers!
bool KexiTableViewData::saveRow(KexiTableItem& item, bool insert, bool repaint)
{
	if (!m_pRowEditBuffer)
		return true; //nothing to do

	//check constraints:
	//-check if every NOT NULL and NOT EMPTY field is filled
	KexiTableViewColumn::ListIterator it_f(columns);
	KexiDB::RowData::ConstIterator it_r = item.constBegin();
	int col = 0;
	const QVariant *val;
	for (;it_f.current() && it_r!=item.constEnd();++it_f,++it_r,col++) {
		KexiDB::Field *f = it_f.current()->field();
		val = 0;
		if (f->isNotNull()) {
			GET_VALUE;
			//check it
			if (val->isNull() && !f->isAutoIncrement()) {
				//NOT NULL violated
				m_result.msg = i18n("\"%1\" column requires a value to be entered.")
					.arg(f->captionOrName()) + "\n\n" + Kexi::msgYouCanImproveData();
				m_result.desc = i18n("The column's constraint is declared as NOT NULL.");
				m_result.column = col;
				return false;
			}
		}
		if (f->isNotEmpty()) {
			GET_VALUE;
			if (!f->isAutoIncrement() && (val->isNull() || KexiDB::isEmptyValue( f, *val ))) {
				//NOT EMPTY violated
				m_result.msg = i18n("\"%1\" column requires a value to be entered.")
					.arg(f->captionOrName()) + "\n\n" + Kexi::msgYouCanImproveData();
				m_result.desc = i18n("The column's constraint is declared as NOT EMPTY.");
				m_result.column = col;
				return false;
			}
		}
	}

	if (m_cursor) {//db-aware
		if (insert) {
			if (!m_cursor->insertRow( static_cast<KexiDB::RowData&>(item), *rowEditBuffer(), 
				m_containsROWIDInfo/*also retrieve ROWID*/ )) 
			{
				m_result.msg = i18n("Row inserting failed.") + "\n\n" 
					+ Kexi::msgYouCanImproveData();
				KexiDB::getHTMLErrorMesage(m_cursor, &m_result);

/*			if (desc)
			*desc = 
js: TODO: use KexiMainWindowImpl::showErrorMessage(const QString &title, KexiDB::Object *obj)
	after it will be moved somewhere to kexidb (this will require moving other 
	  showErrorMessage() methods from KexiMainWindowImpl to libkexiutils....)
	then: just call: *desc = KexiDB::errorMessage(m_cursor);
*/
				return false;
			}
		}
		else {
//			if (m_containsROWIDInfo)
//				ROWID = item[columns.count()].toULongLong();
			if (!m_cursor->updateRow( static_cast<KexiDB::RowData&>(item), *rowEditBuffer(),
					m_containsROWIDInfo/*use ROWID*/))
			{
				m_result.msg = i18n("Row changing failed.") + "\n\n" + Kexi::msgYouCanImproveData();
				KexiDB::getHTMLErrorMesage(m_cursor, m_result.desc);
				return false;
			}
		}
	}
	else {//js UNTESTED!!! - not db-aware version
		KexiDB::RowEditBuffer::SimpleMap b = m_pRowEditBuffer->simpleBuffer();
		for (KexiDB::RowEditBuffer::SimpleMap::ConstIterator it = b.constBegin();it!=b.constEnd();++it) {
			uint i=0;
			for (KexiTableViewColumn::ListIterator it2(columns);it2.current();++it2, i++) {
				if (it2.current()->field()->name()==it.key()) {
					kdDebug() << it2.current()->field()->name()<< ": "<<item[i].toString()<<" -> "<<it.data().toString()<<endl;
					item[i] = it.data();
				}
			}
		}
	}
	if (repaint)
		emit rowRepaintRequested(item);
	return true;
}

bool KexiTableViewData::saveRowChanges(KexiTableItem& item, bool repaint)
{
	kdDebug() << "KexiTableViewData::saveRowChanges()..." << endl;
	m_result.clear();
	emit aboutToUpdateRow(&item, rowEditBuffer(), &m_result);
	if (!m_result.success)
		return false;

	if (saveRow(item, false /*update*/, repaint)) {
		emit rowUpdated(&item);
		return true;
	}
	return false;
}

bool KexiTableViewData::saveNewRow(KexiTableItem& item, bool repaint)
{
	kdDebug() << "KexiTableViewData::saveNewRow()..." << endl;
	m_result.clear();
	emit aboutToInsertRow(&item, &m_result, repaint);
	if (!m_result.success)
		return false;
	
	if (saveRow(item, true /*insert*/, repaint)) {
		emit rowInserted(&item, repaint);
		return true;
	}
	return false;
}

bool KexiTableViewData::deleteRow(KexiTableItem& item, bool repaint)
{
	m_result.clear();
	emit aboutToDeleteRow(item, &m_result, repaint);
	if (!m_result.success)
		return false;

	if (m_cursor) {//db-aware
		m_result.success = false;
		if (!m_cursor->deleteRow( static_cast<KexiDB::RowData&>(item), m_containsROWIDInfo/*use ROWID*/ )) {
			m_result.msg = i18n("Row deleting failed.");
/*js: TODO: use KexiDB::errorMessage() for description (desc) as in KexiTableViewData::saveRow() */
			KexiDB::getHTMLErrorMesage(m_cursor, &m_result);
			m_result.success = false;
			return false;
		}
	}

	if (!removeRef(&item)) {
		//aah - this shouldn't be!
		kdWarning() << "KexiTableViewData::deleteRow(): !removeRef() - IMPL. ERROR?" << endl;
		m_result.success = false;
		return false;
	}
	emit rowDeleted();
	return true;
}

void KexiTableViewData::deleteRows( const QValueList<int> &rowsToDelete, bool repaint )
{
	if (rowsToDelete.isEmpty())
		return;
	int last_r=0;
	first();
	for (QValueList<int>::ConstIterator r_it = rowsToDelete.constBegin(); r_it!=rowsToDelete.constEnd(); ++r_it) {
		for (; last_r<(*r_it); last_r++) {
			next();
		}
		remove();
		last_r++;
	}
//DON'T CLEAR BECAUSE KexiTableViewPropertyBuffer will clear BUFFERS!
//-->	emit reloadRequested(); //! \todo more effective?
	emit rowsDeleted( rowsToDelete );
}

void KexiTableViewData::insertRow(KexiTableItem& item, uint index, bool repaint)
{
	if (!insert( index = QMIN(index, count()), &item ))
		return;
	emit rowInserted(&item, index, repaint);
}

//void KexiTableViewData::clear()
void KexiTableViewData::clearInternal()
{
	clearRowEditBuffer();
	qApp->processEvents( 1 );
//TODO: this is time consuming: find better data model
	KexiTableViewDataBase::clear();
}

bool KexiTableViewData::deleteAllRows(bool repaint)
{
	clearInternal();

	bool res = true;
	if (m_cursor) {
		//db-aware
		res = m_cursor->deleteAllRows();
	}

	if (repaint)
		emit reloadRequested();
	return res;
}

int KexiTableViewData::autoIncrementedColumn()
{
	if (m_autoIncrementedColumn==-2) {
		//find such a column
		m_autoIncrementedColumn = 0;
		KexiTableViewColumn::ListIterator it(columns);
		for (; it.current(); ++it, m_autoIncrementedColumn++) {
			if (it.current()->field()->isAutoIncrement())
				break;
		}
		if (!it.current())
			m_autoIncrementedColumn = -1;
	}
	return m_autoIncrementedColumn;
}

void KexiTableViewData::preloadAllRows()
{
	if (!m_cursor)
		return;

	const uint fcount = m_cursor->fieldCount() + (m_containsROWIDInfo ? 1 : 0);
	m_cursor->moveFirst();
	for (int i=0;!m_cursor->eof();i++) {
		KexiTableItem *item = new KexiTableItem(fcount);
		m_cursor->storeCurrentRow(*item);
		append( item );
		m_cursor->moveNext();
		if ((i % 1000) == 0)
			qApp->processEvents( 1 );
	}
}

#include "kexitableviewdata.moc"

