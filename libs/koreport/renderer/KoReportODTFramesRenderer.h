/*
 * Kexi Report Plugin
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2012 by Dag Andersen (danders@get2net.dk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KOREPORTODTFRAMESRENDERER_H
#define KOREPORTODTFRAMESRENDERER_H

#include <KoReportRendererBase.h>

class QTextDocument;

class OROTextBox;

class KoReportODTFramesRenderer : public KoReportRendererBase
{
public:
    KoReportODTFramesRenderer();
    virtual ~KoReportODTFramesRenderer();
    virtual bool render(const KoReportRendererContext& context, ORODocument* document, int page = -1);
        
};

#endif // KOREPORTODTFRAMESRENDERER_H
