/*  This file is part of the Calligra project, made within the KDE community.

    Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

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
    Boston, MA 02110-1301, USA.
*/

#ifndef XFIGDOCUMENT_H
#define XFIGDOCUMENT_H

// Qt
#include <QHash>
#include <QVector>
#include <QColor>
#include <QString>
#include <QFont>

typedef qint32 XFigCoord;


enum XFigCapType {
    XFigCapButt,
    XFigCapRound,
    XFigCapProjecting
};

enum XFigJoinType {
    XFigJoinMiter,
    XFigJoinRound,
    XFigJoinBevel
};

struct XFigPoint
{
public:
    XFigPoint() : m_X(0), m_Y(0) {}
    XFigPoint( XFigCoord x, XFigCoord y ) : m_X(x), m_Y(y) {}

    XFigCoord x() const { return m_X; }
    XFigCoord y() const { return m_Y; }
private:
    XFigCoord m_X;
    XFigCoord m_Y;
};

enum XFigArrowHeadType
{
    XFigArrowHeadStick, ///>  -->
    XFigArrowHeadClosedTriangle, ///>  --|>
    XFigArrowHeadClosedIndentedButt, ///>  -->>
    XFigArrowHeadClosedPointedButt ///> --<>
};

class XFigArrowHead
{
public:
    XFigArrowHead()
    : m_Type(XFigArrowHeadStick), m_IsHollow(false), m_Thickness(0.0), m_Width(0.0), m_Height(0.0)
    {}

    void setType(XFigArrowHeadType type) { m_Type = type; }
    void setIsHollow(bool isHollow) { m_IsHollow = isHollow; }
    void setThickness(double thickness) { m_Thickness = thickness; }
    void setSize(double width, double height) { m_Width = width; m_Height = height; }

    XFigArrowHeadType type() const { return m_Type; }
    bool isHollow() const { return m_IsHollow; }
    double thickness() const { return m_Thickness; }
    double width() const { return m_Width; }
    double height() const { return m_Height; }
private:
    XFigArrowHeadType m_Type;
    bool m_IsHollow;
    double m_Thickness;
    double m_Width;
    double m_Height;
};

class XFigAbstractObject
{
public:
    enum TypeId
    {
        EllipseId,
        PolylineId,
        PolygonId,
        BoxId,
        PictureBoxId,
        SplineId,
        ArcId,
        TextId,
        CompoundId
    };

protected:
    explicit XFigAbstractObject(TypeId typeId) : m_TypeId( typeId ) {}
private:
    XFigAbstractObject( const XFigAbstractObject& );
    XFigAbstractObject& operator=( const XFigAbstractObject& );
public:
    virtual ~XFigAbstractObject() {}

    void setComment(const QString& comment) { m_Comment = comment; }

    TypeId typeId() const { return m_TypeId; }
    const QString& comment() const { return m_Comment; }
private:
    TypeId m_TypeId;
    QString m_Comment;
};


class XFigFillable
{
protected:
    XFigFillable() {}
public:
    void setFill( qint32 styleId, qint32 colorId ) { mFillStyleId = styleId; mFillColorId = colorId; }

    qint32 fillStyleId() const { return mFillStyleId; }
    qint32 fillColorId() const { return mFillColorId; }
private:
    qint32 mFillStyleId;
    qint32 mFillColorId;
};

class XFigAbstractGraphObject : public XFigAbstractObject
{
protected:
    explicit XFigAbstractGraphObject(TypeId typeId)
    : XFigAbstractObject( typeId ) {}
public:
    void setDepth( qint32 depth ) { m_Depth = depth; }
    void setPen( qint32 styleId, qint32 colorId ) { m_PenStyleId = styleId; m_PenColorId = colorId; }

    qint32 depth() const { return m_Depth; }
    qint32 penStyleId() const { return m_PenStyleId; }
    qint32 penColorId() const { return m_PenColorId; }
private:
    qint32 m_Depth;
    qint32 m_PenStyleId;
    qint32 m_PenColorId;
};

enum XFigLineType
{
    XFigLineDefault = -1,
    XFigLineSolid = 0,
    XFigLineDashed = 1,
    XFigLineDotted = 2,
    XFigLineDashDotted = 3,
    XFigLineDashDoubleDotted = 4,
    XFigLineDashTrippleDotted = 5
};

class XFigLineable
{
protected:
    XFigLineable() {}
public:
    void setLine( XFigLineType type, qint32 thickness, float styleValue, qint32 colorId  )
    { m_Type = type; m_Thickness = thickness; m_StyleValue = styleValue; m_ColorId = colorId; }

    XFigLineType lineType() const { return m_Type; }
    qint32 lineThickness() const { return m_Thickness; }
    float lineStyleValue() const { return m_StyleValue; }
    qint32 lineColorId() const { return m_ColorId; }
private:
    XFigLineType m_Type;
    qint32 m_Thickness;
    float m_StyleValue;
    qint32 m_ColorId;
};

class XFigEllipseObject : public XFigAbstractGraphObject, public XFigFillable, public XFigLineable
{
public:
    enum Subtype { EllipseByRadii, EllipseByDiameter, CircleByRadius, CircleByDiameter };

    XFigEllipseObject() : XFigAbstractGraphObject(EllipseId), m_Subtype(EllipseByRadii)  {}

    void setSubtype( Subtype subtype ) { m_Subtype = subtype; }
    void setCenterPoint( XFigPoint centerPoint ) { m_CenterPoint = centerPoint; }
    void setStartEnd( XFigPoint startPoint, XFigPoint endPoint ) { m_StartPoint = startPoint; m_EndPoint = endPoint; }
    void setRadii( qint32 xRadius, qint32 yRadius ) { m_XRadius = xRadius; m_YRadius = yRadius; }
    void setXAxisAngle( double xAxisAngle ) { m_XAxisAngle = xAxisAngle; }

    Subtype subtype() const { return m_Subtype; }
    XFigPoint centerPoint() const { return m_CenterPoint; }
    XFigPoint startPoint() const { return m_StartPoint; }
    XFigPoint endPoint() const { return m_EndPoint; }
    qint32 xRadius() const { return m_XRadius; }
    qint32 yRadius() const { return m_YRadius; }
    double xAxisAngle() const { return m_XAxisAngle; }
private:
    Subtype m_Subtype;
    XFigPoint m_CenterPoint;
    XFigPoint m_StartPoint;
    XFigPoint m_EndPoint;
    qint32 m_XRadius;
    qint32 m_YRadius;
    double m_XAxisAngle;
};

class XFigAbstractPolylineObject : public XFigAbstractGraphObject, public XFigFillable, public XFigLineable
{
protected:
    explicit XFigAbstractPolylineObject(TypeId typeId)
    : XFigAbstractGraphObject( typeId ), m_JoinType(XFigJoinMiter)
    {}
public:
    virtual void setPoints(const QVector<XFigPoint>& points) = 0;

    void setJoinType(XFigJoinType joinType) { m_JoinType = joinType; }

    XFigJoinType joinType() const { return m_JoinType; }
private:
    XFigJoinType m_JoinType;
};

class XFigPolylineObject : public XFigAbstractPolylineObject
{
public:
    XFigPolylineObject()
    : XFigAbstractPolylineObject(PolylineId), m_CapType(XFigCapButt), m_ForwardArrow(0), m_BackwardArrow(0)
    {}
    ~XFigPolylineObject() { delete m_ForwardArrow; delete m_BackwardArrow; }

    virtual void setPoints(const QVector<XFigPoint>& points) { m_Points = points; }

    void setCapType(XFigCapType capType) { m_CapType = capType; }
    void setForwardArrow( XFigArrowHead* forwardArrow ) { delete m_ForwardArrow; m_ForwardArrow = forwardArrow; }
    void setBackwardArrow( XFigArrowHead* backwardArrow ) { delete m_BackwardArrow; m_BackwardArrow = backwardArrow; }

    const QVector<XFigPoint>& points() const { return m_Points; }
    XFigCapType capType() const { return m_CapType; }
    const XFigArrowHead* forwardArrow() const { return m_ForwardArrow; }
    const XFigArrowHead* backwardArrow() const { return m_BackwardArrow; }
private:
    QVector<XFigPoint> m_Points;
    XFigCapType m_CapType;
    XFigArrowHead* m_ForwardArrow;
    XFigArrowHead* m_BackwardArrow;
};

class XFigPolygonObject : public XFigAbstractPolylineObject
{
public:
    XFigPolygonObject() : XFigAbstractPolylineObject(PolygonId) {}

    virtual void setPoints(const QVector<XFigPoint>& points) { m_Points = points; }

    const QVector<XFigPoint>& points() const { return m_Points; }
private:
    QVector<XFigPoint> m_Points;
};

class XFigBoxObject : public XFigAbstractPolylineObject
{
protected:
    explicit XFigBoxObject(TypeId typeId)
    : XFigAbstractPolylineObject( typeId ), m_Width(0), m_Height(0), m_Radius(0) {}

public:
    XFigBoxObject() : XFigAbstractPolylineObject(BoxId), m_Width(0), m_Height(0), m_Radius(0) {}

    virtual void setPoints(const QVector<XFigPoint>& points);

    void setRadius( qint32 radius ) { m_Radius = radius; }

    XFigPoint upperLeft() const { return m_UpperLeftCorner; }
    qint32 width() const { return m_Width; }
    qint32 height() const { return m_Height; }
    qint32 radius() const { return m_Radius; }
private:
    XFigPoint m_UpperLeftCorner;
    qint32 m_Width;
    qint32 m_Height;
    qint32 m_Radius;
};

class XFigPictureBoxObject : public XFigBoxObject
{
public:
    XFigPictureBoxObject() : XFigBoxObject(PictureBoxId) {}

    void setIsFlipped( bool isFlipped ) { m_IsFlipped = isFlipped; }
    void setFileName( const QString& fileName ) { m_FileName = fileName; }

    bool isFlipped() const { return m_IsFlipped; }
    const QString& fileName() const { return m_FileName; }
private:
    bool m_IsFlipped;
    QString m_FileName;
};

class XFigSplineObject : public XFigAbstractGraphObject, public XFigFillable, public XFigLineable
{
public:
    enum Subtype { OpenApproximated, ClosedApproximated, OpenInterpolated, ClosedInterpolated, OpenX, ClosedX };

    XFigSplineObject() : XFigAbstractGraphObject(SplineId), m_Subtype(OpenApproximated) {}

    void setSubtype( Subtype subtype ) { m_Subtype = subtype; }
//     void addPathPoint( const XFigPathPoint& pathPoint ) { mPathPoints.append(pathPoint); }

    Subtype subtype() const { return m_Subtype; }
//     const QVector<XFigPathPoint>& pathPoints() const { return mPathPoints; }
private:
    Subtype m_Subtype;
//     QVector<XFigPathPoint> mPathPoints;
};

class XFigArcObject : public XFigAbstractGraphObject, public XFigFillable, public XFigLineable
{
public:
    XFigArcObject() : XFigAbstractGraphObject(ArcId) {}

//     void addPathPoint( const XFigPathPoint& pathPoint ) { mPathPoints.append(pathPoint); }

//     const QVector<XFigPathPoint>& pathPoints() const { return mPathPoints; }
private:
//     QVector<XFigPathPoint> mPathPoints;
};


enum XFigTextAlignment {
    XFigTextLeftAligned,
    XFigTextCenterAligned,
    XFigTextRightAligned
};

struct XFigFontData
{
    QString mFamily;
    QFont::Weight mWeight;
    QFont::Style mStyle;
    float mSize; ///> in points
};

class XFigTextObject : public XFigAbstractGraphObject, public XFigFillable
{
public:
    XFigTextObject()
    : XFigAbstractGraphObject(TextId), m_TextAlignment(XFigTextLeftAligned), m_Length(0), m_Height(0) {}

    void setText(const QString& text) { m_Text = text; }
    void setTextAlignment(XFigTextAlignment textAlignment) { m_TextAlignment = textAlignment; }
    void setBaselineStartPoint(XFigPoint baselineStartPoint) { m_BaselineStartPoint = baselineStartPoint; }
    void setSize(double length, double height) { m_Length = length; m_Height = height; }
    void setXAxisAngle(double xAxisAngle) { m_XAxisAngle = xAxisAngle; }
    void setColorId(qint32 colorId) { m_ColorId = colorId; }
    void setFontData(const XFigFontData& fontData) { m_FontData = fontData; }
    void setIsHidden(bool isHidden) { m_IsHidden = isHidden; }

    const QString& text() const { return m_Text; }
    XFigTextAlignment textAlignment() const { return m_TextAlignment; }
    XFigPoint baselineStartPoint() const { return m_BaselineStartPoint; }
    double height() const { return m_Height; }
    double length() const { return m_Length; }
    double xAxisAngle() const { return m_XAxisAngle; }
    qint32 colorId() const { return m_ColorId; }
    const XFigFontData& fontData() const { return m_FontData; }
    bool isHidden() const { return m_IsHidden; }
private:
    QString m_Text;
    XFigTextAlignment m_TextAlignment;
    XFigPoint m_BaselineStartPoint;
    double m_Length;
    double m_Height;
    double m_XAxisAngle;
    qint32 m_ColorId;
    XFigFontData m_FontData;
    bool m_IsHidden :1;
};


class XFigBoundingBox
{
public:
    XFigBoundingBox() {}
    XFigBoundingBox( XFigPoint upperLeft, XFigPoint lowerRight)
    : m_UpperLeft(upperLeft), m_LowerRight(lowerRight)
    {}

    void setUpperLeft( XFigPoint upperLeft ) { m_UpperLeft = upperLeft; }
    void setLowerRight( XFigPoint lowerRight ) { m_LowerRight = lowerRight; }

    XFigPoint upperLeft() const { return m_UpperLeft; }
    XFigPoint lowerRight() const { return m_LowerRight; }
private:
    XFigPoint m_UpperLeft;
    XFigPoint m_LowerRight;
};

class XFigCompoundObject : public XFigAbstractObject
{
public:
    XFigCompoundObject() : XFigAbstractObject(CompoundId) {}
    virtual ~XFigCompoundObject() { qDeleteAll( m_Objects );}

    void addObject( XFigAbstractObject* object ) { m_Objects.append(object); }
    void setBoundingBox( XFigBoundingBox boundingBox ) { m_BoundingBox = boundingBox; }

    const QVector<XFigAbstractObject*>& objects() const { return m_Objects; }
    XFigBoundingBox boundingBox() const { return m_BoundingBox; }
private:
    QVector<XFigAbstractObject*> m_Objects;
    XFigBoundingBox m_BoundingBox;
};

class XFigPage
{
public:
    XFigPage() {}
    ~XFigPage() { qDeleteAll( m_Objects ); }

    void addObject( XFigAbstractObject* object ) { m_Objects.append(object); }

    const QVector<XFigAbstractObject*>& objects() const { return m_Objects; }
private:
    QVector<XFigAbstractObject*> m_Objects;
};

enum XFigUnitType
{
    XFigUnitTypeUnknown,
    XFigUnitMetric,
    XFigUnitInches
};

enum XFigCoordSystemOriginType
{
    XFigCoordSystemOriginTypeUnknown,
    XFigCoordSystemOriginUpperLeft,
    XFigCoordSystemOriginLowerLeft
};

enum XFigPageOrientation
{
    XFigPageOrientationUnknown,
    XFigPagePortrait,
    XFigPageLandscape
};

enum XFigPageSizeType
{
    XFigPageSizeUnknown,
    XFigPageSizeLetter,
    XFigPageSizeLegal,
    XFigPageSizeLedger,
    XFigPageSizeTabloid,
    XFigPageSizeA,
    XFigPageSizeB,
    XFigPageSizeC,
    XFigPageSizeD,
    XFigPageSizeE,
    XFigPageSizeA4,
    XFigPageSizeA3,
    XFigPageSizeA2,
    XFigPageSizeA1,
    XFigPageSizeA0,
    XFigPageSizeB5
};

class XFigDocument
{
public:
    XFigDocument();
    ~XFigDocument() { qDeleteAll( m_Pages); }

    void setPageOrientation( XFigPageOrientation pageOrientation ) { m_PageOrientation = pageOrientation; }
    void setCoordSystemOriginType( XFigCoordSystemOriginType coordSystemOriginType )
    { m_CoordSystemOriginType = coordSystemOriginType; }
    void setUnitType( XFigUnitType unitType ) { m_UnitType = unitType; }
    void setPageSizeType( XFigPageSizeType pageSizeType ) { m_PageSizeType = pageSizeType; }
    void setResolution( qint32 resolution ) { m_Resolution = resolution; }
    void setComment(const QString& comment) { m_Comment = comment; }
    void addPage( XFigPage* page ) { m_Pages.append(page); }
    void setUserColor( int id, const QColor& color )
    { if ((32<=id) && (id<=543)) m_ColorTable.insert(id, color); }

    XFigPageOrientation pageOrientation() const { return m_PageOrientation; }
    XFigCoordSystemOriginType coordSystemOriginType() const { return m_CoordSystemOriginType; }
    XFigUnitType unitType() const { return m_UnitType; }
    XFigPageSizeType pageSizeType() const { return m_PageSizeType; }
    qint32 resolution() const { return m_Resolution; }
    const QString& comment() const { return m_Comment; }
    const QVector<XFigPage*>& pages() const { return m_Pages; }
    const QColor* color( int id ) const;
private:
    XFigPageOrientation m_PageOrientation;
    XFigCoordSystemOriginType m_CoordSystemOriginType;
    XFigUnitType m_UnitType;
    XFigPageSizeType m_PageSizeType;
    qint32 m_Resolution;
    QString m_Comment;

    QHash<int, QColor> m_ColorTable;

    QVector<XFigPage*> m_Pages;
};

#endif
