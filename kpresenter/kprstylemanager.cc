/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "kpresenter_doc.h"
#include "kprstylemanager.h"
#include "kprstylemanager.moc"
#include <koUnit.h>
#include <kdebug.h>
#include <koStylist.h>
#include <kostyle.h>
/******************************************************************/
/* Class: KWStyleManager                                          */
/******************************************************************/

KPrStyleManager::KPrStyleManager( QWidget *_parent, KoUnit::Unit unit,KPresenterDoc *_doc, const QPtrList<KoStyle> & style)
    : KoStyleManager(_parent,unit,style)
{
    m_doc = _doc;
}

KoStyle* KPrStyleManager::addStyleTemplate(KoStyle *style)
{
    return m_doc->styleCollection()->addStyleTemplate(style);
}

void KPrStyleManager::applyStyleChange( KoStyle * changedStyle, int paragLayoutChanged, int formatChanged )
{
    m_doc->applyStyleChange( changedStyle, paragLayoutChanged,formatChanged );
}

void KPrStyleManager::removeStyleTemplate( KoStyle *style )
{
    m_doc->styleCollection()->removeStyleTemplate(style);
}

void KPrStyleManager::updateAllStyleLists()
{
    m_doc->updateAllStyleLists();
}
