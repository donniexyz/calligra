/*
 *
 *  Copyright (C) 2011 Torio Mlshi <mlshi@lavabit.com>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "animator_lt_updater.h"

#include "animator_switcher.h"

AnimatorLTUpdater::AnimatorLTUpdater(AnimatorManager* manager) : AnimatorUpdater(manager)
{
    m_LT = new AnimatorLT();
    connect(m_LT, SIGNAL(opacityChanged(int)), this, SLOT(updateRelFrame(int)));
    connect(m_LT, SIGNAL(visibilityChanged(int)), this, SLOT(updateRelFrame(int)));
}

AnimatorLTUpdater::~AnimatorLTUpdater()
{
}

void AnimatorLTUpdater::updateLayer(AnimatedLayer* layer, int oldFrame, int newFrame)
{
    if (playerMode())
    {
        AnimatorUpdater::updateLayer(layer, oldFrame, newFrame);
        return;
    }
    
    FramedAnimatedLayer* framedLayer = qobject_cast<FramedAnimatedLayer*>(layer);
    if (!framedLayer)
    {
        AnimatorUpdater::updateLayer(layer, oldFrame, newFrame);
        return;
    }
    
    for (int i = -getLT()->getNear(); i <= getLT()->getNear(); ++i)
    {
        frameUnvisible(framedLayer->frameAt(i+oldFrame));
    }
    
    for (int i = -getLT()->getNear(); i <= getLT()->getNear(); ++i)
    {
        if (getLT()->getVisibility(i))
            frameVisible(framedLayer->frameAt(i+newFrame), getLT()->getOpacityU8(i));
    }
}

void AnimatorLTUpdater::updateRelFrame(int relFrame)
{
    int cFrame = m_manager->getSwitcher()->currentFrame();
    QList<AnimatedLayer*> layers = m_manager->layers();
    AnimatedLayer* layer;
    foreach (layer, layers)
    {
        FramedAnimatedLayer* framedLayer = qobject_cast<FramedAnimatedLayer*>(layer);
        if (framedLayer)
            frameVisible(framedLayer->frameAt(cFrame+relFrame),
                         getLT()->getVisibility(relFrame),
                         getLT()->getOpacityU8(relFrame));
    }
}


AnimatorLT* AnimatorLTUpdater::getLT()
{
    return m_LT;
}
