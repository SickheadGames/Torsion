// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScriptView.h"

#include "TorsionApp.h"
#include "ScriptCtrl.h"
#include "ScriptPrintout.h"
#include "MainFrame.h"
#include "ProjectView.h"
#include "ReloadFile.h"
#include "Breakpoint.h"
#include "Bookmark.h"
#include "Debugger.h"
#include "Platform.h"
#include "AutoComp.h"
#include "tsToolBar.h"
#include "Icons.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

#include <wx/clipbrd.h>
#include <wx/file.h>
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ScriptViewList);

ScriptViewList ScriptView::s_ScriptViews;

IMPLEMENT_CLASS(tsFileRenameHint, wxObject)

IMPLEMENT_DYNAMIC_CLASS( ScriptView, wxView )

BEGIN_EVENT_TABLE( ScriptView, wxView )

   EVT_MENU(tsID_FINDNEXT, ScriptView::OnFindNext)
   EVT_TEXT_ENTER(tsID_FINDCOMBO, ScriptView::OnFindNext)
   EVT_MENU(tsID_FINDPREV, ScriptView::OnFindNext)

   EVT_MENU(tsID_RELOAD_SCRIPT, ScriptView::OnReloadScript)
   EVT_UPDATE_UI( tsID_RELOAD_SCRIPT, ScriptView::OnUpdateReloadScript )
   EVT_MENU(tsID_MOVE_INSTPTR, ScriptView::OnMoveInstPtr)
   EVT_UPDATE_UI( tsID_MOVE_INSTPTR, ScriptView::OnUpdateMoveInstPtr )

   EVT_MENU(tsID_PROJECT_OPENWITH, ScriptView::OnOpenWith)
   EVT_MENU(tsID_EXPLORE, ScriptView::OnExplore)
   EVT_UPDATE_UI( tsID_PROJECT_OPENWITH, ScriptView::OnUpdateFileExists )
   EVT_UPDATE_UI( tsID_EXPLORE, ScriptView::OnUpdateFileExists )

   EVT_MENU(tsID_COPY_FULL_PATH, ScriptView::OnCopyFullPath)
   EVT_UPDATE_UI(tsID_COPY_FULL_PATH, ScriptView::OnUpdateEnabled)

   EVT_MENU(tsID_LINEBREAKS, ScriptView::OnLineBreaks)
   EVT_MENU(tsID_TABSANDSPACES, ScriptView::OnTabsAndSpaces)
   EVT_MENU(tsID_LINENUMBERS, ScriptView::OnLineNumbers)
   EVT_MENU(tsID_LINEWRAP, ScriptView::OnLineWrap)
   EVT_UPDATE_UI(tsID_LINEBREAKS, ScriptView::OnUpdateLineBreaks)
   EVT_UPDATE_UI(tsID_TABSANDSPACES, ScriptView::OnUpdateTabsAndSpaces)
   EVT_UPDATE_UI(tsID_LINENUMBERS, ScriptView::OnUpdateLineNumbers)
   EVT_UPDATE_UI(tsID_LINEWRAP, ScriptView::OnUpdateLineWrap)

   EVT_MENU(wxID_CUT, ScriptView::OnCut)
   EVT_MENU(wxID_COPY, ScriptView::OnCopy)
   EVT_MENU(wxID_PASTE, ScriptView::OnPaste)
   EVT_MENU(wxID_UNDO, ScriptView::OnUndo)
   EVT_MENU(wxID_REDO, ScriptView::OnRedo)
   EVT_MENU(wxID_CLEAR, ScriptView::OnDelete)
   EVT_MENU(wxID_DELETE, ScriptView::OnDelete)
   EVT_MENU(wxID_SELECTALL, ScriptView::OnSelectAll)
   EVT_UPDATE_UI(wxID_CUT, ScriptView::OnUpdateHasSelection)
   EVT_UPDATE_UI(wxID_COPY, ScriptView::OnUpdateHasSelection)
   EVT_UPDATE_UI(wxID_PASTE, ScriptView::OnUpdateHasSelection)
   EVT_UPDATE_UI(wxID_UNDO, ScriptView::OnUpdateUndo)
   EVT_UPDATE_UI(wxID_REDO, ScriptView::OnUpdateRedo)
   EVT_UPDATE_UI(wxID_CLEAR, ScriptView::OnUpdateHasSelection)
   EVT_UPDATE_UI(wxID_DELETE, ScriptView::OnUpdateHasSelection)
   EVT_UPDATE_UI(wxID_SELECTALL, ScriptView::OnUpdateSelectAll)

   EVT_MENU(tsID_COMMENT_SELECTION, ScriptView::OnCommentSel)
   EVT_UPDATE_UI(tsID_COMMENT_SELECTION, ScriptView::OnUpdateHasSelection)
   EVT_MENU(tsID_UNCOMMENT_SELECTION, ScriptView::OnUncommentSel)
   EVT_UPDATE_UI(tsID_UNCOMMENT_SELECTION, ScriptView::OnUpdateHasSelection)
   EVT_MENU(tsID_INDENT_SELECTION, ScriptView::OnIndentSel)
   EVT_UPDATE_UI(tsID_INDENT_SELECTION, ScriptView::OnUpdateHasSelection)
   EVT_MENU(tsID_UNINDENT_SELECTION, ScriptView::OnUnindentSel)
   EVT_UPDATE_UI(tsID_UNINDENT_SELECTION, ScriptView::OnUpdateHasSelection)
   EVT_MENU(tsID_UPPER_SELECTION, ScriptView::OnUpperSel)
   EVT_UPDATE_UI(tsID_UPPER_SELECTION, ScriptView::OnUpdateHasSelection)
   EVT_MENU(tsID_LOWER_SELECTION, ScriptView::OnLowerSel)
   EVT_UPDATE_UI(tsID_LOWER_SELECTION, ScriptView::OnUpdateHasSelection)

   EVT_MENU(tsID_MATCH_BRACE, ScriptView::OnMatchBrace)
   EVT_UPDATE_UI(tsID_MATCH_BRACE, ScriptView::OnUpdateHasSelection)

   EVT_BREAKPOINT_EVENTS( ScriptView::OnBreakpointEvent )

   EVT_TIMER( wxID_ANY, ScriptView::OnUpdateCombos )
   EVT_COMMAND(wxID_ANY, tsEVT_AUTOCOMPDATA_UPDATED, ScriptView::OnAutoCompUpdated )
   EVT_COMBOBOX(wxID_HIGHEST, ScriptView::OnUpdateMembers)
   EVT_COMBOBOX(wxID_HIGHEST+1, ScriptView::OnSelectMember)
   //EVT_TEXT(wxID_HIGHEST+1, ScriptView::OnSelectMember)
   //EVT_TEXT_ENTER(wxID_HIGHEST+1, ScriptView::OnSelectMember)

END_EVENT_TABLE()


ScriptView::ScriptView()
   :  wxView(),
      m_ScriptCtrl( NULL ),
      m_Types( NULL ),
      m_Members( NULL ),
      m_AutoCompUpdated( false ),
      m_LastLine( -1 )
{
   wxASSERT( !s_ScriptViews.Find( this ) );
   s_ScriptViews.Append( this );

   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->AddNotify( this );
}

ScriptView::~ScriptView()
{
   wxASSERT( s_ScriptViews.Find( this ) );
   s_ScriptViews.Erase( s_ScriptViews.Find( this ) );
   wxASSERT( !s_ScriptViews.Find( this ) );

   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->RemoveNotify( this );
}

bool ScriptView::OnCreate( wxDocument *doc, long WXUNUSED(flags) )
{
   ScriptFrame* frame = ScriptFrame::GetFrame( true );

   // Create parent panel to hold all the view controls.
   wxPanel* panel = new wxPanel;
   panel->Show( false );
   panel->Create( frame->GetViewControlParent(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN, wxEmptyString );
   wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
   panel->SetSizer(sizer);

   // Create a horizontal sizer to hold the types and members
   // dropdown boxes as well as a toolbar with a few buttons.
   wxBoxSizer* hsizer = new wxBoxSizer(wxHORIZONTAL);
   m_Types = new tsComboBox();
   m_Types->Create( panel, wxID_HIGHEST, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxSTATIC_BORDER | wxCB_DROPDOWN | wxCB_READONLY );
   m_Members = new tsComboBox();
   m_Members->Create( panel, wxID_HIGHEST+1, "", wxDefaultPosition, wxDefaultSize, 0, NULL, wxSTATIC_BORDER | wxCB_DROPDOWN | wxCB_READONLY );
   /*
   wxToolBar* toolbar = new tsToolBar( panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER|wxNO_BORDER );
   toolbar->SetToolPacking(0);
   toolbar->SetToolBitmapSize(wxSize(16, 16));
   toolbar->AddTool( tsID_RELOAD_SCRIPT, _T( "Reload" ), ts_refreshpage16, _T( "Reload" ) );
   toolbar->Realize();
   */
   hsizer->Add( m_Types, 1, wxEXPAND | wxRIGHT, 4 );
   hsizer->AddSpacer( 3 );
   hsizer->Add( m_Members, 1, wxEXPAND | wxRIGHT, 4 );
   //hsizer->Add( toolbar, 0, wxEXPAND );
   sizer->Add( hsizer, 0, wxEXPAND | wxBOTTOM, 2); 

   // Create the script editor control.
   m_ScriptCtrl = new ScriptCtrl;
   m_ScriptCtrl->Create( panel, this, wxDefaultPosition, wxDefaultSize );
   sizer->Add(m_ScriptCtrl, 1, wxEXPAND );

   // Make sure the script control is the first in the 
   // tab order so that it always gets the default focus.
   m_Types->MoveAfterInTabOrder( m_ScriptCtrl );
   m_Members->MoveAfterInTabOrder( m_Types );
   //toolbar->MoveAfterInTabOrder( m_Members );

   frame->AddScriptPage( this, panel );
   panel->Show( true );

   // Do an update the next idle cycle.
   m_AutoCompUpdated = true;
   m_UpdateCombosTimer.SetOwner( this, wxID_ANY );

   SetFrame( frame );
   wxASSERT( GetFrame() );

   //Activate( true );

   return true;
}

bool ScriptView::ProcessEvent( wxEvent& event )
{
   // Let the control deal with the event first... don't
   // let it propigate the event.
   if (  m_ScriptCtrl && 
         event.IsKindOf(CLASSINFO(wxCommandEvent)) &&
         (  event.GetEventType() == wxEVT_UPDATE_UI || 
            event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED ||
            event.GetEventType() == wxEVT_COMMAND_FIND ||
            event.GetEventType() == wxEVT_COMMAND_FIND_NEXT ||
            event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE ||
            event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL ||
            event.GetEventType() == EVT_COMMAND_BREAKPOINT ) )
   {
      wxPropagationDisabler disable( event );
      //TODO ? if ( m_ScriptCtrl->ProcessEvent( event ) )
	  if(ProcessEvent(event))
         return true;
   }

   // Now let the doc and the view handle it.
   return wxView::ProcessEvent( event );
}

void ScriptView::OnAutoCompUpdated( wxCommandEvent& event )
{
   // Just know that we updated... fire off the timer.
   m_AutoCompUpdated = true;
}

void ScriptView::OnUpdateCombos( wxTimerEvent& event )
{
   // Do we need to update the autocomp data?
   if ( m_AutoCompUpdated )
      UpdateTypeList();

   // This allows us to reselect the existing 
   // selection in the members combobox.
   m_Members->EnableReselection();

   // Did the cursor position change?
   wxASSERT( m_ScriptCtrl );
	int line = m_ScriptCtrl->LineFromPosition( m_ScriptCtrl->GetCurrentPos() );
   if ( m_LastLine == line )
      return;

   // Find out what lines we're between.
   bool foundMember = false;
   for ( int i=0; i < m_Functions.GetCount(); i++ )
   {
      int start = m_Lines[i];
      int end = m_EndLines[i];
      if ( line < start || line > end )
         continue;

      // Ok... select it.
      wxString func = m_Functions[i];
      int sep = func.Find( "::" );
      if ( sep == -1 )
      {
         if ( m_Types->GetSelection() != 0 )
         {
            m_Types->SetSelection( 0 );
            OnUpdateMembers( wxCommandEvent() );
         }
         m_Members->SetStringSelection( func );
         foundMember = true;
         break;
      }

      // Break out the type and select it.
      wxString type = func.Mid( 0, sep );
      if ( m_Types->GetStringSelection() != type )
      {
         m_Types->SetStringSelection( type );
         OnUpdateMembers( wxCommandEvent() );
      }

      wxString member = func.Mid( sep + 2 );
      m_Members->SetStringSelection( member );
      foundMember = true;
      break;
   }

   /*
   wxColour color;
   if ( foundMember )
      color = *wxBLACK;
   else
      color = *wxLIGHT_GREY;
   //m_Types->SetForegroundColour( color );
   //m_Members->SetForegroundColour( color );
   */

   m_LastLine = line;
}

void ScriptView::UpdateTypeList()
{
   // First grab the latest function data.
   if ( m_AutoCompUpdated )
   {
      wxASSERT( tsGetAutoComp() );
      m_Functions.Empty();
      m_Lines.Empty();
      m_EndLines.Empty();
      const AutoCompData* data = tsGetAutoComp()->Lock();
      if ( data )
      {
         data->BuildPageFunctions( GetDocument()->GetFilename(), m_Functions, m_Lines, m_EndLines );
         tsGetAutoComp()->Unlock();
      }

      m_AutoCompUpdated = false;
      m_LastLine = -1;
   }

   wxString typeSel = m_Types->GetStringSelection();

   m_Types->Freeze();
   m_Types->Clear();
   m_Types->Append( "(Globals)" );

   for ( int i=0; i < m_Functions.GetCount(); i++ )
   {
      const wxString& func = m_Functions[i];

      int sep = func.Find( "::" );
      if ( sep == -1 )
         continue;

       wxString type = func.Mid( 0, sep );
       if ( m_Types->FindString( type ) == wxNOT_FOUND )
         m_Types->Append( type );
   }

   m_Types->Enable( m_Types->GetCount() > 0 );

   if ( !typeSel.IsEmpty() )
      m_Types->SetStringSelection( typeSel );
   if ( m_Types->GetSelection() == wxNOT_FOUND )
      m_Types->SetSelection( 0 );

   m_Types->Thaw();

   OnUpdateMembers( wxCommandEvent() );
}

void ScriptView::OnUpdateMembers( wxCommandEvent& event )
{
   wxString typeSel = m_Types->GetStringSelection();
   wxString memberSel = m_Members->GetStringSelection();
   m_Members->Freeze();
   m_Members->Clear();

   if ( typeSel.IsEmpty() )
   {
      m_Members->Thaw();
      return;
   }

   if ( m_Types->GetSelection() == 0 )
   {
      for ( int i=0; i < m_Functions.GetCount(); i++ )
      {
         const wxString& func = m_Functions[i];

         if ( func.Find( "::" ) == -1 )
            m_Members->Append( func );
      }
   }
   else
   {
      for ( int i=0; i < m_Functions.GetCount(); i++ )
      {
         const wxString& func = m_Functions[i];

         int sep = func.Find( "::" );
         if ( sep == -1 )
            continue;
         wxString type = func.Mid( 0, sep );
         if ( type != typeSel )
            continue;

         wxString member = func.Mid( sep + 2 );
         m_Members->Append( member );
      }
   }

   m_Members->Enable( m_Members->GetCount() > 0 );

   if ( !memberSel.IsEmpty() )
      m_Members->SetStringSelection( memberSel );
   if ( m_Members->GetSelection() == wxNOT_FOUND )
      m_Members->SetSelection( 0 );

   m_Members->Thaw();
   m_Members->EnableReselection();
}

void ScriptView::OnSelectMember( wxCommandEvent& event )
{ 
   wxString member = m_Members->GetStringSelection();
   wxString function;
   if ( m_Types->GetSelection() > 0 )
      function = m_Types->GetStringSelection() + "::" + member;
   else
      function = member;

   int index = m_Functions.Index( function, false );
   if ( index == wxNOT_FOUND )
      return;
   
   // TODO: How do we allow the same item to be selected over and over?
   //m_Members->SetWindowStyle( wxSTATIC_BORDER | wxCB_DROPDOWN );
   //m_Members->SetValue( member );
   //m_Members->SetWindowStyle( wxSTATIC_BORDER | wxCB_DROPDOWN | wxCB_READONLY );

   int line = m_Lines[index];
   wxASSERT( m_ScriptCtrl );
	m_ScriptCtrl->SetLineSelected( line );
   m_ScriptCtrl->SetFocus();

}

void ScriptView::OnActivateView( bool activate, wxView* activeView, wxView* deactiveView )
{
   if ( !activate || activeView != this || !m_ScriptCtrl )
   {
      m_UpdateCombosTimer.Stop();
      return;
   }

   // Always make sure we get the focus!
   //m_ScriptCtrl->SetFocus();

   // Start the combo updater.
   m_UpdateCombosTimer.Start( 350, wxTIMER_CONTINUOUS );

   // If we don't have a file name then we've got nothing to do!
   if ( GetDocument()->GetFilename().IsEmpty() )
      return;

   // Make sure the project view selects us!
   ProjectView* projectView = tsGetMainFrame()->GetProjectView();
   if ( projectView )
      projectView->Select( GetDocument()->GetFilename() );

   // Look for a change in the read only state.
   bool readOnly =   wxFileExists( GetDocument()->GetFilename() ) && 
                     !wxFile::Access( GetDocument()->GetFilename(), wxFile::read_write );
   if ( m_ScriptCtrl->GetReadOnly() != readOnly ) {

      m_ScriptCtrl->SetReadOnly( readOnly );
      OnCtrlModifyed();
   }
}

int ScriptView::ReloadFile( int lastResult, bool force )
{
   if ( !force && !m_FileWatcher.IsSignaled( 0 ) )
      return lastResult;

   // Check to see if the file was deleted...
   wxFileName FilePath( GetDocument()->GetFilename() );
   if ( !FilePath.FileExists() )
      return lastResult;

   // Do we need to ask?
   if ( lastResult != wxID_NOTOALL && lastResult != wxID_YESTOALL  ) 
   {
      // To avoid recursion when we're called from outside
      // the message loop... we clear and restore the watcher.
      FileWatcher hold( m_FileWatcher );
      m_FileWatcher.Clear();

      // Ask the user if he wants to update it if
      // the last result isn't wxID_YESTOALL already.
      ReloadFileDlg dlg( tsGetMainFrame(), FilePath.GetFullPath() );
      lastResult = dlg.ShowModal();

      m_FileWatcher = hold;
   }

   // Do we do the reload now?
   if ( lastResult == wxID_YESTOALL || lastResult == wxID_YES )
   {
      GetDocument()->OnOpenDocument( FilePath.GetFullPath() );

      // Reset the notification for next time.
      //m_FileChange = FindFirstChangeNotification( FilePath.GetPath(), false, FILE_NOTIFY_CHANGE_LAST_WRITE );
      //m_LastWrite = FilePath.GetModificationTime();

      return lastResult;
   }

   // Force a reload of breakpoints... these should
   // be valid even if the file is not.
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->UpdateBreakpoints();

   // Mark the document as dirty and update the tab 
   // title... triggers a save on close if they allow it.
   GetDocument()->Modify( true );
   // m_ScriptCtrl->SetModify( true ); TODO this needs fixing!
   wxASSERT( GetFrame() );
   GetFrame()->SetTabState( this, true, m_ScriptCtrl->GetReadOnly() );

   return lastResult;
}

wxPrintout* ScriptView::OnCreatePrintout()
{
   wxString title;
   //TODO ? GetDocument()->GetPrintableName(title);
   title = GetDocument()->GetTitle();
   ScriptPrintout* printout = new ScriptPrintout( title, m_ScriptCtrl );
   return printout;
}

void ScriptView::OnDraw( wxDC *dc )
{
}

void ScriptView::OnUpdate( wxView *WXUNUSED(sender), wxObject* hint )
{
   if ( wxDynamicCast( hint, tsPrefsUpdateHint) )
   {
      wxASSERT( m_ScriptCtrl );
      m_ScriptCtrl->UpdatePrefs( true );
   }
   else if ( wxDynamicCast( hint, tsBookmarksUpdateHint) )
   {
      wxASSERT( m_ScriptCtrl );
      m_ScriptCtrl->UpdateBookmarks();
   }
   else if ( wxDynamicCast( hint, tsAutoCompStateHint ) )
   {
      tsAutoCompStateHint* stateHint = (tsAutoCompStateHint*)hint;
      ScriptDoc* doc = (ScriptDoc*)GetDocument();
      doc->OnAutoCompHint( stateHint->isEnabled );
   }
   else if ( wxDynamicCast( hint, tsFileRenameHint ) )
   {
      tsFileRenameHint* renameHint = (tsFileRenameHint*)hint;
      ScriptDoc* doc = (ScriptDoc*)GetDocument();
      if ( wxFileName( doc->GetFilename() ) == renameHint->oldPath )
         doc->OnFileRename( renameHint->newPath );
   }
}

void ScriptView::OnBreakpointEvent( BreakpointEvent& event )
{
   // TODO: Should we just make BreakpointEvent a wxCommandEvent
   // which would then be forwarded by ScriptView::ProcessEvent?
   wxASSERT( m_ScriptCtrl );
   //TODO ?   m_ScriptCtrl->ProcessEvent( event );
   ProcessEvent(event);
}

bool ScriptView::OnClose( bool deleteWindow )
{
   if ( !deleteWindow && !GetDocument()->Close() )
      return false;

   Activate( false );

   if ( deleteWindow ) {

      wxASSERT( GetFrame() );

      // This will destroy the controls as well!
      m_ScriptCtrl = NULL;
      GetFrame()->RemoveScriptPage( this );
   }
   
   return true;
}

bool ScriptView::OnSaveFile( const wxString& filename )
{
   wxASSERT( m_ScriptCtrl );
   if ( !m_ScriptCtrl->SaveFile( filename ) ) {
      return false;
   }

   m_FileWatcher.SetWatch( filename, FILECHANGE_SIZE | FILECHANGE_LAST_WRITE );

   return true;
}

bool ScriptView::OnLoadFile( const wxString& filename )
{
   wxASSERT( m_ScriptCtrl );
   if ( m_ScriptCtrl->LoadFile( filename ) ) 
   {
      GetDocument()->Modify( false );
      m_FileWatcher.SetWatch( filename, FILECHANGE_SIZE | FILECHANGE_LAST_WRITE );
      OnCtrlModifyed();
      return true;
   }

   return false;
}

void ScriptView::OnChangeFilename()
{
   // Clear the file watcher so we don't get 
   // a false change notification.
   m_FileWatcher.Clear();

   // This is normally done in wxView::OnChangeFilename(),
   // but it doesn't take into account the active state
   // of the view... so it would set the title even if
   // it wasn't the active view!
   wxDocument* doc = GetDocument();
   if (GetFrame() && doc )
   {
      wxString title;
      //doc->GetPrintableName(title);
	  title = doc->GetTitle();
      if ( GetFrame()->GetView() == this )
         GetFrame()->SetTitle(title);

      // Reset the watcher.
      if ( !doc->GetFilename().IsEmpty() )
      {
         m_FileWatcher.SetWatch( doc->GetFilename(), 
            FILECHANGE_SIZE | FILECHANGE_LAST_WRITE );
      }
   }

   OnCtrlModifyed();
}

void ScriptView::SetCtrlFocused()
{
   if ( m_ScriptCtrl )
   {
      m_ScriptCtrl->SetFocus();
      Activate( true );
   }
}

void ScriptView::OnCtrlModifyed()
{
   if ( GetFrame() && m_ScriptCtrl )
      GetFrame()->SetTabState( this, m_ScriptCtrl->GetModify(), m_ScriptCtrl->GetReadOnly() );
}

void ScriptView::ClearSelectedText()
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->SetSelection( -1, m_ScriptCtrl->GetSelectionStart() );
}

wxString ScriptView::GetSelectedText( bool AtCursor ) const
{
   wxASSERT( m_ScriptCtrl );

   wxString text = m_ScriptCtrl->GetSelectedText();
   if ( text.IsEmpty() && AtCursor ) {
   
      int pos = m_ScriptCtrl->GetCurrentPos();

      int start = m_ScriptCtrl->WordStartPosition( pos, true );
      int end = m_ScriptCtrl->WordEndPosition( start, true );
      text = m_ScriptCtrl->GetTextRange( start, end );
   }

   return text;
}

void ScriptView::OnUpperSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->UpperCase();
}

void ScriptView::OnLowerSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->LowerCase();
}

void ScriptView::OnUpdateHasSelection(wxUpdateUIEvent& event)
{
   wxASSERT( m_ScriptCtrl );
   event.Enable(  m_ScriptCtrl->GetCurrentPos() != -1 && 
                  m_ScriptCtrl->GetSelectionStart() != -1 && 
                  m_ScriptCtrl->GetSelectionEnd() != -1 );
}

void ScriptView::OnCommentSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->CommentSel( true );
}

void ScriptView::OnUncommentSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->CommentSel( false );
}

void ScriptView::OnIndentSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->Tab();
}

void ScriptView::OnUnindentSel(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->BackTab();
}

void ScriptView::OnMatchBrace( wxCommandEvent& event )
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->MatchBrace();
}

void ScriptView::OnCut(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   if ( m_ScriptCtrl->GetReadOnly() ) 
   {
      wxBell();
      return;
   }
   m_ScriptCtrl->Cut();
}

void ScriptView::OnCopy(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->Copy();
}

void ScriptView::OnPaste(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   if ( m_ScriptCtrl->GetReadOnly() ) {
      wxBell();
      return;
   }
   m_ScriptCtrl->Paste();
}

void ScriptView::OnUndo(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->Undo();
}

void ScriptView::OnRedo(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->Redo();
}

void ScriptView::OnDelete(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->Clear();
}

void ScriptView::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->SelectAll();
}

void ScriptView::OnUpdateFileExists(wxUpdateUIEvent& event)
{
   wxFileName file( GetDocument()->GetFilename() );
   event.Enable( file.FileExists() );
}

void ScriptView::OnUpdateUndo(wxUpdateUIEvent& event)
{
   wxASSERT( m_ScriptCtrl );
   event.Enable( m_ScriptCtrl->CanUndo() );
}

void ScriptView::OnUpdateRedo(wxUpdateUIEvent& event)
{
   wxASSERT( m_ScriptCtrl );
   event.Enable( m_ScriptCtrl->CanRedo() );
}

void ScriptView::OnUpdateSelectAll(wxUpdateUIEvent& event)
{
   wxASSERT( m_ScriptCtrl );
   event.Enable( m_ScriptCtrl->GetLength() > 0 );
}

void ScriptView::OnLineBreaks( wxCommandEvent& event )
{
   tsGetPrefs().ShowLineBreaks( event.IsChecked() );

   ScriptViewList::Node* node = s_ScriptViews.GetFirst();
   while ( node ) {

      ScriptView* view = node->GetData();
      wxASSERT( view );
      wxASSERT( view->m_ScriptCtrl );
      view->m_ScriptCtrl->SetViewEOL( tsGetPrefs().GetLineBreaks() );
      node = node->GetNext();
   }
}

void ScriptView::OnUpdateLineBreaks( wxUpdateUIEvent& event )
{
   event.Enable( true );      
   event.Check( tsGetPrefs().GetLineBreaks() );
}

void ScriptView::OnTabsAndSpaces( wxCommandEvent& event )
{
   tsGetPrefs().ShowTabsAndSpaces( event.IsChecked() );

   ScriptViewList::Node* node = s_ScriptViews.GetFirst();
   while ( node ) {

      ScriptView* view = node->GetData();
      wxASSERT( view );
      wxASSERT( view->m_ScriptCtrl );
      view->m_ScriptCtrl->SetViewWhiteSpace( tsGetPrefs().GetTabsAndSpaces() ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE );
      node = node->GetNext();
   }
}

void ScriptView::OnUpdateTabsAndSpaces( wxUpdateUIEvent& event )
{
   event.Enable( true );      
   event.Check( tsGetPrefs().GetTabsAndSpaces() );
}

void ScriptView::OnLineNumbers( wxCommandEvent& event )
{
   tsGetPrefs().ShowLineNumbers( event.IsChecked() );

   ScriptViewList::Node* node = s_ScriptViews.GetFirst();
   while ( node ) {

      wxASSERT( node->GetData() );
      wxASSERT( node->GetData()->m_ScriptCtrl );
      ScriptCtrl* ctrl = node->GetData()->m_ScriptCtrl;
      ctrl->ShowLineNumbers( event.IsChecked() );
      node = node->GetNext();
   }
}

void ScriptView::OnUpdateLineNumbers( wxUpdateUIEvent& event )
{
   event.Enable( true );      
   event.Check( tsGetPrefs().GetLineNumbers() );
}

void ScriptView::OnLineWrap( wxCommandEvent& event )
{
   tsGetPrefs().SetLineWrap( event.IsChecked() );

   ScriptViewList::Node* node = s_ScriptViews.GetFirst();
   while ( node ) {

      ScriptView* view = node->GetData();
      wxASSERT( view );
      wxASSERT( view->m_ScriptCtrl );
      view->m_ScriptCtrl->SetWrapMode( tsGetPrefs().GetLineWrap() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE );
      node = node->GetNext();
   }
}

void ScriptView::OnUpdateLineWrap( wxUpdateUIEvent& event )
{
   event.Enable( true );      
   event.Check( tsGetPrefs().GetLineWrap() );
}

void ScriptView::OnMoveInstPtr( wxCommandEvent& event )
{
   wxASSERT( tsGetDebugger() );
   wxASSERT( tsGetDebugger()->IsRunning() );
   wxASSERT( tsGetDebugger()->IsAtBreakpoint() );

   wxASSERT( m_ScriptCtrl );
   int line = m_ScriptCtrl->LineFromPosition( m_ScriptCtrl->GetSelectionStart() );

   tsGetDebugger()->MoveInstPtr( GetDocument()->GetFilename(), line+1 );
}

void ScriptView::OnUpdateMoveInstPtr( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetDebugger() );
   event.Enable( tsGetDebugger()->IsAtBreakpoint() );      
}

void ScriptView::OnReloadScript( wxCommandEvent& event )
{
   wxASSERT( tsGetDebugger() );
   wxASSERT( tsGetDebugger()->IsRunning() );
   if ( GetDocument()->Save() )
   {
      wxArrayString scripts;
      scripts.Add( GetDocument()->GetFilename() );
      tsGetDebugger()->ReloadScripts( scripts );
   }
}

void ScriptView::OnUpdateReloadScript( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetDebugger() );
   ScriptDoc* doc = (ScriptDoc*)GetDocument();
   event.Enable( tsGetDebugger()->IsRunning() && doc->IsModified() && !doc->GetFilename().IsEmpty() );      
}

void ScriptView::OnOpenWith( wxCommandEvent& event )
{
   wxFileName file( GetDocument()->GetFilename() );
   tsShellOpenAs( file.GetFullPath() );
}

void ScriptView::OnExplore( wxCommandEvent& event )
{
   wxFileName file( GetDocument()->GetFilename() );
   tsExecuteVerb( file.GetPath(), "explore" );
}

void ScriptView::OnCopyFullPath( wxCommandEvent& event )
{
   wxFileName file( GetDocument()->GetFilename() );
   if ( wxTheClipboard->Open() )
   {
      wxTheClipboard->SetData( new wxTextDataObject( file.GetFullPath() ) );
      wxTheClipboard->Flush();
      wxTheClipboard->Close();
   }
}

void ScriptView::OnUpdateEnabled( wxUpdateUIEvent& event )
{
   event.Enable( true );
}

void ScriptView::OnFindNext( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   wxFindReplaceData& FindData = tsGetMainFrame()->GetFindData();

   if ( ::wxGetKeyState( WXK_CONTROL ) )
   {
      wxASSERT( m_ScriptCtrl );
      wxString text = m_ScriptCtrl->GetSelectedText();
      if ( text.IsEmpty() )
      {
         tsBellEx( wxICON_INFORMATION );
         return;
      }
      else
         FindData.SetFindString( text );
   }

   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_NEXT );
   fevent.SetFindString( FindData.GetFindString() );

   if ( event.GetId() == tsID_FINDPREV ) {
      fevent.SetFlags( FindData.GetFlags() & ~wxFR_DOWN );
   } else {
      fevent.SetFlags( FindData.GetFlags() | wxFR_DOWN );
   }
   
   wxASSERT( m_ScriptCtrl );
   m_ScriptCtrl->OnFindNext( fevent ); 
}

void ScriptView::ClearBreakline()
{
   wxASSERT( m_ScriptCtrl );
	m_ScriptCtrl->ClearBreakline();
}

void ScriptView::SetBreakline( int Line )
{
   wxASSERT( m_ScriptCtrl );
	m_ScriptCtrl->SetBreakline( Line );
}

bool ScriptView::SetLineSelected( int Line )
{
   wxASSERT( m_ScriptCtrl );
	return m_ScriptCtrl->SetLineSelected( Line );
}

int ScriptView::GetSelectionLine() const
{
   wxASSERT( m_ScriptCtrl );
   int pos = m_ScriptCtrl->GetCurrentPos();
   return m_ScriptCtrl->LineFromPosition( pos );
}

void ScriptView::OnDebugCallTip( const wxString& Expression, const wxString& Value )
{
   wxASSERT( m_ScriptCtrl );
	m_ScriptCtrl->OnDebugCallTip( Expression, Value );
}
