// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"

#include "ClientWindow.h"
#include "MainFrame.h"
#include <wx/laywin.h>

#pragma warning( disable : 4309 )
#include "icons/torsion_mono.xbm"
wxBitmap ts_mono_logo( torsion_mono_bits, torsion_mono_width, torsion_mono_height );


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_CLASS( ClientWindow, wxMDIClientWindow )

BEGIN_EVENT_TABLE( ClientWindow, wxMDIClientWindow )
   EVT_PAINT(OnPaint)
   EVT_SIZE(OnSize) 
END_EVENT_TABLE()


ClientWindow::ClientWindow()
{
}

bool ClientWindow::CreateClient( wxMDIParentFrame* parent, long style )
{
   return wxMDIClientWindow::CreateClient( parent, style | wxCLIP_CHILDREN );
}

void ClientWindow::OnPaint( wxPaintEvent& event )
{
   wxRect dirty = GetUpdateClientRect();
   if ( dirty.IsEmpty() )
      return;

   wxPaintDC dc(this);
   
   // TOTAL HACKERY!
   // For some reason when the sashes are resized the
   // size of the client area is wrong.  So i have to
   // work it out from the positions of the project and
   // bottom sashes.
   wxSize frameSize = tsGetMainFrame()->GetClientSize();
   wxPoint root = ClientToScreen( wxPoint( 0, 0 ) );
   wxSashLayoutWindow* bottom = tsGetMainFrame()->m_BottomSash;
   if ( bottom->IsShown() )
      root.y = bottom->ClientToScreen( wxPoint( 0, 0 ) ).y;
   root = GetParent()->ScreenToClient( root );
   if ( !bottom->IsShown() )
      root.y = frameSize.y;
   wxSize corner( frameSize.x - root.x, root.y );

   int x = corner.x - ts_mono_logo.GetWidth() - 20;
   int y = corner.y - ts_mono_logo.GetHeight() - 20;

   wxColor back = wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE );
   wxColor fore( back.Red() + 7, back.Green() + 7, back.Blue() + 7 );
   dc.SetTextForeground( back );
   dc.SetTextBackground( fore ); 
   dc.DrawBitmap( ts_mono_logo, x, y, false );
}

void ClientWindow::OnSize( wxSizeEvent& event )
{
   /*
   wxLogDebug( "ClientWindow::OnSize() %d, %d", event.GetSize().x, event.GetSize().y );

   wxSize size = GetSize();  
   wxRect rect = GetRect();  
   wxRect client = GetClientRect(); 
   wxLogDebug( "size = %d, %d", size.x, size.y );
   wxLogDebug( "rect = %d, %d, %d, %d", rect.x, rect.y, rect.width, rect.height );
   wxLogDebug( "client = %d, %d, %d, %d", client.x, client.y, client.width, client.height );
   */

   Refresh();
   event.Skip();
}

