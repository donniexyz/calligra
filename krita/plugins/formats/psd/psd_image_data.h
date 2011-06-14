/*
 *  Copyright (c) 2011 siddharth SHARMA <siddharth.kde@gmail.com>
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

#ifndef PSD_IMAGE_DATA_H
#define PSD_IMAGE_DATA_H

#include "psd_header.h"
#include "psd_layer_section.h"
#include "psd_layer_record.h"
#include <QImage>
#include <QFile>

#include <kis_paint_device.h>
#include <kis_types.h>
class QIODevice;

class PSDImageData
{

public:
    PSDImageData(PSDHeader *header);
    virtual ~PSDImageData();

    bool read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header);
    bool readRawData(KisPaintDeviceSP dev,QIODevice *io, PSDHeader *header);
    bool readRLEData(KisPaintDeviceSP dev,QIODevice *io, PSDHeader *header);

    quint16 compression;
    quint64 channelDataLength;
    quint32 channelSize;

    QByteArray r,g,b,a; // RGB
    QByteArray cba,mba,yba,kba; // CMYK
    QByteArray lb,ab,bb; // LAB
};

#endif // PSD_IMAGE_DATA_H
