/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 2002 Igor Jansen (rm@kde.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "DashEditDialog.h"

#include <qlayout.h>
#include <qlabel.h>

#include <klocale.h>

DashEditDialog::DashEditDialog(KontourView *aView, GDocument *aGDoc, QWidget *parent, const char *name):
KDialogBase(KDialogBase::Plain, i18n("Dashes Manager"), Ok|Apply|Cancel, Ok, parent, name, true)
{
  mView = aView;
  mGDoc = aGDoc;
  createWidget(plainPage());
}

void DashEditDialog::slotApply()
{

}

void DashEditDialog::slotOk()
{

}

void DashEditDialog::createWidget(QWidget *parent)
{
  QLabel *mNumLabel =  new QLabel(i18n("Number"), parent);

  QGridLayout *layout = new QGridLayout(parent, 2, 3);
  layout->addWidget(mNumLabel, 0, 0);
}

#include "DashEditDialog.moc"
