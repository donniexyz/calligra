/* This library is distributed under the conditions of the GNU LGPL.
 * WMF Metafile Structures
 * Author: 2002/2003 thierry lorthiois
 */
#ifndef _KOWMFSTRUCT_H_
#define _KOWMFSTRUCT_H_

#include <qglobal.h>
#include <qnamespace.h>

#define APMHEADER_KEY 0x9AC6CDD7
#define ENHMETA_SIGNATURE       0x464D4520

struct WmfMetaHeader
{
  quint16  fileType;      // Type of metafile (0=memory, 1=disk)
  quint16  headerSize;    // always 9
  quint16  version;
  quint32  fileSize;      // Total size of the metafile in WORDs
  quint16  numOfObjects;    // Maximum Number of objects in the stack
  quint32  maxRecordSize;   // The size of largest record in WORDs
  quint16  numOfParameters; // not used (always 0)
};


struct WmfPlaceableHeader
{
  quint32  key;        // Magic number (always 9AC6CDD7h)
  quint16  handle;     // Metafile HANDLE number (always 0)
  qint16   left;       // Left coordinate in metafile units
  qint16   top;
  qint16   right;
  qint16   bottom;
  quint16  inch;       // Number of metafile units per inch
  quint32  reserved;
  quint16  checksum;   // Checksum value for previous 10 WORDs
};


struct WmfEnhMetaHeader
{
  quint32  recordType;       // Record type (is always 00000001h)
  quint32  recordSize;       // Record size in bytes.  This may be greater
                              // than the sizeof( ENHMETAHEADER ).
  qint32   boundsLeft;       // Inclusive-inclusive bounds in device units
  qint32   boundsTop;
  qint32   boundsRight;
  qint32   boundsBottom;
  qint32   frameLeft;        // Inclusive-inclusive Picture Frame
  qint32   frameTop;
  qint32   frameRight;
  qint32   frameBottom;
  quint32  signature;        // Signature.  Must be ENHMETA_SIGNATURE.
  quint32  version;          // Version number
  quint32  size;             // Size of the metafile in bytes
  quint32  numOfRecords;     // Number of records in the metafile
  quint16  numHandles;       // Number of handles in the handle table
  // Handle index zero is reserved.
  quint16  reserved;         // always 0
  quint32  sizeOfDescription;   // Number of chars in the unicode description string
                                 // This is 0 if there is no description string
  quint32  offsetOfDescription; // Offset to the metafile description record.
                                 // This is 0 if there is no description string
  quint32  numPaletteEntries;   // Number of color palette entries
  qint32   widthDevicePixels;   // Size of the reference device in pixels
  qint32   heightDevicePixels;
  qint32   widthDeviceMM;       // Size of the reference device in millimeters
  qint32   heightDeviceMM;
};


struct WmfMetaRecord
{
  quint32  size;         // Total size of the record in WORDs
  quint16  function;     // Record function number
  quint16  param[ 1 ];   // quint16 array of parameters
};


struct WmfEnhMetaRecord
{
  quint32  function;     // Record function number
  quint32  size;         // Record size in bytes
  quint32  param[ 1 ];   // quint32 array of parameters
};

// Static data
    static const struct OpTab
    {
        quint32  winRasterOp;
        Qt::RasterOp  qtRasterOp;
    } koWmfOpTab32[] =
    {
        { 0x00CC0020, Qt::CopyROP },
        { 0x00EE0086, Qt::OrROP },
        { 0x008800C6, Qt::AndROP },
        { 0x00660046, Qt::XorROP },
        { 0x00440328, Qt::AndNotROP },
        { 0x00330008, Qt::NotCopyROP },
        { 0x001100A6, Qt::NandROP },
        { 0x00C000CA, Qt::CopyROP },
        { 0x00BB0226, Qt::NotOrROP },
        { 0x00F00021, Qt::CopyROP },
        { 0x00FB0A09, Qt::CopyROP },
        { 0x005A0049, Qt::CopyROP },
        { 0x00550009, Qt::NotROP },
        { 0x00000042, Qt::ClearROP },
        { 0x00FF0062, Qt::SetROP }
    };

    static const Qt::RasterOp koWmfOpTab16[] =
    {
        Qt::CopyROP,
        Qt::ClearROP, Qt::NandROP, Qt::NotAndROP, Qt::NotCopyROP,
        Qt::AndNotROP, Qt::NotROP, Qt::XorROP, Qt::NorROP,
        Qt::AndROP, Qt::NotXorROP, Qt::NopROP, Qt::NotOrROP,
        Qt::CopyROP, Qt::OrNotROP, Qt::OrROP, Qt::SetROP
    };

    static const Qt::BrushStyle koWmfHatchedStyleBrush[] =
    {
        Qt::HorPattern,
        Qt::VerPattern,
        Qt::FDiagPattern,
        Qt::BDiagPattern,
        Qt::CrossPattern,
        Qt::DiagCrossPattern
    };

    static const Qt::BrushStyle koWmfStyleBrush[] =
    { Qt::SolidPattern,
      Qt::NoBrush,
      Qt::FDiagPattern,   /* hatched */
      Qt::Dense4Pattern,  /* should be custom bitmap pattern */
      Qt::HorPattern,     /* should be BS_INDEXED (?) */
      Qt::VerPattern,     /* should be device-independent bitmap */
      Qt::Dense6Pattern,  /* should be device-independent packed-bitmap */
      Qt::Dense2Pattern,  /* should be BS_PATTERN8x8 */
      Qt::Dense3Pattern   /* should be device-independent BS_DIBPATTERN8x8 */
    };

    static const Qt::PenStyle koWmfStylePen[] =
    { Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine, Qt::DashDotDotLine,
      Qt::NoPen, Qt::SolidLine };

#endif

