// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "OutputCtrl.h"

#include "MainFrame.h"
#include "TorsionApp.h"
#include "Debugger.h"
#include "tsMenu.h"

#include "Icons.h"

#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>
#include <algorithm>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BEGIN_EVENT_TABLE( OutputCtrl, wxStyledTextCtrl )

   EVT_LEFT_DCLICK( OnDblClick )
   EVT_CONTEXT_MENU( OnContextMenu )  

   EVT_MENU( tsID_OUTPUT_GOTOFILE, OnGotoFile )
   EVT_UPDATE_UI( tsID_OUTPUT_GOTOFILE, OnUpdateGotoFile )
   EVT_MENU( wxID_COPY, OnCopy )
   EVT_UPDATE_UI( wxID_COPY, OnUpdateCopy )

END_EVENT_TABLE()

OutputCtrl::OutputCtrl( wxWindow* parent, wxWindowID id, const wxString& name,
                        const wxPoint& pos, const wxSize& size, long style )
   :  wxStyledTextCtrl(),
      m_Selected( -1 )
{
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

   StyleSetBackground( 1, wxColour( 255, 0, 0 ) );
   StyleSetForeground( 1, wxColour( 255, 255, 255 ) );
   StyleSetBackground( 2, wxColour( 255, 255, 255 ) );
   StyleSetForeground( 2, wxColour( 255, 0, 0 ) );
 
   /*
   StyleSetBackground( wxSTC_STYLE_DEFAULT, tsGetPrefs().GetBgColor() );
   SetCaretForeground( tsGetPrefs().GetDefaultColor() );

	StyleSetForeground( wxSTC_TCL_DEFAULT, tsGetPrefs().GetDefaultColor() );
	StyleSetBackground( wxSTC_TCL_DEFAULT, tsGetPrefs().GetBgColor() );

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

   // Create the regular expression here once.
   // TODO: This possibly needs fixing for non Win32 platforms.
   m_ErrorExpr.Compile( "([^*?:<>|]*)[ \t]Line:[ \t]([0-9]*)[ \t]-[ \t](.*)",
                        wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( m_ErrorExpr.IsValid() );

   m_WarnExpr.Compile( "([^*?:<>|]*)[ \t][(]([0-9]*)[)]:[ \t](.*)",
                        wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( m_WarnExpr.IsValid() );
}

OutputCtrl::~OutputCtrl()
{
   Clear();
}

void OutputCtrl::UpdatePrefs()
{
   StyleSetBackground( wxSTC_STYLE_DEFAULT, tsGetPrefs().GetBgColor() );
   SetCaretForeground( tsGetPrefs().GetDefaultColor() );

	StyleSetForeground( wxSTC_TCL_DEFAULT, tsGetPrefs().GetDefaultColor() );
	StyleSetBackground( wxSTC_TCL_DEFAULT, tsGetPrefs().GetBgColor() );

   SetSelBackground( 1, tsGetPrefs().GetSelBgColor() );
   SetSelForeground( 1, tsGetPrefs().GetSelColor() );

   SetTabWidth( tsGetPrefs().GetTabWidth() );
   StyleSetBackground( wxSTC_STYLE_LINENUMBER, tsGetPrefs().GetBgColor() );
}

void OutputCtrl::AppendText( const wxString& text )
{
   SetReadOnly( false );

   bool scrollEnd = false;
   int pos = GetLength();
   if (  GetCurrentPos() == pos && 
         GetAnchor() == pos )
      scrollEnd = true;

   // For each line...
   const wxChar* ptr = text.c_str();
   const wxChar* const end = ptr + text.Length();
   const wxChar* next;

   wxString line;
   bool isError = false;
   bool isWarn = false;

   //wxTextAttr errorStyle( *wxRED, *wxWHITE ); 
   //wxTextAttr warnStyle( *wxRED, *wxWHITE ); //wxColour( 255, 128, 0 ), *wxWHITE ); 
   long lnumb;

   while( ptr != end )
   {
      wxASSERT( ptr < end );
      next = std::find( ptr, end, '\n' );
      if ( next != end ) ++next;
      line.assign( ptr, next );
      ptr = next;

      // Look for error lines and highlight them...
      //
      // TODO: I need to optimize the regex here... maybe i 
      // shouldn't use a regex, but use my own logic to spot
      // errors... could be much faster.
      //
      if ( m_ErrorExpr.Matches( line ) && m_ErrorExpr.GetMatchCount() > 3 )
         isError = true;
      else if ( m_WarnExpr.Matches( line ) && m_ErrorExpr.GetMatchCount() > 3 )
         isWarn = true;

      pos = GetLength();
      SetTargetStart( pos );
      SetTargetEnd( pos );
      ReplaceTarget( line );

      if ( isError ) 
      {
         StartStyling( pos, 0xFF );
         SetStyling( line.Len(), 2 );

         // TODO: SetStyle will screw with the current scroll position.  The
         // trick is to disable ECO_AUTOVSCROLL and ECO_AUTOHSCROLL before
         // changing the selection to change the style.  We need to submit this
         // fix back to wxWindows.
         //::SendMessage( GetHWND(), EM_SETOPTIONS, ECOOP_XOR, ECO_AUTOVSCROLL | ECO_AUTOHSCROLL );
         //SetStyle( start, last, errorStyle );
         //::SendMessage( GetHWND(), EM_SETOPTIONS, ECOOP_OR, ECO_AUTOVSCROLL | ECO_AUTOHSCROLL );

         // Add the error to the debugger state.
         m_ErrorExpr.GetMatch( line, 2 ).ToLong( &lnumb );

         ScriptError* error = new ScriptError;
         error->file = m_ErrorExpr.GetMatch( line, 1 );
         error->line = lnumb;
         error->start = pos;
         error->end = pos + line.Len();
         error->row = LineFromPosition( pos );
         error->error = m_ErrorExpr.GetMatch( line, 3 );
         error->error.Trim();
         error->warning = false;
         AddError( error );

         isError = false;
      } 
      else if ( isWarn ) 
      {
         StartStyling( pos, 0xFF );
         SetStyling( line.Len(), 2 );

         // TODO: SetStyle will screw with the current scroll position.  The
         // trick is to disable ECO_AUTOVSCROLL and ECO_AUTOHSCROLL before
         // changing the selection to change the style.  We need to submit this
         // fix back to wxWindows.
         //::SendMessage( GetHWND(), EM_SETOPTIONS, ECOOP_XOR, ECO_AUTOVSCROLL | ECO_AUTOHSCROLL );
         //SetStyle( start, last, warnStyle );
         //::SendMessage( GetHWND(), EM_SETOPTIONS, ECOOP_OR, ECO_AUTOVSCROLL | ECO_AUTOHSCROLL );

         // Add the error to the debugger state.
         m_WarnExpr.GetMatch( line, 2 ).ToLong( &lnumb );

         ScriptError* error = new ScriptError;
         error->file = m_WarnExpr.GetMatch( line, 1 );
         error->line = lnumb;
         error->start = pos;
         error->end = pos + line.Len();
         error->row = LineFromPosition( pos );
         error->error = m_WarnExpr.GetMatch( line, 3 );
         error->error.Trim();
         error->warning = true;
         AddError( error );

         isWarn = false;
      }
   }

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

void OutputCtrl::Clear()
{
   SetReadOnly( false );
   ClearAll();
   SetReadOnly( true );
   m_Selected = -1;

   ClearErrors();
}

void OutputCtrl::OnContextMenu( wxContextMenuEvent& event )
{ 
   tsMenu* menu = new tsMenu;

   menu->AppendIconItem( wxID_COPY, _T( "&Copy" ), ts_copy_clipboard16 );
   menu->AppendSeparator();
   menu->Append( tsID_OUTPUT_CLEAR, _T( "Clear" ) );
   menu->AppendSeparator();
   menu->Append( tsID_OUTPUT_GOTOFILE, _T( "&Go To File" ) );

   PopupMenu( menu );
   delete menu;

   //wxSetCursor( lastCursor );
}

void OutputCtrl::OnGotoFile( wxCommandEvent& event )
{
   ActivateLine( m_Selected, true );
}

void OutputCtrl::OnUpdateGotoFile( wxUpdateUIEvent& event )
{
   event.Enable( m_Selected != -1 );
}

void OutputCtrl::OnCopy( wxCommandEvent& event )
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

void OutputCtrl::OnUpdateCopy( wxUpdateUIEvent& event )
{
   event.Enable( GetCurrentPos() != GetAnchor() || m_Selected != -1 );
}

void OutputCtrl::OnDblClick( wxMouseEvent& event )
{
   int pos = PositionFromPoint( event.GetPosition() );
   int line = LineFromPosition( pos );
   ActivateLine( line, true );
}

void OutputCtrl::ActivateLine( int line, bool openFile )
{
   // Get the file name and line.
   bool isError = false;
   wxString File;
   long Line;
   if ( line != -1 ) 
   {
      wxString Text = GetLine( line );
      if ( m_ErrorExpr.Matches( Text ) && m_ErrorExpr.GetMatchCount() > 3 ) 
      {
         isError = true;
         File = m_ErrorExpr.GetMatch( Text, 1 );
         m_ErrorExpr.GetMatch( Text, 2 ).ToLong( &Line );
      } 
      else if ( m_WarnExpr.Matches( Text ) && m_WarnExpr.GetMatchCount() > 3 ) 
      {
         isError = true;
         File = m_WarnExpr.GetMatch( Text, 1 );
         m_WarnExpr.GetMatch( Text, 2 ).ToLong( &Line );
      }
   }

   if ( m_Selected != -1 ) 
   {
      int start = PositionFromLine( m_Selected );
      int len = GetLineEndPosition( m_Selected ) - start;

      // Reverse the style!
      StartStyling( start, 0xFF );
      SetStyling( len, 2 );
   }

   m_Selected = line;
   if ( !isError )
      m_Selected = -1;

   if ( m_Selected != -1 ) 
   {
      int start = PositionFromLine( m_Selected );
      int len = GetLineEndPosition( m_Selected ) - start;

      // Reverse the style!
      StartStyling( start, 0xFF );
      SetStyling( len, 1 );
   }

   if ( m_Selected != -1 )
      ShowLine( m_Selected );

   // Ok read the file name from the list.
   if ( openFile && !File.IsEmpty() ) 
   {
      // TODO: Should we launch 3rd party apps here or allow the 
      // file to be opened as text?
      wxASSERT( tsGetMainFrame() );
      tsGetMainFrame()->OpenFile( File, Line-1 );
   }
}

void OutputCtrl::AddError( ScriptError* error )
{
   wxASSERT( error );
   m_Errors.push_back( error );
}

void OutputCtrl::ClearErrors()
{
   for ( int i=0; i < m_Errors.size(); i++ )
      delete m_Errors[i];
   m_Errors.clear();
}

void OutputCtrl::ShowLine( int line )
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



