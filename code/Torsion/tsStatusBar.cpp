// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsStatusBar.h"

#include <wx/dcbuffer.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


tsStatusBar::tsStatusBar(  wxWindow *parent,
                           wxWindowID id,
                           long style,
                           const wxString& name ) : 
   wxStatusBar( parent, id, style, name )
{
}

BEGIN_EVENT_TABLE( tsStatusBar, wxStatusBar )  
   EVT_ERASE_BACKGROUND( tsStatusBar::OnEraseBackground )
   EVT_PAINT( tsStatusBar::OnPaint ) 
END_EVENT_TABLE()

#ifdef __WXMSW__

   void tsStatusBar::OnEraseBackground( wxEraseEvent& event )
   {
   }

   void tsStatusBar::OnPaint( wxPaintEvent& event )
   {
      // To clear up flicker we draw the background here
      // ourselves in only the invalid area.
      wxBufferedPaintDC dc( this );
      wxDCClipper clipper( dc, GetUpdateRegion().GetBox() );
	   dc.SetBackground( GetBackgroundColour() );
	   dc.Clear();

      // Draw a seperating line between us and whatever is above us.
      wxPen pen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ), 1, wxSOLID );
      dc.SetPen( pen );
      dc.DrawLine( 0, 1, GetClientSize().x, 1 );
      dc.SetPen( wxNullPen );

      // Unhook ourselves from message handling for a sec
      // and let the native control render to our buffered dc.
      SetEvtHandlerEnabled( false );
      ::SendMessage( GetHWND(), WM_PRINT, (WPARAM)dc.GetHDC(), PRF_CLIENT );
      SetEvtHandlerEnabled( true );
   }

#endif // __WXMSW__
