// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_TSMENUBAR_H
#define TORSION_TSMENUBAR_H
#pragma once

#include <wx/toolbar.h>


class tsMenuBar : public wxMenuBar
{
public:
   tsMenuBar( long style = 0 );
   virtual ~tsMenuBar();

protected:

   /*
   virtual bool OnLeftClick(int toolid, bool toggleDown);
   virtual void OnRightClick(int toolid, long x, long y);
   virtual void OnMouseEnter(int toolid);
   void OnPaint( wxPaintEvent& event );
   void OnEraseBackground( wxEraseEvent& event );
   bool m_HasPainted;
   //int m_Selected;
   */

   /*
   #ifdef __WXMSW__
   WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
   #endif
   */

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_TSMENUBAR_H