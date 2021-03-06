/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DEBUG_AREAS_H_
#define KIS_DEBUG_AREAS_H_

#include <kdebug.h>

#define dbgResources kDebug(30009)
#define dbgKrita kDebug(41000) // For temporary debug lines, where you'd have used kDebug() before.
#define dbgImage kDebug(41001)
#define dbgRegistry kDebug(41002)
#define dbgTools kDebug(41003)
#define dbgTiles kDebug(41004)
#define dbgFilters kDebug(41005)
#define dbgPlugins kDebug(41006)
#define dbgUI kDebug(41007)
#define dbgFile kDebug(41008)
#define dbgMath kDebug(41009)
#define dbgRender kDebug(41010)
#define dbgScript kDebug(41011)

#define warnKrita kWarning(41000) // For temporary debug lines, where you'd have used kWarning() before.
#define warnImage kWarning(41001)
#define warnRegistry kWarning(41002)
#define warnTools kWarning(41003)
#define warnTiles kWarning(41004)
#define warnFilters kWarning(41005)
#define warnPlugins kWarning(41006)
#define warnUI kWarning(41007)
#define warnFile kWarning(41008)
#define warnMath kWarning(41009)
#define warnRender kWarning(41010)
#define warnScript kWarning(41011)

#define errKrita kError(41000) // For temporary debug lines, where you'd have used kError() before.
#define errImage kError(41001)
#define errRegistry kError(41002)
#define errTools kError(41003)
#define errTiles kError(41004)
#define errFilters kError(41005)
#define errPlugins kError(41006)
#define errUI kError(41007)
#define errFile kError(41008)
#define errMath kError(41009)
#define errRender kError(41010)
#define errScript kError(41011)

#define fatalKrita kFatal(41000) // For temporary debug lines, where you'd have used kFatal() before.
#define fatalImage kFatal(41001)
#define fatalRegistry kFatal(41002)
#define fatalTools kFatal(41003)
#define fatalTiles kFatal(41004)
#define fatalFilters kFatal(41005)
#define fatalPlugins kFatal(41006)
#define fatalUI kFatal(41007)
#define fatalFile kFatal(41008)
#define fatalMath kFatal(41009)
#define fatalRender kFatal(41010)
#define fatalScript kFatal(41011)
#endif

/**
 * Use this macro to display in the output stream the name of a variable followed by its value.
 */
#define ppVar( var ) #var << "=" << var

#ifdef __GNUC__
#define ENTER_FUNCTION() qDebug() << "Entering" << __func__
#define LEAVE_FUNCTION() qDebug() << "Leaving " << __func__
#else
#define ENTER_FUNCTION() qDebug() << "Entering" << "<unknown>"
#define LEAVE_FUNCTION() qDebug() << "Leaving " << "<unknown>"
#endif

#  ifndef QT_NO_DEBUG
#    undef Q_ASSERT
#    define Q_ASSERT(cond) if(!(cond)) { kError() << kBacktrace(); qt_assert(#cond,__FILE__,__LINE__); } qt_noop()
#  endif


#include "krita_export.h"
#include "kis_types.h"
class QRect;
class QString;

void KRITAIMAGE_EXPORT kis_debug_save_device_incremental(KisPaintDeviceSP device,
                                                         int i,
                                                         const QRect &rc,
                                                         const QString &suffix, const QString &prefix);

/**
 * Saves the paint device incrementally. Put this macro into a
 * function that is called several times and you'll have as many
 * separate dump files as the number of times the function was
 * called. That is very convenient for debugging canvas updates:
 * adding this macro will let you track the whole history of updates.
 *
 * The files are saved with pattern: <counter>_<suffix>.png
 */
#define KIS_DUMP_DEVICE_1(device, rc, suffix)                           \
    do {                                                                \
        static int i = -1; i++;                                         \
        kis_debug_save_device_incremental((device), i, (rc), (suffix), QString()); \
    } while(0)

/**
 * Saves the paint device incrementally. Put this macro into a
 * function that is called several times and you'll have as many
 * separate dump files as the number of times the function was
 * called. That is very convenient for debugging canvas updates:
 * adding this macro will let you track the whole history of updates.
 *
 * The \p prefix parameter makes it easy to sort out dumps from
 * different functions.
 *
 * The files are saved with pattern: <prefix>_<counter>_<suffix>.png
 */
#define KIS_DUMP_DEVICE_2(device, rc, suffix, prefix)                   \
    do {                                                                \
        static int i = -1; i++;                                         \
        kis_debug_save_device_incremental((device), i, (rc), (suffix), (prefix)); \
    } while(0)


#include "kis_assert.h"
