/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

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
#include "kis_painterlymixer.h"

#include <QButtonGroup>
#include <QGridLayout>

#include <KoColorSpace.h>
#include <KoToolProxy.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>

#include "colorspot.h"
#include "kis_ksf32_colorspace.h"
#include "mixercanvas.h"
#include "mixertool.h"

KisPainterlyMixer::KisPainterlyMixer(QWidget *parent)
        : QWidget(parent)
        , m_tool(0)
        , m_colorspace(0)
{
    setupUi(this);

    kDebug() << ">>>>>>>>>>>>>>>>>>>>";

    QString csid = KisKSF32ColorSpace<3>::ColorSpaceId().id();
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    // TODO Illuminant and colorspace has to be chosen at runtime
    QList<const KoColorProfile*> profiles = f->profilesFor(csid);
    if (profiles.count()) { // don't crash if the profile is not available.
        const KoColorProfile* testp = profiles.at(0);
        m_colorspace = f->colorSpace(csid, testp);
    } else {
        setEnabled(false);
        return;
    }

    m_canvas->setLayer(m_colorspace);
    m_canvas->setToolProxy(new KoToolProxy(m_canvas));

    m_tool = new MixerTool(m_canvas);
    m_canvas->toolProxy()->setActiveTool(m_tool);
    m_tool->activate();

    initSpots();

    m_bErase->setIcon(KIcon("edit-delete"));
    connect(m_bErase, SIGNAL(clicked()), m_canvas, SLOT(slotClear()));
}

KisPainterlyMixer::~KisPainterlyMixer()
{
    delete m_tool;
}

#define ROWS 2
#define COLS 4

void KisPainterlyMixer::initSpots()
{
    kDebug();
    int row, col;
    QGridLayout *l = new QGridLayout(m_spotsFrame);

    m_bgColors = new QButtonGroup(m_spotsFrame);
    loadColors();

    l->setSpacing(5);
    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int index = row * COLS + col;
            QToolButton *curr = new ColorSpot(m_spotsFrame, m_vColors[index]);
            l->addWidget(curr, row, col);

            m_bgColors->addButton(curr, index);
        }
    }

    l->setColumnStretch(col++, 1);

    connect(m_bgColors, SIGNAL(buttonClicked(int)), this, SLOT(slotChangeColor(int)));
}

void KisPainterlyMixer::loadColors()
{
    kDebug();
    // TODO We need to handle save/load of user-defined colors in the spots.
    const KoColorSpace *cs = m_colorspace;
    m_vColors.append(KoColor(QColor("#FF0000"), cs)); // Red
    m_vColors.append(KoColor(QColor("#00FF00"), cs)); // Green
    m_vColors.append(KoColor(QColor("#0000FF"), cs)); // Blue
    m_vColors.append(KoColor(QColor("#006464"), cs)); // Seablue
    m_vColors.append(KoColor(QColor("#FFFF00"), cs)); // Yellow
    m_vColors.append(KoColor(QColor("#700070"), cs)); // Violet
    m_vColors.append(KoColor(QColor("#FFFFFF"), cs)); // White
    m_vColors.append(KoColor(QColor("#101010"), cs)); // Black
}

void KisPainterlyMixer::slotChangeColor(int index)
{
    emit colorChanged(m_vColors[index]);
}

void KisPainterlyMixer::setColor(const KoColor& color)
{
    m_canvas->resourceProvider()->setForegroundColor(color);
}

#include "kis_painterlymixer.moc"
