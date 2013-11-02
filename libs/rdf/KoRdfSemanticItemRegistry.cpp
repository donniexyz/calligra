/* This file is part of the Calligra project, made with-in the KDE community

   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2013 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoRdfSemanticItemRegistry.h"

#include <kdebug.h>
#include <kglobal.h>

class KoRdfSemanticItemRegistry::Private
{
public:
//     void insertFactory(KoRdfSemanticItemFactoryBase *factory);
    void init(KoRdfSemanticItemRegistry *q);

    QHash<QPair<QString, QString>, KoRdfSemanticItemFactoryBase *> factories;
};


void KoRdfSemanticItemRegistry::Private::init(KoRdfSemanticItemRegistry *q)
{
#if 0
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextInlinePlugins";
    config.blacklist = "TextInlinePluginsDisabled";
    config.group = "calligra";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/Text-InlineObject"),
                                     QString::fromLatin1("[X-KoText-PluginVersion] == 28"), config);

    foreach (KoRdfSemanticItemFactoryBase *factory, q->values()) {
        QString nameSpace = factory->odfNameSpace();
        if (nameSpace.isEmpty() || factory->odfElementNames().isEmpty()) {
            kDebug(32500) << "Variable factory" << factory->id() << " does not have odfNameSpace defined, ignoring";
        } else {
            foreach (const QString &elementName, factory->odfElementNames()) {
                factories.insert(QPair<QString, QString>(nameSpace, elementName), factory);

                kDebug(32500) << "Inserting variable factory" << factory->id() << " for"
                    << nameSpace << ":" << elementName;
            }
        }
    }
#endif
}

KoRdfSemanticItemRegistry* KoRdfSemanticItemRegistry::instance()
{
    K_GLOBAL_STATIC(KoRdfSemanticItemRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->d->init(s_instance);
    }
    return s_instance;
}

#if 0
QList<QAction*> KoRdfSemanticItemRegistry::createInsertVariableActions(KoCanvasBase *host) const
{
    QList<QAction*> answer;
    foreach (const QString &key, keys()) {
        KoRdfSemanticItemFactoryBase *factory = value(key);
        if (factory->type() == KoRdfSemanticItemFactoryBase::TextVariable) {
            foreach (const KoInlineObjectTemplate &templ, factory->templates()) {
                answer.append(new InsertVariableAction(host, factory, templ));
            }
#ifndef NDEBUG
           if (factory->templates().isEmpty()) {
                kWarning(32500) << "Variable factory" << factory->id() << "has no templates, skipping.";
           }
#endif
        }
    }
    return answer;
}

KoInlineObject *KoRdfSemanticItemRegistry::createFromOdf(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    KoRdfSemanticItemFactoryBase *factory = d->factories.value(
            QPair<QString, QString>(element.namespaceURI(), element.tagName()));
    if (factory == 0) {
        kDebug(32500) << "No factory for" << element.namespaceURI() << ":" << element.tagName();
        return 0;
    }

    KoInlineObject *object = factory->createInlineObject(0);
    if (object) {
        object->loadOdf(element, context);
    }

    return object;
}
#endif

KoRdfSemanticItemRegistry::~KoRdfSemanticItemRegistry()
{
//     qDeleteAll(doubleEntries());
//     qDeleteAll(values());
    delete d;
}

KoRdfSemanticItemRegistry::KoRdfSemanticItemRegistry()
  : d(new Private())
{
}
