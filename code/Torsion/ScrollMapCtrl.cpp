// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScrollMapCtrl.h"

#include <wx/stc/stc.h>
#include <wx/dcbuffer.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( ScrollMapCtrl, wxControl )
   EVT_PAINT(OnPaint)
   EVT_ERASE_BACKGROUND(OnErase)
   EVT_LEFT_DOWN(OnClick)
   EVT_LEFT_UP(OnRelease)
   EVT_LEAVE_WINDOW(OnLeave)
   EVT_MOTION(OnDrag)
END_EVENT_TABLE()

ScrollMapCtrl::ScrollMapCtrl( wxWindow* parent, wxWindowID id, 
                              const wxPoint& pos, const wxSize& size, long style )
   :  wxControl(),
      m_HasDrawn( false )
{
   Create( parent, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE | wxNO_BORDER );
}

ScrollMapCtrl::~ScrollMapCtrl()
{
   for ( int i=0; i < m_Items.size(); i++ )
      delete m_Items[i];
   m_Items.clear();
}

void ScrollMapCtrl::OnClick( wxMouseEvent& event )
{
   CaptureMouse();
   OnScroll( event );
}

void ScrollMapCtrl::OnRelease( wxMouseEvent& event )
{
   if ( !HasCapture() )
      return;

   ReleaseMouse();
}

void ScrollMapCtrl::OnLeave( wxMouseEvent& event )
{
}

void ScrollMapCtrl::OnScroll( wxMouseEvent& event )
{
   wxASSERT( m_ScrollWin );

   int topPad, bottomPad;
   CalcPad( topPad, bottomPad );

   int range;

   wxStyledTextCtrl* text = wxDynamicCast( m_ScrollWin, wxStyledTextCtrl );
   if ( text )
      range = m_Range;
   else
      range = m_ScrollWin->GetScrollRange( wxVERTICAL );

   float scale = (float)range / (GetClientSize().GetHeight() - (topPad + bottomPad));

   int pos = (event.GetPosition().y - topPad) * scale;

   if ( text )
   {
      if ( pos < 0 ) pos = 0;
      if ( pos > m_Range ) pos = m_Range-1;
      //pos = text->PositionFromPoint( wxPoint( 0, pos ) );
      int line = pos;
      int first = text->GetFirstVisibleLine();
      text->LineScroll( 0, line - first );
   }
   else
      m_ScrollWin->SetScrollPos( wxVERTICAL, pos, true );
}

void ScrollMapCtrl::OnDrag( wxMouseEvent& event )
{
   if ( !HasCapture())
   {
      if ( m_Items.empty() )
         return;

      int topPad, bottomPad;
      CalcPad( topPad, bottomPad );

      float scale = (float)m_Range / (GetClientSize().GetHeight() - (topPad + bottomPad));
      float dist;

      // Use the scale to transform the each 
      // line then measure the distance to it
      // in pixels.
      for ( int i=0; i < m_Items.size(); i++ )
      {
         dist = m_Items[i]->line / scale;
         dist -= event.GetPosition().y - topPad;
         dist = fabs( dist );
         if ( dist < 5 )
         {
            SetToolTip( m_Items[i]->tip );      
            return;
         }
      }

      SetToolTip( (wxToolTip*)NULL );      
      return;
   }

   SetToolTip( (wxToolTip*)NULL );
   OnScroll( event );
}

void ScrollMapCtrl::SetRange( int range )
{
   m_Range = range;
   for ( int i=0; i < m_Items.size(); i++ )
   {
      if ( m_Items[i]->line < range )
         continue;

      delete m_Items[i];               
      m_Items.erase( m_Items.begin() + i );
      --i;
   }

   Refresh();
}

void ScrollMapCtrl::AddItem( int line, const wxColor& color, const wxString& tip )
{
   wxString fixedTip = tip;
   fixedTip.Replace( "\r\n", "\n", true );
   fixedTip.Replace( "\t", "   ", true );

   for ( int i=0; i < m_Items.size(); i++ )
   {
      if ( m_Items[i]->line == line )
      {
         m_Items[i]->color = color;
         m_Items[i]->tip = fixedTip;
         return;
      }
   }

   Item* item = new Item;
   item->line = line;
   item->color = color;
   item->tip = fixedTip;
   m_Items.push_back( item );

   Refresh();
}

void ScrollMapCtrl::OnErase( wxEraseEvent& event )
{
   if ( !m_HasDrawn )
      event.Skip();
}

void ScrollMapCtrl::CalcPad( int& top, int& bottom ) const
{
   top = bottom = 0;
   
   if ( !m_ScrollWin )
      return;

   wxSize client = m_ScrollWin->GetClientSize(); 
   wxSize window = m_ScrollWin->GetSize(); 

   bool hasVScroll = true,hasHScroll = true; 

   if ( window.x != client.x )
   {
      top += wxSystemSettings::GetMetric( wxSYS_HSCROLL_ARROW_X );
      bottom += wxSystemSettings::GetMetric( wxSYS_HSCROLL_ARROW_X );
   }

   if ( window.y != client.y )
      bottom += wxSystemSettings::GetMetric( wxSYS_HSCROLL_ARROW_Y );
}

void ScrollMapCtrl::OnPaint( wxPaintEvent& event )
{
   m_HasDrawn = true;

   // Use the buffered DC to avoid flicker.
   wxBufferedPaintDC dc(this);
   wxRect dirty = GetUpdateClientRect();
   dc.SetClippingRegion( dirty );
   
   const wxSize size = GetSize();

   dc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	dc.Clear();

   wxPen highPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHILIGHT ), 1, wxSOLID );
   wxPen darkPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ), 1, wxSOLID );

   dc.SetPen( darkPen );
   dc.DrawLine( 1, 0, size.GetWidth()-2, 0 );
   dc.DrawLine( 1, 0, 1, size.GetHeight()-1 );
   dc.SetPen( highPen );
   dc.DrawLine( 1, size.GetHeight()-1, size.GetWidth()-2, size.GetHeight()-1 );
   dc.DrawLine( size.GetWidth()-2, 0, size.GetWidth()-2, size.GetHeight() );

   if ( m_Items.empty() )
   {
      dc.SetPen( wxNullPen );
      return;
   }

   int topPad, bottomPad;
   CalcPad( topPad, bottomPad );

   int length = size.GetHeight() - (topPad + bottomPad); //- m_ScrollWin->GetScrollThumb( wxVERTICAL );

   const double bytes = (double)length / (double)m_Range;
   const int yy = ceil( bytes );

   for ( int i=0; i < m_Items.size(); i++ )
   {
      int y = (double)m_Items[i]->line * bytes;

      wxPen pen( m_Items[i]->color );
      wxBrush brush( m_Items[i]->color );
      dc.SetPen( pen );
      dc.SetBrush( brush );
      dc.DrawRectangle( 3, (wxCoord)topPad + y, size.GetWidth()-6, (wxCoord)yy );
   }

   // Cleanup
   dc.SetPen( wxNullPen );
   dc.SetBrush( wxNullBrush );
}




