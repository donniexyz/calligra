/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#ifndef KPRPAGE_H
#define KPRPAGE_H

#include <qwidget.h>
#include <qptrlist.h>
#include <global.h>
#include "kpbackground.h"
class KPTextView;
class KPObject;
class KPresenterDoc;
class KPBackGround;
class KPresenterView;
class KoDocumentEntry;
class KoRect;
class KoPageLayout;
class KCommand;
class KoPointArray;
class DCOPObject;

class KPrPage
{
public:

    // constructor - destructor
    KPrPage(KPresenterDoc *_doc);
    virtual ~KPrPage();

    virtual DCOPObject* dcopObject();

    KPresenterDoc * kPresenterDoc() const {return m_doc; }

    QString getManualTitle()const;
    void insertManualTitle(const QString & title);
    QString pageTitle( const QString &_title ) const;

    void setNoteText( const QString &_text );
    QString getNoteText( )const;

    const QPtrList<KPObject> & objectList() const { return m_objectList;}

    KPObject *getObject(int num);

    void appendObject(KPObject *);
    void insertObject(KPObject *_oldObj, KPObject *_newObject);
    void takeObject(KPObject *_obj);
    void removeObject( int pos);
    void insertObject(KPObject *_obj,int pos);
    void completeLoading( bool _clean, int lastObj );



    KoRect getPageRect() const;

    QRect getZoomPageRect()const;

    void setObjectList( QPtrList<KPObject> _list ) {
        m_objectList.setAutoDelete( false ); m_objectList = _list; m_objectList.setAutoDelete( false );
    }

    unsigned int objNums() const { return m_objectList.count(); }

    void deleteObjs( bool _add=true );
    int numSelected() const;
    void pasteObjs( const QByteArray & data );
    KCommand * replaceObjs( bool createUndoRedo, unsigned int _orastX,unsigned int _orastY,const QColor & _txtBackCol, const QColor & _otxtBackCol);

    void copyObjs();
    KPObject* getSelectedObj();
    void groupObjects();
    void ungroupObjects();
    void raiseObjs();
    void lowerObjs();
    bool getPolygonSettings( bool *_checkConcavePolygon, int *_cornersValue, int *_sharpnessValue );
    int getRndY( int _ry );
    int getRndX( int _rx );
    int getPieAngle( int pieAngle );
    int getPieLength( int pieLength );
    bool getSticky( bool s );
    PieType getPieType( PieType pieType );
    int getGYFactor( int yfactor )const;
    int getGXFactor( int xfactor )const;
    bool getGUnbalanced( bool  unbalanced );
    bool getBackUnbalanced( unsigned int );
    BCType getGType( BCType gt )const;
    QColor getGColor2( const QColor &g2 ) const;
    QColor getGColor1( const QColor & g1)const;
    FillType getFillType( FillType ft );
    QBrush getBrush( const QBrush &brush )const;
    LineEnd getLineEnd( LineEnd le );
    LineEnd getLineBegin( LineEnd lb );
    bool setLineEnd( LineEnd le );
    bool setLineBegin( LineEnd lb );

    bool setPenBrush( const QPen &pen, const QBrush &brush, LineEnd lb, LineEnd le, FillType ft,const  QColor& g1, const QColor &g2,
			   BCType gt, bool unbalanced, int xfactor, int yfactor, bool sticky );

    QPen getPen( const QPen & pen );

    // insert an object
    virtual void insertObject( const KoRect&, KoDocumentEntry& );

    void insertRectangle( const KoRect &r, const QPen & pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor & g2,BCType gt, int rndX, int rndY, bool unbalanced, int xfactor, int yfactor );

    void insertCircleOrEllipse( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor &g2, BCType gt, bool unbalanced, int xfactor, int yfactor );

    void insertPie( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor &g2,BCType gt, PieType pt, int _angle, int _len, LineEnd lb,LineEnd le,bool unbalanced, int xfactor, int yfactor );

    void insertTextObject( const KoRect& r, const QString& text = QString::null, KPresenterView *_view = 0L );
    void insertLine( const KoRect &r, const QPen &pen, LineEnd lb, LineEnd le, LineType lt );

    void insertAutoform( const KoRect &r, const QPen &pen, const QBrush &brush, LineEnd lb, LineEnd le, FillType ft,const QColor &g1, const QColor &g2, BCType gt, const QString &fileName, bool unbalanced,int xfactor, int yfactor );

    void insertFreehand( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertPolyline( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertQuadricBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertCubicBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );

    void insertPolygon( const KoPointArray &points, const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft,const QColor &g1, const QColor &g2, BCType gt, bool unbalanced, int xfactor, int yfactor, bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );


    void alignObjsLeft();
    void alignObjsCenterH();
    void alignObjsRight();
    void alignObjsTop();
    void alignObjsCenterV();
    void alignObjsBottom();

    int getPenBrushFlags()const;
    bool setPieSettings( PieType pieType, int angle, int len );
    bool setRectSettings( int _rx, int _ry );
    bool setPolygonSettings( bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );
    bool setPenColor( const QColor &c, bool fill );
    bool setBrushColor( const QColor &c, bool fill );

    void slotRepaintVariable();
    void recalcPageNum();
    void changePicture( const QString & filename );
    void changeClipart( const QString & filename );
    void insertPicture( const QString &, int _x = 10, int _y = 10 );
    void insertClipart( const QString & );
    void insertPicture( const QString &_file, const KoRect &_rect );
    void insertClipart( const QString &_file, const KoRect &_rect );

    void enableEmbeddedParts( bool f );
    void deletePage( );

    KPBackGround *background(){return kpbackground;}

    void makeUsedPixmapList();

    void setBackColor( const QColor &backColor1, const QColor &backColor2, BCType bcType,
			    bool unbalanced, int xfactor, int yfactor );
    void setBackPixmap( const KPImageKey & key );
    bool getBackUnbalanced(  )const;
    void setBackClipart(  const KPClipartKey & key );
    void setBackView( BackView backView );
    void setBackType( BackType backType );

    void setPageEffect(  PageEffect pageEffect );
    void setPageTimer(  int pageTimer );
    void setPageSoundEffect(  bool soundEffect );
    void setPageSoundFileName(  const QString &fileName );
    BackType getBackType(  ) const ;
    BackView getBackView( )const ;
    KoImageKey getBackPixKey( )const ;
    KPClipartKey getBackClipKey(  )const ;
    QColor getBackColor1( )const ;
    QColor getBackColor2()const ;
    int getBackXFactor()const ;
    int getBackYFactor( )const;
    BCType getBackColorType( )const;
    PageEffect getPageEffect( )const;
    int getPageTimer(  )const;
    bool getPageSoundEffect( )const;
    QString getPageSoundFileName()const;

    QValueList<int> reorderPage();

    bool isSlideSelected()const {return  m_selectedSlides;}
    void slideSelected(bool _b){m_selectedSlides=_b;}

    void setInsPictureFile( const QString &_file ) { m_pictureFile = _file; }
    void setInsClipartFile( const QString &_file ) { m_clipartFile = _file; }

    QString getInsPictureFile() const { return m_pictureFile; }
    QString getInsClipartFile() const { return m_clipartFile; }

protected:

private:
    // list of objects
    QPtrList<KPObject> m_objectList;
    KPresenterDoc *m_doc;
    KPBackGround *kpbackground;
    QString manualTitle;
    QString noteText;
    DCOPObject *dcop;
    bool m_selectedSlides;

    QString m_pictureFile;
    QString m_clipartFile;
};
#endif //KPRPAGE_H
