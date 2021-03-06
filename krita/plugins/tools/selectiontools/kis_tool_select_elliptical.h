/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include "KoToolFactoryBase.h"
#include "kis_tool_ellipse_base.h"
#include "kis_selection_tool_config_widget_helper.h"
#include <KoIcon.h>
#include <kshortcut.h>

class KisToolSelectElliptical : public KisToolEllipseBase
{
    Q_OBJECT

public:
    KisToolSelectElliptical(KoCanvasBase *canvas);
    QWidget* createOptionWidget();

private:
    void keyPressEvent(QKeyEvent *event);
    void finishRect(const QRectF &rect);

private:
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
};

class KisToolSelectEllipticalFactory : public KoToolFactoryBase
{

public:
    KisToolSelectEllipticalFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectElliptical") {
        setToolTip(i18n("Elliptical Selection Tool"));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_elliptical_selection"));
        setShortcut(KShortcut(Qt::Key_J));
        setPriority(53);
    }

    virtual ~KisToolSelectEllipticalFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectElliptical(canvas);
    }

};





#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

