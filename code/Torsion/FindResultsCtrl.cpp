// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "FindResultsCtrl.h"

#include "TorsionApp.h"
#include "MainFrame.h"
#include "FindThread.h"
#include "tsMenu.h"

#include "Icons.h"

#include <wx/clipbrd.h>
#include <wx/regex.h>

#include "LexTCS.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( FindResultsCtrl, wxStyledTextCtrl )
   
   EVT_LEFT_DCLICK( OnDblClick )
   EVT_CONTEXT_MENU( OnContextMenu )  
   
   EVT_MENU( tsID_GOTOFILE, OnGotoFile )
   EVT_UPDATE_UI( tsID_GOTOFILE, OnUpdateGotoFile )
   EVT_MENU( tsID_CLEARALL, OnClearAll )
   EVT_UPDATE_UI( tsID_CLEARALL, OnUpdateClearAll )
   EVT_MENU( wxID_COPY, OnCopyLines )
   EVT_UPDATE_UI( wxID_COPY, OnUpdateCopyLines )
   EVT_MENU( tsID_FINDCANCEL, OnStopFind )
   EVT_UPDATE_UI( tsID_FINDCANCEL, OnUpdateStopFind )

END_EVENT_TABLE()


FindResultsCtrl::FindResultsCtrl( wxWindow* parent, wxWindowID id, FindThread* findThread )
   :  wxStyledTextCtrl(),
      m_FindThread( findThread ),
      m_Selected( -1 )
{
   wxASSERT( m_FindThread );

   Hide();

   wxStyledTextCtrl::Create( parent, id, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxSTATIC_BORDER );

   UsePopUp(false);

   // initialize styles
   StyleClearAll();

   // Set the lexer language
   SetLexer( wxSTC_LEX_TEX );

   // Clear the zoom keys and mouse mapping.
   CmdKeyClear( wxSTC_KEY_ADD, wxSTC_SCMOD_CTRL );
   CmdKeyClear( wxSTC_KEY_SUBTRACT, wxSTC_SCMOD_CTRL );
   CmdKeyClear( wxSTC_KEY_DIVIDE, wxSTC_SCMOD_CTRL );

   StyleClearAll();

   // TODO: Make the font style a configuration option or use some OS default!
   wxFont* font = wxTheFontList->FindOrCreateFont( 8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Courier New" );
   for ( int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; i++ )
      StyleSetFont( i, *font );

   StyleSetForeground( 1, wxColour( 255, 255, 0 ) );
   StyleSetBackground( 1, wxColour( 0, 0, 128 ) );
   //StyleSetBackground( 2, wxColour( 255, 255, 255 ) );
   //StyleSetForeground( 2, wxColour( 255, 0, 0 ) );
 
   /*
   StyleSetBackground( wxSTC_STYLE_DEFAULT, tsGetPrefs().GetBgColor() );
   SetCaretForeground( tsGetPrefs().GetDefaultColor() );

	StyleSetForeground( wxSTC_TCS_DEFAULT, tsGetPrefs().GetDefaultColor() );
	StyleSetBackground( wxSTC_TCS_DEFAULT, tsGetPrefs().GetBgColor() );

   SetSelBackground( 1, tsGetPrefs().GetSelBgColor() );
   SetSelForeground( 1, tsGetPrefs().GetSelColor() );
   */

   IndicatorSetForeground( 0, wxColour( 255, 0, 0 ) );

   //SetTabWidth( tsGetPrefs().GetTabWidth() );

   // setup the caret.
   SetCaretWidth( 2 );
   SetVisiblePolicy( wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1 );
   SetXCaretPolicy( wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1 );
   SetYCaretPolicy( wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1 );

   // Set the margin.
   SetMarginLeft( 0 );
   SetMarginType( 0, wxSTC_MARGIN_SYMBOL );
   SetMarginWidth( 0, 12 );
   SetMarginWidth( 1, 0 );
   SetMarginWidth( 2, 0 );
   SetMarginSensitive( 0, false );
   //StyleSetBackground( wxSTC_STYLE_LINENUMBER, tsGetPrefs().GetBgColor() );

   SetWrapMode( wxSTC_WRAP_NONE );

   SetLayoutCache( wxSTC_CACHE_PAGE );

   SetReadOnly( true );

   UpdatePrefs();
}

FindResultsCtrl::~FindResultsCtrl()
{
}

void FindResultsCtrl::UpdatePrefs()
{
   StyleSetBackground( wxSTC_STYLE_DEFAULT, tsGetPrefs().GetBgColor() );
   SetCaretForeground( tsGetPrefs().GetDefaultColor() );

	StyleSetForeground( wxSTC_TCS_DEFAULT, tsGetPrefs().GetDefaultColor() );
	StyleSetBackground( wxSTC_TCS_DEFAULT, tsGetPrefs().GetBgColor() );

   SetSelBackground( 1, tsGetPrefs().GetSelBgColor() );
   SetSelForeground( 1, tsGetPrefs().GetSelColor() );

   SetTabWidth( tsGetPrefs().GetTabWidth() );

   StyleSetBackground( wxSTC_STYLE_LINENUMBER, tsGetPrefs().GetBgColor() );
}

void FindResultsCtrl::AppendText( const wxString& text )
{
   SetReadOnly( false );

   bool scrollEnd = false;
   const int pos = GetLength();
   if (  GetCurrentPos() == pos && 
         GetAnchor() == pos )
      scrollEnd = true;

   SetTargetStart( pos );
   SetTargetEnd( pos );
   ReplaceTarget( text );

   if ( scrollEnd )
   {
      const int endPos = GetLength();
      SetAnchor( endPos );
      SetCurrentPos( endPos );
      ShowLine( LineFromPosition( endPos ) );
   }

   EmptyUndoBuffer();

   SetReadOnly( true );
}

void FindResultsCtrl::Clear()
{
   SetReadOnly( false );
   ClearAll();
   SetReadOnly( true );
   m_Selected = -1;
}

void FindResultsCtrl::OnDblClick( wxMouseEvent& event )
{
   int pos = PositionFromPoint( event.GetPosition() );
   int line = LineFromPosition( pos );
   ActivateLine( line, true );
}

void FindResultsCtrl::ActivateLine( int line, bool openFile )
{
   // Get the file name and line.
   wxString File;
   long Line;
   if ( line != -1 ) 
   {
      // TODO: This possibly needs fixing for non Win32 platforms.
      wxString fmt;
      fmt << "([A-Z][:][^" << wxFileName::GetForbiddenChars() << "]*)[(]([0-9]*)[):]";
      wxRegEx expr( fmt, wxRE_ADVANCED | wxRE_ICASE );
      wxASSERT( expr.IsValid() );
      wxString Text = GetLine( line );
      if ( expr.Matches( Text ) && expr.GetMatchCount() > 2 ) 
      {
         File = expr.GetMatch( Text, 1 );
         expr.GetMatch( Text, 2 ).ToLong( &Line );
      }
   }

   if ( m_Selected != -1 ) 
   {
      int start = PositionFromLine( m_Selected );
      int len = GetLineEndPosition( m_Selected ) - start;

      // Reverse the style!
      StartStyling( start, 0xFF );
      SetStyling( len, 0 );
   }

   m_Selected = line;
   if ( File.IsEmpty() )
      m_Selected = -1;

   if ( m_Selected != -1 ) 
   {
      int start = PositionFromLine( m_Selected );
      int len = GetLineEndPosition( m_Selected ) - start;

      // Reverse the style!
      StartStyling( start, 0xFF );
      SetStyling( len, 1 );
   }

   // Ok read the file name from the list.
   if ( openFile && !File.IsEmpty() ) 
   {
      // TODO: Should we launch 3rd party apps here or allow the 
      // file to be opened as text?
      wxASSERT( tsGetMainFrame() );
      tsGetMainFrame()->OpenFile( File, Line-1 );
   }
}

void FindResultsCtrl::ShowLine( int line )
{
   // First make sure the line is not folded.
   EnsureVisible( line );

   line = VisibleFromDocLine( line );

   int firstVisible = GetFirstVisibleLine();
   int lastVisible = firstVisible + ( LinesOnScreen() - 1 );
   if ( line <= firstVisible ) {

      int lines = line - firstVisible;
      if ( lines < -1 ) {
         lines -= LinesOnScreen() / 2;
      }
      LineScroll( 0, lines );

   } else if ( line >= lastVisible ) {

      int lines = line - lastVisible;
      if ( lines > 1 ) {
         lines += LinesOnScreen() / 2;
      }
      LineScroll( 0, lines );
   }
}


void FindResultsCtrl::OnContextMenu( wxContextMenuEvent& event )
{
   int pos = PositionFromPoint( event.GetPosition() );
   int line = LineFromPosition( pos );
   ActivateLine( line, false );

   tsMenu* menu = new tsMenu;

   menu->Append( tsID_GOTOFILE, _T( "&Go To File" ) );
   menu->AppendSeparator();
   menu->AppendIconItem( wxID_COPY, _T( "&Copy" ), ts_copy_clipboard16 );
   menu->Append( tsID_CLEARALL, _T( "Clear &All" ) );
   menu->AppendSeparator();
   menu->Append( tsID_FINDCANCEL, _T( "&Stop Find" ) );

   PopupMenu( menu );
   delete menu;
}

void FindResultsCtrl::OnGotoFile( wxCommandEvent& event )
{
   ActivateLine( m_Selected, true );
}

void FindResultsCtrl::OnUpdateGotoFile( wxUpdateUIEvent& event )
{
   event.Enable( m_Selected != -1 );
}

void FindResultsCtrl::OnClearAll( wxCommandEvent& event )
{
   Clear();
}

void FindResultsCtrl::OnUpdateClearAll( wxUpdateUIEvent& event )
{
   event.Enable( GetLength() > 0 && !m_FindThread->IsSearching() );
}

void FindResultsCtrl::OnCopyLines( wxCommandEvent& event )
{
   if ( GetCurrentPos() != GetAnchor() )
      Copy();

   else if ( wxTheClipboard->Open() ) 
   {
      wxASSERT( m_Selected != -1 );
      wxTheClipboard->SetData( new wxTextDataObject( GetLine( m_Selected ) ) );
      wxTheClipboard->Close();
   } 
}

void FindResultsCtrl::OnUpdateCopyLines( wxUpdateUIEvent& event )
{
   event.Enable( GetCurrentPos() != GetAnchor() || m_Selected != -1 );
}

void FindResultsCtrl::OnStopFind( wxCommandEvent& event )
{
   m_FindThread->Cancel();
}

void FindResultsCtrl::OnUpdateStopFind( wxUpdateUIEvent& event )
{
   event.Enable( m_FindThread->IsSearching() );
}


