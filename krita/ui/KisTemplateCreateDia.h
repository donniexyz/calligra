/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2000 Werner Trobin <trobin@kde.org>

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

#ifndef KIS_TEMPLATE_CREATE_DIA_H
#define KIS_TEMPLATE_CREATE_DIA_H

#include <kdialog.h>
#include "krita_export.h"

class QString;
class QPixmap;
class KComponentData;
class KisDocument;
class KisTemplateCreateDiaPrivate;

/****************************************************************************
 *
 * Class: koTemplateCreateDia
 *
 ****************************************************************************/

class KRITAUI_EXPORT KisTemplateCreateDia : public KDialog
{
    Q_OBJECT

private:
    KisTemplateCreateDia( const char *templateType, const KComponentData &instance,
                         const QString &filePath, const QPixmap &thumbnail, QWidget *parent=0 );
    ~KisTemplateCreateDia();

public:
    static void createTemplate(const char *templateType, const char *suffix,
                               const KComponentData &componentData,
                               KisDocument *document, QWidget *parent = 0);

private Q_SLOTS:
    void slotOk();

    void slotDefault();
    void slotCustom();
    void slotSelect();
    void slotNameChanged(const QString &name);

    void slotAddGroup();
    void slotRemove();
    void slotSelectionChanged();
private:
    void updatePixmap();
    void fillGroupTree();

private:
    KisTemplateCreateDiaPrivate * const d;
};

#endif
