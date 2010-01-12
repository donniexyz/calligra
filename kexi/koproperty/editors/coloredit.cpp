/* This file is part of the KDE project
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "coloredit.h"
#include "koproperty/Utils_p.h"

#include <QVariant>
#include <QLayout>
#include <QColor>
#include <QPainter>

#include <kcolorcombo.h>
#include <kcolorscheme.h>

using namespace KoProperty;

ColorCombo::ColorCombo(QWidget *parent)
        : KColorCombo(parent)
{
    connect(this, SIGNAL(activated(QColor)), this, SLOT(slotValueChanged(QColor)));
}

ColorCombo::~ColorCombo()
{
}

QVariant ColorCombo::value() const
{
    return color();
}

void ColorCombo::setValue(const QVariant &value)
{
    setColor(value.value<QColor>());
}

void ColorCombo::slotValueChanged(const QColor&)
{
    emit commitData(this);
}

QWidget * ColorComboDelegate::createEditor( int type, QWidget *parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    return new ColorCombo(parent);
}

void ColorComboDelegate::paint( QPainter * painter, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
//    painter->eraseRect(option.rect);
    painter->save();
    const QBrush b(index.data(Qt::EditRole).value<QColor>());
    painter->setBrush(b);
    painter->setPen(QPen());
    painter->drawRect(option.rect);
    painter->setBrush(KoProperty::contrastColor(b.color()));
    painter->setPen(KoProperty::contrastColor(b.color()));
    QFont f(option.font);
    f.setFamily("courier");
    painter->setFont(f);
    painter->drawText(option.rect, Qt::AlignCenter, b.color().name());
    painter->restore();
}

#include "coloredit.moc"
