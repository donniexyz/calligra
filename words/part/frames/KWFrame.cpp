/* This file is part of the KDE project
 * Copyright (C) 2000-2006 David Faure <faure@kde.org>
 * Copyright (C) 2005-2011 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2005-2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
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

#include "KWFrame.h"
#include "KWFrameSet.h"
#include "KWTextFrameSet.h"
#include "KWCopyShape.h"
#include "KWOutlineShape.h"
#include "KoTextAnchor.h"
#include "KWPage.h"
#include <KoTextShapeData.h>
#include <KoXmlWriter.h>
#include <kdebug.h>

KWFrame::KWFrame(KoShape *shape, KWFrameSet *parent, int pageNumber)
        : m_shape(shape),
        m_frameBehavior(KWord::AutoExtendFrameBehavior),
        m_newFrameBehavior(KWord::NoFollowupFrame),
        m_anchoredPageNumber(pageNumber),
        m_frameSet(parent),
        m_minimumFrameHeight(0.0) // no minimum height per default

{
    Q_ASSERT(shape);
    shape->setApplicationData(this);
    if (parent)
        parent->addFrame(this);

    KWTextFrameSet* parentFrameSet = dynamic_cast<KWTextFrameSet*>(parent);
    if (parentFrameSet) {
        if (KWord::isHeaderFooter(parentFrameSet)) {
            if (KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData())) {
                data->setResizeMethod(KoTextShapeDataBase::AutoGrowHeight);
            }
        }
        if (parentFrameSet->textFrameSetType() != KWord::OtherTextFrameSet) {
            shape->setGeometryProtected(true);
        }
    }

    kDebug() << "frame=" << this << "frameSet=" << frameSet() << "pageNumber=" << pageNumber;
}

KWFrame::~KWFrame()
{
    kDebug() << "frame=" << this << "frameSet=" << frameSet();

    KoShape *ourShape = m_shape;
    m_shape = 0; // no delete is needed as the shape deletes us.
    if (m_frameSet) {
        bool justMe = m_frameSet->frameCount() == 1;
        m_frameSet->removeFrame(this, ourShape); // first remove me so we won't get double
                                                 // deleted. ourShape is needed to mark any
                                                 // copyShapes as retired
#if 0
        if (justMe)
            delete m_frameSet;
        m_frameSet = 0;
#else
        if (justMe) {
            kDebug() << "Last KWFrame removed from frameSet=" << m_frameSet;
        }
#endif
    }
}

qreal KWFrame::minimumFrameHeight() const
{
    return m_minimumFrameHeight;
}

void KWFrame::setMinimumFrameHeight(qreal minimumFrameHeight)
{
    m_minimumFrameHeight = minimumFrameHeight;
}
    
void KWFrame::setFrameSet(KWFrameSet *fs)
{
    if (fs == m_frameSet)
        return;
    Q_ASSERT_X(!fs, __FUNCTION__, "Changing the FrameSet afterwards needs to invalidate lots of stuff including whatever is done in the KWRootAreaProvider. The better way would be to not allow this.");
    if (m_frameSet)
        m_frameSet->removeFrame(this);
    m_frameSet = fs;
    if (fs)
        fs->addFrame(this);
}

void KWFrame::copySettings(const KWFrame *frame)
{
    setFrameBehavior(frame->frameBehavior());
    setNewFrameBehavior(frame->newFrameBehavior());
    shape()->copySettings(frame->shape());
}

void KWFrame::saveOdf(KoShapeSavingContext &context, const KWPage &page, int pageZIndexOffset) const
{
    QString value;
    switch (frameBehavior()) {
    case KWord::AutoCreateNewFrameBehavior:
        value = "auto-create-new-frame";
        break;
    case KWord::IgnoreContentFrameBehavior:
        value = "clip";
        break;
    case KWord::AutoExtendFrameBehavior:
        // the third case, AutoExtendFrame is handled by min-height
        value.clear();
        if (minimumFrameHeight() > 1)
            m_shape->setAdditionalAttribute("fo:min-height", QString::number(minimumFrameHeight()) + "pt");
        break;
    }
    if (!value.isEmpty())
        m_shape->setAdditionalStyleAttribute("style:overflow-behavior", value);

    switch (newFrameBehavior()) {
    case KWord::ReconnectNewFrame: value = "followup"; break;
    case KWord::NoFollowupFrame: value.clear(); break; // "none" is the default
    case KWord::CopyNewFrame: value = "copy"; break;
    }
    if (!value.isEmpty()) {
        m_shape->setAdditionalStyleAttribute("koffice:frame-behavior-on-new-page", value);
    }

    // shape properties
    const qreal pagePos = page.offsetInDocument();

    const int effectiveZIndex = m_shape->zIndex() + pageZIndexOffset;
    m_shape->setAdditionalAttribute("draw:z-index", QString::number(effectiveZIndex));
    m_shape->setAdditionalAttribute("text:anchor-type", "page");
    m_shape->setAdditionalAttribute("text:anchor-page-number", QString::number(page.pageNumber()));
    context.addShapeOffset(m_shape, QTransform(1, 0, 0 , 1, 0, -pagePos));
    m_shape->saveOdf(context);
    context.removeShapeOffset(m_shape);
    m_shape->removeAdditionalAttribute("draw:z-index");
    m_shape->removeAdditionalAttribute("fo:min-height");
    m_shape->removeAdditionalAttribute("text:anchor-page-number");
    m_shape->removeAdditionalAttribute("text:anchor-page-number");
    m_shape->removeAdditionalAttribute("text:anchor-type");
}

bool KWFrame::isCopy() const
{
    return dynamic_cast<KWCopyShape*>(shape());
}
