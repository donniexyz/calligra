/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoSection.h"

#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoTextSharedLoadingData.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoSectionStyle.h>
#include <KoSectionManager.h>
#include <KoSectionEnd.h>
#include <KoTextDocument.h>
#include <KoTextInlineRdf.h>

#include <KDebug>

#include <QTextBlock>

class KoSectionPrivate
{
public:
    explicit KoSectionPrivate(const QTextDocument *_document)
        : document(_document)
        , manager(KoTextDocument(_document).sectionManager())
        , sectionStyle(0)
        , inlineRdf(0)
    {
        Q_ASSERT(manager);
        name = manager->possibleNewName();
    }

    const QTextDocument *document;
    KoSectionManager *manager;

    QString condition;
    QString display;
    QString name;
    QString text_protected;
    QString protection_key;
    QString protection_key_digest_algorithm;
    QString style_name;
    KoSectionStyle *sectionStyle;

    QScopedPointer<KoSectionEnd> sectionEnd; //< pointer to the corresponding section end
    int level; //< level of the section in document, root sections have 0 level
    QPair<int, int> bounds; //< start and end position of section in QDocument

    KoTextInlineRdf *inlineRdf;
};

KoSection::KoSection(const QTextCursor &cursor)
    : d_ptr(new KoSectionPrivate(cursor.block().document()))
{
    Q_D(KoSection);
    d->manager->registerSection(this);
}

KoSection::~KoSection()
{
    Q_D(KoSection);
    d->manager->unregisterSection(this);
}

QString KoSection::name() const
{
    Q_D(const KoSection);
    return d->name;
}

QPair<int, int> KoSection::bounds() const
{
    Q_D(const KoSection);
    d->manager->update();
    return d->bounds;
}

int KoSection::level() const
{
    Q_D(const KoSection);
    d->manager->update();
    return d->level;
}

bool KoSection::setName(const QString &name)
{
    Q_D(KoSection);

    if (name == d->name) {
        return true;
    }

    if (d->manager->isValidNewName(name)) {
        d->name = name;
        d->manager->invalidate();
        return true;
    }
    return false;
}

bool KoSection::loadOdf(const KoXmlElement &element, KoTextSharedLoadingData *sharedData, bool stylesDotXml)
{
    Q_D(KoSection);
    // check whether we really are a section
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "section") {
        // get all the attributes
        d->condition = element.attributeNS(KoXmlNS::text, "condition");
        d->display = element.attributeNS(KoXmlNS::text, "display");

        if (d->display == "condition" && d->condition.isEmpty()) {
            kWarning(32500) << "Section display is set to \"condition\", but condition is empty.";
        }

        if (!setName(element.attributeNS(KoXmlNS::text, "name"))) {
            kWarning(32500) << "Sections name \"" << element.attributeNS(KoXmlNS::text, "name")
                << "\" repeated, but must be unique.";
        }

        d->text_protected = element.attributeNS(KoXmlNS::text, "text-protected");
        d->protection_key = element.attributeNS(KoXmlNS::text, "protection-key");
        d->protection_key_digest_algorithm = element.attributeNS(KoXmlNS::text, "protection-key-algorithm");
        d->style_name = element.attributeNS(KoXmlNS::text, "style-name", "");

        if (!d->style_name.isEmpty()) {
            d->sectionStyle = sharedData->sectionStyle(d->style_name, stylesDotXml);
        }

        // lets handle associated xml:id
        if (element.hasAttribute("id")) {
            KoTextInlineRdf* inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument *>(d->document), this);
            if (inlineRdf->loadOdf(element)) {
                d->inlineRdf = inlineRdf;
            } else {
                delete inlineRdf;
                inlineRdf = 0;
            }
        }

        return true;
    }
    return false;
}

void KoSection::saveOdf(KoShapeSavingContext &context) const
{
    Q_D(const KoSection);
    KoXmlWriter *writer = &context.xmlWriter();
    Q_ASSERT(writer);
    writer->startElement("text:section", false);

    if (!d->condition.isEmpty()) writer->addAttribute("text:condition", d->condition);
    if (!d->display.isEmpty()) writer->addAttribute("text:display", d->condition);
    if (!d->name.isEmpty()) writer->addAttribute("text:name", d->name);
    if (!d->text_protected.isEmpty()) writer->addAttribute("text:text-protected", d->text_protected);
    if (!d->protection_key.isEmpty()) writer->addAttribute("text:protection-key", d->protection_key);
    if (!d->protection_key_digest_algorithm.isEmpty()) writer->addAttribute("text:protection-key-digest-algorihtm", d->protection_key_digest_algorithm);
    if (!d->style_name.isEmpty()) writer->addAttribute("text:style-name", d->style_name);

    if (d->inlineRdf) {
        d->inlineRdf->saveOdf(context, writer);
    }
}

void KoSection::setSectionEnd(KoSectionEnd* sectionEnd)
{
    Q_D(KoSection);
    d->sectionEnd.reset(sectionEnd);
}

void KoSection::setBeginPos(int pos)
{
    Q_D(KoSection);
    d->bounds.first = pos;
}

void KoSection::setEndPos(int pos)
{
    Q_D(KoSection);
    d->bounds.second = pos;
}

void KoSection::setLevel(int level)
{
    Q_D(KoSection);
    d->level = level;
}

KoTextInlineRdf *KoSection::inlineRdf() const
{
    Q_D(const KoSection);
    return d->inlineRdf;
}

void KoSection::setInlineRdf(KoTextInlineRdf *inlineRdf)
{
    Q_D(KoSection);
    d->inlineRdf = inlineRdf;
}
