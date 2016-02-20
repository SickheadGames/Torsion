// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ProjectView.h"

#include "ProjectDoc.h"
#include "ProjectPanel.h"
#include "TorsionApp.h"
#include "MainFrame.h"
#include "Platform.h"
#include "CodeBrowserPanel.h"
#include "shNotebook.h"
#include "ScriptView.h"
#include "ScriptDoc.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


IMPLEMENT_DYNAMIC_CLASS( ProjectView, wxView )

BEGIN_EVENT_TABLE( ProjectView, wxView )

   EVT_MENU( tsID_PROJECT_OPEN, OnOpenSel )
   EVT_UPDATE_UI( tsID_PROJECT_OPEN, OnUpdateMultSelected )

   EVT_MENU( tsID_PROJECT_OPENWITH, OnOpenWith )
   EVT_UPDATE_UI( tsID_PROJECT_OPENWITH, OnUpdateSelected )
   
   EVT_MENU( tsID_EXPLORE, OnExplore )
   EVT_UPDATE_UI( tsID_EXPLORE, OnUpdateSelected )

   EVT_MENU( wxID_PROPERTIES, OnProperties )
   EVT_UPDATE_UI( wxID_PROPERTIES, OnUpdateSelected )

   EVT_MENU( tsID_RENAME, OnRename )
   EVT_UPDATE_UI( tsID_RENAME, OnUpdateRename )

   EVT_MENU( wxID_DELETE, OnDelete )
   EVT_UPDATE_UI( wxID_DELETE, OnUpdateDelete )

   EVT_MENU( tsID_PROJECT_SHOWALLMODS, OnShowAllMods )
   EVT_UPDATE_UI( tsID_PROJECT_SHOWALLMODS, OnUpdateShowAllMods )

   EVT_MENU( tsID_PROJECT_SHOWALLFILES, OnShowAllFiles )
   EVT_UPDATE_UI( tsID_PROJECT_SHOWALLFILES, OnUpdateShowAllFiles )

   EVT_MENU( tsID_PROJECT_SYNCTREE, OnSyncTree )
   EVT_UPDATE_UI( tsID_PROJECT_SYNCTREE, OnUpdateSyncTree )

   EVT_MENU( tsID_NEW_FOLDER, OnNewFolder )
   EVT_UPDATE_UI( tsID_NEW_FOLDER, OnUpdateSelected )

   EVT_MENU( tsID_NEW_FILE, OnNewFile )
   EVT_UPDATE_UI( tsID_NEW_FILE, OnUpdateSelected )

   EVT_MENU( tsID_PROJECT_REFRESH, OnRefresh )

   EVT_MENU( tsID_BOOKMARK_TOGGLE, OnBookmarkToggle )
   EVT_UPDATE_UI( tsID_BOOKMARK_TOGGLE, OnUpdateGotActiveScriptView )
   EVT_MENU( tsID_BOOKMARK_CLEARALL, OnBookmarkClearAll )
   EVT_UPDATE_UI( tsID_BOOKMARK_CLEARALL, OnUpdateBookmarkClearAll )
   EVT_MENU( tsID_BOOKMARK_NEXT, OnBookmarkNext )
   EVT_UPDATE_UI( tsID_BOOKMARK_NEXT, OnUpdateBookmarkClearAll )
   EVT_MENU( tsID_BOOKMARK_PREV, OnBookmarkPrev )
   EVT_UPDATE_UI( tsID_BOOKMARK_PREV, OnUpdateBookmarkClearAll )

   // TODO: We need to fix this at some point.
   //EVT_UPDATE_UI_RANGE(wxID_CUT, wxID_CLEAR, OnUpdateDisable)
   //EVT_UPDATE_UI_RANGE(wxID_DUPLICATE, wxID_REPLACE_ALL, OnUpdateDisable)

   // If the project is the active document then
   // disable Save As... we don't allow the project
   // to be "renamed" in that fashion.
   EVT_UPDATE_UI(wxID_SAVEAS, OnUpdateGotActiveScriptView)

END_EVENT_TABLE()

ProjectView::ProjectView()
   :  wxView(),
      m_ProjectPanel( NULL ),
      m_ProjectCtrl( NULL ),
      m_BrowserPanel( NULL ),
      m_ShowAllMods( false ),
      m_ShowAllFiles( true ),
      m_SyncTree( true ),
      m_LastBookmark( NULL )
{
}

ProjectView::~ProjectView()
{
}

wxString ProjectView::GetSelectedPath() const 
{ 
   wxASSERT( m_ProjectCtrl );
   return m_ProjectCtrl->GetSelectedPath(); 
}

void ProjectView::OnUpdateGotActiveScriptView( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetMainFrame() );
   event.Enable( tsGetMainFrame()->GetActiveView() != NULL );
}

void ProjectView::OnUpdateDisable( wxUpdateUIEvent& event )
{
   event.Enable( false );
}

void ProjectView::OnUpdateSelected( wxUpdateUIEvent& event )
{
   event.Enable( m_ProjectCtrl && m_ProjectCtrl->GetSelectedCount() == 1 );
}

void ProjectView::OnUpdateMultSelected( wxUpdateUIEvent& event )
{
   event.Enable( m_ProjectCtrl && m_ProjectCtrl->GetSelectedCount() > 0 );
}

bool ProjectView::OnCreate( wxDocument* doc, long flags )
{
   // The project doesn't have a traditional frame.  It
   // is displayed in the left sash in a tree control.
   // To do this we need to tell the mainframe to give
   // us the notebook for the left sash and we add our
   // project control to it.
   shNotebook* notebook = tsGetMainFrame()->ShowProjectSash( true );
   m_ProjectPanel = new ProjectPanel( notebook );
   m_ProjectCtrl = m_ProjectPanel->GetCtrl();
   notebook->AddPage( m_ProjectPanel, m_ProjectPanel->GetLabel(), wxEmptyString, false, 0 );
   m_BrowserPanel = new CodeBrowserPanel( notebook );
   notebook->AddPage( m_BrowserPanel, m_BrowserPanel->GetLabel(), wxEmptyString, false, 0 );
   notebook->SetSelection( 0 );

   // Setup some app prefs.
   m_ShowAllMods     = tsGetPrefs().GetShowAllMods();
   m_ShowAllFiles    = tsGetPrefs().GetShowAllFiles();
   m_SyncTree        = tsGetPrefs().GetSyncProjectTree();

   /*
   m_ChildFrame->Show( true );
   Activate( true );
   */

   return true;
}

void ProjectView::OnClosingDoocument()
{
}

void ProjectView::OnDraw( wxDC* dc )
{
}

void ProjectView::OnUpdate( wxView* sender, wxObject* hint )
{
   wxASSERT( m_ProjectCtrl );
   m_ProjectCtrl->Refresh( (ProjectDoc*)GetDocument(), m_ShowAllMods, m_ShowAllFiles, false );
}

void ProjectView::OnRefresh( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   m_ProjectCtrl->Refresh( (ProjectDoc*)GetDocument(), m_ShowAllMods, m_ShowAllFiles, true );
}

void ProjectView::OnOpenSel( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );

   // Grab the current selection.
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );

   // We should never have gotten the root to delete.
   if ( selected.Index( m_ProjectCtrl->GetRootItem().m_pItem ) != wxNOT_FOUND )
   {
      tsBellEx( wxICON_INFORMATION );
      return;
   }

   for ( int i=0; i < selected.GetCount(); i++ )
   {
      wxTreeItemId id = selected[i];
      m_ProjectCtrl->OpenItem( id );
   }
}

void ProjectView::OnOpenWith( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );

   // Grab the current selection.
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   
   wxASSERT( selected.GetCount() == 1 );
   m_ProjectCtrl->OpenItemWith( selected[0] );
}

void ProjectView::OnExplore( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxFileName path( m_ProjectCtrl->GetSelectedPath() );
   if ( !path.IsDir() ) {
      path.SetFullName( "" );
   }
   tsExecuteVerb( path.GetFullPath(), "open" );
}

void ProjectView::OnProperties( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxFileName path( m_ProjectCtrl->GetSelectedPath() );
   tsExecuteVerb( path.GetFullPath(), "properties" );
}

void ProjectView::OnUpdateDelete( wxUpdateUIEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   const wxTreeItemId root = m_ProjectCtrl->GetRootItem();

   event.Enable(  selected.GetCount() > 0 &&
                  selected.Index( root.m_pItem ) == wxNOT_FOUND );
}

void ProjectView::OnDelete( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );

   // If the control doesn't have focus... then 
   // ignore it.  It must not be for us!
   if ( m_ProjectCtrl != wxWindow::FindFocus() )
   {
      event.Skip();
      return;
   }

   // Grab the current selection.
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->UnnestSelections();
   m_ProjectCtrl->GetSelections( selected );

   // We should never have gotten the root to delete.
   if ( selected.Index( m_ProjectCtrl->GetRootItem().m_pItem ) != wxNOT_FOUND )
   {
      tsBellEx( wxICON_INFORMATION );
      return;
   }

   // Gather the files and folders into an array.
   wxArrayString files;
   wxString path;
   for ( size_t i=0; i < selected.GetCount(); i++ )
   {
      path = m_ProjectCtrl->GetItemPath( selected[i] );
      files.Add( path );

      // If this script has a DSO add it to the deletion array.
      if ( tsGetPrefs().IsScriptFile( path ) )
      {
         wxString dsoPath = tsGetPrefs().GetDSOForScript( path );
         if ( !dsoPath.IsEmpty() )
            files.Add( dsoPath );
      }
   }

   // We use the recycle bin... or OSX/Linux equivelent
   // if possible here to save the user from totally 
   // screwing themselves.
   if ( tsSendToRecycleBin( files, false ) )
   {
      // Close the open documents we deleted.
      wxASSERT( tsGetMainFrame() );
      for ( size_t i=0; i < files.Count(); i++ )
      {
         ScriptDoc* doc = tsGetMainFrame()->GetOpenDoc( files[i] );
         if ( doc )
         {
            // Disable the modify flag so we don't get
            // prompts about saving changes.
            doc->Modify( false );
            doc->DeleteAllViews();
         }
      }
   }
}

void ProjectView::OnUpdateRename( wxUpdateUIEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   const wxTreeItemId root = m_ProjectCtrl->GetRootItem();

   event.Enable(  selected.GetCount() == 1 &&
                  selected[0] != root );
}

void ProjectView::OnRename( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   wxASSERT( selected.GetCount() == 1 );
   m_ProjectCtrl->EditLabel( selected[0] );
}

void ProjectView::OnNewFolder( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   wxASSERT( selected.GetCount() == 1 );
   m_ProjectCtrl->NewFolder( selected[0] );
}

void ProjectView::OnNewFile( wxCommandEvent& event )
{
   wxASSERT( m_ProjectCtrl );
   wxArrayTreeItemIds selected;
   m_ProjectCtrl->GetSelections( selected );
   wxASSERT( selected.GetCount() == 1 );
   m_ProjectCtrl->NewFile( selected[0] );
}

bool ProjectView::OnClose( bool deleteWindow )
{
   // Save the app prefs.
   tsGetPrefs().SetShowAllMods( m_ShowAllMods );
   tsGetPrefs().SetShowAllFiles( m_ShowAllFiles );
   tsGetPrefs().SetSyncProjectTree( m_SyncTree );

   if ( !deleteWindow && !GetDocument()->Close() )
      return false;

   //Activate( false );

   if ( deleteWindow ) 
   {
      // Destroy the control, but also tell the 
      // mainframe to hide the left sash.
      shNotebook* notebook = tsGetMainFrame()->ShowProjectSash( false );
      notebook->DeleteAllPages();
      m_BrowserPanel = NULL;
      m_ProjectPanel = NULL;
      m_ProjectCtrl = NULL;
   }
   
   return true;
}

void ProjectView::Select( const wxString& path )
{
   if ( !m_SyncTree )
      return;

   wxASSERT( m_ProjectCtrl );
   m_ProjectCtrl->Select( path );
}

void ProjectView::OnShowAllMods( wxCommandEvent& event )
{
   m_ShowAllMods = !m_ShowAllMods;

   wxASSERT( m_ProjectCtrl );
   m_ProjectCtrl->Refresh( (ProjectDoc*)GetDocument(), m_ShowAllMods, m_ShowAllFiles, false );
}

void ProjectView::OnUpdateShowAllMods( wxUpdateUIEvent& event )
{
   event.Check( m_ShowAllMods );
}

void ProjectView::OnShowAllFiles( wxCommandEvent& event )
{
   m_ShowAllFiles = !m_ShowAllFiles;

   wxASSERT( m_ProjectCtrl );
   m_ProjectCtrl->Refresh( (ProjectDoc*)GetDocument(), m_ShowAllMods, m_ShowAllFiles, false );
}

void ProjectView::OnUpdateShowAllFiles( wxUpdateUIEvent& event )
{
   event.Check( m_ShowAllFiles );
}

void ProjectView::OnSyncTree( wxCommandEvent& event )
{
   m_SyncTree = !m_SyncTree;

   if ( m_SyncTree )
   {
      wxView* view = (wxView*)tsGetMainFrame()->GetActiveView();
      if ( view )
         Select( view->GetDocument()->GetFilename() );
   }
}

void ProjectView::OnUpdateSyncTree( wxUpdateUIEvent& event )
{
   event.Check( m_SyncTree );
}

void ProjectView::OnBookmarkToggle( wxCommandEvent& event )
{
   wxASSERT( tsGetMainFrame() );

   ScriptView* view = tsGetMainFrame()->GetActiveView();
   if ( !view )
      return;

   // Get the file and line.
   wxString file = view->GetDocument()->GetFilename();
   int line = view->GetSelectionLine();

   // See if we already have one for that line.
   ProjectDoc* doc = (ProjectDoc*)GetDocument();
   wxASSERT( doc );
   Bookmark* mark = doc->FindBookmark( file, line );
   if ( mark )
      doc->DeleteBookmark( mark );
   else
      doc->AddBookmark( file, line );
}

void ProjectView::OnBookmarkClearAll( wxCommandEvent& event )
{
   ProjectDoc* doc = (ProjectDoc*)GetDocument();
   wxASSERT( doc );
   
   wxMessageDialog dialog( tsGetMainFrame(), "Are you sure you want to delete all of the bookmarks?", "Torsion", wxYES_NO );
	if ( dialog.ShowModal() != wxID_YES )
      return;

   doc->DeleteAllBookmarks();
}

void ProjectView::OnUpdateBookmarkClearAll( wxUpdateUIEvent& event )
{
   ProjectDoc* doc = (ProjectDoc*)GetDocument();
   wxASSERT( doc );
   event.Enable( doc->GetBookmarks().GetCount() > 0 );
}

void ProjectView::DoBookmarkNav( bool prev, int fromIndex )
{
   ProjectDoc* doc = (ProjectDoc*)GetDocument();
   wxASSERT( doc );

   const BookmarkArray& bookmarks = doc->GetBookmarks();
   if ( bookmarks.IsEmpty() )
      return;

   int index;
   if ( fromIndex != wxNOT_FOUND )
      index = fromIndex;
   else
      index = bookmarks.Index( m_LastBookmark );

   if ( index != wxNOT_FOUND )
   {
      index += prev ? -1 : 1;

      if ( index < 0 )
         m_LastBookmark = bookmarks.Last();
      else if ( index >= bookmarks.GetCount() )
         m_LastBookmark = bookmarks[0];
      else
         m_LastBookmark = bookmarks[index];
   }
   else
   {
      if ( prev )
         m_LastBookmark = bookmarks.Last();
      else
         m_LastBookmark = bookmarks[0];
   }

   wxASSERT( m_LastBookmark );
   wxString absolutePath = doc->MakeAbsoluteTo( m_LastBookmark->GetFile() );
   if ( !wxFileName::FileExists( absolutePath ) )
   {
      // Remove the bookmark and try again!
      index = bookmarks.Index( m_LastBookmark ) - 1;
      doc->DeleteBookmark( m_LastBookmark );
      if ( bookmarks.IsEmpty() )
         return;

      if ( index < 0 )
         index = bookmarks.GetCount() - 1;

      DoBookmarkNav( prev, index );
      return;
   }

   // Open the file and try to select the line.
   ScriptView* view = tsGetMainFrame()->OpenFile( absolutePath );
   if ( !view || !view->SetLineSelected( m_LastBookmark->GetLine() ) )
   {
      // Remove the bookmark and try again!
      index = bookmarks.Index( m_LastBookmark ) - 1;
      doc->DeleteBookmark( m_LastBookmark );
      if ( bookmarks.IsEmpty() )
         return;

      if ( index < 0 )
         index = bookmarks.GetCount() - 1;

      DoBookmarkNav( prev, index );
      return;
   }
}

void ProjectView::OnBookmarkNext( wxCommandEvent& event )
{
   DoBookmarkNav( false );
}

void ProjectView::OnBookmarkPrev( wxCommandEvent& event )
{
   DoBookmarkNav( true );
}

