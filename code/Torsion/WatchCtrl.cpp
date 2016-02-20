// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "WatchCtrl.h"

#include "TorsionApp.h"
#include "Debugger.h"

#include <wx/dnd.h>
#include <wx/dcbuffer.h>
#include <vector>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


class ListCtrlEdit;

class ListCtrl : public wxScrolledWindow
{
	public:
		ListCtrl( WatchCtrl* parent, HeaderCtrl* header );
		~ListCtrl();

		void OnChanged();
		void BeginEdit( int Row, int Col );

	protected:

		friend class ListCtrlEdit;

		void OnFocus( wxFocusEvent& Event );
		void OnChar( wxKeyEvent& Event );
		void OnMouse( wxMouseEvent& Event );
		void OnPaint( wxPaintEvent& Event );
		void OnEraseBackground( wxEraseEvent& Event ) {}

		wxRect _GetRect( int Row, int Col ) const;

		void _OnVarEdit( int Row, int Col, const wxString& Value );

		bool HitTest( const wxPoint& Point, int& Row, int& Column ) const;

		bool HitExpansionBox( const wxPoint& Point, int Row ) const;

		void DrawExpandBox( wxDC& dc, const wxRect& rect, bool expanded );

      virtual bool MSWShouldPreProcessMessage( WXMSG* pMsg );

		int				m_Selected;

		ListCtrlEdit*	m_EditCtrl;

		long			m_LineHeight;
		wxFont*			m_Font;
		WatchCtrl*		m_WatchCtrl;
		HeaderCtrl*		m_HeaderCtrl;

	DECLARE_EVENT_TABLE()
};


class TextDropTarget : public wxTextDropTarget
{
	public:

		TextDropTarget( WatchCtrl* Target )
			:	wxTextDropTarget(),
				m_Target( Target )
		{
		}

	public:

      virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
      {
         return wxDragCopy;            
      }

      virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
      {
         if ( !GetData() )
            return wxDragNone;

         wxTextDataObject *dobj = (wxTextDataObject *)m_dataObject;
         return OnDropText(x, y, dobj->GetText()) ? wxDragCopy : wxDragNone;
      }

		bool OnDropText( wxCoord x, wxCoord y, const wxString& data )
		{
			wxString VarName( data );
			
			VarName.Trim();
			VarName.Trim( true );

			// Take only the first line.
			int Pos = wxMin( VarName.First( '\n' ), VarName.First( '\r' ) );
			if ( Pos != -1 ) {
				VarName = VarName.Left( Pos - 1 );
			}

			m_Target->AddVar( VarName );

			return true;
		}

	protected:

		WatchCtrl* m_Target;

};

class ListCtrlEdit : public wxTextCtrl
{
public:

	ListCtrlEdit( ListCtrl* Owner, const wxRect& Rect, int Row, int Col, const wxString& DefaultText );

protected:

	void OnChar( wxKeyEvent &event );
	void OnKeyUp( wxKeyEvent &event );
	void OnKillFocus( wxFocusEvent &event );
	void Finish( bool SendValue = false );

   virtual bool MSWShouldPreProcessMessage( WXMSG* pMsg );

protected:

	ListCtrl*			m_Owner;
	int					m_Row;
	int					m_Col;
	bool				m_Finished;

	DECLARE_EVENT_TABLE()
};


class HeaderCtrl : public wxWindow
{
	public:
		HeaderCtrl( WatchCtrl* parent );

		void SetList( ListCtrl* list ) { m_ListCtrl = list; }

		static void DrawText( wxDC& dc, const wxRect& clipped, const wxString& text, int x, int y, int spacer = 0 );

		int GetColWidth( int col ) const;
      
      int GetColCount() const { return m_Titles.size(); }

	protected:

		void OnMouse( wxMouseEvent& Event );
		void OnPaint( wxPaintEvent& Event );
		void OnEraseBackground( wxEraseEvent& Event ) {}
		void ValidateAndRefreshWidths( wxSizeEvent& Event = wxSizeEvent() );

		void Draw3dBox( wxDC& dc, const wxRect& rect, bool pressed = false );

		int			m_PressedCol;
		int			m_DragCol;

		const int	m_Spacer;

		std::vector<wxString>	m_Titles;
		std::vector<int>		m_Widths;

		WatchCtrl*		m_WatchCtrl;
		ListCtrl*		m_ListCtrl;

		DECLARE_EVENT_TABLE()
};


HeaderCtrl::HeaderCtrl( WatchCtrl* parent )
	:	wxWindow( parent, -1, wxDefaultPosition, wxSize( 100, 17 ), wxNO_BORDER ),
		m_WatchCtrl( parent ),
		m_ListCtrl( NULL ),
		m_DragCol( -1 ),
		m_Spacer( wxSystemSettings::GetMetric( wxSYS_FRAMESIZE_X ) )
{
	m_Titles.resize( 2 );
	m_Titles[0] = "Name";
	m_Titles[1] = "Value";
	//m_Titles[2] = "Type";

	m_Widths.resize( 2 );
	m_Widths[0]=150;
	m_Widths[1]=300;
	//m_Widths[2]=400;
}

BEGIN_EVENT_TABLE( HeaderCtrl, wxWindow )
	EVT_PAINT( OnPaint )
	EVT_MOUSE_EVENTS( OnMouse )
	EVT_SIZE( ValidateAndRefreshWidths )
	EVT_ERASE_BACKGROUND( OnEraseBackground )
END_EVENT_TABLE()

int HeaderCtrl::GetColWidth( int col ) const
{
	return m_Widths[ col ];
}

void HeaderCtrl::ValidateAndRefreshWidths( wxSizeEvent& Event )
{
	int ClientX = GetClientSize().GetX();
    int Space = m_Spacer * 4;

	// Make sure none are too small!
	int TotalWidth = 0;
	for ( int i=0; i < m_Widths.size(); i++ ) {
		m_Widths[i] = wxMax( m_Widths[i], Space );
		TotalWidth += m_Widths[i];
	}

	if ( ClientX > 0 ) {

		int Last = m_Widths.size() - 1;
		int Remainder = ClientX - TotalWidth;
		if ( Remainder > 0 ) {

			m_Widths[ Last ] += Remainder;

		} else {

			m_Widths[ Last ] -= -Remainder;
			if ( m_Widths[ Last ] < Space ) {
				
				m_Widths[ Last ] = Space;
                                
			}
		}
	}

	if ( m_ListCtrl ) {
		m_ListCtrl->Refresh();
	}
	Refresh();
}

void HeaderCtrl::OnPaint( wxPaintEvent& Event )
{
	wxBufferedPaintDC dc( this );

	dc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	dc.Clear();

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

   dc.SetFont( wxNullFont );
}

void HeaderCtrl::OnMouse( wxMouseEvent& Event )
{
	if ( m_DragCol != -1 && !Event.LeftIsDown() ) {

		m_DragCol = -1;

		if ( HasCapture() ) {
			ReleaseMouse();
		}

		SetCursor( wxCursor( wxCURSOR_DEFAULT ) );
	}

	if ( m_PressedCol != -1 && !Event.LeftIsDown() ) {

		m_PressedCol = -1;
		Refresh();
	}

	if ( m_DragCol == -1 ) {

		int Col = -1;
		if ( abs( Event.GetPosition().x - m_Widths[0] ) < m_Spacer ) {
			Col = 0;
		} else if ( abs( Event.GetPosition().x - ( m_Widths[0] + m_Widths[1] ) ) < m_Spacer ) {
			Col = 1;
		}

		if ( Col != -1 ) {

			SetCursor( wxCursor( wxCURSOR_SIZEWE ) );

			if ( Event.LeftDown() ) {

				m_DragCol = Col;
				CaptureMouse();
			}

		} else {

			SetCursor( wxCursor( wxCURSOR_DEFAULT ) );

			int Col = -1;
			if ( Event.GetPosition().x < m_Widths[0] ) {

				Col = 0;

			} else if ( Event.GetPosition().x < ( m_Widths[0] + m_Widths[1] ) ) {

				Col = 1;

			} else {

				Col = 2;
			}

			if ( Event.LeftDown() && !Event.ButtonDClick() && m_PressedCol != Col ) {

            
				m_PressedCol = Col;
				Refresh();
			}
		}
	}

	if ( m_DragCol == 0 ) {

		m_Widths[0] = Event.GetPosition().x;

	} else if ( m_DragCol == 1 ) {

		m_Widths[1] = Event.GetPosition().x - m_Widths[0];
	}

	if ( m_DragCol != -1 ) {
		ValidateAndRefreshWidths();
	}
}

void HeaderCtrl::DrawText( wxDC& dc, const wxRect& Clipped, const wxString& text, int x, int y, int spacer )
{
	wxCoord ex, ey;
	wxCoord width = Clipped.GetWidth() - ( x - Clipped.GetLeft() );

   // Do a sort of binary search to find the 
   // perfect amount of letters to clip.
	wxString clippedText = text;
   size_t   low = 0,
            high = clippedText.Length(),
            curr = high;
   
   int passes = 0;

   while ( low != high ) 
   {
      passes++;

      // Check the current text first.
      dc.GetTextExtent( clippedText, &ex, &ey );

		if ( ( ex + spacer ) < width )
      {
         if ( curr == text.Length() )
            break;

         low = curr + 1;
      }
      else
         high = curr;
   
      curr = ( low + high ) / 2;

      // Generate the new string to test.
      clippedText = text.Left( curr );
      clippedText.Trim( true );
      clippedText.Append( "..." );
   }

	dc.DestroyClippingRegion();
	dc.SetClippingRegion( Clipped );
	dc.DrawText( clippedText, x, y );
	dc.DestroyClippingRegion();

	wxWindow* Window = dc.GetWindow();
	if ( Window )
        dc.SetClippingRegion( Window->GetUpdateRegion() );
}

void HeaderCtrl::Draw3dBox( wxDC& dc, const wxRect& rect, bool pressed )
{
	wxPen OldPen = dc.GetPen();

	if ( pressed ) {

		dc.SetPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );
		dc.DrawLine( rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetTop() );
		dc.DrawLine( rect.GetLeft(), rect.GetBottom(), rect.GetRight(), rect.GetBottom() );
		dc.DrawLine( rect.GetLeft(), rect.GetTop(), rect.GetLeft(), rect.GetBottom() );
		dc.DrawLine( rect.GetRight(), rect.GetTop(), rect.GetRight(), rect.GetBottom() );

	} else {

		dc.SetPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );
		dc.DrawLine( rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetTop() );
		dc.DrawLine( rect.GetLeft(), rect.GetBottom(), rect.GetLeft(), rect.GetTop() );

		dc.SetPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );
		dc.DrawLine( rect.GetRight()-1, rect.GetTop()+1, rect.GetRight()-1, rect.GetBottom()-1 );
		dc.DrawLine( rect.GetRight()-1, rect.GetBottom()-1, rect.GetLeft(), rect.GetBottom()-1 );

		dc.SetPen( wxSystemSettings::GetColour( wxSYS_COLOUR_3DDKSHADOW ) );
		dc.DrawLine( rect.GetRight(), rect.GetTop(), rect.GetRight(), rect.GetBottom() );
		dc.DrawLine( rect.GetLeft(), rect.GetBottom(), rect.GetRight()+1, rect.GetBottom() );
	}

	dc.SetPen( OldPen );
}



ListCtrl::ListCtrl( WatchCtrl* parent, HeaderCtrl* header )
	:	wxScrolledWindow( parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxHSCROLL | wxVSCROLL ),
		m_WatchCtrl( parent ),
		m_HeaderCtrl( header ),
		m_Selected( -1 ),
		m_EditCtrl( NULL ),
      m_Font( NULL )		
{
   m_Font = wxTheFontList->FindOrCreateFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Arial" );
   wxClientDC dc( this );
	dc.SetFont( *m_Font );
	m_LineHeight = dc.GetCharHeight() + 2;
   dc.SetFont( wxNullFont );
}

ListCtrl::~ListCtrl()
{
}

BEGIN_EVENT_TABLE( ListCtrl, wxScrolledWindow )
	EVT_PAINT( OnPaint )
   EVT_MOUSE_EVENTS( OnMouse )
   EVT_CHAR( OnChar )
   EVT_SET_FOCUS( OnFocus )
   EVT_KILL_FOCUS( OnFocus )
	EVT_ERASE_BACKGROUND( OnEraseBackground )
END_EVENT_TABLE()

void ListCtrl::OnFocus( wxFocusEvent& Event )
{
	Refresh();
   Event.Skip();
}

bool ListCtrl::MSWShouldPreProcessMessage( WXMSG* pMsg )
{
   MSG *msg = (MSG*)pMsg;

   if ( msg->message != WM_KEYDOWN )
      return true;

   WORD vkey = (WORD)msg->wParam;
   if ( vkey == VK_DELETE )
      return false;

   return true;
}

void ListCtrl::OnChar( wxKeyEvent& Event )
{
	wxASSERT( m_WatchCtrl );

    switch ( Event.m_keyCode )
    {
        case WXK_RETURN:
			if ( m_WatchCtrl->m_EditableRow ) {
				BeginEdit( m_Selected, 0 );		                        
			}
            break;

		case WXK_DELETE:
			if ( m_WatchCtrl->m_EditableRow ) {
				m_WatchCtrl->DeleteAt( m_Selected );		                        
			}
            break;

		case WXK_PAGEUP:
		case WXK_PRIOR:
            if ( m_WatchCtrl->m_Vars.GetCount() > 0 ) {
                m_Selected = 0;
				Refresh();
			}
            break;

		case WXK_PAGEDOWN:
		case WXK_NEXT:
            m_Selected = (int)m_WatchCtrl->m_Vars.GetCount() - 1;
			Refresh();
            break;

		case WXK_UP:
			if ( m_Selected > 0 ) {
				--m_Selected;
				Refresh();
			}
            break;

		case WXK_DOWN:
			if ( m_Selected < ( (int)m_WatchCtrl->m_Vars.GetCount() - 1 ) ) {
				++m_Selected;
				Refresh();
			}
            break;

        default:
            Event.Skip();
    }
}

void ListCtrl::BeginEdit( int Row, int Col )
{
	WatchVarArray& Vars = m_WatchCtrl->m_Vars;

   // We never allow editing when not at a breakpoint!
   wxASSERT( tsGetDebugger() );
   if ( !tsGetDebugger()->IsAtBreakpoint() )
      return;

   // Filter out other cases where editing is not allowed.
   if (	Row < 0 || 
			Row >= Vars.GetCount() ||
			Vars[ Row ]->GetParent() ||
         ( Vars[ Row ]->GetName().IsEmpty() && Col > 0 ) ) {

		return;
	}

   wxString Value;
   if ( Col == 0 )
      Value = Vars[ Row ]->GetName();
   else
      Value = Vars[ Row ]->GetValue();

	// Do modal edit box!
	if ( m_EditCtrl )
        m_EditCtrl->Destroy();

	wxRect Rect( _GetRect( Row, Col ) );
	Rect.Inflate( 0, 2 );

	m_EditCtrl = new ListCtrlEdit( this, Rect, Row, Col, Value );  
	m_EditCtrl->SetFocus();
}

void ListCtrl::OnMouse( wxMouseEvent& Event )
{
   int Row, Column;
	if (  m_Selected != -1 && 
         Event.ButtonDClick() && 
         m_WatchCtrl->m_EditableRow &&
         HitTest( Event.GetPosition(), Row, Column ) &&
         Row == m_Selected &&
         Column < 2 ) {

      BeginEdit( m_Selected, Column );
		return;
	}

	if ( Event.LeftDown() ) {

		// Hacky... but it works!
		if ( m_EditCtrl ) {
			m_EditCtrl->Show( false );
			m_EditCtrl->Destroy();
			m_EditCtrl = NULL;
		}
		SetFocus();

		const WatchVarArray& Vars = m_WatchCtrl->m_Vars;
		bool DoRefresh = false;

		int Selected = CalcUnscrolledPosition( wxPoint( 0, Event.GetY() ) ).y / m_LineHeight;
		if ( Selected < Vars.GetCount() && m_Selected != Selected ) {
			m_Selected = Selected;
			DoRefresh = true;
		}

		// Expanding?
		if (	m_Selected >= 0 && 
				Vars[ m_Selected ]->IsExpandable() && 
				HitExpansionBox( Event.GetPosition(), m_Selected ) ) {

			m_WatchCtrl->_ToggleExpanded( m_Selected );			
			return;
		}

		if ( DoRefresh ) {
			Refresh();
			return;
		}
	}

	// Deal with wheel scrolling events.
	float Wheel = Event.GetWheelRotation();
	if ( Wheel != 0 ) {

		Wheel /= (float)Event.GetWheelDelta();

		wxPoint Start;
		GetViewStart( &Start.x, &Start.y );
		Start.y -= Wheel;
		Scroll( Start.x, Start.y );
		return;
	}

	Event.Skip();
}

wxRect ListCtrl::_GetRect( int Row, int Col ) const
{
	wxASSERT( m_HeaderCtrl );

   int x = 0;
   for ( int c=0; c < Col; c++ ) {
      x += m_HeaderCtrl->GetColWidth( c );
   }

   const int ColWidth = m_HeaderCtrl->GetColWidth( Col );
	wxPoint Pos( x, m_LineHeight * Row );
	Pos = CalcScrolledPosition( Pos );
	return wxRect( Pos, wxSize( ColWidth, m_LineHeight ) );
}

bool ListCtrl::HitTest( const wxPoint& Point, int& Row, int& Column ) const
{
	wxASSERT( m_HeaderCtrl );

   const int cols = m_HeaderCtrl->GetColCount();
   const int rows = m_WatchCtrl->m_Vars.GetCount();

   for ( int r=0; r < rows; r++ ) {

      for ( int c=0; c < cols; c++ ) {

         wxRect rect = _GetRect( r, c );
         if ( rect.Inside( Point ) ) {

            Row = r;
            Column = c;
            return true;
         }
      }
   }

   return false;
}

void ListCtrl::_OnVarEdit( int Row, int Col, const wxString& Value )
{
   wxASSERT( Row == m_Selected );

   wxASSERT( tsGetDebugger()->IsAtBreakpoint() );

	wxString Fixed( Value );
	Fixed.Trim();
	Fixed.Trim( true );

	WatchVarArray& Vars = m_WatchCtrl->m_Vars;
	WatchVar* Var = Vars[ Row ];

   if ( Col == 0 ) {

      Var->SetName( Fixed );
      if ( !Fixed.IsEmpty() )
   		tsGetDebugger()->UpdateVar( Var );
      else
         Var->SetValue( wxEmptyString );
   }
   else if ( Col == 1 ) 
   {
      // If the value hasn't changed then we've
      // got nothing further to do here.
      if ( Var->GetValue() == Fixed )
         return;

		tsGetDebugger()->ChangeVar( Var, Fixed );
   }

   // Do we need to add an empty one?
	wxASSERT( m_WatchCtrl->m_EditableRow );
	wxASSERT( Vars.GetCount() >= 1 );
   Var = Vars.Last();
	if ( !Var->GetName().IsEmpty() ) {

		wxASSERT( !Var->GetParent() );
		m_WatchCtrl->_AddVar( new WatchVar( NULL, "", 0, "", "" ), true );
	}

	// Clear any extra empties from the bottom of the list.
	for ( int i = Vars.GetCount()-2; i >= 0; i-- ) {

		Var = Vars[ i ];
		if ( Var->GetName().IsEmpty() ) {

			delete Var;
			Vars.RemoveAt( i );
		}
	}

	OnChanged();
}

void ListCtrl::OnChanged()
{
	const WatchVarArray& Vars = m_WatchCtrl->m_Vars;

	m_Selected = wxMin( m_Selected, Vars.GetCount()-1 );

	// Get the current row.
	wxPoint Start;
	GetViewStart( &Start.x, &Start.y );

	// Set the new virtual size.
   SetScrollRate( m_LineHeight, m_LineHeight );
	SetVirtualSize( -1, Vars.Count() * m_LineHeight );

	// Try to set the old row as the top.
	Scroll( Start.x, Start.y );

	Refresh();
}

void ListCtrl::DrawExpandBox( wxDC& dc, const wxRect& rect, bool expanded )
{
	wxPen OldPen		= dc.GetPen();
	wxBrush OldBrush	= dc.GetBrush();

	wxPen Pen( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ), 1, wxSOLID );
	dc.SetPen( Pen );
	wxBrush Brush( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ), wxSOLID );
	dc.SetBrush( Brush );

	dc.DrawRectangle( rect );

	int Mid = rect.GetTop() + ( rect.GetHeight() / 2 );
	dc.DrawLine( rect.GetLeft() + 2, Mid, rect.GetRight() - 1, Mid );

	if ( !expanded ) {

		Mid = rect.GetLeft() + ( rect.GetWidth() / 2 );
		dc.DrawLine( Mid, rect.GetTop() + 2, Mid, rect.GetBottom() - 1 );
	}

	dc.SetPen( OldPen );
}


bool ListCtrl::HitExpansionBox( const wxPoint& Point, int Row ) const
{
	const wxArrayInt& Indents = m_WatchCtrl->m_Indent;

	wxASSERT( Row >= 0 );
	wxASSERT( Row < Indents.size() );

	const int Spacer = wxSystemSettings::GetMetric( wxSYS_FRAMESIZE_X );
	const int Box = m_LineHeight - 5;
	const int Pad = Spacer + ( Indents[ Row ] * ( Box + Spacer + 1 ) );
	const int Line = m_LineHeight * Row;

	wxRect ExpandBox( Pad, Line + (( m_LineHeight - Box ) / 2 ) + 1, Box, Box );
	return ExpandBox.Inside( CalcUnscrolledPosition( Point ) );
}

void ListCtrl::OnPaint( wxPaintEvent& Event )
{
	wxBufferedPaintDC dc( this );

   //dc.DestroyClippingRegion();
   //dc.SetClippingRegion( GetClientRect() );

	dc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	dc.Clear();

   //DoPrepareDC( PaintDC );

	//wxSize VirtualSize;
	//GetTargetSize(
	//GetVirtualSize( &VirtualSize.x, &VirtualSize.y );
	//VirtualSize.y += m_LineHeight * 4;
	//wxBufferedDC dc( &PaintDC, VirtualSize, wxBUFFER_VIRTUAL_AREA );
   //dc.DestroyClippingRegion();
	int top;
	GetViewStart( NULL, &top );
   
	//wxBufferedPaintDC dc( this );
	//DoPrepareDC( dc );

	const WatchVarArray& Vars = m_WatchCtrl->m_Vars;
	const wxArrayInt& Indents = m_WatchCtrl->m_Indent;

	dc.SetBackground( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	dc.Clear();

    // Draw each var line and text.
	wxPen LinePen( wxSystemSettings::GetColour( wxSYS_COLOUR_3DFACE ), 1, wxSOLID );
	dc.SetPen( LinePen );
	dc.SetFont( *m_Font );
	wxPoint Line( 0, 0 );
	const wxPoint End( GetClientSize().GetX(), 0 );
	int Spacer	= wxSystemSettings::GetMetric( wxSYS_FRAMESIZE_X );
	int Box		= m_LineHeight - 5;

	dc.SetBackgroundMode( wxTRANSPARENT ); 
	dc.SetTextForeground( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) ); 

   dc.DrawLine( Line, Line + End );

	for ( int i=top; i < Vars.Count(); i++ ) {

		if ( m_Selected == i && FindFocus() == this ) {

			wxBrush Highlight( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
			wxBrush OldBrush = dc.GetBrush();
			dc.SetBrush( Highlight );
			dc.DrawRectangle( 0, Line.y, GetClientSize().GetX(), m_LineHeight+1 );
			dc.SetBrush( OldBrush );

			dc.SetTextForeground( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) ); 
		}

		wxPoint TextPos( 0, Line.y + ( m_LineHeight - dc.GetCharHeight() ) );
		for ( int c=0; c < m_HeaderCtrl->GetColCount(); c++ ) {

			int ColWidth = m_HeaderCtrl->GetColWidth( c );
			int Pad = Spacer;

			wxRect ColBox( TextPos.x, Line.y, ColWidth, m_LineHeight );

			if ( c == 0 ) {

				dc.DestroyClippingRegion();
				dc.SetClippingRegion( ColBox );

				Pad += ( Indents[i] * ( Box + Spacer + 1 ) );

				if ( Vars[i]->IsExpandable() ) {

					wxRect ExpandBox( Pad, Line.y + (( m_LineHeight - Box ) / 2 ) + 1, Box, Box );
					DrawExpandBox( dc, ExpandBox, Vars[i]->IsExpanded() );
				}

				Pad += Box + Spacer + 1;

				dc.DestroyClippingRegion();
				dc.SetClippingRegion( GetUpdateRegion() );
			}

			wxString Text;
			if ( c == 0 ) {
				Text = Vars[i]->GetName();
			} else if ( c == 1 ) {
				Text = Vars[i]->GetValue();
			} else {
				Text = Vars[i]->GetType();
			}
			
			HeaderCtrl::DrawText( dc, ColBox, Text, TextPos.x + Pad, TextPos.y, Spacer );

			TextPos.x += ColWidth;

			if ( c < m_HeaderCtrl->GetColCount()-1 ) {
				dc.DrawLine( wxPoint( TextPos.x - 2, Line.y ), wxPoint( TextPos.x - 2, Line.y + m_LineHeight ) );
			}
		}

		Line.y += m_LineHeight;
		dc.DrawLine( Line, Line + End );

		if ( m_Selected == i ) {
			dc.SetTextForeground( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) ); 
		}

		if ( Vars[i]->IsExpanded() ) {
			
		}
	}

   // Cleanup
   dc.SetFont( wxNullFont );
   dc.SetPen( wxNullPen );
   dc.SetBrush( wxNullBrush );
}


ListCtrlEdit::ListCtrlEdit( ListCtrl* Owner, const wxRect& Rect, int Row, int Col, const wxString& DefaultText )
	: wxTextCtrl( Owner, -1, DefaultText, Rect.GetPosition(), Rect.GetSize() ),
		m_Owner( Owner ),
		m_Row( Row ),
		m_Col( Col ),
		m_Finished( false )
{
	SetSelection( -1, -1 );
}

BEGIN_EVENT_TABLE( ListCtrlEdit, wxTextCtrl )
    EVT_CHAR( OnChar )
    EVT_KEY_UP( OnKeyUp )
    EVT_KILL_FOCUS( OnKillFocus )
END_EVENT_TABLE()

bool ListCtrlEdit::MSWShouldPreProcessMessage( WXMSG* pMsg )
{
   MSG *msg = (MSG*)pMsg;

   if ( msg->message != WM_KEYDOWN )
      return true;

   WORD vkey = (WORD)msg->wParam;
   if ( vkey == VK_DELETE )
      return false;

   // Cut, copy, paste...
   if ( wxIsCtrlDown() && ( vkey == 'C' || vkey == 'X' || vkey == 'V' ) )
      return false;

   return true;
}

void ListCtrlEdit::OnChar( wxKeyEvent& Event )
{
    switch ( Event.m_keyCode )
    {
        case WXK_RETURN:
			Finish( true );
            break;

		case WXK_ESCAPE:
            Finish();
            break;

        default:
            Event.Skip();
    }
}

void ListCtrlEdit::OnKeyUp( wxKeyEvent& event )
{
	if ( !m_Finished ) {

        /*
		// auto-grow the textctrl:
        wxSize parentSize = m_owner->GetSize();
        wxPoint myPos = GetPosition();
        wxSize mySize = GetSize();
        int sx, sy;
        GetTextExtent(GetValue() + _T("M"), &sx, &sy);
        if (myPos.x + sx > parentSize.x)
            sx = parentSize.x - myPos.x;
        if (mySize.x > sx)
            sx = mySize.x;
        SetSize(sx, wxDefaultCoord);
		*/
    }

	event.Skip();
}

void ListCtrlEdit::OnKillFocus( wxFocusEvent& event )
{
	Finish( true );

	// We must let the native text control handle focus, too, otherwise
    // it could have problems with the cursor (e.g., in wxGTK):
    event.Skip();
}

void ListCtrlEdit::Finish( bool SendValue )
{
	if ( m_Finished ) 
      return;

	if ( SendValue && m_Owner ) 
		m_Owner->_OnVarEdit( m_Row, m_Col, GetValue() );

	m_Finished = true;
	m_Owner->SetFocus();
	m_Owner = NULL;
	Show( false );
}


WatchCtrl::WatchCtrl( wxWindow* parent, bool EditableRow )
	:  wxPanel(),
      m_ShowMetatables( false ),
    	m_EditableRow( EditableRow ),
		m_Call( NULL )
{
   Hide();
   Create( parent, -1, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER );

   m_Sizer = new wxBoxSizer( wxVERTICAL );
	m_HeaderCtrl = new HeaderCtrl( this );
	m_ListCtrl = new ListCtrl( this, m_HeaderCtrl );
	m_HeaderCtrl->SetList( m_ListCtrl );
	m_Sizer->Add( m_HeaderCtrl, 0, wxEXPAND | wxALIGN_TOP, 0 );   
	m_Sizer->Add( m_ListCtrl, 1, wxEXPAND | wxALIGN_TOP, 0 );
	SetSizer( m_Sizer );
	m_Sizer->SetSizeHints( this );

	if ( m_EditableRow ) {

		_AddVar( new WatchVar( NULL, "", 0, "", "" ), true );
		m_ListCtrl->OnChanged();

		SetDropTarget( new TextDropTarget( this ) );
	}
}

WatchCtrl::~WatchCtrl()
{
	for ( int i=0; i < m_Vars.GetCount(); ++i ) {
        
		WatchVar* Var = m_Vars[i];
		if ( !Var->GetParent() ) {
			delete Var;
		}
	}
}

BEGIN_EVENT_TABLE( WatchCtrl, wxPanel )
END_EVENT_TABLE()

void WatchCtrl::AddVar( const wxString& Name )
{
	if ( !m_EditableRow ) {
		return;
	}

	WatchVar* Var = new WatchVar( NULL, "", 0, Name, "" );
	_AddVar( Var );

	wxASSERT( tsGetDebugger() );
	if ( tsGetDebugger()->IsAtBreakpoint() ) {
		tsGetDebugger()->UpdateVar( Var );
	}

	m_ListCtrl->OnChanged();
}

void WatchCtrl::ClearAll()
{
	for ( int i=0; i < m_Vars.GetCount(); ++i ) {
        
		WatchVar* Var = m_Vars[i];

		if ( !Var->GetParent() ) {
			delete Var;
		}
	}

	m_Vars.Clear();
	m_Indent.Clear();

	// Add an editble row back in.
	if ( m_EditableRow ) {
		_AddVar( new WatchVar( NULL, "", 0, "", "" ), true );
	}

	m_ListCtrl->OnChanged();
}

void WatchCtrl::_ToggleExpanded( int Row )
{
	WatchVar* ExpandVar = m_Vars[ Row ];
	wxASSERT( ExpandVar );
	wxASSERT( ExpandVar->IsExpandable() );

	// Remove my children.
	for ( int i=Row+1; i < m_Vars.GetCount(); i++ ) {
        
		WatchVar* Var = m_Vars[ i ];
		wxASSERT( Var );
		if ( Var->IsAncestor( ExpandVar ) ) {
            
			m_Vars.RemoveAt( i );
			m_Indent.erase( m_Indent.begin() + i );
			--i;
            continue;                                                                                    
		}

		break;
	}

	// Toggle the expansion.
	ExpandVar->SetExpanded( !ExpandVar->IsExpanded() );

	wxASSERT( tsGetDebugger() );
	if (	!ExpandVar->IsExpanded() ||
			!tsGetDebugger()->IsAtBreakpoint() ) {

		m_ListCtrl->OnChanged();
		return;
	}

	tsGetDebugger()->UpdateExpanded( ExpandVar );

	// Insert the expand the children in place.
	_InsertVars( Row + 1, ExpandVar->GetVars() );

	m_ListCtrl->OnChanged();
}

void WatchCtrl::UpdateWatch()
{
	if ( !tsGetDebugger()->IsAtBreakpoint() ) {

		/*
		for ( int i=0; i < m_Vars.GetCount(); ++i ) {
			WatchVar* Var = m_Vars[i];
			Var->SetValue( "<error: not found!>" );
			Var->SetType( "" );
		}
		*/

		return;
	}

	for ( int i=0; i < m_Vars.GetCount(); ++i ) {
        
		WatchVar* Var = m_Vars[i];

		if ( !Var->GetParent() && !Var->GetName().IsEmpty() ) {

         tsGetDebugger()->UpdateVar( Var );

			if ( Var->IsExpanded() ) {
            tsGetDebugger()->UpdateExpanded( Var );
			}
		}
	}

	wxASSERT( m_ListCtrl );
	m_ListCtrl->Refresh();
}

void WatchCtrl::SetFunctionCall( FunctionCall* Call )
{
	wxASSERT( Call );
	m_Call = Call;

	m_Vars.Clear();
	m_Indent.Clear();

	_AddVars( m_Call->GetLocals() );

	m_ListCtrl->OnChanged();
}

void WatchCtrl::DeleteAt( int Row )
{
	if (	Row < 0 || 
			Row >= m_Vars.GetCount()-1 || // don't delete the last row!
			m_Vars[ Row ]->GetParent() ) {

		return;
	}

	WatchVar* DeleteRow = m_Vars[ Row ];
	m_Vars.RemoveAt( Row );
	m_Indent.erase( m_Indent.begin() + Row );

	// Remove all the vars below us that have a 
	// parent ( should be us or one of our siblings ).
	for ( int i=Row; i < m_Vars.GetCount(); i++ ) {

		WatchVar* Var = m_Vars[i];
		if ( !Var->GetParent() ) {
			break;
		}

		m_Vars.RemoveAt( i );
		--i;
	}

	delete DeleteRow;

	m_ListCtrl->OnChanged();
}

void WatchCtrl::_AddVars( const WatchVarArray& Vars )
{
	for ( int i=0; i < Vars.GetCount(); i++ ) {

		wxASSERT( Vars[i] );
		_AddVar( Vars[i] );

		if ( Vars[i]->IsExpanded() ) {
			_AddVars( Vars[i]->GetVars() );
		}
	}
}

void WatchCtrl::_AddVar( WatchVar* Var, bool EditableVar  )
{
	wxASSERT( Var );

	// Skip metatables if requested.
	if ( Var->GetName() == "(metatable)" && !m_ShowMetatables ) {
		 return;
	}

	if ( !EditableVar && m_EditableRow ) {
		
		wxASSERT( m_Vars.GetCount() >= 1 );
		wxASSERT( !m_Vars[ m_Vars.GetCount() - 1 ]->GetParent() );
		wxASSERT( m_Vars[ m_Vars.GetCount() - 1 ]->GetName().IsEmpty() );

		m_Vars.Insert( Var, m_Vars.GetCount() - 1 );

	} else {

		m_Vars.Add( Var );
	}

	m_Indent.push_back( Var->GetAncestorDepth() );
}

int WatchCtrl::_InsertVars( int Row, const WatchVarArray& Vars )
{
	const int Start = Row;

	for ( int i=0; i < Vars.GetCount(); Row++, i++ ) {

		wxASSERT( Vars[i] );

		// Skip metatables if requested.
		if ( Vars[i]->GetName() == "(metatable)" && !m_ShowMetatables ) {

			--Row;
			continue;
		}

        m_Vars.Insert( Vars[i], Row );
        m_Indent.insert( m_Indent.begin() + Row, Vars[i]->GetAncestorDepth() );

		if ( Vars[i]->IsExpanded() ) {

			Row += _InsertVars( Row + 1, Vars[i]->GetVars() );
		}
	}

	return Row - Start;
}
