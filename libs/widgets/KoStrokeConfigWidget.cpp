/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
 * Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005-2006, 2011 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2006 Peter Simonsson <psn@linux.se>
 * Copyright (C) 2006 Laurent Montel <montel@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <t.zachmann@zagge.de>
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
#include "KoStrokeConfigWidget.h"

// Posix
#include <math.h>

// Qt
//#include <QtGui/QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QWidget>
#include <QGridLayout>
#include <QButtonGroup>

// KDE
#include <klocale.h>
#include <kiconloader.h>

// Calligra
#include <KoUnit.h>
#include <KoLineBorder.h>
#include <KoLineStyleSelector.h>
#include <KoUnitDoubleSpinBox.h>
#include "KoMarkerSelector.h"

#include <QBuffer>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoMarker.h>


class KoStrokeConfigWidget::Private
{
public:
    Private()
    {
    }

    KoLineStyleSelector *lineStyle;
    KoUnitDoubleSpinBox *lineWidth;
    KoUnitDoubleSpinBox *miterLimit;
    QButtonGroup        *capGroup;
    QButtonGroup        *joinGroup;
    KoMarkerSelector    *beginMarkerSelector;
    KoMarkerSelector    *endMarkerSelector;
    
    QSpacerItem *spacer;
    QGridLayout *layout;
};

KoStrokeConfigWidget::KoStrokeConfigWidget(QWidget * parent)
    : QWidget(parent)
    , d(new Private())
{
    QGridLayout *mainLayout = new QGridLayout(this);

    // Line style
    QLabel * styleLabel = new QLabel(i18n("Style:"), this);
    mainLayout->addWidget(styleLabel, 0, 0);
    d->lineStyle = new KoLineStyleSelector(this);
    mainLayout->addWidget(d->lineStyle, 0, 1, 1, 3);

    // Line width
    QLabel* widthLabel = new QLabel(i18n("Width:"), this);
    mainLayout->addWidget(widthLabel, 1, 0);
    // set min/max/step and value in points, then set actual unit
    d->lineWidth = new KoUnitDoubleSpinBox(this);
    d->lineWidth->setMinMaxStep(0.0, 1000.0, 0.5);
    d->lineWidth->setDecimals(2);
    d->lineWidth->setUnit(KoUnit(KoUnit::Point));
    d->lineWidth->setToolTip(i18n("Set line width of actual selection"));
    mainLayout->addWidget(d->lineWidth, 1, 1, 1, 3);

    // The cap group
    QLabel* capLabel = new QLabel(i18n("Cap:"), this);
    mainLayout->addWidget(capLabel, 2, 0);
    d->capGroup = new QButtonGroup(this);
    d->capGroup->setExclusive(true);
    d->capGroup->setExclusive(true);

    QRadioButton *button = 0;

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("cap_butt"));
    button->setCheckable(true);
    button->setToolTip(i18n("Butt cap"));
    d->capGroup->addButton(button, Qt::FlatCap);
    mainLayout->addWidget(button, 2, 1);

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("cap_round"));
    button->setCheckable(true);
    button->setToolTip(i18n("Round cap"));
    d->capGroup->addButton(button, Qt::RoundCap);
    mainLayout->addWidget(button, 2, 2);

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("cap_square"));
    button->setCheckable(true);
    button->setToolTip(i18n("Square cap"));
    d->capGroup->addButton(button, Qt::SquareCap);
    mainLayout->addWidget(button, 2, 3);

    // The join group
    QLabel* joinLabel = new QLabel(i18n("Join:"), this);
    mainLayout->addWidget(joinLabel, 3, 0);

    d->joinGroup = new QButtonGroup(this);
    d->joinGroup->setExclusive(true);

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("join_miter"));
    button->setCheckable(true);
    button->setToolTip(i18n("Miter join"));
    d->joinGroup->addButton(button, Qt::MiterJoin);
    mainLayout->addWidget(button, 3, 1);

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("join_round"));
    button->setCheckable(true);
    button->setToolTip(i18n("Round join"));
    d->joinGroup->addButton(button, Qt::RoundJoin);
    mainLayout->addWidget(button, 3, 2);

    button = new QRadioButton(this);
    button->setIcon(SmallIcon("join_bevel"));
    button->setCheckable(true);
    button->setToolTip(i18n("Bevel join"));
    d->joinGroup->addButton(button, Qt::BevelJoin);
    mainLayout->addWidget(button, 3, 3);

    // Miter limit
    QLabel* miterLabel = new QLabel(i18n("Miter limit:"), this);
    mainLayout->addWidget(miterLabel, 4, 0);
    // set min/max/step and value in points, then set actual unit
    d->miterLimit = new KoUnitDoubleSpinBox(this);
    d->miterLimit->setMinMaxStep(0.0, 1000.0, 0.5);
    d->miterLimit->setDecimals(2);
    d->miterLimit->setUnit(KoUnit(KoUnit::Point));
    d->miterLimit->setToolTip(i18n("Set miter limit"));
    mainLayout->addWidget(d->miterLimit, 4, 1, 1, 3);

#if 1
    d->beginMarkerSelector = new KoMarkerSelector(this);
    d->endMarkerSelector = new KoMarkerSelector(this);
    
    KoXmlDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    QBuffer xmldevice;
    xmldevice.open(  QIODevice::WriteOnly );
    QTextStream xmlstream(  &xmldevice );

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:koffice=\"http://www.koffice.org/2005/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    xmlstream << "<draw:marker draw:name=\"Arrow\" svg:viewBox=\"0 0 20 30\" svg:d=\"m10 0-10 30h20z\"/>";
    xmlstream << "</office:document-content>";
    xmldevice.close();
    doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn );
    qDebug() << __PRETTY_FUNCTION__ << errorMsg << errorLine << errorColumn;
    KoXmlElement content = doc.documentElement();
    KoXmlElement element( KoXml::namedItemNS( content, KoXmlNS::draw, "marker" ) );
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext odfContext( stylesReader, 0 );
    KoShapeLoadingContext shapeContext( odfContext, 0 );

    KoMarker *marker = new KoMarker();
    marker->loadOdf( element, shapeContext );
    
    QList<KoMarker*> markers;
    markers << marker;
    markers << marker;
    markers << marker;
    markers << marker;
    markers << marker;
    markers << marker;
    markers << marker;
    markers << marker;
    
    d->beginMarkerSelector->updateMarkers( markers );
    mainLayout->addWidget(d->beginMarkerSelector, 5, 0, 1, 2);
    
    d->endMarkerSelector->updateMarkers( markers );
    mainLayout->addWidget(d->endMarkerSelector, 5, 2, 1, 2);
#endif

    // Spacer
    d->spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->addItem(d->spacer, 5, 4);

    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    d->layout = mainLayout;

    // Make the signals visible on the outside of this widget.
    connect(d->lineStyle,  SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged()));
    connect(d->lineWidth,  SIGNAL(valueChangedPt(qreal)),    this, SIGNAL(widthChanged()));
    connect(d->capGroup,   SIGNAL(buttonClicked(int)),       this, SIGNAL(capChanged(int)));
    connect(d->joinGroup,  SIGNAL(buttonClicked(int)),       this, SIGNAL(joinChanged(int)));
    connect(d->miterLimit, SIGNAL(valueChangedPt(qreal)),    this, SIGNAL(miterLimitChanged()));
    connect(d->beginMarkerSelector,  SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentBeginMarkerChanged()));
    connect(d->endMarkerSelector,  SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentEndMarkerChanged()));
}

KoStrokeConfigWidget::~KoStrokeConfigWidget()
{
    delete d;
}

// ----------------------------------------------------------------
//                         getters and setters


Qt::PenStyle KoStrokeConfigWidget::lineStyle() const
{
    return d->lineStyle->lineStyle();
}


QVector<qreal> KoStrokeConfigWidget::lineDashes() const
{
    return d->lineStyle->lineDashes();
}

qreal KoStrokeConfigWidget::lineWidth() const
{
    return d->lineWidth->value();
}

qreal KoStrokeConfigWidget::miterLimit() const
{
    return d->miterLimit->value();
}

KoMarker *KoStrokeConfigWidget::beginMarker() const
{
    return d->beginMarkerSelector->marker();
}

KoMarker *KoStrokeConfigWidget::endMarker() const
{
    return d->endMarkerSelector->marker();
}

// ----------------------------------------------------------------
//                         Other public functions

void KoStrokeConfigWidget::updateControls(KoLineBorder &border)
{
    blockChildSignals(true);

    d->capGroup->button(border.capStyle())->setChecked(true);
    d->joinGroup->button(border.joinStyle())->setChecked(true);
    d->lineWidth->changeValue(border.lineWidth());
    d->miterLimit->changeValue(border.miterLimit());
    d->lineStyle->setLineStyle(border.lineStyle(), border.lineDashes());

    blockChildSignals(false);
}

void KoStrokeConfigWidget::setUnit(const KoUnit &unit)
{
    blockChildSignals(true);

    d->lineWidth->setUnit(unit);
    d->miterLimit->setUnit(unit);

    blockChildSignals(false);
}

void KoStrokeConfigWidget::blockChildSignals(bool block)
{
    d->lineWidth->blockSignals(block);
    d->capGroup->blockSignals(block);
    d->joinGroup->blockSignals(block);
    d->miterLimit->blockSignals(block);
    d->lineStyle->blockSignals(block);
    d->beginMarkerSelector->blockSignals(block);
    d->endMarkerSelector->blockSignals(block);
}

void KoStrokeConfigWidget::locationChanged(Qt::DockWidgetArea area)
{
    switch (area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            d->spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            d->spacer->changeSize(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            break;
        default:
            break;
    }
    d->layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    d->layout->invalidate();
}


#include <KoStrokeConfigWidget.moc>
