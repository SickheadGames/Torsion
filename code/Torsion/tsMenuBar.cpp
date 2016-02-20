// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsMenuBar.h"

#include "ColourHLS.h"

#include <wx/dcbuffer.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( tsMenuBar, wxMenuBar )
	//EVT_PAINT( OnPaint )
	//EVT_ERASE_BACKGROUND( OnEraseBackground )
END_EVENT_TABLE()


tsMenuBar::tsMenuBar( long style )
   : wxMenuBar( style )
{
}

tsMenuBar::~tsMenuBar()
{
}

/*
void tsMenuBar::OnEraseBackground( wxEraseEvent& event )
{
   //if ( m_HasPainted || !event.GetDC() )
      //return;

   //event.GetDC()->SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
   //event.GetDC()->Clear();
}

void tsMenuBar::OnPaint( wxPaintEvent& event )
{
   m_HasPainted = true;
}
*/

// TODO: Look into owner drawing the menu bar!
/*
#ifdef __WXMSW__
WXLRESULT tsMenuBar::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
   if ( nMsg == WM_PAINT )
   {
      wxBufferedPaintDC dc( this );
      SetEvtHandlerEnabled( false );
      ::SendMessage( GetHwndOf( this ), WM_PAINT, (WXWPARAM)GetHdcOf( dc ), lParam );
      SetEvtHandlerEnabled( true );
   }

   return wxMenuBar::MSWWindowProc(nMsg, wParam, lParam);
}
#endif
*/