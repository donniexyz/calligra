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

// Own
#include "Scene.h"

// Qt
#include <QString>

// KDE
#include <KDebug>

// Calligra
#include <KoXmlReader.h>
#include <KoXmlNS.h>
//#include <KoXmlWriter.h>

// Shape
#include "utils.h"


Scene::Scene()
{
}

Scene::~Scene()
{
}


bool Scene::loadOdf(const KoXmlElement &sceneElement)
{
    QString dummy;

    // 1. Load the scene attributes.

    // Camera attributes
    dummy = sceneElement.attributeNS(KoXmlNS::dr3d, "vrp", "");
    m_vrp = odfToVector3D(dummy);
    dummy = sceneElement.attributeNS(KoXmlNS::dr3d, "vpn", "");
    m_vpn = odfToVector3D(dummy);
    dummy = sceneElement.attributeNS(KoXmlNS::dr3d, "vup", "(0.0 0.0 1.0)");
    m_vup = odfToVector3D(dummy);

    dummy = sceneElement.attributeNS(KoXmlNS::dr3d, "projection", "perspective");
    if (dummy == "parallel")
        m_projection = Parallel;
    else
        m_projection = Perspective;

    m_distance     = sceneElement.attributeNS(KoXmlNS::dr3d, "distance", "");
    m_focalLength  = sceneElement.attributeNS(KoXmlNS::dr3d, "focal-length", "");
    m_shadowSlant  = sceneElement.attributeNS(KoXmlNS::dr3d, "shadow-slant", "");
    m_ambientColor = QColor(sceneElement.attributeNS(KoXmlNS::dr3d, "ambient-color", "#888888"));

    // Rendering attributes
    dummy = sceneElement.attributeNS(KoXmlNS::dr3d, "shade-mode", "gouraud");
    if (dummy == "flat")
        m_shadeMode = Flat;
    else if (dummy == "phong")
        m_shadeMode = Phong;
    else if (dummy == "draft")
        m_shadeMode = Draft;
    else
        m_shadeMode = Gouraud;

    m_lightingMode = (sceneElement.attributeNS(KoXmlNS::dr3d, "lighting-mode", "") == "true");
    m_transform = sceneElement.attributeNS(KoXmlNS::dr3d, "transform", "");

    // 2. Load the child elements, i.e the scene itself.

    // From the ODF 1.1 spec section 9.4.1:
    //
    // The elements that may be contained in the <dr3d:scene> element are:
    //  * Title (short accessible name) – see section 9.2.20.
    //  * Long description (in support of accessibility) – see section 9.2.20.
    //  * Light – see section 9.4.2.
    //  * Scene – see section 9.4.1.
    //  * Extrude – see section 9.4.5.
    //  * Sphere – see section 9.4.4.
    //  * Rotate – see section 9.4.6.
    //  * Cube – see section 9.4.3.
    KoXmlElement  elem;
    forEachElement(elem, sceneElement) {

        if (elem.localName() == "light" && elem.namespaceURI() == KoXmlNS::dr3d) {
            Lightsource  light;
            light.loadOdf(elem);
            m_lights.append(light);

#if 1
            Lightsource  &l = m_lights.back();
            kDebug(31000) << "  Light:" << l.diffuseColor() << l.direction()
                          << l.enabled() << l.specular();
#endif
        }
        else if (elem.localName() == "scene" && elem.namespaceURI() == KoXmlNS::dr3d) {
            // FIXME: Recursive!  How does this work?
        }
        else if (elem.localName() == "sphere" && elem.namespaceURI() == KoXmlNS::dr3d) {
            // Attributes:
            // dr3d:center
            // dr3d:size
            // + a number of other standard attributes
        }
        else if (elem.localName() == "cube" && elem.namespaceURI() == KoXmlNS::dr3d) {
            // Attributes:
            // dr3d:min-edge
            // dr3d:max-edge
            // + a number of other standard attributes
        }
        else if (elem.localName() == "rotate" && elem.namespaceURI() == KoXmlNS::dr3d) {
            // Attributes:
            // dr3d:
        }
        else if (elem.localName() == "extrude" && elem.namespaceURI() == KoXmlNS::dr3d) {
            // Attributes:
            // dr3d:
        }
    }

    return true;
}

void Scene::saveOdf(KoXmlWriter &writer)
{
}
