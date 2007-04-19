/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOENHANCEDPATHCOMMAND_H
#define KOENHANCEDPATHCOMMAND_H

#include <QChar>
#include <QList>
#include <QPointF>
#include <QRectF>

class KoEnhancedPathShape;
class KoEnhancedPathParameter;
class KoPathPoint;

/**
 * A KoEnhancedPathCommand is a command like moveto, curveto, etc.
 * that directly modifies an enhanced paths outline.
 */
class KoEnhancedPathCommand
{
public:
    /// Constructs a new command from the given command type
    explicit KoEnhancedPathCommand( const QChar & command );
    ~KoEnhancedPathCommand();
    /// Excutes the command on the specified path shape
    bool execute( KoEnhancedPathShape * path );
    /// Adds a new parameter to the command
    void addParameter( KoEnhancedPathParameter *parameter );
    /// Returns a string representation of the command
    QString toString() const;
private:
    /// Returns a list of points, created from the parameter list
    QList<QPointF> pointsFromParameters( KoEnhancedPathShape *path );
    /// Calculates angle from given point
    double angleFromPoint( const QPointF & point ) const;
    /// Returns sweep angle from start to stop and given direction
    double radSweepAngle( double start, double stop, bool clockwise ) const;
    /// Returns sweep angle from start to stop and given direction
    double degSweepAngle( double start, double stop, bool clockwise ) const;
    /// Returns the last path point of given path
    KoPathPoint * lastPathPoint( KoEnhancedPathShape * path ) const;
    /// Returns rectangle from given points
    QRectF rectFromPoints( const QPointF &tl, const QPointF &br ) const;

    QChar m_command; ///< the actual command
    QList<KoEnhancedPathParameter*> m_parameters; ///< the commands parameters
};

#endif // KOENHANCEDPATHCOMMAND_H
