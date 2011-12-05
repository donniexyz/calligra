/*
 *  Copyright (c) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CHARTING_H
#define CHARTING_H

#include <QtCore/QString>
//#include <QtCore/QStringList>
#include <QtCore/QRect>
#include <QtCore/QMap>
#include <QtGui/QColor>
#include <QtCore/QDebug>
#include <MsooXmlGlobal.h>

namespace Charting
{
  
    class Gradient
    {        
    public:
        Gradient(){ angle =  0.0; }
        class GradientStop
        {
        public:
            void reset(){ position = 1.0; knownColorValue = QColor(); tintVal = 0; satVal = 0; shadeVal = 0; referenceColor = QString(); }
            qreal position;
            QColor knownColorValue;
            qreal tintVal;
            qreal satVal;
            qreal shadeVal;
            QString referenceColor;
        };
        QVector< GradientStop > gradientStops;
        qreal angle;
    };

    class Value
    {
    public:
        enum DataId {
            SeriesLegendOrTrendlineName = 0x00, ///< Referenced data specifies the series, legend entry, or trendline name. Error bars name MUST be empty.
            HorizontalValues = 0x01, ///< Referenced data specifies the values or horizontal values on bubble and scatter chart groups of the series and error bars.
            VerticalValues = 0x02, ///< Referenced data specifies the categories or vertical values on bubble and scatter chart groups of the series and error bars.
            BubbleSizeValues = 0x03 ///< Referenced data specifies the bubble size values of the series.
        };
        DataId m_dataId;

        enum DataType {
            AutoGeneratedName = 0x00, ///< The data source is a category (3) name, series name or bubble size that was automatically generated.
            TextOrValue = 0x01, ///< The data source is the text or value as specified by the formula field.
            CellRange = 0x02 ///< The data source is the value from a range of cells in a sheet specified by the formula field.
        };
        DataType m_type;

        bool m_isUnlinkedFormat; ///< false=data uses the number formatting of the referenced data, true=data uses the custom formatting specified via m_numberFormat.
        unsigned m_numberFormat; ///< specifies the numnber format to use for the data.
        QString m_formula; ///< the optional formula. could be for example "[Sheet1.$D$2:$F$2]"

        Value(DataId dataId, DataType type, const QString& formula = QString(), bool isUnlinkedFormat = false, unsigned numberFormat = 0) : m_dataId(dataId), m_type(type), m_isUnlinkedFormat(isUnlinkedFormat), m_numberFormat(numberFormat), m_formula(formula) {}
        virtual ~Value() {}
    };

    class Format
    {
    public:
        Format() {}
        virtual ~Format() {}
    };

    class LineFormat : public Format
    {
    public:
        enum Style {
            Solid = 0x0000,
            Dash = 0x0001,
            Dot = 0x0002,
            DashDot = 0x0003,
            DashDotDot = 0x0004,
            None = 0x0005,
            DarkGrayPattern = 0x0006,
            MediumGrayPattern = 0x0007,
            LightGrayPattern = 0x0008,
        };
        enum Tickness {
            Hairline = 0xFFFF,
            NarrowSingle = 0x0000,
            MediumDouble = 0x0001,
            WideTriple = 0x0002,
        };
        Style m_style;
        Tickness m_tickness;
        LineFormat(const Style& style = None, const Tickness& tickness = Hairline) : Format(), m_style(style), m_tickness(tickness) {} 
    };
    
    class PieFormat : public Format
    {
    public:
        int m_pcExplode; // from PieFormat
        PieFormat(int pcExplode = 0) : Format(), m_pcExplode(pcExplode) {}
    };
    
    class AreaFormat : public Format
    {
    public:
        QColor m_foreground;
        QColor m_background;
        bool m_fill;
        AreaFormat(const QColor &foreground = QColor(), const QColor &background = QColor(), bool fill = false) : Format(), m_foreground(foreground), m_background(background), m_fill(fill) {}
    };
    
    class Fill
    {
    public:        
        enum FillType{ Blip, Gradient, Group, None, Pattern, Solid };
        Fill():type( None ), valid( false ){};
        void setColor( const QColor& color ){ solidColor = color; valid = true; type = Solid; }
        void setType( FillType type ){ this->type = type; valid = true; }
        QColor solidColor;
        QString pixmapFile;
        Charting::Gradient gradient;
        FillType type;
        bool valid;
    };
    
    class ShapeProperties
    {
    public:
        int lineWidth;
        Fill lineFill;
        Fill areaFill;
    };

    class ChartImpl
    {
    public:
        ChartImpl() {}
        virtual ~ChartImpl() {}
        virtual QByteArray name() const = 0;
    };

    class PieImpl : public ChartImpl
    {
    public:
        /// Starting angle of the first data point clockwise from the top of the circle.
        int m_anStart;
        PieImpl(int anStart = 0) : ChartImpl(), m_anStart(anStart) {}
        virtual QByteArray name() const { return "circle"; }
    };

    class RingImpl : public PieImpl
    {
    public:
        /// Size of the center hole in a doughnut chart group as a percentage of the plot area size.
        int m_pcDonut;
        RingImpl(int anStart = 0, int pcDonut = 0) : PieImpl(anStart), m_pcDonut(pcDonut) {}
        virtual QByteArray name() const { return "ring"; }
    };

    class BarImpl : public ChartImpl
    {
    public:
        virtual QByteArray name() const { return "bar"; }
    };
    
    class LineImpl : public ChartImpl
    {
    public:
        virtual QByteArray name() const { return "line"; }
    };
    
    class RadarImpl : public ChartImpl
    {
    public:
        /// If true then the radar-chart is filled (an RadarArea char) else not.
        bool m_filled;
        RadarImpl(bool filled = false) : ChartImpl(), m_filled(filled) {}
        virtual QByteArray name() const { return m_filled ? "filled-radar" : "radar"; }
    };

    class AreaImpl : public ChartImpl
    {
    public:
        AreaImpl() : ChartImpl() {}
        virtual QByteArray name() const { return "area"; }
    };

    class StockImpl : public ChartImpl
    {
    public:
        StockImpl() : ChartImpl() {}
        virtual QByteArray name() const { return "stock"; }
    };

    class ScatterImpl : public ChartImpl
    {
    public:
        enum ScatterStyle{ None, Line, LineMarker, Marker, Smooth, SmoothMarker };
        ScatterImpl() : ChartImpl(), style( LineMarker ) {}
        virtual QByteArray name() const { return "scatter"; }
        ScatterStyle style;
    };

    class BubbleImpl : public ChartImpl
    {
    public:
        enum SizeType {
            Area = 0x0001, ///< The area of the data point represents the value.
            Width = 0x0002 ///< The width of the data point represents the value.
        };
        /// Specifies how the default size of the data points represents the value.
        SizeType m_sizeType;
        /// The size of the data points as a percentage of their default size. A value of 100 shows all the data points in their default size, as determined by the application.
        unsigned int m_sizeRatio;
        /// Specifies whether data points with negative values are shown.
        bool m_showNegativeBubbles;
        BubbleImpl(SizeType sizeType = Area, unsigned int sizeRatio = 100, bool showNegativeBubbles = true) : ChartImpl(), m_sizeType(sizeType), m_sizeRatio(sizeRatio), m_showNegativeBubbles(showNegativeBubbles) {}
        virtual QByteArray name() const { return "bubble"; }
    };

    class SurfaceImpl : public ChartImpl
    {
    public:
        /// Specifies that the surface is either filled or a wireframe.
        bool m_fill;
        SurfaceImpl(bool fill = false) : ChartImpl(), m_fill(fill) {}
        virtual QByteArray name() const { return "surface"; }
    };

    class Obj
    {
    public:
        unsigned int m_mdTopLt, m_mdBotRt, m_x1, m_y1, m_x2, m_y2;
        Charting::AreaFormat *m_areaFormat;
        explicit Obj() : m_mdTopLt(0), m_mdBotRt(0), m_x1(0), m_y1(0), m_x2(0), m_y2(0), m_areaFormat(0) {}
        virtual ~Obj() { delete m_areaFormat; }
    };

    class Text : public Obj
    {
    public:
        QString m_text;
        explicit Text(const QString &text = QString()) : Obj(), m_text(text) {}
        virtual ~Text() {}
    };
    
    class Axis : public Obj
    {
    public:
        enum Type {
            HorizontalValueAxis = 0x0000,
            VerticalValueAxis = 0x0001,
            SeriesAxis = 0x0002,
        };
        Type m_type;

        class Gridline
        {
        public:
            LineFormat m_format;
            Gridline(const LineFormat &format = LineFormat()) : m_format(format) {} 
        };
        Gridline m_majorGridlines;
        Gridline m_minorGridlines;

        LineFormat m_format;

        Axis(Type type) : Obj(), m_type(type) {}
        virtual ~Axis() {}
    };

    class Cell
    {
    public:
        int m_column;
        int m_row;
        QString m_value;
        QString m_valueType;
        Cell(int columnIndex, int rowIndex) : m_column(columnIndex), m_row(rowIndex), m_valueType("string"){};
    };

    /// cell data represetation of internal table
    class InternalTable
    {
    public:
        InternalTable():m_maxRow(0),m_maxColumn(0) {  }
        ~InternalTable() { qDeleteAll(m_cells); }

        Cell* cell(int columnIndex, int rowIndex, bool autoCreate)
        {
            const uint maximumSpreadsheetColumns = 0x7FFF; // MSOOXML::maximumSpreadsheetColumns()
            const unsigned hashed = (rowIndex + 1) * maximumSpreadsheetColumns + columnIndex + 1;
            Cell* c = m_cells[ hashed ];
            if (!c && autoCreate) {
                c = new Cell(columnIndex, rowIndex);
                m_cells[ hashed ] = c;
                if (rowIndex > m_maxRow) m_maxRow = rowIndex;
                if (columnIndex > m_maxColumn) m_maxColumn = columnIndex;
                if (!m_maxCellsInRow.contains(rowIndex) || columnIndex > m_maxCellsInRow[rowIndex])
                    m_maxCellsInRow[rowIndex] = columnIndex;
            }
            return c;
        }

        int maxCellsInRow(int rowIndex) const { return m_maxCellsInRow[rowIndex]; }
        int maxRow() const { return m_maxRow; }
        int maxColumn() const { return m_maxColumn; }

    private:
        int m_maxRow;
        int m_maxColumn;
        QHash<unsigned, Cell*> m_cells;
        QHash<int, int> m_maxCellsInRow;
    };

    /// Different types of markers.
    enum MarkerType {
        NoMarker,
        AutoMarker,
        SquareMarker,
        DiamondMarker,
        StarMarker,
        DotMarker,
        DashMarker,
        PlusMarker,
        CircleMarker,
        SymbolXMarker,
        TriangleMarker
        // TODO fill the missing marker types in
    };

    class Series : public Obj
    {
    public:
        /// the type of data in categories, or horizontal values on bubble and scatter chart groups, in the series. MUST be either 0x0001=numeric or 0x0003=text.
        int m_dataTypeX;
        /// the count of categories (3), or horizontal values on bubble and scatter chart groups, in the series.
        int m_countXValues;
        /// the count of values, or vertical values on bubble and scatter chart groups, in the series.
        int m_countYValues;
        /// the count of bubble size values in the series.
        int m_countBubbleSizeValues;
        /// termines if the data values are shown in raw values as labels
        bool m_showDataLabelValues;
        /// termines if the data values are shown in percent as labels
        bool m_showDataLabelPercent;
        /// termines if the category data is shown as labels
        bool m_showDataLabelCategory;
        /// termines if the name of the series is shown as labels
        bool m_showDataLabelSeries;
        /// Range that contains the values that should be visualized by the dataSeries.
        QString m_valuesCellRangeAddress;
        /// Ranges that contains the values that should be visualized by the dataSeries.
        QStringList m_domainValuesCellRangeAddress;
        /// The referenced values used in the chart
        QMap<Value::DataId, Value*> m_datasetValue;
        /// The formatting for the referenced values
        QList<Format*> m_datasetFormat;
        /// List of text records attached to the series.
        QList<Text*> m_texts;
        /// range that contains label
        QString m_labelCell;
        /// marker type
        MarkerType m_markerType;
        ShapeProperties* spPr;

        explicit Series() : Obj(), m_dataTypeX(0), m_countXValues(0), m_countYValues(0), m_countBubbleSizeValues(0), m_showDataLabelValues(false), m_showDataLabelPercent(false), m_showDataLabelCategory(false), m_showDataLabelSeries(false), m_markerType(NoMarker),spPr(0) {}
        virtual ~Series() { qDeleteAll(m_datasetValue); qDeleteAll(m_datasetFormat); delete spPr; }
    };

    class PlotArea : public Obj
    {
    public:
        explicit PlotArea() : Obj() {}
        virtual ~PlotArea() {}
    };

    class Legend : public Obj
    {
    public:
        explicit Legend() : Obj() {}
        virtual ~Legend() {}
    };

    /// The main charting class that represents a single chart.
    class Chart : public Obj
    {
    public:
        QString m_sheetName;
        
        /// If true then the chart is a 3d chart else teh chart is 2d.
        bool m_is3d;
        /// Specifies a counter clockwise rotation of a polar coordinate in a circle, ring or polar chart.
        int m_angleOffset;
        //int anRot, anElv, pcDist;
        /// Margins around the chart object
        int m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin;
        /// List of series
        QList<Series*> m_series;
        /// List of text records attached to the chart.
        QList<Text*> m_texts;
        /// Range of all referenced cells.
        QRect m_cellRangeAddress;
        /// Range that contains the vertical values (the categories) for the plot-area.
        QString m_verticalCellRangeAddress;
        // The ChartTitle
        QString m_title;
        /// The more concrete chart implementation like e.g. a PieImpl for a pie chart.
        ChartImpl *m_impl;
        /// The plot-area.
        PlotArea *m_plotArea;
        /// The legend.
        Legend *m_legend;
        /// List of defined axes.
        QList<Axis*> m_axes;
        /// Whether the chart is vertical or not.
        bool m_transpose;
        /// Whether the chart is stacked or not.
        bool m_stacked;
        /// Whether the chart is percentage or not.
        bool m_f100;
        /// style for colors fills line types, etc
        int m_style;
        Gradient* m_fillGradient;
        Gradient* m_plotAreaFillGradient;
        MarkerType m_markerType;
        bool m_showLines;
        qreal m_textSize;

        // charts internal table
        InternalTable m_internalTable;

        explicit Chart() : Obj(), m_is3d(false), m_angleOffset(0), m_leftMargin(0), m_topMargin(0), m_rightMargin(0), m_bottomMargin(0), m_impl(0), m_plotArea(0), m_legend(0), m_transpose(false), m_stacked(false), m_f100(false), m_style(2), m_fillGradient(0), m_plotAreaFillGradient(0), m_markerType(NoMarker), m_showLines( false ), m_textSize( 10 ) {
            m_x1 = m_y1 = m_x2 = m_y2 = -1; // -1 means autoposition/autosize
        }
        virtual ~Chart() { qDeleteAll(m_series); qDeleteAll(m_texts); delete m_impl; delete m_plotArea; delete m_legend; delete m_fillGradient; delete m_plotAreaFillGradient; }
        
        void addRange(const QRect& range)
        {
            if (range.isValid()) {
                if (m_cellRangeAddress.isValid()) {
                    if (range.left() < m_cellRangeAddress.left())
                        m_cellRangeAddress.setLeft(range.left());
                    if (range.top() < m_cellRangeAddress.top())
                        m_cellRangeAddress.setTop(range.top());
                    if (range.right() > m_cellRangeAddress.right())
                        m_cellRangeAddress.setRight(range.right());
                    if (range.bottom() > m_cellRangeAddress.bottom())
                        m_cellRangeAddress.setBottom(range.bottom());
                } else {
                    m_cellRangeAddress = range;
                }
            }
        }
    };

} // namespace Charting

#endif
