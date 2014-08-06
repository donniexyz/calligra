/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation_layer.h"

#include <QPainter>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QCheckBox>

KisAnimationLayer::KisAnimationLayer(KisAnimationLayerBox *parent, int index)
{
    this->setParent(parent);
    this->m_layerBox = parent;

    m_inputLayerName = new QLineEdit(this);
    m_inputLayerName->setGeometry(QRect(10, 0, 100, 20));
    m_inputLayerName->hide();

    m_lblLayerName = new QLabel(this);
    m_lblLayerName->setText("Layer " + QString::number(index));
    m_lblLayerName->setGeometry(QRect(10, 0, 100, 20));

    m_visibilityToggle = new QCheckBox(this);
    m_visibilityToggle->setGeometry(QRect(110, 0, 20, 20));
    m_visibilityToggle->setChecked(true);

    m_lockToggle = new QCheckBox(this);
    m_lockToggle->setGeometry(QRect(130, 0, 20, 20));

    m_onionSkinToggle = new QCheckBox(this);
    m_onionSkinToggle->setGeometry(QRect(150, 0, 20, 20));
    m_onionSkinToggle->setChecked(true);

    this->setFixedSize(200, 20);

    connect(m_inputLayerName, SIGNAL(returnPressed()), this, SLOT(onLayerNameEdited()));
}

void KisAnimationLayer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(QRect(0,0, 200, height()));
}

void KisAnimationLayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_lblLayerName->hide();
    m_inputLayerName->setText(m_lblLayerName->text());
    m_inputLayerName->show();
}

void KisAnimationLayer::onLayerNameEdited()
{
    m_inputLayerName->hide();
    m_lblLayerName->setText(m_inputLayerName->text());
    m_lblLayerName->show();
}