/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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


#ifndef _KIS_META_DATA_ENTRY_H_
#define _KIS_META_DATA_ENTRY_H_

#include <krita_export.h>

class QString;

namespace KisMetaData {
    class Value;
    class Store;
    class Schema;
    class KRITAIMAGE_EXPORT Entry {
        struct Private;
        friend class Store;
        public:
            /**
             * Create an invalid entry
             */
            Entry();
            /**
             * Create a new entry.
             * @param name
             * @param namespacePrefix
             * @param value
             */
            Entry(QString name, const KisMetaData::Schema* schema, const KisMetaData::Value& value);
            Entry(const Entry&);
            ~Entry();
            /**
             * @return the name of this entry
             */
            QString name() const;
            /**
             * @return the namespace of this entry
             */
            const KisMetaData::Schema* schema() const;
            /**
             * @return the qualified name of this entry, which is the concatenation of the
             * namespace and of the name
             */
            QString qualifiedName() const;
            /**
             * @return the value of this entry
             */
            const KisMetaData::Value& value() const;
            /**
             * @return the value of this entry
             */
            KisMetaData::Value& value();
            /**
             * @return true if this entry is valid
             */
            bool isValid() const;
            /**
             * Affect the content of entry to this entry if entry is valid
             */
            Entry& operator=(const Entry& entry);
            bool operator==(const Entry&);
        private:
            void setSchema(const KisMetaData::Schema* schema);
        private:
            Private* const d;
    };
}

KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisMetaData::Entry &c);

#endif
