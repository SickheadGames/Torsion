// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScriptCtrl.h"

#include "TorsionApp.h"
#include "MainFrame.h"
#include "ScriptView.h"
#include "ProjectDoc.h"
#include "Debugger.h"
#include "AutoComp.h"
#include "AutoCompText.h"
#include "BreakpointPropertyDlg.h"
#include "WatchCtrl.h"
#include "Platform.h"
#include "GotoDefDialog.h"
#include "tsMenu.h"

#include "Icons.h"

#include <wx/file.h>
#include <wx/fdrepdlg.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>

#include "LexTCS.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

extern wxMenu *wxCurrentPopupMenu;

// TODO: fooopsSß is a valid C++ identifier... is it valid TorqueScript?
// Investigate and add latin characters to Torsion.

const wxString ScriptCtrl::sm_WordChars(  "abcdefghijklmnopqrstuvwxyz"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "0123456789"
                                          "_$%"
                                          "" );


ScriptCtrl::ScriptCtrl()
	:	wxStyledTextCtrl(),
      m_ScriptView( NULL ),
      m_LastModify( false ),
      m_Loading( false ),
      m_IsGlobalFullups( false )
{
}

ScriptCtrl::~ScriptCtrl()
{
   WX_CLEAR_ARRAY( m_ScriptEvents );
}

// TODO: Look at EVT_STC_SAVEPOINTLEFT and EVT_STC_SAVEPOINTREACHED to 
// set and remove the dirty document *.

BEGIN_EVENT_TABLE( ScriptCtrl, wxStyledTextCtrl )

   EVT_KEY_DOWN( ScriptCtrl::OnKeyDown )
   EVT_KEY_UP( ScriptCtrl::OnKeyUp )

   EVT_CONTEXT_MENU( ScriptCtrl::OnContextMenu )

   EVT_STC_MARGINCLICK( wxID_ANY, ScriptCtrl::OnMarginClick )
   EVT_STC_NEEDSHOWN( wxID_ANY, ScriptCtrl::OnNeedUnfold )

   EVT_STC_DWELLSTART( wxID_ANY, ScriptCtrl::OnHoverStart )
   EVT_STC_DWELLEND( wxID_ANY, ScriptCtrl::OnHoverEnd )

   //EVT_STC_PAINTED( wxID_ANY, OnPaintDone )

   EVT_STC_MODIFIED( wxID_ANY, ScriptCtrl::OnTextChanged )
   EVT_STC_UPDATEUI( wxID_ANY, ScriptCtrl::OnUpdateUI )

   EVT_CHAR( ScriptCtrl::OnChar )

   //EVT_STC_USERLISTSELECTION( wxID_ANY, ScriptCtrl::OnAutoCompSel )

   EVT_MENU( tsID_SCRIPT_OPENEXEC, ScriptCtrl::OnOpenExec )
   EVT_MENU( tsID_SCRIPT_OPENDEF, ScriptCtrl::OnOpenDef )
   EVT_MENU( tsID_ADD_WATCH, ScriptCtrl::OnAddWatch )
   EVT_MENU( tsID_SCRIPT_REMOVEBREAKPOINT, ScriptCtrl::OnRemoveBreakpoint )
   EVT_MENU( tsID_SCRIPT_TOGGLEBREAKPOINT, ScriptCtrl::OnToggleBreakpoint )
   EVT_MENU( tsID_SCRIPT_INSERTBREAKPOINT, ScriptCtrl::OnInsertBreakpoint )
   EVT_MENU( tsID_BREAKPOINT_PROPERTIES, ScriptCtrl::OnBreakpointProperties )

   EVT_MENU( wxID_PROPERTIES, ScriptCtrl::OnProperties )

   EVT_FIND(wxID_ANY, ScriptCtrl::OnFindNext)
   EVT_FIND_NEXT(wxID_ANY, ScriptCtrl::OnFindNext)
   EVT_FIND_REPLACE(wxID_ANY, ScriptCtrl::OnFindReplace)
   EVT_FIND_REPLACE_ALL(wxID_ANY, ScriptCtrl::OnFindReplaceAll)

   EVT_MENU( tsID_GOTO, ScriptCtrl::OnGoto )

   EVT_UPDATE_UI( wxID_FIND, ScriptCtrl::OnUpdateEnable)
   EVT_UPDATE_UI( tsID_FINDNEXT, ScriptCtrl::OnUpdateEnable)
   EVT_UPDATE_UI( tsID_FINDPREV, ScriptCtrl::OnUpdateEnable)
   EVT_UPDATE_UI( tsID_GOTO, ScriptCtrl::OnUpdateEnable )

   EVT_SET_FOCUS( ScriptCtrl::OnSetFocus )
   EVT_KILL_FOCUS( ScriptCtrl::OnKillFocus )

   EVT_MOUSEWHEEL( ScriptCtrl::OnMouseWheel )

   EVT_LEFT_DOWN( ScriptCtrl::OnMouseLeftDown )
   //EVT_LEFT_UP( ScriptCtrl::OnMouseLeftUp )

   EVT_BREAKPOINT_EVENTS( ScriptCtrl::OnBreakpointEvent )

	EVT_MENU_RANGE( tsID_FOLDING_FIRST, tsID_FOLDING_LAST, ScriptCtrl::OnFolding )
	EVT_UPDATE_UI_RANGE( tsID_FOLDING_FIRST, tsID_FOLDING_LAST, ScriptCtrl::OnFoldingUpdateUI )

   EVT_MENU( tsID_LIST_MEMBERS, ScriptCtrl::OnListMembers )
   EVT_MENU( tsID_PARAMETER_INFO, ScriptCtrl::OnParameterInfo )
	EVT_UPDATE_UI_RANGE( tsID_SCRIPTSENSE_FIRST, tsID_SCRIPTSENSE_LAST, ScriptCtrl::OnUpdateEnable )
   
   EVT_TIMER( wxID_ANY, ScriptCtrl::OnUpdateAutoCompText )

END_EVENT_TABLE()


bool ScriptCtrl::Create( wxWindow* parent, ScriptView* view, const wxPoint& pos, const wxSize& size )
{
   wxASSERT( parent );
   wxASSERT( view );

   m_ScriptView = view;
   m_LastModify = false;
   m_Loading = false;
   m_StartFindPos = -1;
   m_LastFindPos = -1;

   wxStyledTextCtrl::Create( parent, wxID_ANY, pos, size, wxVSCROLL | wxSTATIC_BORDER );

   UsePopUp(false);
   InitializePrefs();
   SetLayoutCache( wxSTC_CACHE_PAGE );

   m_AutoCompTimer.SetOwner( this, wxID_ANY );

   return true;
}

/*
void ScriptCtrl::UpdateWindowUI( long flags )
{
   SetViewEOL( s_LineBreaks );
   SetViewWhiteSpace( s_TabsAndSpaces ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE );

   wxStyledTextCtrl::UpdateWindowUI( flags );
}
*/

bool ScriptCtrl::LoadFile( const wxString& filename )
{
   SetReadOnly( false );
   
   wxPoint scroll( GetXOffset(), GetFirstVisibleLine() );

   // Don't notify the view while loading.
   m_Loading = true;
   if ( !wxStyledTextCtrl::LoadFile( filename ) ) {
      return false;
   }
   m_Loading = false;

   // Add an active autocomp text buffer for dynamic
   // autocomp generation while editing.
   AutoCompText* TextBuffer = GetDocument()->GetTextBuffer();
   if ( TextBuffer )
   {
      TextBuffer->Lock();
      GetTextRangeRaw( wxAtoi(TextBuffer->GetWriteBuf( GetTextLength() )), 0 );
      TextBuffer->Unlock();
   }

   m_FilePath = filename;
   m_LastModify = false;
   WX_CLEAR_ARRAY( m_ScriptEvents );

   LineScroll( 0, scroll.y );
   SetXOffset( scroll.x );

   m_StartFindPos = -1;
   m_LastFindPos = -1;

   // If the file is read-only then don't allow editing.
   if ( !wxFile::Access( filename, wxFile::read_write ) ) {
      SetReadOnly( true );
   }

   UpdateBookmarks();
   UpdateBreakpoints();

   return true;
}

void ScriptCtrl::UpdateBookmarks()
{
   if ( m_Loading )
      return;

   // We get the bookmarks from the project.
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project )
      return;

   MarkerDeleteAll( MARKER_BOOKMARK );

   BookmarkArray marks;
   project->GetBookmarks( m_FilePath, marks );
   for ( int b=0; b < marks.GetCount(); b++ ) 
   {
      wxASSERT( marks[b] );

      const int line = marks[b]->GetLine();
      wxASSERT( line >= 0 );

      MarkerAdd( line, MARKER_BOOKMARK );
   }
}

void ScriptCtrl::UpdateBreakpoints()
{
   if ( m_Loading )
      return;

   MarkerDeleteAll( MARKER_BREAKPOINT );
   MarkerDeleteAll( MARKER_CONDITIONAL_BREAKPOINT );
   MarkerDeleteAll( MARKER_BREAKPOINT_DISABLED );

   // Load the breakpoints if we got them.
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project ) {
      return;
   }

   BreakpointArray breaks;
   project->GetBreakpoints( m_FilePath, breaks );
   for ( int b=0; b < breaks.GetCount(); b++ ) {

      wxASSERT( breaks[b] );

      int line = breaks[b]->GetLine() - 1;
      wxASSERT( line >= 0 );

      if ( breaks[b]->GetEnabled() ) {

         if ( breaks[b]->HasCondition() )
            MarkerAdd( line, MARKER_CONDITIONAL_BREAKPOINT );
         else
            MarkerAdd( line, MARKER_BREAKPOINT );

	   } else {
         MarkerAdd( line, MARKER_BREAKPOINT_DISABLED );
	   }
   }
}

bool ScriptCtrl::SaveFile( const wxString& filename )
{
   if ( !wxStyledTextCtrl::SaveFile( filename ) ) {
      return false;
   }

   // TODO: This sucks... i'm only doing this because there
   // is not a good way to update the filename for an autocomp
   // buffer.  I need to refactor all this.
   AutoCompText* TextBuffer = GetDocument()->GetTextBuffer();
   if ( TextBuffer )
   {
      TextBuffer->Lock();
      GetTextRangeRaw( wxAtoi(TextBuffer->GetWriteBuf( GetTextLength() )), 0 );
      TextBuffer->Unlock();
   }

   m_FilePath = filename;
   m_LastModify = false;
   WX_CLEAR_ARRAY( m_ScriptEvents );

   return true;
}

void ScriptCtrl::SetStyle( int style, const wxColour& fore, const wxColour& back )
{
	StyleSetForeground( style, fore );
	StyleSetBackground( style, back );
}

bool ScriptCtrl::InitializePrefs() 
{
   // Set the lexer language
   SetLexer( wxSTC_LEX_TCS );

   // Clear the zoom keys and mouse mapping.
   CmdKeyClear( wxSTC_KEY_ADD, wxSTC_SCMOD_CTRL );
   CmdKeyClear( wxSTC_KEY_SUBTRACT, wxSTC_SCMOD_CTRL );
   CmdKeyClear( wxSTC_KEY_DIVIDE, wxSTC_SCMOD_CTRL );
   CmdKeyClear( '[', wxSTC_SCMOD_CTRL );
   CmdKeyClear( ']', wxSTC_SCMOD_CTRL );

   // Assign some special command keys.
   CmdKeyAssign( wxSTC_KEY_PRIOR, wxSTC_SCMOD_CTRL, wxSTC_CMD_STUTTEREDPAGEUP);
   CmdKeyAssign( wxSTC_KEY_NEXT, wxSTC_SCMOD_CTRL, wxSTC_CMD_STUTTEREDPAGEDOWN);

   // TODO: We need to support unicode directly and to
   // do so we need to compile with STC unicode support
   // and maybe compile Torsion with unicode on... yuck.
   //SetCodePage( wxSTC_CP_UTF8 );

   UpdatePrefs( false );

   // setup the caret.
   SetCaretWidth( 2 );
   SetVisiblePolicy( wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1 );
   SetXCaretPolicy( wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1 );
   SetYCaretPolicy( wxSTC_CARET_EVEN, 0 );

   SetWordChars( sm_WordChars );

   // Set the breakpoint margin.
   SetMarginType( MARGIN_MARKERS, wxSTC_MARGIN_SYMBOL );
   SetMarginWidth( MARGIN_MARKERS, 16 );
   SetMarginSensitive( MARGIN_MARKERS, true );
   SetMarginMask( MARGIN_MARKERS,   ( 1 << MARKER_BOOKMARK ) | 
                                    ( 1 << MARKER_BREAKPOINT ) | 
                                    ( 1 << MARKER_CONDITIONAL_BREAKPOINT ) | 
                                    ( 1 << MARKER_BREAKPOINT_DISABLED ) | 
                                    ( 1 << MARKER_BREAKLINE ) );
   MarkerDefineBitmap( MARKER_BOOKMARK, ts_bookmark16 );
   MarkerDefineBitmap( MARKER_BREAKPOINT, ts_breakpoint16 );
   MarkerDefineBitmap( MARKER_CONDITIONAL_BREAKPOINT, ts_breakpointc16 );
   MarkerDefineBitmap( MARKER_BREAKPOINT_DISABLED, ts_breakpoint_disabled16 );
   MarkerDefineBitmap( MARKER_BREAKLINE, ts_currentline16 );

   // Set margin for line numbers
   SetMarginType( MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER );
   SetMarginMask( MARGIN_LINE_NUMBERS, 0 );
   ShowLineNumbers( tsGetPrefs().GetLineNumbers() );

   // Setup code folding.
   StyleSetChangeable( wxSTC_TCS_FOLDED, false );
   IndicatorSetStyle( 2, wxSTC_INDIC_BOX );
   SetProperty( "fold.comment", "1" );
   SetProperty( "fold.at.else", "1" );
   SetProperty( "fold.compact", "0" ); // NOT SUPPORTED!
   SetProperty( "fold.preprocessor", "0" ); // NOT SUPPORTED!
   
   SetFoldFlags( 0x0020 ); // 64 ... for debugging folding levels // 
   SetMarginType( MARGIN_FOLDING, wxSTC_MARGIN_SYMBOL );
   SetMarginWidth( MARGIN_FOLDING, 12 );
   SetMarginMask( MARGIN_FOLDING, wxSTC_MASK_FOLDERS );
   SetMarginSensitive( MARGIN_FOLDING, true );

   // Put a little space between the last margin and the text.
   SetMarginLeft( 2 );
   SetWrapMode( tsGetPrefs().GetLineWrap() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE );

   // Autocomplete stuff... be sure the images match
   // the idents in AutoCompData.
   AutoCompSetIgnoreCase( true );
   AutoCompSetAutoHide( true );
   AutoCompSetCancelAtStart( false );
   //AutoCompStops( " \t\r\n:.,)([]{}<>+-*/" );

   RegisterImage( 0, ts_globals16 );           // IDENT_VAR
   RegisterImage( 1, ts_functions16 );         // IDENT_FUNCTION
   RegisterImage( 2, ts_class16 );             // IDENT_CLASS
   RegisterImage( 3, ts_datablock16 );         // IDENT_DATABLOCK
   RegisterImage( 4, ts_object16 );            // IDENT_OBJECT
   RegisterImage( 5, ts_datablock_object16 );  // ITYPE_DATABLOCKOBJECT
   RegisterImage( 6, ts_namespace16 );         // IDENT_NAMESPACE
   RegisterImage( 7, ts_package16 );           // ITYPE_PACKAGE
   RegisterImage( 8, ts_keyword16 );           // ITYPE_KEYWORD

   // Set the initial state for tabs, spaces, and line breaks.
   SetViewWhiteSpace( tsGetPrefs().GetTabsAndSpaces() ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE );
   SetViewEOL( tsGetPrefs().GetLineBreaks() );

   SetModEventMask( wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT | wxSTC_PERFORMED_USER | wxSTC_PERFORMED_UNDO | wxSTC_PERFORMED_REDO );

   SetMouseDwellTime( 500 );

   return true;
}

void ScriptCtrl::OnUpdateUI( wxStyledTextEvent& event )
{
   // Update the styling on the visible folded lines.
   /*
   int sl = GetFirstVisibleLine();
   int el = sl + LinesOnScreen();
   for ( ; sl <= el; sl++ ) {

      if ( !( GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG ) )
         continue;

      int pos = PositionFromLine( sl );
      int es = GetEndStyled();
      if ( GetFoldExpanded( sl ) ) {

         StartStyling( pos, wxSTC_INDICS_MASK );
         SetStyling( LineLength( sl ), 0 );

         //Colourise( pos, pos + LineLength( sl ) );

      } else {

         StartStyling( pos, wxSTC_INDICS_MASK );
         SetStyling( LineLength( sl ), wxSTC_INDIC2_MASK );
      }

      StartStyling( es, 0xF1 );
   }
   */

   // Do auto brace matching...
   if ( GetSelectionStart() == GetSelectionEnd() ) {

      // Find the two positions.
      int pos = GetCurrentPos();
      char ch = GetCharAt( pos );
      int pos2 = wxSTC_INVALID_POSITION;
      if (  ch == '(' || ch == ')' ||
            ch == '{' || ch == '}' ||
            ch == '[' || ch == ']' ) {

         pos2 = BraceMatch( pos );

      } else {

         pos = GetCurrentPos() - 1;
         ch = GetCharAt( pos );
         if (  ch == '(' || ch == ')' ||
               ch == '{' || ch == '}' ||
               ch == '[' || ch == ']' ) {

            pos2 = BraceMatch( pos );

         } else {

            pos = wxSTC_INVALID_POSITION;
         }
      }

      // Do the highlight.
      if ( pos != wxSTC_INVALID_POSITION ) {

         if ( pos2 != wxSTC_INVALID_POSITION ) {

            BraceHighlight( pos, pos2 );

         } else {
            
            BraceBadLight( pos );
         }

      } else {
   
         BraceHighlight( wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION );
      }

   } else {

      BraceHighlight( wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION );
   }

   {
      wxASSERT( tsGetMainFrame() );
      wxStatusBar* bar = tsGetMainFrame()->GetStatusBar();
      
      // NOTE: The freeze and thaw here is required else our
      // custom flicker free code in tsStatusBar::OnPaint()
      // will leak it's dc once each time the line is changed.
      // No clue why this occurs... but this solves it.
	  bar->Freeze();

      int Pos = GetCurrentPos();
      wxString Text;
      Text << "Ln " << ( LineFromPosition( Pos ) + 1 );
      bar->SetStatusText( Text, tsSTATUS_LINE );
      Text.Clear();
      Text << "Col " << ( GetColumn( Pos ) + 1 );
      bar->SetStatusText( Text, tsSTATUS_COL );
      Text.Clear();
      Text << "Off " << Pos;
      bar->SetStatusText( Text, tsSTATUS_OFFSET );
      Text.Clear();
      Text << "Chr " << GetCharAt( Pos ) ;
      bar->SetStatusText( Text, tsSTATUS_CHAR );

      if ( !GetOvertype() ) 
         bar->SetStatusText( "INS", tsSTATUS_OVERWRITE );
      else
         bar->SetStatusText( "OVR", tsSTATUS_OVERWRITE );

      bar->Thaw();
   }
}

void ScriptCtrl::OnTextChanged( wxStyledTextEvent& event )
{
   if ( GetReadOnly() ) 
      return;

   // Let the doc know the script control changed.
   wxASSERT( m_ScriptView );
   wxASSERT( m_ScriptView->GetDocument() );
   m_ScriptView->GetDocument()->Modify( GetModify() );

   //
   // TODO: This will all be refactored... would parsing this be faster
   // in the idle process instead of trying to do these memory copies for
   // passing to a worker thread?
   //

   // Did we get a change in the text?
   if ( !m_Loading && event.GetModificationType() & (wxSTC_MOD_INSERTTEXT|wxSTC_MOD_DELETETEXT) ) 
   {
      // We incrementally update the autocomp buffer,
      // so store the events that we'll later process
      // in the idle call.

      // Can we combine the current event with the last?
      bool add = true;
      if ( !m_ScriptEvents.IsEmpty() ) 
      {
         wxStyledTextEvent* last = m_ScriptEvents.Last();
         wxASSERT( last );

         if (  event.GetModificationType() & wxSTC_MOD_DELETETEXT && 
               last->GetModificationType() & wxSTC_MOD_DELETETEXT &&
               ( last->GetPosition() - event.GetLength() ) == event.GetPosition() ) 
         {
            last->SetPosition( event.GetPosition() );
            last->SetLength( last->GetLength() + event.GetLength() );
            add = false;

         } 
         else if ( event.GetModificationType() & wxSTC_MOD_INSERTTEXT && 
                     last->GetModificationType() & wxSTC_MOD_INSERTTEXT &&
                     ( last->GetPosition() + last->GetText().Len() ) == event.GetPosition() ) 
         {
            //last->SetPosition( event.GetPosition() );
			//TODO
			 last->SetText(event.GetText());
            add = false;
         }
      }

      // If all else fails add it.
      if ( add )
         m_ScriptEvents.Add( new wxStyledTextEvent( event ) );

      // Fire off the timer.
      m_AutoCompTimer.Start( 350, wxTIMER_ONE_SHOT );
   }

   // Update the margin line number columns
   if ( tsGetPrefs().GetLineNumbers() && event.GetLinesAdded() != 0 ) {

      int width = GetLineMarginWidth();
      if ( GetMarginWidth( MARGIN_LINE_NUMBERS ) != width )
         SetMarginWidth( MARGIN_LINE_NUMBERS, width );
   }

   // Let the view know if we've been modified
   // so it can update the tab state.
   if ( !m_Loading && m_LastModify != GetModify() ) {
      m_ScriptView->OnCtrlModifyed();
      m_LastModify = GetModify();
   }

   // If we have breakpoints or bookmarks then fix them up.
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( project && !m_Loading && event.GetLinesAdded() != 0 )
   {
      const int line = 1 + LineFromPosition( event.GetPosition() );
      //bool startOfLine = PositionFromLine( LineFromPosition( event.GetPosition() ) ) == event.GetPosition();

      // Now reposition breakpoints and re-enable them.
      BreakpointArray breaks;
      project->GetBreakpoints( m_FilePath, breaks );
      breaks.Sort( Breakpoint::LineCompare );
      for ( int i=0; i < breaks.GetCount(); i++ ) 
      {
         Breakpoint* bp = breaks[i];
         wxASSERT( bp );

         // If the break is before the insert position 
         // we can skip over it.
         if ( bp->GetLine() <= line )
            continue;

         // If the break is within deleted text then
         // it needs to be deleted.
         if (  event.GetLinesAdded() < 0 && 
               bp->GetLine() < (line + -event.GetLinesAdded()) ) 
         {
            project->DeleteBreakpoint( bp );
            continue;
         }

         // Ok the breakpoint needs to move up or down
         // based on the lines added or removed.
         project->MoveBreakpoint( bp, bp->GetLine() + event.GetLinesAdded() );
      }

      // Next reposition bookmarks.
      BookmarkArray bookmarks;
      project->GetBookmarks( m_FilePath, bookmarks );
      bookmarks.Sort( Bookmark::Compare );
      bool updateBookmarks = false;
      for ( int i=0; i < bookmarks.GetCount(); i++ ) 
      {
         Bookmark* bm = bookmarks[i];
         wxASSERT( bm );

         // If the bookmark is before the insert position 
         // we can skip over it.
         if ( bm->GetLine() < line )
            continue;

         // If the break is within deleted text then
         // it needs to be deleted.
         if (  event.GetLinesAdded() < 0 && 
               bm->GetLine() < (line + -event.GetLinesAdded()) ) 
         {
            project->DeleteBookmark( bm );
            continue;
         }

         // TODO: Maybe implement a move function and event
         // like we have for breakpoints.

         // Ok the breakpoint needs to move up or down
         // based on the lines added or removed.
         updateBookmarks = true;
         bm->SetLine( bm->GetLine() + event.GetLinesAdded() );
      }

      if (updateBookmarks)
         UpdateBookmarks();
   }
}

void ScriptCtrl::ShowLineNumbers( bool enabled )
{
   int width = 0;
   if ( enabled )
      width = GetLineMarginWidth();

   SetMarginWidth( MARGIN_LINE_NUMBERS, width );
}

int ScriptCtrl::GetLineMarginWidth()
{
   wxString Cols;
   Cols << GetLineCount();
   Cols = wxString( '9', Cols.Length()+1 );
   return TextWidth( wxSTC_STYLE_LINENUMBER, Cols ); // default to 1 char.
}

void ScriptCtrl::OnUpdateAutoCompText( wxTimerEvent& event )
{
   if ( m_Loading || m_ScriptEvents.IsEmpty() )
      return;

   // Update our buffer with the events we've stored.
   AutoCompText* TextBuffer = GetDocument()->GetTextBuffer();
   if ( !TextBuffer )
   {
      m_ScriptEvents.Empty();
      return;
   }

   wxASSERT( TextBuffer );
   TextBuffer->Lock();
   for ( int i=0; i < m_ScriptEvents.GetCount(); i++ ) {

      wxASSERT( m_ScriptEvents[i] );
      const wxStyledTextEvent& event = *m_ScriptEvents[i];

      if ( event.GetModificationType() & wxSTC_MOD_DELETETEXT ) {
       
         wxASSERT( !( event.GetModificationType() & wxSTC_MOD_INSERTTEXT ) );
         TextBuffer->DeleteRange( event.GetPosition(), event.GetLength() );

      } else {

         wxASSERT( event.GetModificationType() & wxSTC_MOD_INSERTTEXT );
         wxASSERT( !( event.GetModificationType() & wxSTC_MOD_DELETETEXT ) );
         TextBuffer->InsertRange( event.GetPosition(), event.GetText() );
      }

      delete m_ScriptEvents[i];
   }

   // TODO: Comment this out once we feel this works 
   // as it's slow ass shit.
   wxASSERT( GetText() == TextBuffer->GetBuf() );

   TextBuffer->Unlock();
   m_ScriptEvents.Empty();
}

wxString ScriptCtrl::GetLineIndentString( int line )
{
    int currLine = (line == -1)
                    ? LineFromPosition(GetCurrentPos())
                    : line;

    wxString text = GetLine(currLine);
    unsigned int len = text.Length();
    wxString indent;
    for (unsigned int i = 0; i < len; ++i)
    {
        if (text[i] == _T(' ') || text[i] == _T('\t'))
            indent << text[i];
        else
            break;
    }

    return indent;
}

wxChar ScriptCtrl::GetLastNonWhitespaceChar( int pos )
{
   if (pos == -1)
      pos = GetCurrentPos();

   int blanks = 0; 
   bool foundlf = false;

   while ( pos )
   {
      wxChar c = GetCharAt(--pos);
      if (c == _T('\n'))
      {
         blanks++;
         foundlf = true;
      }
      else if (c == _T('\r') && !foundlf)
         blanks++;
      else
         foundlf = false;

      if (blanks > 1) return 0; // Don't over-indent

      if ( !wxIsspace( c ) )
         return c;
   }

   return 0;
}

int ScriptCtrl::FindBlockStart( int pos, wxChar blockStart, wxChar blockEnd, bool skipNested )
{
   int lvl = 0;
   wxChar b = GetCharAt( pos );
   while (b) {

      if (b == blockEnd)
         ++lvl;

      else if (b == blockStart) {

         if (lvl == 0)
            return pos;
         --lvl;
      }

      --pos;
      b = GetCharAt(pos);
   }
   return -1;
}

void ScriptCtrl::OnChar( wxKeyEvent& event )
{
   if ( GetReadOnly() )
   {
      wxBell(); 
      return;
   }

   if ( event.GetKeyCode() == ')' ) 
      CallTipCancel();

   // Let the base work it...
   wxStyledTextCtrl::OnChar( event );

   if ( event.GetKeyCode() == '}' ) 
   {
      BeginUndoAction();

      // undo block indentation, if needed
      wxString str = GetLine(GetCurrentLine());
      str.Trim(false);
      str.Trim(true);
      if ( str.Matches( _T("}") ) ) 
      {
         // just the brace here; unindent
         // find opening brace (skipping nested blocks)
         int pos = GetCurrentPos() - 2;
         pos = FindBlockStart( pos, _T('{'), _T('}') );
         if ( pos != -1 ) 
         {
            wxString indent = GetLineIndentString( LineFromPosition( pos ) );
            indent << _T('}');
            DelLineLeft();
            DelLineRight();
            pos = GetCurrentPos();
            InsertText(pos, indent);
            GotoPos(pos + indent.Length());
            SendMsg( 2399 ); // SCI_CHOOSECARETX ... else up cursor won't line up!
         }
      }

      EndUndoAction();
      return;
   }
   
   if ( AutoCompActive() && event.GetKeyCode() != '.' )
      return;

   // See if we need to list ScriptSense members.
   if ( ListMembers( false ) )
      return;

   // See if we need to show parameter info.
   if ( ParameterInfo( false ) )
      return;
}

void ScriptCtrl::GetWordsAtPos( int pos, wxString* prevWord, wxString* sep, wxString* currWord )
{
   wxASSERT( prevWord );
   wxASSERT( sep );
   wxASSERT( currWord );

   int start = WordStartPosition( pos, true );
   *currWord = GetTextRange( start, pos );
   int last = WordStartPosition( start, false );
   *sep = GetTextRange( last, start );
   start = WordStartPosition( last, true );
   last = WordEndPosition( start, true );
   *prevWord = GetTextRange( start, last );

   // Check for a global by looking backwards for 
   // the start of it.  Look for a non identifier
   // character except for : and that it begins 
   // with a $.
   wxString global;
   start = pos - 1;
   while ( start >= 0 )
   {
      wxChar c = GetCharAt( start );
      if (  c != ':' && 
            sm_WordChars.Find( c ) == wxNOT_FOUND )
      {
         break;
      }

      global.Append( c );
      start--;
   }

   if ( global.Len() > 0 && global.Last() == '$' )
   {
      prevWord->Empty();
      sep->Empty();

      *currWord = GetTextRange( start + 1, pos );
   }
}

void ScriptCtrl::OnListMembers( wxCommandEvent& event )
{
   if ( !AutoCompActive() )
      ListMembers( true );
}

bool ScriptCtrl::ListMembers( bool forced )
{
   // Figure out the path or title for this file.
   wxString filePath = m_FilePath;
   if ( filePath.IsEmpty() )
      filePath = GetDocument()->GetTitle();

   const int pos = GetCurrentPos();
   if ( GetEndStyled() < pos )
   {
      int line = LineFromPosition( pos );
      Colourise( PositionFromLine( line-1 ), PositionFromLine( line + 1 ) );
   }
   const int style = GetStyleAt( pos-1 );

   if (  style == wxSTC_TCS_COMMENT ||
         style == wxSTC_TCS_COMMENTLINE ||
         style == wxSTC_TCS_COMMENTDOC ||
         style == wxSTC_TCS_STRING ||
         style == wxSTC_TCS_STRINGEOL )
   {
      if ( forced )
         tsBellEx( wxICON_INFORMATION );

      return false;
   }

   wxASSERT( tsGetAutoComp() );

   wxString prevWord, sep, currWord;
   const int line = LineFromPosition( pos ) + 1;
   GetWordsAtPos( pos, &prevWord, &sep, &currWord );

   if ( m_IsGlobalFullups )
   {
      AutoCompSetFillUps( m_AutoCompFillups );
      m_IsGlobalFullups = false;
   }

   if ( ( currWord.Len() > 0 || forced ) && prevWord == "datablock" ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      if ( data->IsWordInDatablockList( currWord ) )
         AutoCompShow( currWord.Len(), data->GetDatablockList() );
      else if ( forced )
         tsBellEx( wxICON_INFORMATION );

      tsGetAutoComp()->Unlock();
   }
   else if ( ( currWord.Len() > 0 || forced ) && 
      ( prevWord == "new" || prevWord == "singleton" ) ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      if ( data->IsWordInClassList( currWord ) )
         AutoCompShow( currWord.Len(), data->GetClassList() );
      else if ( forced )
         tsBellEx( wxICON_INFORMATION );

      tsGetAutoComp()->Unlock();
   }

   // Look for locals if the file name is valid.
   /*
   else if ( !filePath.IsEmpty() ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      // Look for entries into objects or datablocks!
      wxString fields;
      if ( data->BuildFieldList( currWord, filePath, line-1, fields ) )
          AutoCompShow( range, fields );

      tsGetAutoComp()->Unlock();
   }
   */

   // Look for a class, datablock, or namespace member expansion.
   else if ( !prevWord.IsEmpty() && ( sep == "::" || sep == "." ) )
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      wxString list;
      if ( prevWord.CmpNoCase( "%this" ) == 0 )
      {
         // Figure out what type %this is.
         wxString funcName = data->GetFunctionNameAt( filePath, line-1 );
         wxString objectName = funcName.BeforeFirst( ':' );
         if ( !objectName.IsEmpty() ) 
            data->BuildMemberList( objectName, list );
      }
      else
         data->BuildMemberList( prevWord, list );

      tsGetAutoComp()->Unlock();

      // Show a member list for this word.
      if ( !list.IsEmpty() )
         AutoCompShow( currWord.Len(), list );
      else if ( forced )
         tsBellEx( wxICON_INFORMATION );
   }

   // Are we looking for a global?
   else if ( ( currWord.Len() > 1 || forced ) && currWord[0] == '$' ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      if ( data->IsWordInGlobalsList( currWord ) )
      {
         // Globals can include the : character... which isn't
         // a normal fillup character.  Set this special fillup
         // character set that doesn't include a colon.
         m_IsGlobalFullups = true;
         AutoCompSetFillUps( m_AutoCompGlobalFillups );
         AutoCompShow( currWord.Len(), data->GetGlobalsList() );
      }
      
      tsGetAutoComp()->Unlock();
   }

   // Are we looking for a local?
   else if ( !filePath.IsEmpty() && ( currWord.Len() > 1 || forced ) && currWord[0] == '%' ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      // Grab a local list for this line.
      wxString list;
      if ( data->BuildLocalsList( filePath, line-1, list ) > 0 )
         AutoCompShow( currWord.Len(), list );

      tsGetAutoComp()->Unlock();
   }

   // Just show the full completion list.
   else if ( currWord.Len() > 0 || forced )
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      // First look to see if we're trying to enter a field 
      // within a datablock or object
      //
      // TODO: BuildFieldList will fail for objects and datablocks
      // that do not have a name, because... duh... we don't add
      // nameless objects or datablocks to the database.  We need to
      // devise a method to add nameless blocks... possibly in a
      // seperate structure in a page.
      //
      wxString fields;
      if ( !filePath.IsEmpty() && data->BuildFieldList( currWord, filePath, line-1, fields ) )
         AutoCompShow( currWord.Len(), fields );
      else if ( ( currWord.Len() > 2 || forced ) && data->IsWordInCompList( currWord ) )
         AutoCompShow( currWord.Len(), data->GetCompList() );
      else if ( forced )
         tsBellEx( wxICON_INFORMATION );

      tsGetAutoComp()->Unlock();
   }

   // We got no matches... nothing was done.
   else
      return false;

   // To do proper as you type code completion we need 
   // three strings...
   //
   //  <previous word><seperator><current word>
   //
   // The previous word and seperator are used to decide
   // what type of code completion to start.  For example...
   //
   //  previous   seperator   current
   //  ------------------------------
   //  datablock  whitespace  completion
   //  new        whitespace  completion
   //  object     ::          completion
   //  object     .           completion
   //  function   (           calltip
   //

   /*
   int start = WordStartPosition( pos, true );
   const int range = pos - start;
   const int line = LineFromPosition( pos ) + 1;
   wxString currWord = GetTextRange( start, pos );

   // Grab the previous word... we use this to decide if we need to 
   // list objects for 'new' or datablocks for 'datablock'.
   wxString lastWord;
   start = WordStartPosition( WordStartPosition( start, false ), true );
   int last = WordEndPosition( start, true );
   lastWord = GetTextRange( start, last );
   */

   /*
   const int line = LineFromPosition( pos ) + 1;
   wxString lastWord;
   wxString sep;
   int range;
   {
      int start = IdentifierStartPos( pos );
      range = pos - start;
      lastWord = GetTextRange( start, pos );

      if ( lastWord.Right( 2 ) == "::" )
      {
         lastWord.RemoveLast();
         lastWord.RemoveLast();
         sep = "::";
         range = 0;
      }
      else if ( lastWord.Find( '.' ) != -1 )
      {
         sep = '.';
         int find = lastWord.Find( '.', true );
         range = ( lastWord.Len() - find ) - 1;
         lastWord.Truncate( find );
      }
      else if ( lastWord.Right( 1 ) == "(" )
      {
         sep = lastWord.Last();
         lastWord.RemoveLast();
         range = 0;
      }
   }
   */

   /*
   else if ( !lastWord.IsEmpty() && sep == "(" ) 
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      lastWord = GetIdentifierAt( pos-1 );

      // Convert %this to a concrete type.
      if ( lastWord == "%this" || lastWord.StartsWith( "%this." ) )
      {
         wxString funcName = data->GetFunctionNameAt( filePath, line );
         wxString objectName = funcName.BeforeFirst( ':' );

         if ( lastWord == "%this" )
            lastWord = objectName;
         else
            lastWord = objectName + "::" + lastWord.AfterFirst( '.' );
      }

      // Grab the calltip data.
      wxString calltip;
      data->GetCallTip( lastWord, calltip );

      tsGetAutoComp()->Unlock();

      if ( !calltip.IsEmpty() ) {

         // HACK: The font used for the calltips is wxSTC_STYLE_DEFAULT,
         // but the column guide uses this one as well to measure the
         // column width for placement.  So set and restore the font here
         // in order to fix things.
         //wxFont font( 8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Tahoma" );
         //StyleSetFont( wxSTC_STYLE_DEFAULT, font );
         CallTipShow( pos, calltip );
         //font = tsGetPrefs().GetDefaultFont();
         //StyleSetFont( wxSTC_STYLE_DEFAULT, font );
      }

      return;
   }
   */

   return true;
}

void ScriptCtrl::OnParameterInfo( wxCommandEvent& event )
{
   if ( !AutoCompActive() )
      ParameterInfo( true );
}

bool ScriptCtrl::ParameterInfo( bool forced )
{
   const int pos = GetCurrentPos();

   wxASSERT( tsGetAutoComp() );

   // Look at and before the current position to find 
   // the opening brace.
   wxString prevWord, sep, currWord;
   const int line = LineFromPosition( pos ) + 1;
   int start = pos-1, end;
   for ( ; start >= 0; start-- )
   {
      // Get the style... we can pick a bracket inside
      // a comment or string.
      const int style = GetStyleAt( start );
      if (  style == wxSTC_TCS_COMMENT ||
            style == wxSTC_TCS_COMMENTLINE ||
            style == wxSTC_TCS_COMMENTDOC ||
            style == wxSTC_TCS_STRING ||
            style == wxSTC_TCS_STRINGEOL )
         continue;

      // If it's not an open bracket then skip it.
      const char ch = GetCharAt( start );
      if ( ch != '(' )
         continue;

      // Look for the close bracket and if we have one
      // be sure our position is within it.
      end = BraceMatch( start );
      if ( end != wxSTC_INVALID_POSITION && end < pos )
      {
         // We're outside the brackets... do nothing!
         if ( forced )
            tsBellEx( wxICON_INFORMATION );

         CallTipCancel();
         return false;
      }

      // Get the function name... continue to look
      // for brackets if this is a variable.
      GetWordsAtPos( start, &prevWord, &sep, &currWord );
      if ( currWord.IsEmpty() || currWord[0] == '$' || currWord[0] == '%' )
         continue;

      break;
   }

   // Figure out the path or title for this file.
   wxString filePath = m_FilePath;
   if ( filePath.IsEmpty() )
      filePath = GetDocument()->GetTitle();

   // Look for the function info!
   CallTipInfo calltip;
   if ( !currWord.IsEmpty() )
   {
      const AutoCompData* data = tsGetAutoComp()->Lock();

      wxString list;
      if ( prevWord.CmpNoCase( "%this" ) == 0 )
      {
         // Figure out what type %this is.
         wxString funcName = data->GetFunctionNameAt( filePath, line-1 );
         prevWord = funcName.BeforeFirst( ':' );
      }

      // Build the function string.
      wxString func;
      if ( sep == "::" || sep == "." )
         func << prevWord << '.' << currWord;
      else
         func << currWord;

      // Ok... we have the type... look up the call tip!
      data->GetCallTip( func, calltip );

      tsGetAutoComp()->Unlock();
   }

   // If we got a tip then display it!
   if ( !calltip.IsEmpty() ) 
   {
      // If we got a dot seperator then we need to
      // remove the first parameter... it should
      // always be %this.
      if ( sep == "." )
         calltip.RemoveThis();

      // Show it.
      CallTipShow( start+1, calltip );

      // How many commas between the start 
      // and the current position?
      int param = 0;
      while ( start != pos )
      {
         if ( GetCharAt( start ) == ',' )
            param++;

         start++;
      }

      // Highlight that parameter.
      int hstart, hend;
      if ( calltip.GetArgumentRange( param, &hstart, &hend ) )
      {
         CallTipSetHighlight( hstart, hend );
      }
      else
         CallTipSetHighlight( 0, 0 );
   } 
   else
   {
      CallTipCancel();
   }

   return true;
}

/*
void ScriptCtrl::OnAutoCompSel( wxStyledTextEvent& event )
{
   int type = event.GetListType();
   wxString text = event.GetText();
}
*/

/*
int ScriptCtrl::IdentifierStartPos( int pos )
{
   // Walk backwards from the position stopping when
   // we we are no longer within the wxSTC_TCS_IDENTIFIER,
   // wxSTC_TCS_VAR, or wxSTC_TCS_GLOBALCLASS styles.

   // Make sure we're styled to the position.
   if ( GetEndStyled() < pos )
      Colourise( GetEndStyled(), pos );

   int start = pos - 1;

   // Skip the trailing (
   if ( GetCharAt( start ) == '(' )
      start--;

   for ( ; start > 0; start-- )
   {
      int style = GetStyleAt( start );
      if (  style != wxSTC_TCS_VAR &&
            style != wxSTC_TCS_IDENTIFIER &&
            style != wxSTC_TCS_GLOBALCLASS &&
            style != wxSTC_TCS_WORD &&
            style != wxSTC_TCS_WORD2 &&
            GetCharAt( start ) != '.' )
         break;
   }

   return start + 1;
}
*/

wxString ScriptCtrl::GetIdentifierAt( int pos, bool* isFunction )
{
   if ( pos < 0 || pos >= GetLength() )
      return wxEmptyString;

   // Make sure we're colorized passed this position.
   if ( GetEndStyled() < pos )
   {
      int line = LineFromPosition( pos );
      Colourise( PositionFromLine( line-1 ), PositionFromLine( line + 1 ) );
   }

   // Skip if we're in a comment or string.
   const int style = GetStyleAt( pos );
   if (  style == wxSTC_TCS_COMMENT ||
         style == wxSTC_TCS_COMMENTLINE ||
         style == wxSTC_TCS_COMMENTDOC ||
         style == wxSTC_TCS_STRING ||
         style == wxSTC_TCS_STRINGEOL )
      return wxEmptyString;

   // TODO: This function could probably use the syntax
   // highlighting to pull identifiers instead of parsing
   // on it's own.
   //
   // Must be able to grab...
   //
   // functionName
   // object::functionName
   // object.functionName
   // $object.functionName
   // %object.functionName
   // $var
   // %var
   // %name::name::var
   // $name::name::var
   //

   // Do we have a selection?
   if (  GetSelectionStart() < pos &&
         GetSelectionEnd() > pos ) {

      return GetTextRange( GetSelectionStart(), GetSelectionEnd() );
   }

   // Grab the initial identifier.
   int start = WordStartPosition( pos, true );
   int end = WordEndPosition( start, true );
   if ( end <= start )
      return wxEmptyString;
   
   wxString word;
   word << GetTextRange( start, end );
   
   // If we're leading with a number then
   // this cannot be a good identifier.
   if ( wxIsdigit( word[0] ) && style != wxSTC_TCS_VAR )
      return wxEmptyString;
   
   // Look for junk infront of the word.
   while ( word[0] != '$' && word[0] != '%' ) {

      int nend = start;
      int nstart = WordStartPosition( nend, false );
      wxString prev = GetTextRange( nstart, nend );
      if ( prev != '.' && prev != "::" )
         break;

      int nend2 = nstart - prev.Len();
      start = WordStartPosition( nend2, true );
      prev = GetTextRange( start, nend );
      word.Prepend( prev );
   }

   // Look for stuff behind the word.
   //if ( word[0] == '$' || word[0] == '%' )
   //   return word;

   for ( ;; ) {

      int nstart = end;
      int nend = WordEndPosition( nstart, false );
      wxString next = GetTextRange( nstart, nend );

      // Check by chance if it's a function.
      if ( next[0] == '(' && isFunction )
         *isFunction = true;

      if ( /*next != '.' &&*/ next != "::" )
         break;

      if ( word[0] != '$' && word[0] != '%' && (next == "::" || next == ".") )
         break;

      int nstart2 = nend + next.Len();
      end = WordEndPosition( nstart2, true );
      next = GetTextRange( nstart, end );
      word.Append( next );
   }

   return word;
}

void ScriptCtrl::OnHoverStart( wxStyledTextEvent& event )
{
   // Keep calltips from happening when we
   // have a popup menu up.
   if ( wxCurrentPopupMenu )
      return;

   // Don't do calltips if the application isn't
   // activated... or if this control isn't active.
   if ( !tsGetMainFrame()->IsActive() ||
      tsGetMainFrame()->GetActiveView() != m_ScriptView )
      return;

   // wxSTC tends to call dwell with an old mouse position
   // so we need to test for that here.
   wxPoint pt = wxGetMousePosition();
   ScreenToClient( &pt.x, &pt.y );
   if ( !GetClientRect().Contains( pt ) )
      return;

   // Cancel the current tip... if any.
   CallTipCancel();

   // Get the positional info.
   int pos = PositionFromPoint( pt );
   if ( pos ==  wxSTC_INVALID_POSITION )
      return;

   int line = LineFromPosition( pos );
   int end = GetLineEndPosition( line );

   m_LastCallTipPos = pos + 1;

   // Are we in the margin?
   const int marginWidth = GetMarginWidth( 0 ) + GetMarginWidth( 1 ) + GetMarginWidth( 2 );
   if ( pt.x <= marginWidth ) {
      
      // Are we in the breakpoint margin?
      if ( pt.x < GetMarginWidth( 0 ) ) {

         pos = PositionFromPointClose( marginWidth + marginWidth, pt.y );
         line = LineFromPosition( pos ) + 1;
         pos = PositionFromLine( line-1 );

         wxASSERT( tsGetMainFrame() );
         ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
         if ( !project )
            return;

         Breakpoint* bp = project->FindBreakpoint( m_FilePath, line );
         if ( bp ) 
         {
            wxString calltip = bp->GetToolTip();
            if ( !calltip.IsEmpty() )
               CallTipShow( pos, calltip );

            return;
         }
      }

      return;
   }

   // Look for other identifiers at this position.
   bool isFunction = false;
   wxString word = GetIdentifierAt( pos, &isFunction );
   if ( !word.IsEmpty() ) 
   {
      const int curline = LineFromPosition( pos );

      // If this is a local and our breakpoint is in the active 
      // file then we can safely try to evaluate it.
      wxASSERT( tsGetDebugger() );
      wxString brfile; int brline;
      if (  word[0] == '%' && !isFunction &&
            tsGetDebugger()->GetBreakpointFileAndLine( brfile, brline ) && 
            brfile == m_FilePath )
      {
         // Get the current function line range o the entire file line range.
         int startl = 0, endl = GetLineCount();
         const AutoCompData* data = tsGetAutoComp()->Lock();
         data->GetFunctionLineRange( brfile, brline, startl, endl );
         tsGetAutoComp()->Unlock();

         if ( curline >= startl && curline <= endl )
         {
            tsGetDebugger()->EvalCallTip( word );
            return;
         }
      }

      // If this is a global then its always safe to evaluate.
      if ( word[0] == '$' && !isFunction && tsGetDebugger()->IsAtBreakpoint() )
      {
         tsGetDebugger()->EvalCallTip( word );
         return;
      }

      const AutoCompData* data = tsGetAutoComp()->Lock();

      // Convert %this to a concrete type.
      if ( word == "%this" || word.StartsWith( "%this." ) )
      {
         wxString funcName = data->GetFunctionNameAt( m_FilePath, curline );
         wxString objectName = funcName.BeforeFirst( ':' );

         if ( word == "%this" )
            word = objectName;
         else
            word = objectName + "::" + word.AfterFirst( '.' );
      }

      CallTipInfo calltip;
      data->GetCallTip( word, calltip );
      tsGetAutoComp()->Unlock();

      if ( !calltip.IsEmpty() ) 
      {
         CallTipShow( m_LastCallTipPos, calltip );
         return;
      }
   }

   // Are we on a folded line?  The tooltip is the 
   // first 10 lines of what is within the fold.
   if ( !GetFoldExpanded( line ) ) 
   {
      wxString calltip;

      int lastLine = GetLastChild( line, GetFoldLevel( line ) );
      int lines = wxMin( lastLine - line, 10 );
      for ( int l=0; l < lines; l++ )
         calltip << GetLine( line + 1 + l );

      calltip = CallTipInfo::FormatTip( calltip, 3 );
      if ( lines == 10 )
         calltip << "...";

      if ( !calltip.IsEmpty() )
         CallTipShow( m_LastCallTipPos, calltip );

      return;
   }
}

void ScriptCtrl::OnDebugCallTip( const wxString& Expression, const wxString& Value )
{
   CallTipCancel();

   wxString calltip;
   calltip << Expression << " = " << Value;

   if ( calltip.IsEmpty() )
      return;

   // Cancel the current tip... if any then show the new tip.
   CallTipShow( m_LastCallTipPos, calltip );
}

void ScriptCtrl::Cut()
{
   // Handle line cut!
   if ( GetSelectionStart() == GetSelectionEnd() )
      LineCut();
   else
      wxStyledTextCtrl::Cut();
}

void ScriptCtrl::OnHoverEnd( wxStyledTextEvent& event )
{
   // TODO: The dwell is too sensitive to mouse movement
   // i need to hack a tolerance into wxSTC.
   CallTipCancel();
}


void ScriptCtrl::SetLineVisible( int line )
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

bool ScriptCtrl::SetLineSelected( int line )
{
   int pos = PositionFromLine( line );
   if ( pos < 0 )
      return false;

   SetLineVisible( line );
   SetAnchor( pos );
   SetCurrentPos( GetLineEndPosition( line ) );

   return true;
}

void ScriptCtrl::OnUpdateEnable( wxUpdateUIEvent& event )
{
   event.Enable( true );
}

void ScriptCtrl::OnGoto( wxCommandEvent& event )
{
   wxDialog* GotoDlg = new wxDialog( tsGetMainFrame(), wxID_ANY, "Go To Line", wxDefaultPosition );
   wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );

   sizer->AddSpacer( 7 );

   wxString label;
   label << "&Line number ( 1 - " << GetLineCount() << " ):";
   sizer->Add( new wxStaticText( GotoDlg, wxID_ANY, label, wxDefaultPosition, wxSize( 225, -1 ) ), 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_TOP, 7 );

   sizer->AddSpacer( 7 );

   wxString dummy;
   dummy << ( LineFromPosition( GetCurrentPos() ) + 1 );

   wxTextValidator lineVal( wxFILTER_NUMERIC, &dummy );
   wxTextCtrl* lineCtrl = new wxTextCtrl( GotoDlg, wxID_ANY, dummy, wxDefaultPosition, wxDefaultSize, 0, lineVal );
   sizer->Add( lineCtrl, 0, wxLEFT | wxRIGHT | wxEXPAND | wxALIGN_TOP, 7 );

   sizer->AddSpacer( 7 );

   wxSizer* hsizer = GotoDlg->CreateButtonSizer( wxOK | wxCANCEL );
   sizer->Add( hsizer, 0, wxLEFT | wxRIGHT | wxALIGN_RIGHT, 7 );

   sizer->AddSpacer( 7 );

   GotoDlg->SetSizerAndFit( sizer );
   
   lineCtrl->SetFocus();
   lineCtrl->SetSelection( -1, -1 );

   if ( GotoDlg->ShowModal() == wxID_OK ) {

      long line;
      lineCtrl->GetLabel().ToLong( &line );
      if ( line < 1 ) line = 1;
      if ( line > GetLineCount() ) {
         wxBell();
         line = GetLineCount();
      }
      SetLineSelected( line - 1 );
   }

   GotoDlg->Destroy();
}

void ScriptCtrl::OnKeyDown( wxKeyEvent& event )
{
	if ( event.GetKeyCode() == WXK_F9 && !m_FilePath.IsEmpty() ) 
   {
      // Remember the debugger wants one based line numbers!
      wxASSERT( tsGetMainFrame() );
      ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
      if ( !project )
         return;

      if ( event.ControlDown() )
         OnToggleBreakpoint( wxCommandEvent() );
      else
      {
         // If we find breakpoints... delete them.
         const int start   = LineFromPosition( GetSelectionStart() ) + 1;
         const int end     = LineFromPosition( GetSelectionEnd() ) + 1;
         bool found = false;
         for ( int l=start; l <= end; l++ )
         {
            Breakpoint* bp = project->FindBreakpoint( m_FilePath, l );
	         if ( bp )
            {
               project->DeleteBreakpoint( bp );
               found = true;
            }
         }

         // If we didn't find breakpoints... add one.
         if ( !found )
            OnInsertBreakpoint( wxCommandEvent() );
      }

      return;
	}

   // HACK... stupid wx won't allow this to be an accelerator!
   if ( event.GetKeyCode() == ']' && event.ControlDown() ) 
   {
      MatchBrace();
      return;
   }

   // HACK: We want Shift+Delete to work just like Cut.
   if ( event.GetKeyCode() == WXK_DELETE && event.ShiftDown() )
      Cut();

   // Another hack... eat Ctrl+/ and Ctrl+Shift+/, so that 
   // the ctrl doesn't get it.
   /*
   if (  event.GetKeyCode() == '/' && 
         ( event.ControlDown() || event.ShiftDown() ) )
   {
      //event.Skip();
      return;
   }
   */

   const int lastPos = GetCurrentPos();

   const bool wasAutoCompActive = AutoCompActive();

   // Do the default processing.
   wxStyledTextCtrl::OnKeyDown( event );

   // Do we have a call tip active?
   if ( CallTipActive() )
      ParameterInfo( false );

   // Hack... the control will consume alt, ctrl, and 
   // shifts even though it ignores them... fix that.
   if (  event.GetKeyCode() == WXK_ALT ||
         event.GetKeyCode() == WXK_CONTROL ||
         event.GetKeyCode() == WXK_SHIFT ||
         event.GetKeyCode() == WXK_MENU ) 
   {
      event.Skip();
   }

   // The autoindent was moved from OnChar() because it
   // isn't getting newline events from wxStyledTextCtrl.
   if ( !wasAutoCompActive && 
         (  event.GetKeyCode() == WXK_RETURN || 
            event.GetKeyCode() == WXK_NUMPAD_ENTER ) )
   {
      int pos = GetCurrentPos();
      wxString indent = GetLineIndentString( LineFromPosition( pos ) - 1);
      if ( GetLastNonWhitespaceChar( pos ) == '{' ) 
      {
         if ( GetUseTabs() )
            indent << "\t";
         else
            indent << wxString( _T(' '), GetTabWidth() );
      }

      if ( !indent.IsEmpty() ) 
      {
         BeginUndoAction();

         InsertText( pos, indent );
         GotoPos( pos + indent.Len() );

         EndUndoAction();
      }
   }
}

void ScriptCtrl::OnMarginClick( wxStyledTextEvent& event )
{
   int line = LineFromPosition( event.GetPosition() );

   if ( event.GetMargin() == MARGIN_MARKERS ) {

      wxASSERT( tsGetMainFrame() );
      ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
      if ( !project )
         return;
      
	   // Remember the debugger wants one based line numbers!
      project->ToggleBreakpoint( m_FilePath, line + 1, false );
      return;
   }

   if (  event.GetMargin() == MARGIN_FOLDING && 
         GetFoldLevel( line ) & wxSTC_FOLDLEVELHEADERFLAG ) {

      ToggleFold( line );

      /*
      int pos = PositionFromLine( line );
      int es = GetEndStyled();

      if ( !GetFoldExpanded( line ) ) {

         StartStyling( pos, wxSTC_INDICS_MASK );
         SetStyling( LineLength( line ), wxSTC_INDIC2_MASK );
      } 
      else {

         StartStyling( pos, wxSTC_INDICS_MASK );
         SetStyling( LineLength( line ), 0 );
         //Colourise(pos,pos+LineLength( line ));
      }

      StartStyling(es,0xF1);
      */
   }
}

void ScriptCtrl::OnFolding( wxCommandEvent& event )
{
   int line = LineFromPosition( GetCurrentPos() );
   int parent = GetFoldParent( line );
   
   bool header = GetFoldLevel( line ) & wxSTC_FOLDLEVELHEADERFLAG ? true : false;
   bool expanded = GetFoldExpanded( line );
   bool noselection = GetSelectionStart() == GetSelectionEnd();

   switch ( event.GetId() ) {

      case tsID_FOLDING_TOGGLEBLOCK:

         if ( header )
            ToggleFold( line );
         else if ( parent != -1 )
            ToggleFold( parent );

         break;

      case tsID_FOLDING_TOGGLEALLINBLOCK:
      {
         if ( header )
            parent = line;
 
         // Two passes... one for the state... one for the change.
         bool expanded = false;
         for ( int sl = parent+1; sl < GetLastChild( parent, -1 ); sl++ ) {
            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG &&
                  GetFoldParent( sl ) == parent )
               expanded |= GetFoldExpanded( sl );
         }

         Freeze();
         for ( int sl = parent+1; sl < GetLastChild( parent, -1 ); sl++ ) {
            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG && 
                  GetFoldParent( sl ) == parent &&
                  GetFoldExpanded( sl ) == expanded )
               ToggleFold( sl );
         }
         Thaw();
         break;
      }

      case tsID_FOLDING_TOGGLEALL:
      {
         // We gotta have the whole file colourized!
         if ( GetEndStyled() != GetLength() ) {
            Colourise( GetEndStyled(), GetLength() );
         }

         bool children = false;
         bool expanded = false;

         // Two passes... one for the state... one for the change.
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {

            if ( GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG ) {

               children = true;
               if ( GetFoldExpanded( sl ) ) {
                  expanded = true;
                  break;
               }
            }
         }

         Freeze();
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {
            if ( GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG && GetFoldExpanded( sl ) == expanded )
               ToggleFold( sl );
         }
         Thaw();
         break;
      }

      case tsID_FOLDING_COLLAPSETODEFS:
      {
         // We gotta have the whole file colourized!
         if ( GetEndStyled() != GetLength() ) {
            Colourise( GetEndStyled(), GetLength() );
         }

         bool children = false;
         bool expanded = false;

         // Two passes... one for the state... one for the change.
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {

            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG && 
                  GetFoldParent( sl ) == -1 ) {

               children = true;
               if ( GetFoldExpanded( sl ) ) {
                  expanded = true;
                  break;
               }
            }
         }

         Freeze();
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {

            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG && 
                  GetFoldParent( sl ) == -1 &&
                  GetFoldExpanded( sl ) == expanded )
               ToggleFold( sl );
         }
         Thaw();
         break;
      }
   }
}

void ScriptCtrl::OnFoldingUpdateUI( wxUpdateUIEvent& event )
{
   if ( GetSelectionStart() != GetSelectionEnd() ) {
      event.Enable( false );
      return;
   }

   int line = LineFromPosition( GetCurrentPos() );
   int parent = GetFoldParent( line );
   bool header = GetFoldLevel( line ) & wxSTC_FOLDLEVELHEADERFLAG ? true : false;
   bool expanded = GetFoldExpanded( line );

   switch ( event.GetId() ) {

      case tsID_FOLDING_TOGGLEBLOCK:
         event.SetText( ( parent != -1 || header ) && expanded ? "Collapse Block" : "Expand Block" );
         event.Enable( parent != -1 || header );
         break;

      case tsID_FOLDING_TOGGLEALLINBLOCK:
      {
         if ( header )
            parent = line;

         bool children = false;
         bool expanded = false;
         for ( int sl = parent+1; sl < GetLastChild( parent, -1 ); sl++ ) {

            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG &&
                  GetFoldParent( sl ) == parent ) {

               children = true;

               if ( GetFoldExpanded( sl ) ) {

                  expanded = true;
                  break;
               }
            }
         }

         event.SetText( children && expanded ? "Collapse Children" : "Expand Children" );
         event.Enable( children );
         break;
      }

      case tsID_FOLDING_TOGGLEALL:
      {
         // We gotta have the whole file colourized!
         if ( GetEndStyled() != GetLength() ) {
            Colourise( GetEndStyled(), GetLength() );
         }

         bool children = false;
         bool expanded = false;
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {

            if ( GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG ) {

               children = true;
               if ( GetFoldExpanded( sl ) ) {
                  expanded = true;
                  break;
               }
            }
         }

         event.SetText( children && expanded ? "Collapse All" : "Expand All" );
         event.Enable( children );
         break;
      }

      case tsID_FOLDING_COLLAPSETODEFS:
      {        
         // We gotta have the whole file colourized!
         if ( GetEndStyled() != GetLength() ) {
            Colourise( GetEndStyled(), GetLength() );
         }

         bool children = false;
         bool expanded = false;
         for ( int sl = 0; sl < GetLineCount(); sl++ ) {

            if (  GetFoldLevel( sl ) & wxSTC_FOLDLEVELHEADERFLAG && 
                  GetFoldParent( sl ) == -1 ) {

               children = true;
               if ( GetFoldExpanded( sl ) ) {
                  expanded = true;
                  break;
               }
            }
         }

         event.SetText( children && expanded ? "Collapse Definitions" : "Expand Definitions" );
         event.Enable( children );
         break;
      }
   }
}

void ScriptCtrl::OnNeedUnfold( wxStyledTextEvent& event )
{
   int sl = LineFromPosition( event.GetPosition() );
   int el = LineFromPosition( event.GetPosition() + event.GetLength() );

   for ( ; sl <= el; sl++ ) {

      if ( GetLineVisible( sl ) && GetFoldExpanded( sl ) )
         continue;

      // Recursively show parents.
      int pl = GetFoldParent( sl );
      if ( pl == -1 )
         ToggleFold( sl );
      else
         ToggleFold( pl );   
   }
}

void ScriptCtrl::OnBreakpointEvent( BreakpointEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   wxASSERT( tsGetMainFrame()->GetProjectDoc() );

   const Breakpoint* bp = event.GetBreakpoint();
   wxASSERT( bp );

   int type = MARKER_BREAKPOINT_DISABLED;
   if ( bp->GetEnabled() ) {

      if ( bp->HasCondition() )
         type = MARKER_CONDITIONAL_BREAKPOINT;
      else
         type = MARKER_BREAKPOINT;
   }

   // Remove existing markers.
   MarkerDelete( bp->GetLine()-1, MARKER_BREAKPOINT );
   MarkerDelete( bp->GetLine()-1, MARKER_CONDITIONAL_BREAKPOINT );
   MarkerDelete( bp->GetLine()-1, MARKER_BREAKPOINT_DISABLED );

   if ( event.GetType() == BreakpointEvent::TYPE_ADD )
      MarkerAdd( bp->GetLine()-1, type );

   else if ( event.GetType() == BreakpointEvent::TYPE_REMOVE ) {

      // NOTHING... we've done it.
   }

   else if ( event.GetType() == BreakpointEvent::TYPE_MOVED ) {

      MarkerDelete( event.GetOldLine()-1, MARKER_BREAKPOINT );
      MarkerDelete( event.GetOldLine()-1, MARKER_CONDITIONAL_BREAKPOINT );
      MarkerDelete( event.GetOldLine()-1, MARKER_BREAKPOINT_DISABLED );
      MarkerAdd( bp->GetLine()-1, type );
   }

   else {

      wxASSERT( event.GetType() == BreakpointEvent::TYPE_CHANGED );
      MarkerAdd( bp->GetLine()-1, type );
   }
}

void ScriptCtrl::OnRemoveBreakpoint( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project )
      return;

   const int start   = LineFromPosition( GetSelectionStart() ) + 1;
   const int end     = LineFromPosition( GetSelectionEnd() ) + 1;

   for ( int l=start; l <= end; l++ )
   {
      Breakpoint* TheBreakpoint = project->FindBreakpoint( m_FilePath, l );
	   if ( TheBreakpoint )
         project->DeleteBreakpoint( TheBreakpoint );
   }
}

void ScriptCtrl::OnInsertBreakpoint( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project )
      return;

   const int line = LineFromPosition( GetSelectionStart() ) + 1;
   project->AddBreakpoint( m_FilePath, line, 0, wxEmptyString );
}

void ScriptCtrl::OnToggleBreakpoint( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project )
      return;

   const int start   = LineFromPosition( GetSelectionStart() ) + 1;
   const int end     = LineFromPosition( GetSelectionEnd() ) + 1;

   bool disable = true;

   for ( int l=start; l <= end; l++ )
   {
      Breakpoint* bp = project->FindBreakpoint( m_FilePath, l );
      if ( bp )
      {
         disable = bp->GetEnabled();
         break;
      }
   }

   for ( int l=start; l <= end; l++ )
   {
      Breakpoint* bp = project->FindBreakpoint( m_FilePath, l );
      if ( bp )
         project->EnableBreakpoint( bp, !disable );
   }
}

void ScriptCtrl::OnBreakpointProperties( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( !project )
      return;

   const int start   = LineFromPosition( GetSelectionStart() ) + 1;
   const int end     = LineFromPosition( GetSelectionEnd() ) + 1;

   Breakpoint* bp = NULL;
   for ( int l=start; l <= end; l++ )
   {
      bp = project->FindBreakpoint( m_FilePath, l );
	   if ( bp ) 
         break;
   }

   if ( !bp ) 
      return;
   
   // Copy the breakpoint.
   Breakpoint copy( *bp );
   wxASSERT( tsGetMainFrame() );
   BreakpointPropertyDlg dlg;
   dlg.Create( tsGetMainFrame(), &copy );
   if ( dlg.ShowModal() == wxID_OK )
      project->ChangeBreakpoint( bp, &copy );
} 

void ScriptCtrl::OnKeyUp( wxKeyEvent& event )
{
   // Pass it thru.
   event.Skip();
}

void ScriptCtrl::OnFindNext( wxFindDialogEvent& event )
{
   if ( event.GetFindString().IsEmpty() ) 
      return;

   wxEventType type = event.GetEventType();

   wxASSERT( tsGetMainFrame() );
   tsGetMainFrame()->AddFindString( event.GetFindString() );

   wxASSERT(   type == wxEVT_COMMAND_FIND || 
               type == wxEVT_COMMAND_FIND_NEXT );

   if ( type == wxEVT_COMMAND_FIND ) 
   {
      SetAnchor( GetSelectionStart() );
      SetCurrentPos( GetSelectionStart() );
   }
   else
   {
      int start = GetSelectionStart();
      int end = GetSelectionEnd();
      if ( start != end ) 
      {
         if ( event.GetFlags() & wxFR_DOWN ) 
         {
            SetAnchor( start + 1 );
            SetCurrentPos( start + 1 );
         }
         else 
         {
            SetAnchor( start - 1 );
            SetCurrentPos( start - 1 );
         }
      }
   }

   bool newFind = false;
   if ( m_LastFindPos != GetCurrentPos() )
   {
      m_StartFindPos = -1;
      m_LastFindPos = -1;
      newFind = true;
   }

   SearchAnchor();

   int found = -1;
   if ( event.GetFlags() & wxFR_DOWN ) 
   {
      found = SearchNext( 
         ( event.GetFlags() & wxFR_WHOLEWORD ? wxSTC_FIND_WHOLEWORD : 0 ) |
         ( event.GetFlags() & wxFR_MATCHCASE ? wxSTC_FIND_MATCHCASE : 0 ), 
         event.GetFindString() );
   }
   else
   {
      found = SearchPrev( 
         ( event.GetFlags() & wxFR_WHOLEWORD ? wxSTC_FIND_WHOLEWORD : 0 ) |
         ( event.GetFlags() & wxFR_MATCHCASE ? wxSTC_FIND_MATCHCASE : 0 ), 
         event.GetFindString() );
   }

   if ( found != -1 ) 
   {
      int line = LineFromPosition( found );

      if ( found == m_StartFindPos )
      {
         tsGetMainFrame()->SetStatusText( "Find reached starting point of the search." );
         tsBellEx( wxICON_INFORMATION );
      }
      else
      {
         wxString msg;
         msg << "Found \"" << event.GetFindString() << "\" at line " << (line+1) << ".";
         tsGetMainFrame()->SetStatusText( msg );
      }

      SetLineVisible( line );
   }
   else
   {
      // Do the wrap around.
      int pos = GetCurrentPos();

      if ( event.GetFlags() & wxFR_DOWN )
      {
         SetAnchor( 0 );
         SetCurrentPos( 0 );
         SearchAnchor();

         found = SearchNext( 
            ( event.GetFlags() & wxFR_WHOLEWORD ? wxSTC_FIND_WHOLEWORD : 0 ) |
            ( event.GetFlags() & wxFR_MATCHCASE ? wxSTC_FIND_MATCHCASE : 0 ), 
            event.GetFindString() );
      }
      else 
      {
         SetAnchor( GetLength()-1 );
         SetCurrentPos( GetLength()-1 );
         SearchAnchor();

         found = SearchPrev( 
            ( event.GetFlags() & wxFR_WHOLEWORD ? wxSTC_FIND_WHOLEWORD : 0 ) |
            ( event.GetFlags() & wxFR_MATCHCASE ? wxSTC_FIND_MATCHCASE : 0 ), 
            event.GetFindString() );
      }

      if ( found != -1 )
      {
         int line = LineFromPosition( found );

         if ( found == m_StartFindPos )
         {
            tsGetMainFrame()->SetStatusText( "Find reached starting point of the search." );
            tsBellEx( wxICON_INFORMATION );
         }
         else
         {
            wxString msg;
            msg << "Found \"" << event.GetFindString() << "\" at line " << (line+1) << ".";
            tsGetMainFrame()->SetStatusText( msg );
         }

         SetLineVisible( line );
      }
      else
      {
         SetAnchor( pos );
         SetCurrentPos( pos );
         tsBellEx( wxICON_INFORMATION );
         tsGetMainFrame()->SetStatusText( "The specified text was not found." );
      }
   }

   if ( newFind )
      m_StartFindPos = found;

   if ( event.GetFlags() & wxFR_DOWN )
      m_LastFindPos = found + 1;
   else
      m_LastFindPos = found - 1;
}

void ScriptCtrl::OnFindReplace( wxFindDialogEvent& event )
{
   if ( event.GetFindString().IsEmpty() ) 
      return;
   
   // If the current selection matches the find 
   // string then replace it.
   wxString what = GetTextRange( GetSelectionStart(), GetSelectionEnd() );
   if ( what.IsSameAs( event.GetFindString(), event.GetFlags() & wxFR_MATCHCASE ? true : false ) )
   {
      ReplaceSelection( event.GetReplaceString() );
   }

   // Look for the next selection.
   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_NEXT );
   fevent.SetFindString( event.GetFindString() );
   fevent.SetFlags( event.GetFlags() );
   OnFindNext( fevent );
}

void ScriptCtrl::OnFindReplaceAll( wxFindDialogEvent& event )
{
   wxPoint scroll( GetXOffset(), GetFirstVisibleLine() );

   BeginUndoAction();

   SetAnchor( 0 );
   SetCurrentPos( 0 );
   SearchAnchor();

   int replaced = 0;

   for ( ;; )
   {
      int found = SearchNext(
            ( event.GetFlags() & wxFR_WHOLEWORD ? wxSTC_FIND_WHOLEWORD : 0 ) |
            ( event.GetFlags() & wxFR_MATCHCASE ? wxSTC_FIND_MATCHCASE : 0 ), 
            event.GetFindString() );

      if ( found == -1 )
         break;

      ReplaceSelection( event.GetReplaceString() );

      ++replaced;

      found = GetSelectionEnd();
      SetAnchor( found + 1 );
      SetCurrentPos( found + 1 );
      SearchAnchor();
   }

   EndUndoAction();

   SetXOffset( scroll.x );
   LineScroll( 0, -GetLineCount() );
   LineScroll( 0, scroll.y );

   wxString msg;
   msg << replaced << " occurrence(s) replaced.";
   tsGetMainFrame()->SetStatusText( msg );
   tsBellEx( wxICON_INFORMATION );
}

void ScriptCtrl::OnContextMenu( wxContextMenuEvent& event )
{
   SetFocus();

   wxPoint pt = event.GetPosition();
   ScreenToClient( &pt.x, &pt.y );
   if ( !GetClientRect().Contains( pt ) )
   {
      // Gotta skip else the MDIDocChildFrame won't
      // get it's context menu event.
      event.Skip(); 
      return;
   }

   const int clickPos = PositionFromPoint( pt );
   const int clickLine = LineFromPosition( clickPos );
   if (  GetSelectionStart() == GetSelectionEnd() ||
         clickPos < GetSelectionStart() ||
         clickPos > GetSelectionEnd() ) {

      SetAnchor( clickPos );
      SetCurrentPos( clickPos );
   }

   // Add the menu standards.
   tsMenu* menu = new tsMenu;
   //AppendWithIcon( menu, wxID_CLOSE,   _T( "C&lose" ) );
   //menu->AppendSeparator();
   menu->AppendIconItem( wxID_CUT,     _T( "Cu&t" ), ts_cut_clipboard16 );
   menu->AppendIconItem( wxID_COPY,    _T( "&Copy" ), ts_copy_clipboard16 );
   menu->AppendIconItem( wxID_PASTE,   _T( "&Paste" ), ts_paste_clipboard16 );

   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();

   m_ExecPath.Clear();

   // Do we need to have breakpoint options?
   if ( project ) 
   {
      menu->AppendSeparator();

      Breakpoint* bp = NULL;
      const int start   = LineFromPosition( GetSelectionStart() ) + 1;
      const int end     = LineFromPosition( GetSelectionEnd() ) + 1;
      for ( int l=start; l <= end; l++ )
      {
         bp = project->FindBreakpoint( m_FilePath, l );
	      if ( bp ) 
            break;
      }

      if ( bp ) 
      {
         menu->AppendIconItem( tsID_SCRIPT_REMOVEBREAKPOINT, _T( "Remo&ve Breakpoint" ), ts_breakhand16 );

         if ( bp->GetEnabled() )
            menu->AppendIconItem( tsID_SCRIPT_TOGGLEBREAKPOINT, _T( "&Disable Breakpoint" ), ts_breaktoggle16 );
         else 
            menu->AppendIconItem( tsID_SCRIPT_TOGGLEBREAKPOINT, _T( "Enab&le Breakpoint" ), ts_breaktoggle16 );

         menu->AppendIconItem( tsID_BREAKPOINT_PROPERTIES, _T( "&Breakpoint Properties..." ), ts_breakproperties16 );
      }
      else
         menu->AppendIconItem( tsID_SCRIPT_INSERTBREAKPOINT, _T( "Insert B&reakpoint" ), ts_breakhand16 );

      //AppendWithIcon( menu, tsID_DEBUG_NEWBREAKPOINT, _T( "New Breakpoint..." ) );
   }

   // Did we click on an exec line?
   wxRegEx expr( "\\mexec[ \t]*[(][ \t]*[\"]([~]?)([A-Za-z.\\\\/]*)[\"][ \t]*[)]", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( expr.IsValid() );
   wxString line = GetLine( clickLine );
   if ( expr.Matches( line ) ) 
   {
      bool tilde = !expr.GetMatch( line, 1 ).IsEmpty();
      if ( !tilde ) 
      {
         m_ExecPath = expr.GetMatch( line, 2 );

         wxString text;
         text << "Open \"" << m_ExecPath << "\"";
         menu->AppendSeparator();
         menu->Append( tsID_SCRIPT_OPENEXEC, text );

      } 
      else if ( project ) 
      {

         m_ExecPath = expr.GetMatch( line, 2 );

         wxString text;
         text << "Open \"" << '~' << m_ExecPath << "\"";
         menu->AppendSeparator();
         menu->Append( tsID_SCRIPT_OPENEXEC, text );

         // Ok the tilde means the current script's mod folder
         // is the root of this document.
         wxString modPath = project->GetModPath( m_FilePath );
         m_ExecPath.Clear();
         m_ExecPath << "." << expr.GetMatch( line, 2 );
         wxFileName execPath( m_ExecPath );
         execPath.MakeAbsolute( modPath );
         m_ExecPath = execPath.GetFullPath();
      }
   } 
   else 
   {
      wxString word = GetIdentifierAt( PositionFromPoint( pt ) );
      if ( !word.IsEmpty() ) 
      {
         wxASSERT( tsGetAutoComp() );
         const AutoCompData* data = tsGetAutoComp()->Lock();

         m_DefPaths.Clear();
         m_DefLines.Clear();
         m_DefSymbols.Clear();
         data->GetDefinitionFileAndLine( word, &m_DefPaths, &m_DefLines, &m_DefSymbols );

         tsGetAutoComp()->Unlock();
         
         bool addSep = true;

         if ( tsGetDebugger()->IsRunning() )
         {
            m_WatchText = word;
            menu->AppendSeparator();
            menu->Append( tsID_ADD_WATCH, _T( "Add &Watch" ) );
            addSep = false;
         }

         if ( !m_DefPaths.IsEmpty() ) 
         {
            wxString text;
            text << "Go To \"" << word << "\" Definition";

            if ( addSep )
               menu->AppendSeparator();
            menu->Append( tsID_SCRIPT_OPENDEF, text );
         }
      }
   }

   if ( tsGetDebugger()->IsRunning() )
   {
      menu->AppendSeparator();
      menu->AppendIconItem( tsID_MOVE_INSTPTR, _T( "Set &Next Statement" ), ts_currentline16 );
      // TODO: Run to cursor (aka temporary invisible breakpoint).
      menu->Append( tsID_RELOAD_SCRIPT, _T( "&Reload Script" ) );
   }

   menu->AppendSeparator();
   tsMenu* folding = new tsMenu;
   folding->Append( tsID_FOLDING_TOGGLEBLOCK, "Collapse Block" );
   folding->Append( tsID_FOLDING_TOGGLEALLINBLOCK, "Collapse All In Block" );
   //folding->AppendSeparator();
   folding->Append( tsID_FOLDING_TOGGLEALL, "Expand All" );
   folding->Append( tsID_FOLDING_COLLAPSETODEFS, "Collapse To Definitions" );
   menu->Append( wxID_ANY, "Folding", folding );

   if ( wxFileExists( m_FilePath ) ) 
   {
      menu->AppendSeparator();
      menu->Append( wxID_PROPERTIES, "&Properties" );
   }

   //AppendWithIcon( menu, wxID_SAVE, "&Save", save16_xpm );
   /*
   menu->AppendSeparator();
   AppendWithIcon( menu, wxID_SAVE, "&Save", save16_xpm );
   AppendWithIcon( menu, tsID_CLEARDSO, _T( "&Delete DSO" ) );
   AppendWithIcon( menu, wxID_CLOSE,   _T( "&Close" ) );
   */

   CallTipCancel();
   AutoCompCancel();
   PopupMenu( menu, pt );
   delete menu;
}

void ScriptCtrl::OnProperties( wxCommandEvent& event )
{
   tsExecuteVerb( m_FilePath, "properties" );
}

void ScriptCtrl::OnOpenExec( wxCommandEvent& event )
{
   wxASSERT( !m_ExecPath.IsEmpty() );

   wxFileName path( m_ExecPath );
   if ( !path.IsAbsolute() ) {

      wxFileName curDir( m_FilePath );
      path.MakeAbsolute( curDir.GetPath() );
   }

   wxASSERT( tsGetMainFrame() );
   tsGetMainFrame()->OpenFile( path.GetFullPath() );
}

void ScriptCtrl::OnOpenDef( wxCommandEvent& event )
{
   wxASSERT( !m_DefPaths.IsEmpty() );
   wxASSERT( m_DefPaths.GetCount() == m_DefLines.GetCount() );
   wxASSERT( m_DefPaths.GetCount() == m_DefSymbols.GetCount() );

   wxString path = m_DefPaths[0];
   int line = m_DefLines[0];
   if ( m_DefPaths.GetCount() > 1 )
   {
      GotoDefDialog dlg( this );
      if ( dlg.ShowModal( m_DefPaths, m_DefLines, m_DefSymbols ) != wxID_OK )
         return;

      path = dlg.GetPath();
      line = dlg.GetLine();
   }

   wxASSERT( tsGetMainFrame() );
   tsGetMainFrame()->OpenFile( path, line );
}

void ScriptCtrl::OnAddWatch( wxCommandEvent& event )
{
   wxASSERT( !m_WatchText.IsEmpty() );

   wxASSERT( tsGetMainFrame() );
   WatchCtrl* watchCtrl = tsGetMainFrame()->GetWatchWindow();
   wxASSERT( watchCtrl );

   watchCtrl->AddVar( m_WatchText );
}

void ScriptCtrl::ClearBreakline()
{
	MarkerDeleteAll( MARKER_BREAKLINE );
}

void ScriptCtrl::SetBreakline( int Line )
{
   // The line should be one based... fix that.
   --Line;

   // Check and fix invalid lines!
   if ( Line < 0 )
   {
      Line = 0;
      wxBell();
   }
   else if ( Line >= GetLineCount() )
   {
      Line = GetLineCount() - 1;
      wxBell();
   }

   wxASSERT( Line >= 0 );
	wxASSERT( Line < GetLineCount() );

	// A sanity check here... we should never get a 
	// break line that isn't in our chunk!
	//if ( m_CodeBlock ) {
		//wxASSERT( m_CodeBlock->GetNearestBreakline( Line ) == Line );
	//}

	// Clear any other breakline marker in this file.
	ClearBreakline();

	// Scroll to the line!
   SetLineVisible( Line );

   // Move the cursor position to the break line.
   int pos = PositionFromLine( Line );
   SetAnchor( pos );
   SetCurrentPos( pos );
   SetFocus();

   // Add the marker!
	MarkerAdd( Line, MARKER_BREAKLINE );
}

void ScriptCtrl::OnSetFocus( wxFocusEvent& event )
{
   CallTipCancel();
   AutoCompCancel();

   wxASSERT( m_ScriptView );
   m_ScriptView->Activate( true );
   
   event.Skip();
}

void ScriptCtrl::OnKillFocus( wxFocusEvent& event )
{
   CallTipCancel();
   AutoCompCancel();

   wxASSERT( m_ScriptView );
   m_ScriptView->Activate( false );

   event.Skip();
}

void ScriptCtrl::OnMouseWheel( wxMouseEvent& evt ) 
{
   CallTipCancel();
   AutoCompCancel();

   if ( evt.ControlDown() ) {
      evt.m_linesPerAction = UINT_MAX;
      evt.m_controlDown = false;
   }
   evt.Skip();
}

void ScriptCtrl::OnMouseLeftDown( wxMouseEvent& evt )
{
   // Ctrl click word selection occurs if you click on
   // a non-whitespace, non-selected, area on mouse down.
   //
   // TODO: The behavior here is still not correct.  I should
   // fix this in the Scintilla Editor:: code.  See bug...
   //
   // http://mantis.sickheadgames.com/view.php?id=719
   //
   if ( evt.ControlDown() ) 
   {
      int pos = PositionFromPoint( evt.GetPosition() );
      int start = WordStartPosition( pos, true );
      int end = WordEndPosition( start, false );

      wxChar c = GetCharAt(start);
      if (  start != end && !wxIsspace( c ) && 
            !( pos > GetSelectionStart() && pos < GetSelectionEnd() ) )
      {
         SetFocus();
         SetSelection( start, end );
         return;
      }
   }
   
   evt.Skip();
}

/*
void ScriptCtrl::OnMouseLeftUp( wxMouseEvent& evt )
{
   // On mouse up we always apply the ctrl click selection.
   if ( evt.ControlDown() ) 
   {
      int pos = PositionFromPoint( evt.GetPosition() );
      int start = WordStartPosition( pos, true );
      int end = WordEndPosition( start, false );

      wxChar c = GetCharAt(start);
      if (  start != end && !wxIsspace( c ) && 
            ( GetSelectionStart() != start || GetSelectionEnd() != end ) )
      {
         SetFocus();
         SetSelection( start, end );
         return;
      }
   }
   
   evt.Skip();
}
*/

void ScriptCtrl::UpdatePrefs( bool refresh )
{
   //ClearDocumentStyle();

   // Go ahead and set the default font on everything
   const wxFont& defaultFont = tsGetPrefs().GetDefaultFont();
   for ( int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; i++ )
   {
      StyleSetFontAttr( i, 
         defaultFont.GetPointSize(), 
         defaultFont.GetFaceName(), 
         defaultFont.GetWeight() == wxBOLD, 
         defaultFont.GetStyle() != wxNORMAL,
         defaultFont.GetUnderlined(), 
         defaultFont.GetEncoding() );
   }

   // The default font is used for GUI things.
   StyleSetFontAttr( wxSTC_STYLE_DEFAULT, 8, "Tahoma", false, false, false, wxFONTENCODING_DEFAULT );

   StyleSetBackground( wxSTC_STYLE_DEFAULT, tsGetPrefs().GetBgColor() );
   SetCaretForeground( tsGetPrefs().GetDefaultColor() );

   // Set the colors for the TorqueScript types.
   SetStyle( wxSTC_TCS_DEFAULT, tsGetPrefs().GetDefaultColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_COMMENT, tsGetPrefs().GetCommentColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_COMMENTLINEDOC, tsGetPrefs().GetCommentColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_COMMENTLINE, tsGetPrefs().GetCommentColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_NUMBER, tsGetPrefs().GetNumberColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_STRING, tsGetPrefs().GetStringColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_CHARACTER, tsGetPrefs().GetStringColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_IDENTIFIER, tsGetPrefs().GetDefaultColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_OPERATOR, tsGetPrefs().GetOperatorsColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_VAR, tsGetPrefs().GetLocalsColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_GLOBALVAR, tsGetPrefs().GetGlobalsColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_STRINGEOL, tsGetPrefs().GetBraceMatchErrColor(), tsGetPrefs().GetBraceMatchBgColor() );
   SetStyle( wxSTC_TCS_WORD, tsGetPrefs().GetReservedColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_WORD2, tsGetPrefs().GetExportsColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_GLOBALCLASS, tsGetPrefs().GetExportsColor(), tsGetPrefs().GetBgColor() );
   SetStyle( wxSTC_TCS_FOLDED, tsGetPrefs().GetDefaultColor(), tsGetPrefs().GetMarginColor() );

   // Set the keywords up... note that the functions and var words
   // are fetched from the autocomp system which builds these from 
   // the engine exports only available if your project is setup
   // generate them.  If not... no exports colored for you!
   SetKeyWords( 0, tsGetPrefs().GetReservedWords() );
   wxASSERT( tsGetAutoComp() );
   SetKeyWords( 1, tsGetAutoComp()->GetExportedFunctionsString() );
   SetKeyWords( 2, tsGetAutoComp()->GetExportedVarsString()  );

   // Not sure what this is used for really, but 
   // we use it for the ellipsis on code folding.
   StyleSetForeground( wxSTC_STYLE_INDENTGUIDE, tsGetPrefs().GetFoldingColor() ); 
   StyleSetFontAttr( wxSTC_STYLE_INDENTGUIDE, 
      defaultFont.GetPointSize(), 
      defaultFont.GetFaceName(), 
      true, // wxFONTWEIGHT_BOLD
      defaultFont.GetStyle() != wxNORMAL,
      defaultFont.GetUnderlined(), 
      defaultFont.GetEncoding() );

   // set common styles
   StyleSetForeground( wxSTC_STYLE_BRACELIGHT, tsGetPrefs().GetBraceMatchColor() ); // 49, 106, 0
   StyleSetBackground( wxSTC_STYLE_BRACELIGHT, tsGetPrefs().GetBraceMatchBgColor() );
   StyleSetForeground( wxSTC_STYLE_BRACEBAD, tsGetPrefs().GetBraceMatchErrColor() );
   StyleSetBackground( wxSTC_STYLE_BRACEBAD, tsGetPrefs().GetBraceMatchBgColor() );

   SetSelBackground( 1, tsGetPrefs().GetSelBgColor() );
   SetSelForeground( 1, tsGetPrefs().GetSelColor() );

   // Call tips.
   CallTipSetForeground( tsGetPrefs().GetCalltipColor() );
   CallTipSetBackground( tsGetPrefs().GetCalltipBgColor() );
   CallTipSetForegroundHighlight( tsGetPrefs().GetCalltipHiColor() );

   // Setup column edge and wrap mode.
   SetEdgeMode( tsGetPrefs().GetEdgeMarker() ? wxSTC_EDGE_LINE : wxSTC_EDGE_NONE );
   SetEdgeColumn( tsGetPrefs().GetEdgeMarkerCol() );
   SetEdgeColour( tsGetPrefs().GetEdgeMarkerColor() );

   StyleSetForeground( wxSTC_STYLE_LINENUMBER, tsGetPrefs().GetMarginTextColor() );
   StyleSetBackground( wxSTC_STYLE_LINENUMBER, tsGetPrefs().GetMarginColor() );

   SetFoldMarginColour( true,   tsGetPrefs().GetBgColor() );
   SetFoldMarginHiColour( true, tsGetPrefs().GetBgColor() );

   if ( tsGetPrefs().GetCodeFolding() )
   {
      SetProperty( "fold", "1" );
      Colourise( 0, GetEndStyled() );
   }
   else
   {
      for ( int i=0; i < GetLineCount(); i++ )
      {
         if ( GetFoldLevel( i ) & wxSTC_FOLDLEVELHEADERFLAG && !GetFoldExpanded( i ) )
            ToggleFold( i );

         SetFoldLevel( i, 0 );
      }
      SetProperty( "fold", "0" );
   }

   IndicatorSetForeground( 2, tsGetPrefs().GetFoldingColor() ); 

   MarkerDefine( wxSTC_MARKNUM_FOLDER,          wxSTC_MARK_BOXPLUS );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDER,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDER,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDEROPEN,          wxSTC_MARK_BOXMINUS );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPEN,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPEN,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDEREND,          wxSTC_MARK_BOXPLUSCONNECTED );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDEREND,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDEREND,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDERMIDTAIL,          wxSTC_MARK_TCORNER );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDERMIDTAIL,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDERMIDTAIL,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDEROPENMID,          wxSTC_MARK_BOXMINUSCONNECTED );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDEROPENMID,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDEROPENMID,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDERSUB,          wxSTC_MARK_VLINE );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDERSUB,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDERSUB,   tsGetPrefs().GetBgColor() );

   MarkerDefine( wxSTC_MARKNUM_FOLDERTAIL,          wxSTC_MARK_LCORNER );
   MarkerSetBackground( wxSTC_MARKNUM_FOLDERTAIL,   tsGetPrefs().GetFoldingColor() );
   MarkerSetForeground( wxSTC_MARKNUM_FOLDERTAIL,   tsGetPrefs().GetBgColor() );

   // Set spaces and indention
   SetTabWidth( tsGetPrefs().GetTabWidth() );
   SetIndent( tsGetPrefs().GetTabWidth() );
   SetUseTabs( tsGetPrefs().GetUseTabs() );
   SetTabIndents( 0 );
   SetBackSpaceUnIndents(true);

   ShowLineNumbers( tsGetPrefs().GetLineNumbers() );

   static const wxString sEnhancedFillups( "!\"#&'()*+,-./:;<=>?@[\\]^`{|}~" );
   static const wxString sEnhancedGlobalFillups( "!\"#&'()*+,-./;<=>?@[\\]^`{|}~" );

   if ( tsGetPrefs().GetEnhancedCompletion() )
   {
      m_AutoCompFillups = sEnhancedFillups;
      m_AutoCompGlobalFillups = sEnhancedGlobalFillups;
   }
   else
      m_AutoCompFillups = m_AutoCompGlobalFillups = wxEmptyString;

   AutoCompSetFillUps( m_AutoCompFillups );
   m_IsGlobalFullups = false;
}

void ScriptCtrl::CommentSel( bool comment )
{
   int start   = LineFromPosition(GetSelectionStart());
   int insert  = GetSelectionEnd();
   int last    = LineFromPosition(insert-1);
   if ( last < start )
      last = start;

   BeginUndoAction();

   for ( int l=start; l <= last; l++ )
   {
      wxString text = GetLine(l);

      // Seperate out the whitespace.
      wxString white;
      while ( text[0] == ' ' || text[0] == '\t' )
      {
         white << text[0];
         text.Remove( 0, 1 );
      }

      if ( comment )
      {
         text = white + "//" + text; 
      }
      else
      {
         if ( text.StartsWith( "//" ) )
            text.Remove( 0, 2 );    
         text.Prepend( white );
      }

      int pos = PositionFromLine( l );
      int end = PositionFromLine( l+1 );

      SetSelectionStart( pos );
      SetSelectionEnd( end );

      ReplaceSelection( text );
   }

   if ( start != last )
   {
      int pos = PositionFromLine( start );
      int end = GetLineEndPosition( last );
      SetSelectionStart(pos);
      SetSelectionEnd(end);
   }
   else
   {
      int pos = insert + ( comment ? 2 : -2 );
      SetSelectionStart( pos );
      SetSelectionEnd( pos );
   }

   EndUndoAction();
}

bool ScriptCtrl::MatchBrace()
{
   int pos = GetCurrentPos();
   int match = BraceMatch( pos );
   if ( match == -1 )
   {
      match = BraceMatch( pos-1 );
      if ( match == -1 )
         return false;
   }

   SetCurrentPos( match );
   SetAnchor( match );
   int line = LineFromPosition( match );
   SetLineVisible ( line );
   ShowCaretAtCurrentPosition();
   return true;
}

void ScriptCtrl::ShowCaretAtCurrentPosition()
{
   // The trick here is that GotoLine will
   // turn on the carat for us.
   int pos = GetCurrentPos();
   int line = LineFromPosition( pos );
   GotoLine( line );
   SetCurrentPos( pos );
   SetAnchor( pos );
}

