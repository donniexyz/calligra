/*
    kostep -- handles changetracking using operational transformation for calligra
    Copyright (C) 2013  Luke Wolf <Lukewolf101010@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef STEPSTEPLOCATION_P_H
#define STEPSTEPLOCATION_P_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtGui/QTextCursor>

class StepStepLocationPrivate
{
public:
    explicit StepStepLocationPrivate();
    ~StepStepLocationPrivate();

    StepStepLocationPrivate *operator=(const StepStepLocationPrivate &other);
    //Sets up the Location Stack using the cursor
    void constructor(const QTextCursor &cursor);
    QTextCursor convertToQTextCursor(QTextDocument *ptr);
    //converts the location stack into the format for the XML tag of s="/x/y".
    QString toString();
    void parentFrame(QTextFrame *frame, QTextFrame *rootFrame);

    QStack<int> location;
};

#endif // STEPSTEPLOCATION_P_H
