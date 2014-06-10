/* This file is part of the KDE project

   Copyright (C) 2013 Inge Wallin  <inge@lysator.liu.se>

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
   Boston, MA 02110-1301, USA.
*/


// Own
#include "OdfReaderWikiContext.h"

// Calligra
#include <KoXmlReader.h>
#include <KoOdfStyleManager.h>
#include <KoOdfStyleProperties.h>
#include <KoOdfStyle.h>



// ----------------------------------------------------------------
//                     class OdfReaderWikiContext


OdfReaderWikiContext::OdfReaderWikiContext(KoStore *store, QFile &file)
    : OdfReaderContext(store)
    , outStream(&file)
    , listLevelCounter(0)
{
}

OdfReaderWikiContext::~OdfReaderWikiContext()
{
}

KoOdfStyle *OdfReaderWikiContext::currentStyleProperties(KoXmlStreamReader &reader) const
{
    QString stylename = reader.attributes().value("style-name").toString();
    QString styleFamily = reader.attributes().value("style-family").toString();
    KoOdfStyle *style = styleManager()->style(stylename, styleFamily);
    return style;
}
void OdfReaderWikiContext::pushStyle(KoOdfStyle *style)
{
    styleStack.push(style);
}

KoOdfStyle *OdfReaderWikiContext::popStyle()
{
    return styleStack.pop();
}

void OdfReaderWikiContext::pushListStyle(KoOdfListStyle *style)
{
    listStyleStack.push(style);
}

KoOdfListStyle *OdfReaderWikiContext::popListStyle()
{
    return listStyleStack.pop();
}
