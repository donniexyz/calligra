/* This file is part of the KDE project
   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOMARKER_H
#define KOMARKER_H

#include <QMetaType>

class KoXmlElement;
class KoShapeLoadingContext;
class KoShapeSavingContext;
class QString;
class QPainterPath;

class KoMarker
{
public:
    KoMarker();
    ~KoMarker();

    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    void saveOdf(KoShapeSavingContext &context) const;

    QString name() const;
    QPainterPath path() const;

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoMarker*)

#endif /* KOMARKER_H */
