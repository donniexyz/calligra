/*
 *  Interpolated layer: allows several ways of interpolating
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

#include <iostream>

#include "interpolated_animated_layer.h"

InterpolatedAnimatedLayer::InterpolatedAnimatedLayer(const KisGroupLayer& source): SimpleAnimatedLayer(source)
{
    m_updating = false;
}

void InterpolatedAnimatedLayer::updateFrame(int num)
{
    if (m_updating)
        return;
    
    if (isKeyFrame(num))
    {
        return;
    }
    m_updating = true;
    
    if (getFrameAt(num))
        getNodeManager()->removeNode(getFrameAt(num));
    
    int inxt = getNextKey(num);
    KisCloneLayer* next = 0;
    if (isKeyFrame(inxt))
        next = dynamic_cast<KisCloneLayer*>( getKeyFrame(inxt)->getContent() );
    
    int ipre = getPreviousKey(num);
    KisNode* prev = 0;
    if (isKeyFrame(ipre))
        prev = getKeyFrame(ipre)->getContent();

    if (prev && next && next->inherits("KisCloneLayer"))
    {
        // interpolation here!
        double cur = num;
        double pre = ipre;
        double nxt = inxt;
        double p = (cur-pre) / (nxt-pre);
        
        getNodeManager()->activateNode(this);
        getNodeManager()->createNode("KisGroupLayer");
        
        KisNode* target = getNodeManager()->activeNode().data();
        getNodeManager()->insertNode(interpolate(prev, next, p), target, 0);
        
        target->setName(getNameForFrame(num, false));
        target->at(0)->setName("_");
        
        getNodeManager()->activateNode(this);           // be sure that active node is ok
        
        m_updating = false;
        loadFrames();
        m_updating = true;      // not required, but for ethtetic reasons..
    }
    m_updating = false;
}

const QString& InterpolatedAnimatedLayer::getNameForFrame(int num, bool iskey) const
{
    QString *t = const_cast<QString*>( &SimpleAnimatedLayer::getNameForFrame(num, iskey) );
    if (iskey)
        return *t;
    else
        return * ( new QString (*t+QString("_")) );
}

int InterpolatedAnimatedLayer::getFrameFromName(const QString& name, bool& iskey) const
{
    int result = SimpleAnimatedLayer::getFrameFromName(name, iskey);
    iskey = !name.endsWith("_");
    return result;
}
