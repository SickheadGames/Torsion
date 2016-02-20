// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsSashLayoutWindow.h"

#include <wx/dcbuffer.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( tsSashLayoutWindow, wxSashLayoutWindow )
   EVT_PAINT( tsSashLayoutWindow::OnPaint )
	EVT_ERASE_BACKGROUND( tsSashLayoutWindow::OnEraseBackground )
END_EVENT_TABLE()


void tsSashLayoutWindow::OnEraseBackground( wxEraseEvent& event )
{
   // Let the native control draw the background up
   // until we get our first real paint.
   //if ( m_HasPainted )
      //return;
   //event.Skip();
}

void tsSashLayoutWindow::OnPaint( wxPaintEvent& event )
{
   // Don't erase the background anymore.
   m_HasPainted = true;

   // To clear up flicker we draw the background here
   // ourselves in only the invalid area.
   wxBufferedPaintDC dc( this );
   wxDCClipper clipper( dc, GetUpdateClientRect() );
   dc.SetBackground( GetBackgroundColour() );
   dc.Clear();

   // Now do the normal drawing.
   DrawBorders(dc);
   DrawSashes(dc);
}
