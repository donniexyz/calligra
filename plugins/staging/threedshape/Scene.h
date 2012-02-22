/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SCENE_H
#define SCENE_H

// Qt
#include <QColor>
#include <QVector3D>
#include <QString>
#include <QList>

// Calligra
#include <KoShape.h>

// Shape
#include "Lightsource.h"

class KoXmlWriter;


class Scene
{
public:
    enum Projection {
        Parallel,
        Perspective
    };

    enum Shademode {
        Flat,
        Gouraud,
        Phong,
        Draft                   // Wireframe
    };

    Scene();
    ~Scene();

    bool loadOdf(const KoXmlElement &sceneElement);
    void saveOdf(KoXmlWriter &writer);

    // getters
    QVector3D   vrp()          { return m_vrp;          }
    QVector3D   vpn()          { return m_vpn;          }
    QVector3D   vup()          { return m_vup;          }
    Projection  projection()   { return m_projection;   }
    QString     distance()     { return m_distance;     } // Not yet used, just saved
    QString     focalLength()  { return m_focalLength;  } // Not yet used, just saved
    QString     shadowSlant()  { return m_shadowSlant;  } // Not yet used, just saved
    Shademode   shadeMode()    { return m_shadeMode;    }
    QColor      ambientColor() { return m_ambientColor; }
    bool        lightingMode() { return m_lightingMode; }
    QString     transform()    { return m_transform;    } // Not yet used, just saved

private:
    // 1. Scene attributes
    // Camera attributes
    QVector3D   m_vrp;          // Camera origin
    QVector3D   m_vpn;          // Camera direction
    QVector3D   m_vup;          // Up direction
    Projection  m_projection;
    QString     m_distance;     // Not yet used, just saved
    QString     m_focalLength;  // Not yet used, just saved
    QString     m_shadowSlant;  // Not yet used, just saved

    // Rendering attributes
    Shademode   m_shadeMode;
    QColor      m_ambientColor;
    bool        m_lightingMode; // True: enable lights, false: disable lights
    QString     m_transform;    // Not yet used, just saved

    // 2. The scene itself.
    QList<Lightsource>  m_lights;
};


#endif
