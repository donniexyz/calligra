/* This file is part of the KDE project
   Copyright (C) 2008 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KPrSnakeWipeBottomRightHorizontalStrategy.h"

KPrSnakeWipeBottomRightHorizontalStrategy::KPrSnakeWipeBottomRightHorizontalStrategy()
    : KPrMatrixWipeStrategy( KPrPageEffect::FromBottom, "snakeWipe", "topLeftHorizontal", true, true )
{
}

KPrSnakeWipeBottomRightHorizontalStrategy::~KPrSnakeWipeBottomRightHorizontalStrategy()
{
}

int KPrSnakeWipeBottomRightHorizontalStrategy::squareIndex(int x, int y, int collumns, int rows)
{
    int X = x;
    if (y & 1) X = collumns - x - 1;
    return X + (rows - y - 1) * collumns;
}

KPrMatrixWipeStrategy::Direction KPrSnakeWipeBottomRightHorizontalStrategy::squareDirection(int x, int y, int collumns, int rows)
{
    if (y & 1) return RightToLeft;
    else return LeftToRight;
}

int KPrSnakeWipeBottomRightHorizontalStrategy::maxIndex(int collumns, int rows)
{
    return collumns * rows;
}

