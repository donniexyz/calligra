/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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


#include "kis_animation_doc.h"
#include "kis_animation_part.h"
#include "kis_animation.h"
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kranim/kis_kranim_saver.h>
#include <kranim/kis_kranim_loader.h>
#include <KoFilterManager.h>
#include <kranimstore/kis_animation_store.h>
#include <kis_animation_player.h>
#include <QList>

#define APP_MIMETYPE "application/x-krita-animation"
static const char CURRENT_DTD_VERSION[] = "1.0";


class KisAnimationDoc::KisAnimationDocPrivate
{
public:
    KisAnimationDocPrivate()
        :kranimSaver(0),
          kranimLoader(0)
    {
    }

    ~KisAnimationDocPrivate()
    {
    }

    QDomDocument doc;
    QDomElement root;
    KisKranimSaver *kranimSaver;
    KisKranimLoader *kranimLoader;
    KisLayerSP currentFrame;
    QRect currentFramePosition;
    bool saved;
    KisAnimationStore* store;
    KisAnimationPlayer* player;
    KisImageWSP image;
    QDomElement frameElement;
};

KisAnimationDoc::KisAnimationDoc()
    : KisDoc2(new KisAnimationPart),
      d(new KisAnimationDocPrivate())
{
    setMimeType(APP_MIMETYPE);
    d->kranimLoader = 0;
    d->saved = false;
    d->player = new KisAnimationPlayer(this);
}

KisAnimationDoc::~KisAnimationDoc()
{
    delete d;
}

void KisAnimationDoc::frameSelectionChanged(QRect frame)
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    if (!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
        return;
    }
    this->getFrameFile(frame.x(), frame.y());

    QString location = "frame" + QString::number(frame.x()) +"layer" + QString::number(frame.y());

    d->store->openStore();
    bool hasFile = d->store->hasFile(location);
    d->store->closeStore();

    if(hasFile) {

        d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
        connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
        d->image->setResolution(animation->resolution(), animation->resolution());

        d->currentFramePosition = frame;
        d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
        d->currentFrame->setName("testFrame");
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
        d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

        //Load all the layers here

        d->kranimLoader->loadFrame(d->currentFrame, d->store, d->currentFramePosition);

        setCurrentImage(d->image);
    }
}

QString KisAnimationDoc::getFrameFile(int frame, int layer)
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    QString location;

    for(int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        if(node.attributes().namedItem("layer").nodeValue().toInt() == layer) {
            frameNumbers.append(node.attributes().namedItem("number").nodeValue().toInt());
        }
    }

    qSort(frameNumbers);

    if(frameNumbers.contains(frame)) {
        location = "frame" + QString::number(frame) + "layer" + QString::number(layer);
        return location;
    }

    int frameN;
    for(int i = 0 ; i < frameNumbers.length() ; i++) {
        if(frameNumbers.at(i) < frame) {
            frameN = frameNumbers.at(i);
        }
    }

    location = "frame" + QString::number(frameN) + "layer" + QString::number(layer);

    return location;
}

void KisAnimationDoc::updateXML()
{
    QDomElement frameElement = d->doc.createElement("frame");
    frameElement.setAttribute("number", d->currentFramePosition.x());
    frameElement.setAttribute("layer", d->currentFramePosition.y());
    d->frameElement.appendChild(frameElement);

    d->store->openStore();
    d->store->openFileWriting("maindoc.xml");
    d->store->writeDataToFile(d->doc.toByteArray());
    d->store->closeFileWriting();
    d->store->closeStore();
}
void KisAnimationDoc::addBlankFrame(QRect frame)
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    this->updateXML();

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("testFrame");
    d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(slotFrameModified()));
    setCurrentImage(d->image);
}

void KisAnimationDoc::slotFrameModified()
{
    emit sigFrameModified();
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    this->updateXML();

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    setCurrentImage(d->image);
}

bool KisAnimationDoc::completeSaving(KoStore *store)
{
    return true;
}

QDomDocument KisAnimationDoc::saveXML()
{
    return QDomDocument();
}

bool KisAnimationDoc::loadXML(const KoXmlDocument &doc, KoStore *store)
{
    return true;
}

bool KisAnimationDoc::completeLoading(KoStore *store)
{
    return true;
}

void KisAnimationDoc::preSaveAnimation()
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    KUrl url = this->documentPart()->url();

    QString filename = animation->location() + "/" + animation->name() + ".kranim";

    QFile* fout = new QFile(filename);

    int i = 1;

    while(fout->exists()) {
        filename = animation->location() + "/" + animation->name() + "("+ QString::number(i) +").kranim";
        i++;
        fout = new QFile(filename);
    }

    d->store = new KisAnimationStore(filename);
    d->store->setMimetype();

    QDomDocument doc;
    d->doc = doc;

    d->root = d->doc.createElement("kritaanimation");
    d->root.setAttribute("version", 1.0);
    d->doc.appendChild(d->root);

    d->kranimSaver->saveMetaData(d->doc, d->root);


    d->frameElement = d->doc.createElement("frames");
    d->root.appendChild(d->frameElement);

    d->store->openStore();
    d->store->openFileWriting("maindoc.xml");

    d->store->writeDataToFile(d->doc.toByteArray());

    d->store->closeFileWriting();
    d->store->closeStore();

    d->saved = true;

}

KisAnimationStore* KisAnimationDoc::getStore()
{
    return d->store;
}

KisAnimation* KisAnimationDoc::getAnimation()
{
    return dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();
}

void KisAnimationDoc::play()
{
    if(!d->player->isCached()) {
        d->player->createCache(300);
    }

    d->player->play();

    d->player->dropCache();
}

void KisAnimationDoc::pause()
{
    if(d->player->isPlaying()) {
        d->player->pause();
    }
}

void KisAnimationDoc::stop()
{
    if(d->player->isPlaying()) {
        d->player->stop();
    }

    d->player->dropCache();
}

#include "kis_animation_doc.moc"