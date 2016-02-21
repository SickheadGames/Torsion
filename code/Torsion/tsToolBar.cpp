// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsToolBar.h"

#include "ColourHLS.h"
#include "ImageUtil.h"

#include <wx/dcbuffer.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( tsToolBar, wxToolBar )
   EVT_LEFT_DOWN( OnLeftDown )  
   EVT_LEFT_UP( OnLeftUp )  
   EVT_MOTION( OnMouseMove )
   EVT_LEAVE_WINDOW( OnMouseLeave ) 
	EVT_PAINT( OnPaint )
	EVT_ERASE_BACKGROUND( OnEraseBackground )
   EVT_SYS_COLOUR_CHANGED( OnSysColourChanged ) 
END_EVENT_TABLE()


tsToolBar::tsToolBar(   wxWindow* parent, 
                        wxWindowID id, 
                        const wxPoint& pos, 
                        const wxSize& size, 
                        long style, 
                        const wxString& name )
   : wxToolBar( parent, id, pos, size, style, name )
{
   m_Pressed = m_Selected = wxID_ANY;
   m_HasPainted = false;
   SetToolSeparation( 8 );

   SetWindowStyleFlag( style );
}

tsToolBar::~tsToolBar()
{
}

bool tsToolBar::Realize()
{
   return wxToolBar::Realize();

   /*
   WXDWORD style = MSWGetStyle( GetWindowStyle(), NULL );
   #define TB_SETSTYLE (WM_USER + 56)
   ::SendMessage( GetHWND(), TB_SETSTYLE, 0, style );

   static bool inRealize = false;
   */

   //return true;
}

void tsToolBar::OnLeftDown( wxMouseEvent& event )
{
   const wxToolBarToolBase* tool = FindToolForPosition( event.GetPosition().x, event.GetPosition().y );
   if ( tool && tool->IsEnabled() )
      m_Pressed = tool->GetId();
   else
      m_Selected = wxID_ANY;

   event.Skip();
}

void tsToolBar::OnLeftUp( wxMouseEvent& event )
{
   m_Pressed = wxID_ANY;
   m_Selected = wxID_ANY;
   Refresh();
   event.Skip();
}

void tsToolBar::OnMouseMove( wxMouseEvent& event )
{
   const wxToolBarToolBase* tool = FindToolForPosition( event.GetPosition().x, event.GetPosition().y );
   if ( tool )
      m_Selected = tool->GetId();
   else
      m_Selected = wxID_ANY;

   event.Skip();
}

void tsToolBar::OnMouseLeave( wxMouseEvent& event )
{
   m_Selected = wxID_ANY;
   event.Skip();
}

void tsToolBar::OnSysColourChanged( wxSysColourChangedEvent& event )
{
   // Delete all the disabled icons... they'll be 
   // recreated on the next paint.
   wxToolBarToolsList::compatibility_iterator node;
   for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
   {
      wxToolBarToolBase* tool = node->GetData();
      if ( tool->GetDisabledBitmap().Ok() )
         tool->SetDisabledBitmap( wxNullBitmap );
   }

   event.Skip();
}

void tsToolBar::OnEraseBackground( wxEraseEvent& event )
{
  if ( m_HasPainted )
      return;
  event.Skip();

  /*
  if ( m_HasPainted || !event.GetDC() )
      return;

   event.GetDC()->SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
   event.GetDC()->Clear();
   */
}

void tsToolBar::OnPaint( wxPaintEvent& event )
{
   m_HasPainted = true;

   wxGetMouseState();

   /*
   {
      wxWindowDC wdc( GetParent() );
      wxRect window = getRect();
      wdc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	   wdc.Clear();
   }
   */

   wxBufferedPaintDC dc( this );

   dc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	dc.Clear();

   ColourHLS back( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
   ColourHLS pressed( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
   ColourHLS checked( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
   ColourHLS border( back );
   {
      const ColourHLS from( wxColour( 49, 106, 197 ) );
      //wxASSERT( from.ToRGB() == wxColour( 49, 106, 197 ) );
      const ColourHLS to( wxColour( 193, 210, 238 ) );
      //wxASSERT( to.ToRGB() == wxColour( 193, 210, 238 ) );
      const float satScale = to.m_Saturation / from.m_Saturation;
      const float lumScale = to.m_Luminance / from.m_Luminance;
      back.m_Saturation = wxMin( back.m_Saturation * satScale, 1.0f );
      back.m_Luminance = wxMin( back.m_Luminance * lumScale, 0.98 );
      pressed = back;
      pressed.m_Luminance = wxMax( pressed.m_Luminance - 0.1f, 0 );
      checked = back;
      checked.m_Saturation = wxMax( checked.m_Saturation - 0.4375f, 0 );
      checked.m_Luminance = wxMin( checked.m_Luminance + 0.05f, 0.98 );
   }

   wxPen    selPen( border.ToRGB() );
   wxBrush  selBrush( back.ToRGB() );
   wxBrush  pressBrush( pressed.ToRGB() );
   wxBrush  checkBrush( checked.ToRGB() );
   wxPen    sepPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );

   const wxSize toolSize = GetToolSize();
   //const wxSize marginSize = GetMargins();

   wxCoord x = 0; // marginSize.GetWidth();

   wxToolBarToolsList::compatibility_iterator node;
   for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
   {
      wxToolBarToolBase* tool = node->GetData();

      if ( tool->IsControl() )
      {
         wxASSERT( tool->GetControl() );
         x += tool->GetControl()->GetRect().GetWidth();
         continue;
      }
      else if ( tool->IsSeparator() )
      {
         dc.SetPen( sepPen );
         wxCoord xx = x + ( GetToolSeparation() / 2 ) - 1;
         dc.DrawLine( xx, 0, xx, toolSize.GetHeight() );
         x += GetToolSeparation();
         continue;
      }

      const bool isPressed =  m_Pressed == tool->GetId() &&
                              tool->IsEnabled() &&
                              m_Pressed != wxID_ANY;

      const bool isSelected = m_Selected == tool->GetId() && 
                              tool->IsEnabled() &&
                              m_Selected != wxID_ANY && 
                              m_Pressed == wxID_ANY;

      if ( isPressed )
      {
         dc.SetPen( selPen );

         if ( m_Selected == m_Pressed )
            dc.SetBrush( pressBrush );
         else
            dc.SetBrush( selBrush );

         dc.DrawRectangle( x, 0, toolSize.GetWidth(), toolSize.GetHeight() );
      }
      if ( isSelected )
      {
         dc.SetPen( selPen );

         if ( tool->IsToggled() )
            dc.SetBrush( pressBrush );
         else
            dc.SetBrush( selBrush );

         dc.DrawRectangle( x, 0, toolSize.GetWidth(), toolSize.GetHeight() );
      }
      else if ( tool->IsToggled() )
      {
         dc.SetPen( selPen );
         dc.SetBrush( checkBrush );
         dc.DrawRectangle( x, 0, toolSize.GetWidth(), toolSize.GetHeight() );
      }

      const wxBitmap& bmp = tool->GetNormalBitmap();
      if ( bmp.Ok() )
      {
         if ( !tool->GetDisabledBitmap().Ok() )
         {
            // no disabled bitmap specified but we still need to
            // fill the space in the image list with something, so
            // we grey out the normal bitmap.
            wxImage imgGreyed = bmp.ConvertToImage(); 
            tsColorizeImage( imgGreyed, wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );

            tool->SetDisabledBitmap( wxBitmap( imgGreyed ) );
         }
         const wxBitmap& disabledBmp = tool->GetDisabledBitmap();

         const wxCoord cx = ( ( toolSize.GetHeight() / 2 ) - ( bmp.GetHeight() / 2 ) );
         /*
         wxCoord offset = 0;
         if ( isSelected && tool->IsEnabled() && !tool->IsToggled() )
         {
            offset = 1;
            dc.DrawBitmap( disabledBmp, x + cx, cx, true );
         }
         */

         if ( tool->IsEnabled() )
            //dc.DrawBitmap( bmp, x + cx - offset, cx - offset, true );
            dc.DrawBitmap( bmp, x + cx, cx, true );
         else
            dc.DrawBitmap( disabledBmp, x + cx, cx, true );
      }

      x += toolSize.GetWidth();
   }

   /*
   wxPoint TextPos( 0, GetClientSize().GetY() - dc.GetCharHeight() );
	dc.SetBackgroundMode( wxTRANSPARENT ); 
	dc.SetTextForeground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) ); 

	wxFont Font( 8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Arial" );
	dc.SetFont( Font );

	for ( int i=0; i < m_Widths.size(); i++ ) {

		wxRect ClipBox( TextPos.x, 0, m_Widths[i], GetClientSize().GetY() );
		DrawText( dc, ClipBox, m_Titles[i], TextPos.x + m_Spacer, TextPos.y, m_Spacer );
		Draw3dBox( dc, ClipBox, m_PressedCol == i );
		TextPos.x += m_Widths[i];
	}
   */

   // Cleanup
   dc.SetPen( wxNullPen );
   dc.SetBrush( wxNullBrush );
}
