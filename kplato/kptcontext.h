/* This file is part of the KDE project
   Copyright (C) 2005 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef KPTCONTEXT_H
#define KPTCONTEXT_H

#include <qstring.h>
#include <qstringlist.h>

class QDomElement;

namespace KPlato
{

class KPTContext {
public:
    KPTContext();
    virtual ~KPTContext();
    
    virtual bool load(QDomElement &element);
    virtual void save(QDomElement &element) const;
    

    // KPTView
    QString currentView;
    
    // Ganttview
    QString currentNode;
    QStringList closedNodes;
    
};

}  //KPlato namespace

#endif //KPTCONTEXT_H
