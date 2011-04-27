/* This file is part of the KDE project
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2011 Dag Andersen <danders@get2net.dk>

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

#include "itemsizepolicyedit.h"

#include <QtGui/QSizePolicy>
#include <KLocale>
#include <KGlobal>
#include <kdebug.h>

using namespace KoProperty;

class ItemSizePolicyListData : public Property::ListData
{
public:
    ItemSizePolicyListData() : Property::ListData(keysInternal(), stringsInternal())
    {
    }

    QString nameForPolicy(QSizePolicy::Policy p) {
        const int index = keys.indexOf((int)p);
        if (index == -1)
            return names[0];
        return names[index];
    }
private:
    static QList<QVariant> keysInternal() {
        QList<QVariant> keys;
        keys
         << QSizePolicy::Fixed
         << QSizePolicy::Expanding;
        return keys;
    }

    static QStringList stringsInternal() {
        QStringList strings;
        strings
         << i18nc("Size Policy", "Fixed")
         << i18nc("Size Policy", "Expanding");
        return strings;
    }
};

K_GLOBAL_STATIC(ItemSizePolicyListData, s_itemSizePolicyListData)

//---------

static const char *ITEMSIZEPOLICY_MASK = "%1, %2";

QString ItemSizePolicyDelegate::displayText( const QVariant& value ) const
{
    const QSizePolicy sp(value.value<QSizePolicy>());
    return QString::fromLatin1(ITEMSIZEPOLICY_MASK)
        .arg(s_itemSizePolicyListData->nameForPolicy(sp.horizontalPolicy()))
        .arg(s_itemSizePolicyListData->nameForPolicy(sp.verticalPolicy()));
}

//static
const Property::ListData& ItemSizePolicyDelegate::listData()
{
    return *s_itemSizePolicyListData;
}

//------------

ItemSizePolicyComposedProperty::ItemSizePolicyComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("hor_policy", new ItemSizePolicyListData(),
        QVariant(), i18n("Hor. Policy"), i18n("Horizontal Policy"), ValueFromList, property);
    (void)new Property("vert_policy", new ItemSizePolicyListData(),
        QVariant(), i18n("Vert. Policy"), i18n("Vertical Policy"), ValueFromList, property);
}

void ItemSizePolicyComposedProperty::setValue(Property *property,
    const QVariant &value, bool rememberOldValue)
{
    const QSizePolicy sp( value.value<QSizePolicy>() );
    property->child("hor_policy")->setValue(sp.horizontalPolicy(), rememberOldValue, false);
    property->child("vert_policy")->setValue(sp.verticalPolicy(), rememberOldValue, false);
}

void ItemSizePolicyComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    Q_UNUSED(rememberOldValue);
    QSizePolicy sp( child->parent()->value().value<QSizePolicy>() );
    if (child->name() == "hor_policy") {
        sp.setHorizontalPolicy(static_cast<QSizePolicy::Policy>(value.toInt()));
    } else if (child->name() == "vert_policy") {
        sp.setVerticalPolicy(static_cast<QSizePolicy::Policy>(value.toInt()));
    }
    child->parent()->setValue(sp, true, false);
}
