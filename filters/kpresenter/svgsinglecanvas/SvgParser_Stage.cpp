   /* This file is part of the KDE project
    * Copyright (C) 2011 Aakriti Gupta <aakriti.a.gupta@gmail.com>
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

#include "SvgParser_Stage.h"

#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeRegistry.h>

#include "SvgAnimationData.h"
#include "plugins/presentationviewportshape/PresentationViewPortShape.h"

SvgParser_Stage::SvgParser_Stage(KoResourceManager* documentResourceManager):SvgParser_generic(documentResourceManager)
{
 
    m_appData_tagName = "calligra:frame";
}

SvgParser_Stage::~SvgParser_Stage()
{
}

void SvgParser_Stage::parseAppData(const KoXmlElement& e)
{
    Frame *frame = new Frame(e);
    setAppData(frame);
}

void SvgParser_Stage::setAppData(Frame * frame)
{
    foreach(KoShape* shape, m_shapes){
     if(shape->name() == frame->refId()) {
           
	 SvgAnimationData * appData = new SvgAnimationData();
       
           appData->setFrame(frame);
           shape->setApplicationData(appData);
    }
    }
}

    