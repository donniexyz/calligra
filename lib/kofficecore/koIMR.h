/* This file is part of the KDE libraries
    Copyright (C) 1998 Torben Weis <weis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __koffice_imr_h__
#define __koffice_imr_h__

#include <CORBA.h>
#include <koDocument.h>

#include <qstrlist.h>

class KoDocumentEntry;

void imr_init();
bool imr_create( const char* _name, const char* _mode, const char *_exec, QStrList &_repoids, CORBA::ImplRepository_ptr _imr );
CORBA::Object_ptr imr_activate( const char *_server, const char *_repoid = "IDL:KOffice/DocumentFactory:1.0",
				CORBA::ImplRepository_ptr imr = 0L, const char *_addr = 0L );
KOffice::Document_ptr imr_createDoc( KoDocumentEntry& _e );
KOffice::Document_ptr imr_createDocByMimeType( const char *_mime_type );
KOffice::Document_ptr imr_createDoc( const char *_server_name, const char *_repoid );

#endif
