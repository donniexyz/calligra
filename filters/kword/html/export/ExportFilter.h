// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef EXPORTFILTERFULLPOWER_H
#define EXPORTFILTERFULLPOWER_H

#include <KWEFBaseWorker.h>

class HtmlWorker : public KWEFBaseWorker
{
public:
    HtmlWorker(void) : m_ioDevice(NULL), m_streamOut(NULL), m_inList(false) { }
    virtual ~HtmlWorker(void) { }
public:
    virtual bool doOpenFile(const QString& filenameOut, const QString& to);
    virtual bool doCloseFile(void); // Close file in normal conditions
    virtual bool doOpenDocument(void);
    virtual bool doCloseDocument(void);
    virtual bool doFullParagraph(const QString& paraText, const LayoutData& layout,
        const ValueListFormatData& paraFormatDataList);
    virtual bool doFullDocumentInfo(const KWEFDocumentInfo& docInfo);
    virtual bool doOpenTextFrameSet(void);
    virtual bool doCloseTextFrameSet(void);
    virtual bool doOpenHead(void); // HTML's <head>
    virtual bool doCloseHead(void); // HTML's </head>
    virtual bool doOpenBody(void); // HTML's <body>
    virtual bool doCloseBody(void); // HTML's </body>
protected:
    virtual QString getStartOfListOpeningTag(const CounterData::Style typeList, bool& ordered)=0;
    virtual void openParagraph(const QString& strTag,
        const LayoutData& layout)=0;
    virtual void closeParagraph(const QString& strTag,
        const LayoutData& layout)=0;
    virtual void openSpan(const FormatData& formatOrigin, const FormatData& format)=0;
    virtual void closeSpan(const FormatData& formatOrigin, const FormatData& format)=0;
    virtual void writeDocType(void);
public:
    inline bool isXML  (void) const { return m_xml; }
    inline void setXML (const bool flag ) { m_xml=flag; }
    inline QTextCodec* getCodec(void) const { return m_codec; }
    inline void setCodec(QTextCodec* codec) { m_codec=codec; }
protected:
    QString escapeHtmlText(const QString& strText) const;
private:
    void ProcessParagraphData ( const QString& strTag, const QString &paraText,
        const LayoutData& layout, const ValueListFormatData &paraFormatDataList);
    void formatTextParagraph(const QString& strText,
        const FormatData& formatOrigin, const FormatData& format);
    bool makeTable(const FrameAnchor& anchor);
    bool makeImage(const FrameAnchor& anchor);
    bool makeClipart(const FrameAnchor& anchor);
protected:
    QIODevice* m_ioDevice;
    QTextStream* m_streamOut;
    QTextCodec* m_codec; // QTextCodec in which the file will be written
    QString m_strTitle;
    QString m_fileName; // Name of the output file
    CounterData::Style m_typeList; // What is the style of the current list (undefined, if we are not in a list)
    bool m_inList; // Are we currently in a list?
    bool m_orderedList; // Is the current list ordered or not (undefined, if we are not in a list)
    bool m_xml;
};

#endif /* EXPORTFILTERFULLPOWER_H */
