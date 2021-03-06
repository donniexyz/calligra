/* This file is part of the KDE project

   @@COPYRIGHT@@

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

#ifndef KCHART_TESTLOADING_H_ME07_PERCENTAGE_STACKED_BAR_CHART
#define KCHART_TESTLOADING_H_ME07_PERCENTAGE_STACKED_BAR_CHART

#include "../TestLoadingBase.h"

#include <QStandardItemModel>

namespace KChart {

class TestLoading : public TestLoadingBase
{
    Q_OBJECT

public:
    TestLoading();

private Q_SLOTS:
    void initTestCase();
    void testInternalTable();
    void testDataSets();
    void testLegend();

private:
    /// Faked data model of sheet embedding this chart
    QStandardItemModel m_sheet;
};

} // namespace KChart

#endif // KCHART_TESTLOADING_H_ME07_PERCENTAGE_STACKED_BAR_CHART
