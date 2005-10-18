/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>
   code based on svgexport.cc from Inge Wallin <inge@lysator.liu.se>
   
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA  02110-1301  USA.
*/

#include <qpicture.h>
#include <qpainter.h>

#include <kmessagebox.h>

#include <koFilterChain.h>
#include <koStore.h>
//#include <koStoreDevice.h>
#include <kgenericfactory.h>

#include "kpresenter_doc.h"
#include "kpresenter_view.h"
#include "kprcanvas.h"

#include "svgexport.h"


typedef KGenericFactory<SvgExport, KoFilter> SvgExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkpresentersvgexport, SvgExportFactory( "svgexport" ) )

SvgExport::SvgExport(KoFilter *, const char *, const QStringList&) 
    : KoFilter()
{
}

SvgExport::~SvgExport()
{
}


KoFilter::ConversionStatus
SvgExport::convert(const QCString& from, const QCString& to)
{
    KoDocument * document = m_chain->inputDocument();

    if ( !document )
        return KoFilter::StupidError;

    if ( strcmp(document->className(), "KPresenterDoc") != 0)
    {
        kdWarning() << "document isn't a KPresenterDoc but a "
                     << document->className() << endl;
        return KoFilter::NotImplemented;
    }

    // Check for proper conversion.
    if ( from != "application/x-kpresenter" || to != "image/svg+xml" )
    {
        kdWarning() << "Invalid mimetypes " << to << " " << from << endl;
        return KoFilter::NotImplemented;
    }
    KPresenterDoc * kpresenterdoc = static_cast<const KPresenterDoc *>(document);

    if ( kpresenterdoc->mimeType() != "application/x-kpresenter" )
    {
        kdWarning() << "Invalid document mimetype " << kpresenterdoc->mimeType() << endl;
        return KoFilter::NotImplemented;
    }
    KoPageLayout layoutPage= kpresenterdoc->pageLayout();
    int width =  layoutPage.ptWidth;
    int height = layoutPage.ptHeight;
    
    QPicture  picture;
    QPainter  painter(&picture);
    QRect     rect(QPoint(0, 0), QPoint(width, height));
    kpresenterdoc->paintContent(painter, rect, false);
    painter.end();
    
    if ( !picture.save( m_chain->outputFile(), "SVG" ) ) {
        KMessageBox::error( 0, i18n( "Failed to write file." ),
                            i18n( "SVG Export Error" ) );
    }
	
    return KoFilter::OK;
}


#include "svgexport.moc"


