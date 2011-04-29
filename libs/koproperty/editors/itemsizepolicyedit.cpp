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

    QString nameForPolicy(bool v) {
        const int index = keys.indexOf(v);
        if (index == -1)
            return names[0];
        return names[index];
    }
private:
    static QList<QVariant> keysInternal() {
        QList<QVariant> keys;
        keys
         << false
         << true;
        return keys;
    }

    static QStringList stringsInternal() {
        QStringList strings;
        strings
         << i18nc("Size Policy", "No")
         << i18nc("Size Policy", "Yes");
        return strings;
    }
};

K_GLOBAL_STATIC(ItemSizePolicyListData, s_itemSizePolicyListData)

//---------

static const char *ITEMSIZEPOLICY_MASK = "%1, %2";

QString ItemSizePolicyDelegate::displayText( const QVariant& value ) const
{
    const QVariantList list = value.toList();
    Q_ASSERT(list.count() == 2);
    return QString::fromLatin1(ITEMSIZEPOLICY_MASK)
        .arg(s_itemSizePolicyListData->nameForPolicy(list[0].toBool()))
        .arg(s_itemSizePolicyListData->nameForPolicy(list[1].toBool()));
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
    new KoProperty::Property("horizontal-policy", QVariant(false), i18n("Horizontally"), i18n("Horizontal size policy"), KoProperty::Auto, property);
    new KoProperty::Property("vertical-policy", QVariant(false), i18n("Vertically"), i18n("Vertical size policy"), KoProperty::Auto, property);
}

void ItemSizePolicyComposedProperty::setValue(Property *property,
    const QVariant &value, bool rememberOldValue)
{
    const QVariantList list = value.toList();
    Q_ASSERT(list.count() == 2);
    property->child("horizontal-policy")->setValue(list[0], rememberOldValue, false);
    property->child("vertical-policy")->setValue(list[1], rememberOldValue, false);
}

void ItemSizePolicyComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    Q_UNUSED(rememberOldValue);
    Q_ASSERT(child);
    QVariantList list = child->parent()->value().toList();
    if (child->name() == "horizontal-policy") {
        list[0] = value;
    } else if (child->name() == "vertical-policy") {
        list[1] = value;
    }
    child->parent()->setValue(list, true, false);
}
