/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by  
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <iostream.h>
#include <stdlib.h>
#include "SelectionTool.h"
#include "SelectionTool.moc"
#include "GDocument.h"
#include "Canvas.h"
#include "Coord.h"
#include <qkeycode.h>
#include "TranslateCmd.h"
#include "ScaleCmd.h"
#include "RotateCmd.h"
#include "CommandHistory.h"
#include <kapp.h>
#include <klocale.h>

SelectionTool::SelectionTool (CommandHistory *history) : Tool (history) {
  state = S_Init;
}

void SelectionTool::processEvent (QEvent* e, GDocument *doc, Canvas* canvas) {
  if (e->type () == Event_MouseButtonPress)
    processButtonPressEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == Event_MouseMove) 
    processMouseMoveEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == Event_MouseButtonRelease)
    processButtonReleaseEvent ((QMouseEvent *) e, doc, canvas);
}

void SelectionTool::processButtonReleaseEvent (QMouseEvent *me,
					       GDocument *doc, 
					       Canvas* canvas) {
  canvas->setCursor (arrowCursor);
  ctype = C_Arrow;
  Handle::Mode mode = Handle::HMode_Default;

  doc->handle ().show (true);
  int xpos = me->x (), ypos = me->y ();

  /**********
   * S_Rubberband
   */
  if (state == S_Rubberband) {
    QList<GObject> olist;
    Rect selRect (selPoint[0], selPoint[1]);
    if (doc->findObjectsContainedIn (selRect.normalize (), olist)) {
      QListIterator<GObject> it (olist);
      for (; it.current (); ++it) 
	doc->selectObject (it.current ());
      state = S_Pick;
    }
    else {
      // no object found - repaint canvas to remove the rubberband
      canvas->repaint ();
      state = S_Init;
    }
  }
  /**********
   * S_Pick
   */
  else if (state == S_Pick) {
    // ?
  }
  /**********
   * S_Scale, S_Translate
   */
  else if (state == S_Scale) {
    state = S_Pick;
    canvas->snapPositionToGrid (xpos, ypos);
    scale (doc, oldmask, xpos - firstpos.x (), ypos - firstpos.y (),
	   true);
  }
  else if (state == S_Translate) {
    state = S_Pick;
    canvas->snapPositionToGrid (xpos, ypos);
    translate (doc, xpos - firstpos.x (), ypos - firstpos.y (), true);
  }
  /**********
   * S_Intermediate1
   */
  else if (state == S_Intermediate1) {
    state = S_RotateSelect;
    mode = Handle::HMode_Rotate;
  }
  /**********
   * S_Rotate
   */
  else if (state == S_Rotate) {
    state = S_RotateSelect;
    mode = Handle::HMode_Rotate;
    canvas->snapPositionToGrid (xpos, ypos);
    rotate (doc, xpos - firstpos.x (), ypos - firstpos.y (), 
	    me->x (), me->y (), true);
  }
  /**********
   * S_Intermediate2
   */
  else if (state == S_Intermediate2) {
    state = S_Pick;
    //    doc->handle ().setMode (Handle::HMode_Default);
  }
  doc->handle ().setMode (mode, true);
}

void SelectionTool::processMouseMoveEvent (QMouseEvent *me, GDocument *doc, 
					   Canvas* canvas) {
  int hmask;

  /**********
   * S_Rubberband
   */
  if (state == S_Rubberband) {
    selPoint[1].x (me->x ());
    selPoint[1].y (me->y ());
    canvas->repaint ();
    QPainter painter;
    painter.save ();
    QPen pen (blue, 1, DotLine);
    painter.begin (canvas);
    painter.setPen (pen);
    float sfactor = canvas->scaleFactor ();
    painter.scale (sfactor, sfactor);
    Rect selRect (selPoint[0], selPoint[1]);
    painter.drawRect ((int) selRect.x (), (int) selRect.y (),
		      (int) selRect.width (), (int) selRect.height ());
    painter.restore ();
    painter.end ();
    return;
  }

  if (! doc->selectionIsEmpty ()) {
    int xpos = me->x (), ypos = me->y ();

    //    int xoff = me->x () - firstpos.x ();
    //    int yoff = me->y () - firstpos.y ();
    //    oldpos.setX (me->x ());
    //    oldpos.setY (me->y ());

    /**********
     * S_Pick
     */
    if (state == S_Pick) {
      hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
      if (hmask) {
	if (ctype != C_Size) {
	  ctype = C_Size;
	  canvas->setCursor (crossCursor);
	}
      }
      else if (ctype != C_Arrow) {
	ctype = C_Arrow;
	canvas->setCursor (arrowCursor);
      }

      if (me->state () == LeftButton)
	state = S_Translate;
    }
    /**********
     * S_Intermediate1
     */
    else if (state == S_Intermediate1) {
      if (me->state () == LeftButton) {
	hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
	if (hmask)
	  state = S_Scale;
	else
	  state = S_Translate;
      }
    }
    /**********
     * S_Rotate
     */
    else if (state == S_Rotate) {
      // ?
    }
    else if (state == S_RotateSelect) {
      hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
      if (hmask) {
	if (ctype != C_Size) {
	  ctype = C_Size;
	  canvas->setCursor (crossCursor);
	}
      }
      else if (ctype != C_Arrow) {
	ctype = C_Arrow;
	canvas->setCursor (arrowCursor);
      }
	
    }
    if (me->state () == LeftButton) {
      canvas->snapPositionToGrid (xpos, ypos);
      int xoff = xpos - firstpos.x ();
      int yoff = ypos - firstpos.y ();

      doc->handle ().show (false);

      switch (state) {
      case S_Scale: 
        {
	  if (ctype != C_Size) {
	    ctype = C_Size;
	    canvas->setCursor (crossCursor);
	  }
	  scale (doc, oldmask, xoff, yoff);
	  break;
        }
      case S_Translate:
	if (ctype != C_Move) {
	  ctype = C_Move;
	  canvas->setCursor (sizeAllCursor);
	}
	translate (doc, xoff, yoff);
	break;
      case S_Rotate:
	rotate (doc, xoff, yoff, me->x (), me->y ());
	break;
      default:
	break;
      }
    }
  }
}

void SelectionTool::processButtonPressEvent (QMouseEvent *me, GDocument *doc, 
					     Canvas* canvas) {
  int hmask;
  GObject *obj = 0L;

  //  oldpos.setX (me->x ());
  //  oldpos.setY (me->y ());

  int xpos = me->x (), ypos = me->y ();
  canvas->snapPositionToGrid (xpos, ypos);

  firstpos.setX (xpos);
  firstpos.setY (ypos);
  
  hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
  bool ctrlFlag = me->state () & ControlButton;

  /************
   * S_Init
   */
  if (state == S_Init) {
    obj = doc->findContainingObject (me->x (), me->y ());
    if (obj) {
      // an object will be selected
      state = S_Pick;
      if (!ctrlFlag)
	doc->unselectAllObjects ();
      // add the object to the selection
      doc->selectObject (obj);
    }
    else {
      // no object
      state = S_Rubberband;
      selPoint[0].x(me->x ());
      selPoint[0].y(me->y ());
    }
  }
  /************
   * S_Pick
   */
  else if (state == S_Pick) {
    if (hmask) {
      state = S_Intermediate1;
      oldmask = hmask;
      origbox = doc->boundingBoxForSelection ();
    }
    else {
      obj = doc->findContainingObject (me->x (), me->y ());
      if (obj) { 
	if (obj->isSelected ()) {
	  // the object is already selected
	  if (ctrlFlag)
	    // remove it from the selection
	    doc->unselectObject (obj);
	  else
	    state = S_Intermediate1;
	}
	else {
	  if (!ctrlFlag)
	    doc->unselectAllObjects ();
	  // add the object to the selection
	  doc->selectObject (obj);
	}
      }
      else {
	// nothing selected
	// unselect all
	doc->unselectAllObjects ();
	
	// and switch to rubberband mode
	state = S_Rubberband;
	selPoint[0].x(me->x ());
	selPoint[0].y(me->y ());
      }
    }
  }
  /************
   * S_RotateSelect
   */
  else if (state == S_RotateSelect) {
    if (hmask) {
      origbox = doc->boundingBoxForSelection ();
      oldmask = hmask;
      if (hmask == (Handle_Top | Handle_Left) ||
	  hmask == (Handle_Bottom | Handle_Left) ||
	  hmask == (Handle_Top | Handle_Right) ||
	  hmask == (Handle_Bottom | Handle_Right)) {
	state = S_Rotate;
	rotCenter = doc->boundingBoxForSelection ().center ();
      }
      else
	state = S_Shear;
    }
    else
      state = S_Intermediate2;
  }
}

void SelectionTool::translate (GDocument* doc, int dx, int dy, 
			       bool permanent) {
  if (permanent) {
    TranslateCmd *cmd = new TranslateCmd (doc, dx, dy);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m;
    m.translate (dx, dy);
    QListIterator<GObject> it = doc->getSelection ();
    for (; it.current (); ++it) {
      it.current ()->initTmpMatrix ();
      it.current ()->ttransform (m, true);
    }
  }
}

void SelectionTool::rotate (GDocument* doc, int dx, int dy,
			    int xp, int yp, bool permanent) {
  int adx = abs (dx);
  int ady = abs (dy);
  float angle = 0;

  //  Rect r = doc->boundingBoxForSelection ();
  Rect& r = origbox;

  if (adx > ady) {
    angle = (float) adx / r.width () * 180.0;
    if ((yp > rotCenter.y () && dx > 0) ||
	(yp < rotCenter.y () && dx < 0))
      angle = -angle;
  }
  else if (adx < ady) {
    angle = (float) ady / r.height () * 180.0;
    if ((xp > rotCenter.x () && dy < 0) ||
	(xp < rotCenter.x () && dy > 0))
      angle = -angle;
  }

  if (permanent) {
    RotateCmd *cmd = new RotateCmd (doc, rotCenter, angle);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m1, m2, m3;
    m1.translate (-rotCenter.x (), -rotCenter.y ());
    m2.rotate (angle);
    m3.translate (rotCenter.x (), rotCenter.y ());
    
    QListIterator<GObject> it (doc->getSelection ());
    for (; it.current (); ++it) {
      it.current ()->initTmpMatrix ();
      it.current ()->ttransform (m1);
      it.current ()->ttransform (m2);
      it.current ()->ttransform (m3, true);
    }
  }
}

void SelectionTool::scale (GDocument* doc, int mask, int dx, int dy, 
			   bool permanent) {
  Rect& r = origbox;

  float sx = 1, sy = 1;
  float xoff = r.x (), yoff = r.y ();
  float xback = xoff, yback = yoff;
  
  if (mask & Handle_Right)
    sx = (r.width () + dx) / r.width ();
  if (mask & Handle_Bottom) 
    sy = (r.height () + dy) / r.height ();
  if (mask & Handle_Left) {
    sx = (r.width () - dx) / r.width ();
    xback = r.x () + dx;
  }
  if (mask & Handle_Top) {
    sy = (r.height () - dy) / r.height ();
    yback = r.y () + dy;
  }

  if (permanent) {
    ScaleCmd *cmd = new ScaleCmd (doc, oldmask, sx, sy, r);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m1, m2, m3;

    m1.translate (-xoff, -yoff);
    m2.scale (sx, sy);
    m3.translate (xback, yback);
    
    QListIterator<GObject> it (doc->getSelection ());
    for (; it.current (); ++it) {
      it.current ()->initTmpMatrix ();
      
      it.current ()->ttransform (m1);
      it.current ()->ttransform (m2);
      it.current ()->ttransform (m3, true);
    }
  }
}

void SelectionTool::activate (GDocument* doc, Canvas*) {
  doc->handle ().show (true);
  if (doc->lastObject ()) {
    doc->selectObject (doc->lastObject ());
    state = S_Pick;
  }
  else
    state = S_Init;
  ctype = C_Arrow;
  emit modeSelected (i18n ("Selection Mode"));
}

void SelectionTool::deactivate (GDocument* doc, Canvas* canvas) {
  doc->handle ().show (false);
  canvas->updateView ();
}
