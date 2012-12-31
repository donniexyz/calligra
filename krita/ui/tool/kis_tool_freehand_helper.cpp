/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_freehand_helper.h"

#include <QTimer>

#include <KLocale>

#include <KoPointerEvent.h>
#include <KoCanvasResourceManager.h>

#include "kis_distance_information.h"
#include "kis_painting_information_builder.h"
#include "kis_recording_adapter.h"
#include "kis_image.h"
#include "kis_painter.h"

#include <math.h>

struct KisToolFreehandHelper::Private
{
    KisPaintingInformationBuilder *infoBuilder;
    KisRecordingAdapter *recordingAdapter;
    KisStrokesFacade *strokesFacade;

    bool haveTangent;
    QPointF previousTangent;

    bool hasPaintAtLeastOnce;

    QTime strokeTime;
    QTimer strokeTimeoutTimer;

    QVector<PainterInfo*> painterInfos;
    KisResourcesSnapshotSP resources;
    KisStrokeId strokeId;

    KisPaintInformation previousPaintInformation;
    KisPaintInformation olderPaintInformation;

    bool smooth;
    qreal smoothnessFactor;
    int smoothnessQuality;

    QTimer airbrushingTimer;

    QList<KisPaintInformation> history;
};


KisToolFreehandHelper::KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                                             KisRecordingAdapter *recordingAdapter)
    : m_d(new Private)
{
    m_d->infoBuilder = infoBuilder;
    m_d->recordingAdapter = recordingAdapter;

    m_d->smooth = true;
    m_d->smoothnessFactor = 50.0;
    m_d->smoothnessQuality = 20;

    m_d->strokeTimeoutTimer.setSingleShot(true);
    connect(&m_d->strokeTimeoutTimer, SIGNAL(timeout()), SLOT(finishStroke()));

    connect(&m_d->airbrushingTimer, SIGNAL(timeout()), SLOT(doAirbrushing()));
}

KisToolFreehandHelper::~KisToolFreehandHelper()
{
    delete m_d;
}

void KisToolFreehandHelper::setSmoothness(bool smooth, qreal smoothnessFactor, int smoothnessQuality)
{
    m_d->smooth = smooth;
    m_d->smoothnessQuality = smoothnessQuality;
    m_d->smoothnessFactor = smoothnessFactor;
}

void KisToolFreehandHelper::initPaint(KoPointerEvent *event,
                                      KoCanvasResourceManager *resourceManager,
                                      KisImageWSP image,
                                      KisStrokesFacade *strokesFacade,
                                      KisPostExecutionUndoAdapter *undoAdapter,
                                      KisNodeSP overrideNode)
{
    Q_UNUSED(overrideNode);

    m_d->strokesFacade = strokesFacade;

    m_d->haveTangent = false;
    m_d->previousTangent = QPointF();

    m_d->hasPaintAtLeastOnce = false;

    m_d->strokeTime.start();

    createPainters(m_d->painterInfos);
    m_d->resources = new KisResourcesSnapshot(image,
                                              undoAdapter,
                                              resourceManager);

    if(overrideNode) {
        m_d->resources->setCurrentNode(overrideNode);
    }

    bool indirectPainting = m_d->resources->needsIndirectPainting();

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->startStroke(image, m_d->resources);
    }

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(indirectPainting,
                                   m_d->resources, m_d->painterInfos, i18n("Freehand Stroke"));

    m_d->strokeId = m_d->strokesFacade->startStroke(stroke);

    m_d->previousPaintInformation =
        m_d->infoBuilder->startStroke(event, m_d->strokeTime.elapsed());

    m_d->history.clear();
    m_d->history.append(m_d->previousPaintInformation);

    if(m_d->resources->needsAirbrushing()) {
        m_d->airbrushingTimer.setInterval(m_d->resources->airbrushingRate());
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::paint(KoPointerEvent *event)
{
    KisPaintInformation info =
        m_d->infoBuilder->continueStroke(event,
                                         m_d->previousPaintInformation.pos(),
                                         m_d->strokeTime.elapsed());

    // Smooth the coordinates out using the history and the velocity. See
    // https://bugs.kde.org/show_bug.cgi?id=281267 and http://www24.atwiki.jp/sigetch_2007/pages/17.html.
    // This is also implemented in gimp, which is where I cribbed the code from.
    if (m_d->smooth && m_d->smoothnessQuality > 1 && m_d->smoothnessFactor > 3.0) {

        m_d->history.append(info);

        QPointF pos(0.0, 0.0);

        if (m_d->history.size() > 3) {

            int length = qMin(m_d->smoothnessQuality, m_d->history.size());
            int minIndex = m_d->history.size() - length;

            qreal gaussianWeight = 0.0;
            qreal gaussianWeight2 = m_d->smoothnessFactor * m_d->smoothnessFactor;
            qreal velocitySum = 0.0;
            qreal scaleSum = 0.0;

            if (gaussianWeight2 != 0.0) {
                gaussianWeight = 1 / (sqrt(2 * M_PI) * m_d->smoothnessFactor);
            }

            for (int i = m_d->history.size() - 1; i >= minIndex; i--) {
                qreal rate = 0.0;
                const KisPaintInformation nextInfo = m_d->history.at(i);
                int previousTime = nextInfo.currentTime();
                if (i > 0) {
                    previousTime = m_d->history.at(i - 1).currentTime();
                }

                int deltaTime = qMax(1, nextInfo.currentTime() - previousTime); // make sure deltaTime > 1
                double velocity = info.movement().norm() / deltaTime;
//                // Average it to get nicer result, at the price of being less mathematically correct,
//                // but we quickly reach a situation where dt = 1 and currentMove = 1
//                qreal velocity = qMin(1.0, (m_speed * 0.9 + currentMove * 0.1));
                if (gaussianWeight2 != 0.0) {
                    velocitySum += velocity * 100;
                    rate = gaussianWeight * exp(-velocitySum * velocitySum / (2 * gaussianWeight2));
                }
                scaleSum += rate;
                pos.setX(pos.x() + rate * nextInfo.pos().x());
                pos.setY(pos.y() + rate * nextInfo.pos().y());
            }

            if (scaleSum != 0.0) {
                pos.setX(pos.x() / scaleSum);
                pos.setY(pos.y() / scaleSum);
            }
            if (pos != QPointF(0.0, 0.0)) {
                //m_d->history.last().setPos(pos);
                info.setPos(pos);
            }
        }
    }

    // Now paint between the coordinates, using the bezier curve interpolation
    // The old, completely unsmoothed line-between-points option is gone.
    if (!m_d->haveTangent) {
        m_d->haveTangent = true;

        // XXX: 3.0 is a magic number I don't know anything about
        //      1.0 was the old default value for smoothness, and anything lower than that
        //      gave horrible results, so remove that setting.
        m_d->previousTangent =
                (info.pos() - m_d->previousPaintInformation.pos()) /
                (3.0 * (info.currentTime() - m_d->previousPaintInformation.currentTime()));
    } else {
        QPointF newTangent = (info.pos() - m_d->olderPaintInformation.pos()) /
                (3.0 * (info.currentTime() - m_d->olderPaintInformation.currentTime()));
        qreal scaleFactor = (m_d->previousPaintInformation.currentTime() - m_d->olderPaintInformation.currentTime());
        QPointF control1 = m_d->olderPaintInformation.pos() + m_d->previousTangent * scaleFactor;
        QPointF control2 = m_d->previousPaintInformation.pos() - newTangent * scaleFactor;
        paintBezierCurve(m_d->painterInfos,
                         m_d->olderPaintInformation,
                         control1,
                         control2,
                         m_d->previousPaintInformation);
        m_d->previousTangent = newTangent;
    }
    m_d->olderPaintInformation = m_d->previousPaintInformation;
    m_d->strokeTimeoutTimer.start(100);



    m_d->previousPaintInformation = info;

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::endPaint()
{
    if (!m_d->hasPaintAtLeastOnce) {
        paintAt(m_d->painterInfos, m_d->previousPaintInformation);
    } else if (m_d->smooth) {
        finishStroke();
    }
    m_d->strokeTimeoutTimer.stop();

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.stop();
    }

    /**
     * There might be some timer events still pending, so
     * we should cancel them. Use this flag for the purpose.
     * Please note that we are not in MT here, so no mutex
     * is needed
     */
    m_d->painterInfos.clear();

    m_d->strokesFacade->endStroke(m_d->strokeId);

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->endStroke();
    }
}

const KisPaintOp* KisToolFreehandHelper::currentPaintOp() const
{
    return !m_d->painterInfos.isEmpty() ? m_d->painterInfos.first()->painter->paintOp() : 0;
}


void KisToolFreehandHelper::finishStroke()
{
    if(m_d->haveTangent) {
        m_d->haveTangent = false;

        QPointF newTangent = (m_d->previousPaintInformation.pos() - m_d->olderPaintInformation.pos()) / 3.0;
        qreal scaleFactor = (m_d->previousPaintInformation.currentTime() - m_d->olderPaintInformation.currentTime());
        QPointF control1 = m_d->olderPaintInformation.pos() + m_d->previousTangent * scaleFactor;
        QPointF control2 = m_d->previousPaintInformation.pos() - newTangent;
        paintBezierCurve(m_d->painterInfos,
                         m_d->olderPaintInformation,
                         control1,
                         control2,
                         m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::doAirbrushing()
{
    if(!m_d->painterInfos.isEmpty()) {
        paintAt(m_d->painterInfos, m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::paintAt(PainterInfo *painterInfo,
                                    const KisPaintInformation &pi)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painterInfo, pi));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addPoint(pi);
    }
}

void KisToolFreehandHelper::paintLine(PainterInfo *painterInfo,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painterInfo, pi1, pi2));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addLine(pi1, pi2);
    }
}

void KisToolFreehandHelper::paintBezierCurve(PainterInfo *painterInfo,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painterInfo,
                                         pi1, control1, control2, pi2));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addCurve(pi1, control1, control2, pi2);
    }
}

void KisToolFreehandHelper::createPainters(QVector<PainterInfo*> &painterInfos)
{
    painterInfos << new PainterInfo(new KisPainter(), new KisDistanceInformation());
}

void KisToolFreehandHelper::paintAt(const QVector<PainterInfo*> &painterInfos,
                                    const KisPaintInformation &pi)
{
    paintAt(painterInfos.first(), pi);
}

void KisToolFreehandHelper::paintLine(const QVector<PainterInfo*> &painterInfos,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    paintLine(painterInfos.first(), pi1, pi2);
}

void KisToolFreehandHelper::paintBezierCurve(const QVector<PainterInfo*> &painterInfos,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    paintBezierCurve(painterInfos.first(), pi1, control1, control2, pi2);
}

