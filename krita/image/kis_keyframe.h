/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#ifndef KIS_KEYFRAME_H
#define KIS_KEYFRAME_H

#include <QVariant>

#include "krita_export.h"
#include "kis_keyframe.h"

class KisKeyframeChannel;

class KRITAIMAGE_EXPORT KisKeyframe : public QObject {
    Q_OBJECT

public:
    KisKeyframe(KisKeyframeChannel *channel, int time, const QVariant &value)
        : QObject()
        , ch(channel)
        , val(value)
        , t(time)
    {}

    QVariant value() const;
    int time() const;
    void setTime(int time);
    KisKeyframeChannel *channel() const;

private:
    KisKeyframeChannel *ch;
    QVariant val;
    int t;
};

#endif