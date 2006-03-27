/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tiff_ycbcr_reader.h"

#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>

#include "kis_tiff_stream.h"


KisTIFFYCbCrReaderTarget8Bit::KisTIFFYCbCrReaderTarget8Bit( KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth, uint8 nbcolorssamples, uint8 extrasamplescount,  cmsHTRANSFORM transformProfile, KisTIFFPostProcessor* postprocessor, uint16 hsub, uint16 vsub, KisTIFFYCbCr::Position position ) : KisTIFFReaderBase(device, poses, alphapos, sourceDepth,  nbcolorssamples, extrasamplescount, transformProfile, postprocessor), m_hsub(hsub), m_vsub(vsub), m_position(position)
{
    // Initialize the buffer
    qint32 imagewidth = device->image()->width();
    if(2*(imagewidth / 2) != imagewidth) imagewidth++;
    m_bufferWidth = imagewidth / m_hsub;
    qint32 imageheight = device->image()->height();
    if(2*(imageheight / 2) != imageheight) imageheight++;
    m_bufferHeight = imageheight / m_vsub;
    m_bufferCb = new quint8[ m_bufferWidth * m_bufferHeight ];
    m_bufferCr = new quint8[ m_bufferWidth * m_bufferHeight ];
}

KisTIFFYCbCrReaderTarget8Bit::~KisTIFFYCbCrReaderTarget8Bit()
{
    delete[] m_bufferCb;
    delete[] m_bufferCr;
}

uint KisTIFFYCbCrReaderTarget8Bit::copyDataToChannels( quint32 x, quint32 y, quint32 dataWidth, TIFFStreamBase* tiffstream)
{
    int numcols = dataWidth / m_hsub;
    double coeff = quint8_MAX / (double)( pow(2, sourceDepth() ) - 1 );
//     kDebug(41008) << " depth expension coefficient : " << coeff << endl;
//     kDebug(41008) << " y = " << y << endl;
    uint buffPos = y / m_vsub * m_bufferWidth + x / m_hsub ;
    for(int index = 0; index < numcols; index++)
    {
        KisHLineIterator it = paintDevice() -> createHLineIterator(x + m_hsub * index, y, m_hsub, true);
        for( int vindex = 0; vindex < m_vsub; vindex++)
        {
            while( !it.isDone() )
            {
                quint8 *d = it.rawData();
                d[0] = (quint8)( tiffstream->nextValue() * coeff );
                d[3] = quint8_MAX;
                for(int k = 0; k < nbExtraSamples(); k++)
                {
                    if(k == alphaPos())
                        d[3] = (quint32) ( tiffstream->nextValue() * coeff );
                    else
                        tiffstream->nextValue();
                }
                ++it;
            }
            it.nextRow();
        }
        m_bufferCb[ buffPos ] = (quint8)(tiffstream->nextValue() * coeff);
        m_bufferCr[ buffPos ] = (quint8)(tiffstream->nextValue() * coeff);
        buffPos ++;
    }
    return m_vsub;
}

void KisTIFFYCbCrReaderTarget8Bit::finalize()
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(0, 0, paintDevice()->image()->width(), true);
    for(int y = 0; y < paintDevice()->image()->height(); y++)
    {
        int x = 0;
        while(!it.isDone())
        {
            quint8 *d = it.rawData();
            int index =  x/m_hsub + y/m_vsub * m_bufferWidth;
            d[1] = m_bufferCb[ index ];
            d[2] = m_bufferCr[ index ];
            ++it; ++x;
        }
        it.nextRow();
    }
}

KisTIFFYCbCrReaderTarget16Bit::KisTIFFYCbCrReaderTarget16Bit( KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth, uint8 nbcolorssamples, uint8 extrasamplescount,  cmsHTRANSFORM transformProfile, KisTIFFPostProcessor* postprocessor, uint16 hsub, uint16 vsub, KisTIFFYCbCr::Position position ) : KisTIFFReaderBase(device, poses, alphapos, sourceDepth,  nbcolorssamples, extrasamplescount, transformProfile, postprocessor), m_hsub(hsub), m_vsub(vsub), m_position(position)
{
    // Initialize the buffer
    qint32 imagewidth = device->image()->width();
    if(2*(imagewidth / 2) != imagewidth) imagewidth++;
    m_bufferWidth = imagewidth / m_hsub;
    qint32 imageheight = device->image()->height();
    if(2*(imageheight / 2) != imageheight) imageheight++;
    m_bufferHeight = imageheight / m_vsub;
    m_bufferCb = new quint16[ m_bufferWidth * m_bufferHeight ];
    m_bufferCr = new quint16[ m_bufferWidth * m_bufferHeight ];
}

KisTIFFYCbCrReaderTarget16Bit::~KisTIFFYCbCrReaderTarget16Bit()
{
    delete[] m_bufferCb;
    delete[] m_bufferCr;
}

uint KisTIFFYCbCrReaderTarget16Bit::copyDataToChannels( quint32 x, quint32 y, quint32 dataWidth, TIFFStreamBase* tiffstream)
{
    int numcols = dataWidth / m_hsub;
    double coeff = quint16_MAX / (double)( pow(2, sourceDepth() ) - 1 );
//     kDebug(41008) << " depth expension coefficient : " << coeff << endl;
//     kDebug(41008) << " y = " << y << endl;
    uint buffPos = y / m_vsub * m_bufferWidth + x / m_hsub ;
    for(int index = 0; index < numcols; index++)
    {
        KisHLineIterator it = paintDevice() -> createHLineIterator(x + m_hsub * index, y, m_hsub, true);
        for( int vindex = 0; vindex < m_vsub; vindex++)
        {
            while( !it.isDone() )
            {
                quint16 *d = reinterpret_cast<quint16 *>(it.rawData());
                d[0] = (quint16)( tiffstream->nextValue() * coeff );
                d[3] = quint16_MAX;
                for(int k = 0; k < nbExtraSamples(); k++)
                {
                    if(k == alphaPos())
                        d[3] = (quint32) ( tiffstream->nextValue() * coeff );
                    else
                        tiffstream->nextValue();
                }
                ++it;
            }
            it.nextRow();
        }
        m_bufferCb[ buffPos ] = (quint16)(tiffstream->nextValue() * coeff);
        m_bufferCr[ buffPos ] = (quint16)(tiffstream->nextValue() * coeff);
        buffPos ++;
    }
    return m_vsub;
}

void KisTIFFYCbCrReaderTarget16Bit::finalize()
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(0, 0, paintDevice()->image()->width(), true);
    for(int y = 0; y < paintDevice()->image()->height(); y++)
    {
        int x = 0;
        while(!it.isDone())
        {
            quint16 *d = reinterpret_cast<quint16 *>(it.rawData());
            int index =  x/m_hsub + y/m_vsub * m_bufferWidth;
            d[1] = m_bufferCb[ index ];
            d[2] = m_bufferCr[ index ];
            ++it; ++x;
        }
        it.nextRow();
    }
}
