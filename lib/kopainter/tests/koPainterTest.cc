/* This file is part of the KDE project
   Copyright (C) 2002 Igor Janssen <rm@linux.ru.net>

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

#include "koPainterTest.h"

#include <qpainter.h>
#include <qimage.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kdebug.h>

#include <koPainter.h>
#include <koOutline.h>
#include <koColor.h>

KoPainterTest::KoPainterTest(QWidget *widget, const char *name):
QWidget( widget, name )
{
  setFixedSize(800,600);
  p = new KoPainter(800, 600);
  p->fillAreaRGB(QRect(0,0,800,600), KoColor::white());
  p->fillAreaRGB(QRect(200,100,300,400), KoColor(20,200,180));
  p->drawRectRGB(QRect(199,99,302,402), KoColor::black());
  p->drawHorizLineRGB(200,502,501,KoColor::gray());
  p->drawVertLineRGB(502,100,501,KoColor::gray());
  KoOutline *o = new KoOutline;
  o->width(15.0);
  o->opacity(0xFF);
  o->cap(KoOutline::CapRound);
  o->color(KoColor::blue());
  p->outline(o);
  p->drawLine(100,50,550,200);
  o->cap(KoOutline::CapSquare);
  o->opacity(130);
  o->color(KoColor::yellow());
  p->drawLine(100,200,550,50);
  o->dashResize(2);
  o->setDash(0,5.0);
  o->setDash(1,5.0);
  o->cap(KoOutline::CapButt);
  o->opacity(150);
  o->width(3.0);
  o->color(KoColor::red());
  p->drawRect(100,120,200,200,20,30);
}

KoPainterTest::~KoPainterTest()
{
  delete p;
}

void KoPainterTest::paintEvent(QPaintEvent *)
{
//  p->blit(this);
  bitBlt((QPaintDevice *)this, 0, 0, p->image(), 0, 0, 800, 600);
}

int main(int argc, char **argv)
{
  KApplication *a = new KApplication(argc, argv, "kopaintertest");
  KoPainterTest *t = new KoPainterTest();
  a->setMainWidget(t);
  t->show();
  return a->exec();
}

#include "koPainterTest.moc"
