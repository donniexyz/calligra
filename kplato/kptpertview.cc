/* This file is part of the KDE project
   Copyright (C) 2003, 2004 Dag Andersen <danders@get2net.dk>

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
 
#include "kptpertview.h"

#include "kptview.h"
#include "kptpertcanvas.h"
#include "kptpart.h"
#include "kptproject.h"
#include "kptrelation.h"
#include "kptcontext.h"

#include <kdebug.h>

#include <qsplitter.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qheader.h>
#include <qpopupmenu.h>

#include <kprinter.h>

namespace KPlato
{

KPTPertView::KPTPertView( KPTView *view, QWidget *parent, QLayout *layout )
    : QWidget( parent, "Pert view" ),
    m_mainview( view ),
    m_node( 0 )
{
    init(layout);
}

KPTPertView::~KPTPertView()
{
}

void KPTPertView::init(QLayout *layout)
{
    //kdDebug()<<k_funcinfo<<endl;
    QGridLayout *gl = new QGridLayout(this, 1, 1, -1, -1, "Pert QGridLayout");
    m_canvasview = new KPTPertCanvas(this);
        gl->addWidget(m_canvasview, 0, 0);
    draw();
    connect(m_canvasview, SIGNAL(rightButtonPressed(KPTNode *, const QPoint &)), this, SLOT(slotRMBPressed(KPTNode *,const QPoint &)));
    connect(m_canvasview, SIGNAL(updateView(bool)), m_mainview, SLOT(slotUpdate(bool)));

    connect(m_canvasview, SIGNAL(addRelation(KPTNode*, KPTNode*)), SLOT(slotAddRelation(KPTNode*, KPTNode*)));
    connect(m_canvasview, SIGNAL(modifyRelation(KPTRelation*)), SLOT(slotModifyRelation(KPTRelation*)));
}    

void KPTPertView::draw() 
{
    //kdDebug()<<k_funcinfo<<endl;
    m_canvasview->draw(m_mainview->getPart()->getProject());
    m_canvasview->show();
}

void KPTPertView::slotRMBPressed(KPTNode *node, const QPoint & point)
{
    //kdDebug()<<k_funcinfo<<" node: "<<node->name()<<endl;
    m_node = node;
    if (node && (node->type() == KPTNode::Type_Task || node->type() == KPTNode::Type_Milestone)) {
        QPopupMenu *menu = m_mainview->popupMenu("task_popup");
        if (menu)
        {
            /*int id =*/ menu->exec(point);
            //kdDebug()<<k_funcinfo<<"id="<<id<<endl;
        }
        return;
    }
    if (node && node->type() == KPTNode::Type_Summarytask) {
        QPopupMenu *menu = m_mainview->popupMenu("node_popup");
        if (menu)
        {
            /*int id =*/ menu->exec(point);
            //kdDebug()<<k_funcinfo<<"id="<<id<<endl;
        }
        return;
    }
    //TODO: Other nodetypes
}

void KPTPertView::slotAddRelation(KPTNode* par, KPTNode* child)
{
    kdDebug()<<k_funcinfo<<endl;
    emit addRelation(par, child);
}

void KPTPertView::slotModifyRelation(KPTRelation *rel)
{
    kdDebug()<<k_funcinfo<<endl;
    emit modifyRelation(rel);
}

void KPTPertView::print(KPrinter &printer) {
    kdDebug()<<k_funcinfo<<endl;

}

KPTNode *KPTPertView::currentNode() {
    return m_canvasview->selectedNode(); 
}

bool KPTPertView::setContext(KPTContext &context) {
    kdDebug()<<k_funcinfo<<endl;
    return true;
}

void KPTPertView::getContext(KPTContext &context) const {
    kdDebug()<<k_funcinfo<<endl;
}

}  //KPlato namespace

#include "kptpertview.moc"
