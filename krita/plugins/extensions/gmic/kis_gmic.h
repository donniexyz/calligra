/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_GMIC_H_
#define _KIS_GMIC_H_

#include <kis_types.h>
#include <gmic.h>

class KisView2;
class KoUpdater;

class QImage;

class KisGmic : public QObject
{

    Q_OBJECT

public:

    KisGmic(KisView2 * view);
    virtual ~KisGmic() {}

    void processGmic(KoUpdater * progress, const QString &gmicString);

private:
    KisView2 * m_view;


    QImage convertToQImage(gmic_image<float>& gmicImage);
    void convertFromQImage(const QImage &image, gmic_image<float>& gmicImage);

    void convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float>& gmicImage);

    void convertToGmicImageOpti(KisPaintDeviceSP dev, gmic_image<float>& gmicImage);

    KisPaintDeviceSP convertFromGmicImage(gmic_image<float>& gmicImage);


};

#endif
