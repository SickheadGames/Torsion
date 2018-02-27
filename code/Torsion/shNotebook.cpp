// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "shNotebook.h"

#include "ColourHLS.h"

#include <wx/dcbuffer.h>
#include <wx/tooltip.h>


#pragma warning( disable : 4309 ) // warning C4309: 'initializing' : truncation of constant value

#include "icons\close_mono.xbm"
wxBitmap ts_close_bmp( close_mono_bits, close_mono_width, close_mono_height );

#include "icons\left_mono.xbm"
wxBitmap ts_left_bmp( left_mono_bits, left_mono_width, left_mono_height );

#include "icons\right_mono.xbm"
wxBitmap ts_right_bmp( right_mono_bits, right_mono_width, right_mono_height );


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


DEFINE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGING)
DEFINE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGED)
DEFINE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CLOSE)


IMPLEMENT_CLASS( shNotebook, wxControl )

BEGIN_EVENT_TABLE( shNotebook, wxControl )
   EVT_PAINT( OnPaint ) 
   EVT_SET_FOCUS( OnFocus )
   EVT_SIZE( OnSize )
   EVT_LEFT_DOWN( OnLeftDown )
   EVT_LEFT_UP( OnLeftUp )
   EVT_MIDDLE_DOWN( OnMiddleDown ) 
   EVT_MIDDLE_UP( OnMiddleUp ) 
   EVT_MOTION( OnMouseMove ) 
   EVT_LEAVE_WINDOW( OnMouseLeave )
   EVT_ERASE_BACKGROUND( OnEraseBg )
   EVT_TIMER( 0, OnScrollTimer )
   EVT_TIMER( 1, OnToolTip )
END_EVENT_TABLE()

//

shNotebook::shNotebook( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   :  wxControl( parent, id, pos, size, style | wxNO_BORDER | wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN ),
      m_Focused( -1 ),
      m_Active( -1 ),
      m_Scroll( 0 ),
      m_LastVisible( 0 ),
      m_HoveredButton( -1 ),
      m_TabSpacer( 4, 4 ),
      m_Borders( 2 ),
      m_HasPainted( false ),
      m_TabFont( NULL ),
      m_TabFontBold( NULL )
{
   m_TabFont = wxTheFontList->FindOrCreateFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Tahoma" );
   m_TabFontBold = wxTheFontList->FindOrCreateFont( 8, wxDEFAULT, wxNORMAL, wxFONTWEIGHT_BOLD, false, "Tahoma" );

   m_ScrollTimer.SetOwner( this, 0 );
   m_ToolTipTimer.SetOwner( this, 1 );
}

shNotebook::~shNotebook()
{
   for ( int i=0; i < m_Pages.GetCount(); i++ ) {
      delete m_Pages[i];
   }
}

bool shNotebook::Destroy()
{
   SetToolTip( (wxToolTip*)NULL ); 
   return wxControl::Destroy();
}

int shNotebook::AddPage( wxWindow* page, const wxString& title, const wxString& tooltip, bool select, int imageId )
{
   wxASSERT( page );
   wxASSERT( page->GetParent() == this );

   page->SetSize( m_ClientArea );

   PageInfo* info = new PageInfo;
   info->m_Wnd = page;
   info->m_Title = title;
   info->m_ToolTip = tooltip;
   info->m_ImageId = imageId;
   CalcWidths( info );

   m_Pages.Add( info );

   m_LastVisible = GetLastVisible();

   if ( select || m_Focused == -1 )
      SetSelection( m_Pages.GetCount() - 1 );
   else
      info->m_Wnd->Hide();

   return m_Pages.GetCount() - 1;
}

void shNotebook::CalcWidths( PageInfo* infos, int count )
{
   wxWindowDC dc( this );
   wxCoord w,h;

   for ( int i=0; i < count; i++ ) 
   {
      dc.SetFont( *m_TabFont );
      dc.GetTextExtent( infos[i].m_Title, &w, &h );
      infos[i].m_Width = w;
      dc.SetFont( *m_TabFontBold );
      dc.GetTextExtent( infos[i].m_Title, &w, &h );
      infos[i].m_ActiveWidth = w;
      dc.SetFont( wxNullFont );
   }
}

bool shNotebook::SetSelection( int page )
{
   if ( page == m_Focused )
      return false;

   // Ok... send the page changing event.
   {
      shNotebookEvent event( GetId(), EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGING );
      event.SetOldSelection( m_Focused );
      event.SetSelection( page );
      event.SetEventObject( this );
      GetEventHandler()->ProcessEvent( event );
      if ( !event.IsAllowed() ) {
         return false;
      }
   }

   // Disable the last selection.
   if ( m_Focused != -1 ) {

      PageInfo* info = m_Pages[ m_Focused ];
      info->m_Wnd->Hide();
      Refresh( false );
   }

   // Prepare the page change event.
   shNotebookEvent event( GetId(), EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGED );
   event.SetOldSelection( m_Focused );
   event.SetSelection( page );
   event.SetEventObject( this );

   m_Active = m_Focused = page;

   if ( m_Focused != -1 ) {

      PageInfo* info = m_Pages[ m_Focused ];
      info->m_Wnd->SetSize( m_ClientArea );
      info->m_Wnd->Show();
      info->m_Wnd->SetFocus();

      m_LastVisible = GetLastVisible();

      if ( m_Focused < m_Scroll )
         m_Scroll = m_Focused;
      else if ( m_Focused > m_LastVisible )
      {
         while ( m_Focused > m_LastVisible )
         {
            m_Scroll += ( m_Focused - m_LastVisible );
            m_LastVisible = GetLastVisible();
         }
      }

      Refresh( false );
   }

   //m_LastVisible = GetLastVisible();

   GetEventHandler()->ProcessEvent( event );
   return true;
}

bool shNotebook::RemovePage( int page )
{
   if ( page < 0 )
      return false;

   // If the page is currently focused then
   // we must move the focus to the new page.
   if ( page == m_Focused ) {

      // What is the current index of the next focused page?
      int nextFocus = m_Focused + 1;
      int lastPage = GetLastPageIndex();
      if ( nextFocus > lastPage )
         nextFocus = m_Focused - 1;

      // Set the selection to this page.
      SetSelection( nextFocus );
   }

   // Now remove the old page.
   m_Pages[ page ]->m_Wnd->Destroy();
   delete m_Pages[ page ];
   m_Pages.RemoveAt( page );

   // Fix up the focus index.
   if ( m_Focused > page )
      m_Active = m_Focused = m_Focused - 1;

   m_LastVisible = GetLastVisible();

   if ( GetPageCount() == 0 )
      m_Scroll = 0;
   else if ( m_Scroll > GetLastPageIndex() )
      m_Scroll = GetLastPageIndex();

   Refresh( false );

   return true; // TODO: Fix this.
}

void shNotebook::DeleteAllPages()
{
   for ( int i = 0; i < m_Pages.GetCount(); i++ ) {

      PageInfo* info = m_Pages[ i ];
      info->m_Wnd->Destroy();
      delete info;
   }
   m_Pages.Empty();
   m_Active = m_Focused = -1;
   m_LastVisible = -1;
   m_Scroll = 0;
}

void shNotebook::AssignImageList( wxImageList* imageList )
{
   // TODO: Implement me!
   delete imageList;
}

int shNotebook::GetPrevPageIndex() const
{
   // TODO: Do this right!
   int page = m_Focused - 1;
   if ( page < 0 ) 
      page = GetPageCount() - 1;

   return page;
}

wxWindow* shNotebook::GetPage( int page ) const
{
   if ( page >= m_Pages.GetCount() )
      return NULL;

   return m_Pages[ page ]->m_Wnd;
}

wxWindow* shNotebook::GetCurrentPage() const
{
   if ( m_Focused == -1 )
      return NULL;

   return GetPage( m_Focused );
}

int shNotebook::HitTest( const wxPoint& pt, long *flags )
{
   if ( flags )
      *flags = shNB_HITTEST_NOWHERE;

   // Are we outside the tabs?
   if ( !m_TabArea.Contains( pt ) || pt.x < m_TabSpacer.GetX() ) {
      return wxNOT_FOUND;
   }

   if (  !( GetWindowStyle() & shNB_NOBUTTONS ) &&
         pt.x >= m_ButtonArea.GetLeft() &&
         pt.y >= m_ButtonArea.GetTop() &&
         pt.y < m_ButtonArea.GetBottom() ) {

      if ( flags )
         *flags = shNB_HITTEST_ONBUTTON;

      wxCoord x = m_ButtonArea.GetLeft() + ts_left_bmp.GetWidth();
      if ( pt.x < x ) {
         return 2;
      }

      x += 1 + ts_right_bmp.GetWidth();
      if ( pt.x < x ) {
         return 1;
      }

      x += 1 + ts_close_bmp.GetWidth();
      if ( pt.x < x ) {
         return 0;
      }
   }

   // Check the tab space.
   wxASSERT( m_Scroll == 0 || m_Scroll < m_Pages.GetCount() );
   const wxCoord spacer = (m_TabSpacer.GetX() * 2);  
   wxCoord l = m_TabSpacer.GetX();
   for ( int i = m_Scroll; i < m_Pages.GetCount(); i++ ) {

      PageInfo* info = m_Pages[ i ];
      if ( i == m_Active )
         l += info->m_ActiveWidth + spacer;
      else
         l += info->m_Width + spacer;

      if ( pt.x < l ) {

         // TODO: Get detailed info.
         if ( flags )
            *flags = shNB_HITTEST_ONITEM;

         return i;
      }

      if ( l > m_TabArea.GetRight() )
         break;
   }

   return wxNOT_FOUND;
}

void shNotebook::SetPageText( int page, const wxString& title, const wxString& tooltip )
{
   PageInfo* info = m_Pages[ page ];
   info->m_Title = title;
   info->m_ToolTip = tooltip;
   CalcWidths( info );
   Refresh( false );
}

const wxString& shNotebook::GetPageText( int page ) const
{
   wxASSERT( page >= 0 );
   wxASSERT( page < m_Pages.GetCount() );

   PageInfo* info = m_Pages[ page ];
   return info->m_Title;
}

void shNotebook::OnFocus( wxFocusEvent& event )
{
   // This prevents the highlight on the notebook
   // tabs when clicked.
   if ( GetCurrentPage() )
      GetCurrentPage()->SetFocus();

   event.Skip();
}

int shNotebook::GetLastVisible() const
{
   wxCoord maxTabSpace = m_TabArea.GetWidth() - m_ButtonArea.GetWidth();
   const wxCoord spacer = (m_TabSpacer.GetX() * 2);

   int LastVisible = GetLastPageIndex();

   if ( maxTabSpace == 0 )
      return LastVisible;

   wxCoord length = 0;

   for ( int i=m_Scroll; i < m_Pages.GetCount(); i++ ) {

      PageInfo* info = m_Pages[i];
      wxASSERT( info );

      if ( i == m_Active )
         length += info->m_ActiveWidth + spacer;
      else
         length += info->m_Width + spacer;

      if ( length >= maxTabSpace ) {

         LastVisible = i - 1;
         break;
      }
   }

   return LastVisible;
}

void shNotebook::OffsetScroll( int Dir )
{
   int NewScroll = m_Scroll + Dir;

   // Never allow scrolling outside of the pages.
   if ( NewScroll > GetLastPageIndex() ||
        NewScroll < 0 )
      return;

   // Collect some consts we'll use later.
   const wxCoord maxTabSpace = m_TabArea.GetWidth() - m_ButtonArea.GetWidth();
   const wxCoord spacer = (m_TabSpacer.GetX() * 2);

   // Measure the total length of the in reverse.  This
   // gives us the current maximum scroll offset.
   wxCoord length = 0;
   int FirstVisibleIndex = 0;

   for ( int i=m_Pages.GetCount()-1; i >= 0; i-- ) {

      PageInfo* info = m_Pages[i];
      wxASSERT( info );

      if ( i == m_Active )
         length += info->m_ActiveWidth + spacer;
      else
         length += info->m_Width + spacer;

      if ( length > maxTabSpace ) {

         FirstVisibleIndex = i+1;
         break;
      }
   }

   // Don't allow scrolling right when the first
   // visible index is less than the new scroll.
   if ( FirstVisibleIndex < NewScroll && Dir > 0 )
      return;

   /*
   // Don't allow scrolling when we have enough space.
   if ( length < maxTabSpace && )
      m_Scroll = 0;
   else
      m_Scroll = first;
   */

   m_Scroll = NewScroll;
   m_LastVisible = GetLastVisible();

   Refresh( false );
}

void shNotebook::OnScrollTimer( wxTimerEvent& event )
{
   if ( m_HoveredButton == 1 )
      OffsetScroll( 1 );
   else if ( m_HoveredButton == 2 )
      OffsetScroll( -1 );
   else if ( m_HoveredButton == 0 ) {
      // TODO: Nothing to do in this case no?
   }

   Refresh( false );
}

void shNotebook::OnToolTip( wxTimerEvent& event )
{
   // TODO: This doesn't work within a timer... why?  Maybe
   // switch to idle handler?
   SetToolTip( (wxToolTip*)NULL );
   SetToolTip( m_ToolTipText );
}

void shNotebook::OnLeftDown( wxMouseEvent& event )
{
   CaptureMouse();

   SetToolTip( (wxToolTip*)NULL ); 
   m_ToolTipText.Empty();
   m_ToolTipTimer.Stop();

   long flags;
   int hit = HitTest( event.GetPosition(), &flags );

   if ( flags & shNB_HITTEST_ONBUTTON ) {

      m_HoveredButton = hit;

      OnScrollTimer( wxTimerEvent() );
      m_ScrollTimer.Start( 135 );
      return;
   }

   if ( hit != -1 && hit != m_Active ) {

      m_Active = hit;
      Refresh( false );
   }
}

void shNotebook::OnMouseMove( wxMouseEvent& event )
{
   long flags;
   int hit = HitTest( event.GetPosition(), &flags );

   if ( !HasCapture() ) {

      wxString tooltip;

      if ( flags & shNB_HITTEST_ONBUTTON ) {

         m_HoveredButton = hit;

         if ( m_HoveredButton == 0 ) 
            tooltip = "Close";
         else if ( m_HoveredButton == 1 )
            tooltip = "Scroll Right";
         else if ( m_HoveredButton == 2 )
            tooltip = "Scroll Left";
      } 

      if ( hit > -1 && flags & shNB_HITTEST_ONITEM ) {

         m_HoveredButton = -1;

         PageInfo* info = m_Pages[ hit ];
         wxASSERT( info );

         if ( !info->m_ToolTip.IsEmpty() )
            tooltip = info->m_ToolTip;
      }

      Refresh( false, &m_TabArea );

      if ( tooltip.IsEmpty() ) {

         m_ToolTipTimer.Stop();
         m_ToolTipText.Empty();
         SetToolTip( (wxToolTip*)NULL ); 
         m_HoveredButton = -1;
         return;
      }

      if ( tooltip != m_ToolTipText ) {

         if ( m_ToolTipText.IsEmpty() )
            m_ToolTipTimer.Start( GetDoubleClickTime() * 4, true );
         else
         {
            SetToolTip( (wxToolTip*)NULL );
            SetToolTip( tooltip );
         }

         m_ToolTipText = tooltip;
      }

      return;
   }

   // Disabled dragging of tabs for now!
   /*
   if (  HasCapture() &&
         flags & shNB_HITTEST_ONITEM &&
         hit != -1 && hit != m_Active ) {

      // Swap the pages.
      wxASSERT( m_Active != -1 );
      PageInfo* info = m_Pages[ hit ];
      m_Pages[ hit ] = m_Pages[ m_Active ]; 
      m_Pages[ m_Active ] = info;

      // Do we need to change the focus index?
      if ( m_Focused == hit )
         m_Focused = m_Active;

      m_Active = hit;
      Refresh( false );
   }
   */
}

void shNotebook::OnLeftUp( wxMouseEvent& event )
{
   if ( !HasCapture() )
      return;

   ReleaseMouse();

   if ( m_Focused != m_Active )
      SetSelection( m_Active );

   m_ScrollTimer.Stop();

   long flags;
   int hit = HitTest( event.GetPosition(), &flags );

   if ( flags & shNB_HITTEST_ONBUTTON ) {

      if ( hit == 0 && GetSelection() != -1 ) {

         shNotebookEvent event( GetId(), EVT_COMMAND_SH_NOTEBOOK_PAGE_CLOSE );
         event.SetSelection( GetSelection() );
         event.SetEventObject( this );
         event.Veto();
         GetEventHandler()->ProcessEvent( event );
         if ( event.IsAllowed() ) {
            RemovePage( event.GetSelection() );
         }
      }
   }

   //if ( m_HoveredButton != -1 ) {
      //m_HoveredButton = -1;
      //Refresh( false ); //, &m_TabArea );
  // }
}

void shNotebook::OnMiddleDown( wxMouseEvent& event )
{
   CaptureMouse();

   long flags;
   int hit = HitTest( event.GetPosition(), &flags );
   if ( !( flags & shNB_HITTEST_ONITEM ) ) 
      return;

   if ( hit != -1 && hit != m_Active ) {

      m_Active = hit;
      Refresh( false );
   }
}

void shNotebook::OnMiddleUp( wxMouseEvent& event )
{
   if ( !HasCapture() )
      return;

   ReleaseMouse();

   if ( m_Focused != m_Active )
      SetSelection( m_Active );

   long flags;
   int hit = HitTest( event.GetPosition(), &flags );

   if ( flags & shNB_HITTEST_ONITEM && GetSelection() != -1 ) 
   {
      shNotebookEvent event( GetId(), EVT_COMMAND_SH_NOTEBOOK_PAGE_CLOSE );
      event.SetSelection( GetSelection() );
      event.SetEventObject( this );
      event.Veto();
      GetEventHandler()->ProcessEvent( event );
      if ( event.IsAllowed() ) 
         RemovePage( event.GetSelection() );
   }
}

void shNotebook::OnMouseLeave( wxMouseEvent& event )
{
   if ( m_HoveredButton != -1 ) {
      m_HoveredButton = -1; 
      Refresh( false );
   }

   SetToolTip( (wxToolTip*)NULL ); 
   m_ToolTipText.Empty();
   m_ToolTipTimer.Stop();

   m_ScrollTimer.Stop();
}

void shNotebook::OnSize( wxSizeEvent& event )
{
   wxWindowDC dc( this );
   wxCoord w, h;
   dc.SetFont( *m_TabFont );
   dc.GetTextExtent( "ABCDEFG", &w, &h );
   dc.SetFont( wxNullFont );

   wxSize newSize = GetClientSize();
   newSize = event.GetSize();
   m_TabArea = wxRect( m_Borders, m_Borders, newSize.x-(m_Borders*2), (h * 2)-1 );
   m_ClientArea = wxRect( m_Borders, m_TabArea.GetBottom()+1, newSize.x-(m_Borders*2), (newSize.y-m_Borders)-(m_TabArea.GetBottom()+1) );

   if ( !( GetWindowStyle() & shNB_NOBUTTONS ) ) {

      m_ButtonArea = wxRect(  m_TabArea.GetRight()-1, ( m_TabArea.GetHeight() / 2 ) - ( ts_close_bmp.GetHeight() / 2 ) - 1,
                           ts_close_bmp.GetWidth() + 1 + ts_right_bmp.GetWidth() + 1 + ts_left_bmp.GetWidth() + 1, ts_close_bmp.GetHeight() );
      m_ButtonArea.Offset( -m_ButtonArea.GetWidth(), m_Borders ); 
   }

   //Freeze();

   m_LastVisible = GetLastVisible();

   // Resize all the children.
   for ( int i=0; i < m_Pages.GetCount(); i++ ) {

      PageInfo* info = m_Pages[i];
      wxASSERT( info );
      info->m_Wnd->SetSize( m_ClientArea );
   }

   //Thaw();
}

void shNotebook::OnEraseBg( wxEraseEvent& event )
{
   if ( m_HasPainted || !event.GetDC() )
      return;

   event.GetDC()->SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
   event.GetDC()->Clear();
}


void shNotebook::OnPaint( wxPaintEvent& event )
{
   m_HasPainted = true;

   wxBufferedPaintDC dc( this );

   const wxColour btnface( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	dc.SetBackground( btnface );
	dc.Clear();

   // This is a hack that sort of mimics the results in vstudio.
   /*
   ...Spruce...
   const ColourHLS from( wxColour( 162, 200, 169 ) );
   wxASSERT( from.ToRGB() == wxColour( 162, 200, 169 ) );
   const ColourHLS to( wxColour( 190, 235, 199 ) );
   wxASSERT( to.ToRGB() == wxColour( 190, 235, 199 ) );
   */
   const ColourHLS from( wxColour( 236, 233, 216 ) );
   wxASSERT( from.ToRGB() == wxColour( 236, 233, 216 ) );
   const ColourHLS to( wxColour( 255, 251, 233 ) );
   wxASSERT( to.ToRGB() == wxColour( 255, 251, 233 ) );
   const float satScale = to.m_Saturation / from.m_Saturation;
   const float lumScale = to.m_Luminance / from.m_Luminance;
   ColourHLS hls( btnface );
   hls.m_Saturation = wxMin( hls.m_Saturation * satScale, 1.0f );
   hls.m_Luminance = wxMin( hls.m_Luminance * lumScale, 0.98 );

   // Setup the brushes, pens, and fonts.
   wxBrush activeTabBrush( btnface );
   wxBrush tabBrush( hls.ToRGB() );

   wxPen highPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHILIGHT ), 1, wxSOLID );
   wxPen darkPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ), 1, wxSOLID );

   wxPen nullPen( wxColour( 0, 0, 0 ), 0, wxTRANSPARENT );
   wxPen tabSepPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ), 1, wxSOLID );
   const wxColour tabText( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );
   const wxColour tabActiveText( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT ) );

   // Draw the base background for the tab labels.
   dc.SetPen( nullPen );
   dc.SetBrush( tabBrush );
   dc.DrawRectangle( wxRect( m_TabArea.GetTopLeft(), wxSize( m_TabArea.GetWidth(), m_TabArea.GetHeight()-1 ) ) );

   // Fill the client area.
   //dc.SetBrush( activeTabBrush );
   //dc.DrawRectangle( m_ClientArea );

   // Draw the inner 3d bevel for the background for the tab labels.
   dc.SetPen( darkPen );
   dc.DrawLine( m_TabArea.GetLeft(), m_TabArea.GetTop(), m_TabArea.GetLeft(), m_TabArea.GetBottom() );
   dc.DrawLine( m_TabArea.GetLeft(), m_TabArea.GetTop(), m_TabArea.GetRight(), m_TabArea.GetTop() );
   dc.SetPen( highPen );
   dc.DrawLine( m_TabArea.GetLeft(), m_TabArea.GetBottom() - m_TabSpacer.GetY(), m_TabArea.GetRight()+1, m_TabArea.GetBottom() - m_TabSpacer.GetY() );
   dc.DrawLine( m_TabArea.GetRight(), m_TabArea.GetTop(), m_TabArea.GetRight(), (m_TabArea.GetBottom() - m_TabSpacer.GetY())+1 );

   // Draw the lower lip where the tab labels meet the page.
   dc.SetPen( nullPen );
   dc.SetBrush( activeTabBrush );
   dc.DrawRectangle( m_TabArea.GetLeft(), m_TabArea.GetBottom() - (m_TabSpacer.GetY() - 1), m_TabArea.GetWidth(), m_TabSpacer.GetY()-1 );

   // Draw each tab.
   //wxRect oldClip = dc.GetClippingBox();
   dc.SetClippingRegion( m_TabArea.GetLeft(), m_TabArea.GetTop(), m_TabArea.GetWidth() - m_ButtonArea.GetWidth() - 2, m_TabArea.GetHeight() );
   dc.SetBackgroundMode( wxTRANSPARENT );
   wxRect labelRect( m_TabArea.GetLeft() + m_TabSpacer.GetX(), m_TabArea.GetTop(), 0, m_TabArea.GetHeight() );
   for ( int i=m_Scroll; i < m_Pages.GetCount(); i++ ) 
   {
      PageInfo* info = m_Pages[i];
      wxASSERT( info );
      
      // Set the color and stuff.
      if ( i == m_Active ) 
      {
         dc.SetFont( *m_TabFontBold );
         dc.SetTextForeground( tabActiveText );
         labelRect.SetWidth( info->m_ActiveWidth + (m_TabSpacer.GetX() * 2) );

         wxRect tabRect( wxPoint( labelRect.GetLeft(), labelRect.GetTop() + m_TabSpacer.GetX() ),
                         wxPoint( labelRect.GetRight(), labelRect.GetBottom() - m_TabSpacer.GetX() ) );

         // Draw the active tab itself.
         dc.SetPen( nullPen );
         dc.SetBrush( activeTabBrush );
         dc.DrawRectangle( tabRect );

         // Draw the active tab highlight.
         dc.SetPen( highPen );
         dc.DrawLine( tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetLeft(), tabRect.GetBottom()+1 );
         dc.DrawLine( tabRect.GetLeft(), tabRect.GetTop(), tabRect.GetRight()+1, tabRect.GetTop() );

         // Draw the active tab shadow.
         dc.SetPen( darkPen );
         dc.DrawLine( tabRect.GetRight(), tabRect.GetTop()+1, tabRect.GetRight(), tabRect.GetBottom()+1 );
      }
      else
      {
         dc.SetFont( *m_TabFont );
         dc.SetTextForeground( tabText );
         labelRect.SetWidth( info->m_Width + (m_TabSpacer.GetX() * 2) );

         // Draw sep line.
         dc.SetPen( tabSepPen );
         dc.DrawLine( labelRect.GetRight()+1, labelRect.GetTop() + m_TabSpacer.GetY(), labelRect.GetRight()+1, labelRect.GetBottom() - ( m_TabSpacer.GetY() + 1 ) );
      }

      // Draw the label.
      dc.DrawLabel( info->m_Title, labelRect, wxALIGN_CENTER );

      labelRect.SetLeft( labelRect.GetLeft() + labelRect.GetWidth() );

      // Stop when we run out of room!
      if ( labelRect.GetLeft() >= m_TabArea.GetRight() )
         break;
   }
   dc.DestroyClippingRegion();

   // Cleanup
   dc.SetFont( wxNullFont );
   dc.SetPen( wxNullPen );
   dc.SetBrush( wxNullBrush );

   // Do we need to draw the buttons?
   if ( GetWindowStyle() & shNB_NOBUTTONS )
      return;

   // Now to draw the scroll and close buttons.

   // Draw the button area background.
   //dc.SetPen( nullPen );
   //dc.SetBrush( tabBrush );
   //dc.DrawRectangle( m_ButtonArea.GetLeft(), m_ButtonArea.GetTop(), m_ButtonArea.GetWidth(), m_ButtonArea.GetHeight() + 2 );
   //dc.SetPen( highPen );
   //dc.DrawLine( m_ButtonArea.GetLeft(), m_ButtonArea.GetBottom()+3, m_ButtonArea.GetRight(), m_ButtonArea.GetBottom()+3 );

   wxCoord x = m_ButtonArea.GetRight();
   dc.SetTextBackground( hls.ToRGB() );

   wxASSERT( ts_close_bmp.GetDepth() == 1 );
   x -= ts_close_bmp.GetWidth();
   dc.SetTextForeground( m_HoveredButton == 0 ? tabActiveText : tabText );
   dc.DrawBitmap( ts_close_bmp, x, m_ButtonArea.GetTop(), true ); 

   wxASSERT( ts_right_bmp.GetDepth() == 1 );
   x -= ts_right_bmp.GetWidth() + 1;
   dc.SetTextForeground( m_HoveredButton == 1 ? tabActiveText : tabText );
   dc.DrawBitmap( ts_right_bmp, x, m_ButtonArea.GetTop(), true ); 

   wxASSERT( ts_left_bmp.GetDepth() == 1 );
   x -= ts_left_bmp.GetWidth() + 1;
   dc.SetTextForeground( m_HoveredButton == 2 ? tabActiveText : tabText );
   dc.DrawBitmap( ts_left_bmp, x, m_ButtonArea.GetTop(), true );
}
