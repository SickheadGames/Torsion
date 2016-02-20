// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ProjectCtrl.h"

#include "ProjectDoc.h"
#include "MainFrame.h"
#include "TorsionApp.h"
#include "FileTypeImageList.h"
#include "Platform.h"
#include "tsMenu.h"
#include "ScriptView.h"

#include "Icons.h"

#include <wx/dir.h>
#include <wx/image.h>
#include <wx/mimetype.h>
#include <wx/timer.h>
#include <wx/file.h>
#include <wx/dcbuffer.h>
#include <map>

#include "icons\dragcursor.xpm"
wxBitmap ts_drag_bitmap( dragcursor_xpm );


#ifdef __WXMSW__
   #include <commctrl.h>
   #include "ShellMenu.h"
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(TreeItemIdArray);

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


IMPLEMENT_CLASS( ProjectCtrl, wxTreeCtrl )

BEGIN_EVENT_TABLE( ProjectCtrl, wxTreeCtrl )

	EVT_TREE_ITEM_ACTIVATED( wxID_ANY, OnOpenItem )
   EVT_TREE_ITEM_MENU( wxID_ANY, OnItemMenu )

   EVT_TREE_ITEM_EXPANDING( wxID_ANY, OnItemExpanding )
   EVT_TREE_ITEM_EXPANDED( wxID_ANY, OnItemExpanded )
   EVT_TREE_ITEM_COLLAPSING( wxID_ANY, OnItemCollapsing )
   EVT_TREE_ITEM_COLLAPSED( wxID_ANY, OnItemCollapsed )

   EVT_TREE_ITEM_GETTOOLTIP( wxID_ANY, OnToolTip )

   EVT_TREE_BEGIN_LABEL_EDIT( wxID_ANY, OnBeginRename )
   EVT_TREE_END_LABEL_EDIT( wxID_ANY, OnEndRename )

   // We only use EVT_TREE_BEGIN_DRAG to trigger our
   // custom drag code.  Our EVT_MOTION, EVT_TIMER, 
   // and EVT_LEFT_UP handlers do the rest of the work.
   EVT_TREE_BEGIN_DRAG( wxID_ANY, OnDragBegin )
   EVT_MOTION( OnDragMove )
   EVT_TIMER( 0, OnDragExpandTimer )
   EVT_KEY_UP( OnDragKey )
   EVT_LEFT_UP( OnDragEnd )

   EVT_SET_FOCUS( OnSetFocus )
   EVT_KILL_FOCUS( OnKillFocus )

   // To fix flicker!
   #ifdef __WXMSW__
      EVT_ERASE_BACKGROUND( ProjectCtrl::OnEraseBackground )
      EVT_PAINT( ProjectCtrl::OnPaint ) 
   #endif // __WXMSW__

END_EVENT_TABLE()

ProjectCtrl::ProjectCtrl( wxWindow* parent )
   :  wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxSTATIC_BORDER | wxTR_EDIT_LABELS | wxTR_MULTIPLE ),
      m_ProjectDoc( NULL ),
      m_IsDragging( false ),
      m_ImageList( NULL ),
      m_ShowAllMods( false ),
      m_ShowAllFiles( true ),
      m_DonePaint( false ),
      m_FocusView( NULL )
{
   m_DragExpandTimer.SetOwner( this, 0 );

   #ifdef __WXMSW__
      m_OsMenu = NULL;

      // We need to have an extra pixel space between items
      // because icons are 16x16.
      DWORD style = ::GetWindowLong( GetHwnd(), GWL_STYLE );
      style |= TVS_NONEVENHEIGHT; 
      ::SetWindowLong( GetHwnd(), GWL_STYLE, style );
      ::SendMessage( GetHwnd(), TVM_SETITEMHEIGHT, 17, 0 );
   #endif // __WXMSW__

   // Create the drag cursor.
   {
      wxImage image = ts_drag_bitmap.ConvertToImage();
      image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
      image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
      m_DragCursor = wxCursor( image );
   }

   // Create the image list and add any fixed icons well need.
   m_ImageList = new FileTypeImageList();
	m_ImageList->Add( ts_new_project16 );
	m_ImageList->Add( ts_folder_closed16 );
	m_ImageList->Add( ts_folder_open16 );
   SetImageList( m_ImageList );
}

ProjectCtrl::~ProjectCtrl()
{
   wxDELETE( m_ImageList );
}

#ifdef __WXMSW__

   void ProjectCtrl::OnEraseBackground( wxEraseEvent& event )
   {
      // Let the native control draw the background up
      // until we get our first real paint.
      if ( !m_DonePaint )
         event.Skip();
   }

   void ProjectCtrl::OnPaint( wxPaintEvent& event )
   {
      // Don't erase the background anymore.
      m_DonePaint = true;

      // To clear up flicker we draw the background here
      // ourselves in only the invalid area.
      wxBufferedPaintDC dc( this );
      wxDCClipper clipper( dc, GetUpdateRegion().GetBox() );
	   dc.SetBackground( GetBackgroundColour() );
	   dc.Clear();

      // Unhook ourselves from message handling for a sec
      // and let the native control render to our buffered dc.
      SetEvtHandlerEnabled( false );
      ::SendMessage( GetHwnd(), WM_PAINT, (WPARAM)GetHdcOf( dc ), 0 );
      SetEvtHandlerEnabled( true );
   }

#endif // __WXMSW__

size_t ProjectCtrl::GetSelectedCount()
{
   wxArrayTreeItemIds selection;
   return GetSelections( selection );
}

wxString ProjectCtrl::GetSelectedPath() const
{
   wxArrayTreeItemIds selection;
   if ( GetSelections( selection ) == 0 )
      return wxEmptyString;

   return GetItemPath( selection[0] ); 
}

void ProjectCtrl::OnSetFocus( wxFocusEvent& event )
{
   // If we have a focus view... then redirect the 
   // focus to it instead!
   if ( m_FocusView )
   {
      m_FocusView->SetCtrlFocused();
      m_FocusView = NULL;
      return;
   }

   if ( m_ProjectDoc )
   {
      wxASSERT( m_ProjectDoc->GetFirstView() );
      m_ProjectDoc->GetFirstView()->Activate( true );
   }

   // If we don't skip it the TreeCtrl base
   // will never get to handle it.
   event.Skip();
}

void ProjectCtrl::OnKillFocus( wxFocusEvent& event )
{
   if ( m_ProjectDoc )
   {
      wxASSERT( m_ProjectDoc->GetFirstView() );
      m_ProjectDoc->GetFirstView()->Activate( false );
   }

   // If we don't skip it the TreeCtrl base
   // will never get to handle it.
   event.Skip();
}

int ProjectCtrl::GetFileIcon( const wxString& file )
{
   wxASSERT( m_ImageList );
   return m_ImageList->AddFileIcon( file );
}

class ProjectCtrlItemData : public wxTreeItemData
{
public:
	ProjectCtrlItemData( const wxString& FullPath ) 
		:	wxTreeItemData(),
			m_FullPath( FullPath ) 
	{
	}

   const wxString& GetPath() const { return m_FullPath; }

protected:

	wxString	m_FullPath;
};

class ProjectDirTraverser : public wxDirTraverser
{
public:

   ProjectDirTraverser( ProjectCtrl* treeCtrl, wxTreeItemId parent, TreeItemIdArray& children, bool showAllMods, bool showAllFiles ) 
      :  m_TreeCtrl( treeCtrl ),
         m_Parent( parent ),
         m_Children( children ),
         m_ShowAllMods( showAllMods ),
         m_ShowAllFiles( showAllFiles )
   {
      wxASSERT( m_TreeCtrl );
      m_Project = m_TreeCtrl->GetProjectDoc();
      wxASSERT( m_Project );
   }

   virtual wxDirTraverseResult OnFile( const wxString& filename )
   {
      wxFileName file( filename );

      // The app preferences defines what files to 
      // allow in the tree view.
      if ( tsGetPrefs().IsExcludedFile( filename ) )
         return wxDIR_CONTINUE;

      // If this is a DSO skip it if the script exists.
      if ( tsGetPrefs().IsDSOExt( file.GetExt() ) )
      {
         wxString script = filename.BeforeLast( '.' );
         if ( wxFileName::FileExists( script ) )
            return wxDIR_CONTINUE;
      }

      // Check if we're filtering non-script files.
      if ( !m_ShowAllFiles && !tsGetPrefs().IsScriptFile( filename ) )
         return wxDIR_CONTINUE;

      // If we already have it in the child list then
      // remove it and don't add a new one.
      for ( int i=0; i < m_Children.GetCount(); i++ ) 
      {
         wxASSERT( m_Children[i].IsOk() );
         wxFileName name( m_TreeCtrl->GetItemText( m_Children[i] ) );
         if ( name == file.GetFullName() ) 
         {
            // Colorize it depending on the filters.
            ColorizeItem( m_Children[i], file.GetPath() );

            m_Children.RemoveAt( i );
            return wxDIR_CONTINUE;
         }
      }

      int icon = m_TreeCtrl->GetFileIcon( filename ); 

      // Add this new file to the tree.
      wxTreeItemId Id = m_TreeCtrl->AppendItem( m_Parent, file.GetFullName(),
	      icon, icon, new ProjectCtrlItemData( file.GetFullPath() ) );

      // Colorize it depending on the filters.
      ColorizeItem( Id, file.GetPath() );

      return wxDIR_CONTINUE;
   }

   virtual wxDirTraverseResult OnDir( const wxString& dirname )
   {
      wxASSERT( m_TreeCtrl );
      wxASSERT( m_Project );

      if ( !m_ShowAllMods && !m_Project->IsMod( dirname ) )
         return wxDIR_IGNORE;

      wxFileName dir;
      dir.AssignDir( dirname );

      // If we already have it in the child list then
      // let it be and don't add a new one.
      for ( int i=0; i < m_Children.GetCount(); i++ ) 
      {
         wxASSERT( m_Children[i].IsOk() );
         wxFileName name( m_TreeCtrl->GetItemText( m_Children[i] ) );
         if ( name.SameAs( dir.GetDirs().Last() ) ) 
         {
            //if ( !m_TreeCtrl->IsExpanded( m_Children[i] ) )
               m_TreeCtrl->SetItemHasChildren( m_Children[i], true );

            // If a folder has no children.
            //if ( !m_TreeCtrl->ItemHasChildren( m_Children[i] ) && )
               //m_TreeCtrl->SetItemHasChildren( m_Children[i], true );

            // Colorize it depending on the filters.
            ColorizeItem( m_Children[i], dir.GetFullPath() );

            m_Children.RemoveAt( i );

            // Should we recurse it?
            //if ( m_TreeCtrl->IsExpanded( m_Children[i] ) )
               //return wxDIR_CONTINUE

            return wxDIR_IGNORE;
         }
      }

      if ( tsGetPrefs().IsExcludedFolder( dir.GetDirs().Last() ) )
         return wxDIR_IGNORE;

      // Add the folder to the tree.
      wxTreeItemId item = m_TreeCtrl->AppendItem( m_Parent, dir.GetDirs().Last(),
         1, 1, new ProjectCtrlItemData( dir.GetFullPath() ) );
      m_TreeCtrl->SetItemImage( item, 2, wxTreeItemIcon_Expanded );

      // Add a dummy child to allow for expansion later.
      m_TreeCtrl->SetItemHasChildren( item, true );

      // Colorize it depending on the filters.
      ColorizeItem( item, dir.GetPath() );

      return wxDIR_IGNORE;
   }

private:

   void ColorizeItem( const wxTreeItemId& item, const wxString& folder )
   {
      wxColour color( 0, 0, 0 );

      wxASSERT( m_Project );
      if ( m_ShowAllMods && !m_Project->IsMod( folder ) )
         color.Set( 255, 0, 0 );

      m_TreeCtrl->SetItemTextColour( item, color );
   }

   ProjectCtrl*   m_TreeCtrl;
   ProjectDoc*    m_Project;

   bool  m_ShowAllMods;
   bool  m_ShowAllFiles;

   wxTreeItemId         m_Parent;
   TreeItemIdArray&     m_Children;

};


void ProjectCtrl::Refresh( ProjectDoc* project, bool showAllMods, bool showAllFiles, bool clearFirst )
{
   m_ProjectDoc = project;
   m_ShowAllMods = showAllMods;
   m_ShowAllFiles = showAllFiles;
   wxASSERT( m_ProjectDoc );

   // If we have a root be sure it's ok and has 
   // not been changed on us.
   wxFileName rootdir;
   rootdir.AssignDir( m_ProjectDoc->GetWorkingDir() );
   wxTreeItemId root = GetRootItem();
   if ( root.IsOk() )
   {
      if ( clearFirst || !rootdir.SameAs( GetItemPath( root ) ) ) 
      {
         Delete( root );
         root.Unset();
      }
   }

   // Do we need to create a new root?
   if ( !root.IsOk() ) 
   {
      root = AddRoot( m_ProjectDoc->GetName(), 0, 0, new ProjectCtrlItemData( rootdir.GetFullPath() ) );

      const wxArrayString& excluded = tsGetPrefs().GetExcludedFolders();
      m_DirWatcher.SetWatch( rootdir.GetFullPath(), DIRCHANGE_FILE_NAME | DIRCHANGE_DIR_NAME, excluded );

      // Add a dummy child to allow for expansion.
      SetItemHasChildren( root, true );
      //AppendItem( root, "", -1, -1 );

      // Do an initial expand of the root which 
      // will automaticly update children as needed.
      Expand( root );
      return;
   }

   RefreshFolder( root );
   FixupFolders( root );
}

void ProjectCtrl::OnItemCollapsing( wxTreeEvent& Event )
{
   wxTreeItemId item = Event.GetItem();

   // Never let the root collapse.
   if ( item == GetRootItem() )
   {
      Event.Veto();
      return;
   }
}

void ProjectCtrl::OnItemCollapsed( wxTreeEvent& event )
{
   wxTreeItemId item = event.GetItem();
   DeleteChildren( item );
   SetItemHasChildren( item, true );
}

void ProjectCtrl::OnItemExpanding( wxTreeEvent& Event )
{
   wxTreeItemId item = Event.GetItem();
   RefreshFolder( item );
}

void ProjectCtrl::OnItemExpanded( wxTreeEvent& Event )
{
   wxTreeItemId item = Event.GetItem();
   FixupFolders( item );
}

bool ProjectCtrl::RefreshFolder( wxTreeItemId item )
{
   wxString itemPath = GetItemPath( item );
   wxASSERT( !itemPath.IsEmpty() );

   if ( !wxFileName::DirExists( itemPath ) )
   {
      Delete( item );
      return false;
   }

   // Grab the existing children.
   TreeItemIdArray children;
   children.Alloc( GetChildrenCount( item, false ) );
   wxTreeItemIdValue dummy;
   wxTreeItemId child = GetFirstChild( item, dummy );
   while ( child.IsOk() ) 
   {
      children.Add( child );
      child = GetNextChild( item, dummy );
   }

   // Enumerate the dir adding new children.
   ProjectDirTraverser sink( this, item, children, m_ShowAllMods, m_ShowAllFiles );
	wxDir dir( itemPath );
	if ( dir.IsOpened() )
		dir.Traverse( sink, wxEmptyString, wxDIR_DIRS | wxDIR_FILES );

   // Any children still in the list need to be deleted!
   for ( int i=0; i < children.GetCount(); i++ ) 
   {
      wxASSERT( children[i].IsOk() );
      Delete( children[i] );
   }
   children.Clear();

   // Recurse the remaining expanded folders.
   child = GetFirstChild( item, dummy );
   while ( child.IsOk() ) 
   {
      if ( IsExpanded( child ) )
         RefreshFolder( child );
      child = GetNextChild( item, dummy );
   }

   // If we have no children then clear the children flag!
   if ( GetChildrenCount( item, false ) < 1 )
   {
      AppendItem( item, "dummy", 0, 0, NULL );
      Collapse( item );
      DeleteChildren( item );
      SetItemHasChildren( item, false );
   }

   return true;
}

void ProjectCtrl::FixupFolders( wxTreeItemId item )
{
   bool hasChildren = GetChildrenCount( item, false ) > 0;
   if ( IsExpanded( item ) && !hasChildren )
   {
      // First add a dummy child... else we cannot collapse!
      AppendItem( item, "dummy", 0, 0, NULL );
      Collapse( item );
      DeleteChildren( item );
      SetItemHasChildren( item, true );
      return;
   }

   if ( hasChildren )
      SortChildren( item );

   wxTreeItemIdValue dummy;
   wxTreeItemId child = GetFirstChild( item, dummy );
   while ( child.IsOk() ) 
   {
      if ( IsExpanded( child ) )
         FixupFolders( child );
      child = GetNextChild( item, dummy );
   }
}

void ProjectCtrl::OnInternalIdle()
{
   wxTreeCtrl::OnInternalIdle();
   CheckWatcher();
}

void ProjectCtrl::CheckWatcher()
{
   wxArrayString signaled;
   if ( m_DirWatcher.GetSignaled( &signaled ) <= 0 ) 
      return;

   Freeze();

   for ( int i=0; i < signaled.GetCount(); i++ )
   {
      const wxString& dir = signaled[i];

      // Find the signaled tree item if it exists
      // without needing to expand any items.
      wxTreeItemId item = FindIdFromPath( dir, false );
      if ( !item.IsOk() )
         continue;

      // If the directory doesn't exist delete it.
      if ( !wxFileName::DirExists( dir ) )
      {
         Delete( item );
         continue;
      }

      // Refresh the children in the item.
      RefreshFolder( item );
   }

   Thaw();
}

wxTreeItemId ProjectCtrl::FindIdFromPath( const wxString& signaled, bool expand )
{
   wxTreeItemId item = GetRootItem();
   if ( !item.IsOk() )
      return item;

   // Make it relative to the project.
   wxASSERT( m_ProjectDoc );
   wxFileName path;
   path.AssignDir( m_ProjectDoc->MakeReleativeTo( signaled ) );
   if ( path.IsAbsolute() )
      return wxTreeItemId();

   const wxArrayString& dirs = path.GetDirs();
   wxTreeItemIdValue dummy;

   // Walk the dir paths looking for the the id.
   for ( int i=0; i < dirs.GetCount(); i++ )
   {
      if ( expand && !IsExpanded( item ) )
         Expand( item );

      // Look at the child elements for it.
      wxTreeItemId child = GetFirstChild( item, dummy );
      while ( child.IsOk() ) 
      {
         if ( GetItemText( child ).IsSameAs( dirs[i], wxFileName::IsCaseSensitive() ) )
            break;
         child = GetNextChild( item, dummy );
      }

      item = child;
      if ( !item.IsOk() )
         break;
   }

   return item;
}

void ProjectCtrl::OnBeginRename( wxTreeEvent& Event )
{
   if ( Event.GetItem() == GetRootItem() )
      Event.Veto();
}

void ProjectCtrl::OnEndRename( wxTreeEvent& Event )
{
   // We never allow the label to be changed here...
   // if we want the change, rename the file and the
   // dir watcher will take care of updating the item.
   Event.Veto();

   wxString newName = Event.GetLabel();
   if ( newName.IsEmpty() )
      return;

   wxFileName Source = GetItemPath( Event.GetItem() );
   wxFileName Dest;
   Dest.AssignDir( Source.GetPath() );

   if ( Source.IsDir() ) 
   {
      Dest.RemoveLastDir();
      Dest.AppendDir( newName );
   }
   else
      Dest.SetFullName( newName );

   if ( Dest.SameAs( Source ) )
      return;

   if ( !wxRenameFile( Source.GetFullPath(), Dest.GetFullPath() ) )
      return;

   // Give the views a hint to rename any open docs.
   wxASSERT( tsGetMainFrame() );
   tsFileRenameHint hint;
   hint.oldPath = Source.GetFullPath();
   hint.newPath = Dest.GetFullPath();
   tsGetMainFrame()->SendHintToAllViews( &hint, true );

   // Immediately update the dir watch which
   // will delete this item and replace it with
   // the renamed one.
   CheckWatcher();
}

void ProjectCtrl::OnDragBegin( wxTreeEvent& event )
{
   // We sometimes get a double drag start on wxMSW... this
   // is because wxTreeCtrl implements its own drag detection
   // and doesn't disable the native treeview control from
   // sending it's own begin drag event.  Reported bug.
   if ( m_IsDragging )
      return;

   // Grab the current selection.
   UnnestSelections();
   GetSelections( m_DragItems );

   // We cannot drag the root!
   if ( m_DragItems.Index( GetRootItem().m_pItem ) != wxNOT_FOUND )
      return;

   m_LastHighlight = event.GetItem();
   UnselectAll();
   SetItemDropHighlight( m_LastHighlight );
   m_IsDragging = true;
   CaptureMouse();
   ::wxSetCursor( m_DragCursor );
}

void ProjectCtrl::OnDragMove( wxMouseEvent& event )
{
   if ( !m_IsDragging ) 
   {
      event.Skip();
      return;
   }

   // Check for lost captures.
   if ( !HasCapture() ) 
   {
      m_IsDragging = false;
      SetItemDropHighlight( m_LastHighlight, false );
      m_LastHighlight.Unset();
      //::wxSetCursor( wxNullCursor );
      m_DragExpandTimer.Stop();
      SelectItems( m_DragItems );
      return;
   }

   int flags = 0;
   wxTreeItemId newItem = HitTest( event.GetPosition(), flags );
   if ( !newItem.IsOk() ) 
   {
      ::wxSetCursor( wxCursor( wxCURSOR_NO_ENTRY ) );

      if ( m_LastHighlight.IsOk() )
      {
         SetItemDropHighlight( m_LastHighlight, false );
         SelectItems( m_DragItems );
         m_DragExpandTimer.Stop();
         m_LastHighlight.Unset();
      }
     
      return;
   }
   
   ::wxSetCursor( m_DragCursor );

   if ( newItem != m_LastHighlight )
   {
      UnselectAll();
      SetItemDropHighlight( m_LastHighlight, false );
      m_LastHighlight = newItem;
      SetItemDropHighlight( m_LastHighlight );
      m_DragExpandTimer.Start( 750, true );
   }
}

void ProjectCtrl::OnDragExpandTimer( wxTimerEvent& event )
{
   if (  m_LastHighlight.IsOk() &&
         ItemHasChildren( m_LastHighlight ) &&
         !IsExpanded( m_LastHighlight ) )
      Expand( m_LastHighlight );
}

void ProjectCtrl::OnDragKey( wxKeyEvent& event )
{
   if ( !m_IsDragging ) 
   {
      event.Skip();
      return;
   }

   if ( event.GetKeyCode() == WXK_ESCAPE )
   {
      m_IsDragging = false;
      SetItemDropHighlight( m_LastHighlight, false );
      m_LastHighlight.Unset();
      m_DragExpandTimer.Stop();
      SelectItems( m_DragItems );
      ReleaseMouse();
   }
}

void ProjectCtrl::OnDragEnd( wxMouseEvent& event )
{
   if ( !m_IsDragging ) 
   {
      event.Skip();
      return;
   }

   m_IsDragging = false;
   m_DragExpandTimer.Stop();
   //::wxSetCursor( wxNullCursor );
   SetItemDropHighlight( m_LastHighlight, false );
   m_LastHighlight.Unset();
   if ( !HasCapture() )
   {
      SelectItems( m_DragItems );
      return;
   }

   ReleaseMouse();

   int flags = 0;
   wxTreeItemId destItem = HitTest( event.GetPosition(), flags );
   if ( !destItem.IsOk() ) 
   {
      SelectItems( m_DragItems );
      return;
   }

   // Get the dest item.
   wxFileName dest = GetItemPath( destItem );
   if ( !dest.IsDir() ) 
   {
      // The dest must be a folder... so assume
      // the user means the operation to occur
      // on the dest file's folder.
      destItem = GetItemParent( destItem );
      dest = GetItemPath( destItem );
      if ( !dest.IsDir() )
      {
         SelectItems( m_DragItems );
         return;
      }
   }

   // Make an array of the items that can be moved and copied to the dest.
   wxArrayTreeItemIds sourceItems = m_DragItems;
   {
      // First remove items which are a parent to the dest.
      wxTreeItemId id = destItem;
      while ( id.IsOk() )
      {
         size_t index = sourceItems.Index( id.m_pItem );

         if ( index != wxNOT_FOUND )
            sourceItems.RemoveAt( index );

         id = GetItemParent( id );
      }

      // Now remove items whos parent is the dest and
      // the operation would be redundent.
      for ( int i=0; i < sourceItems.GetCount(); )
      {
         if ( destItem == GetItemParent( sourceItems[i] ) )
         {
            sourceItems.RemoveAt( i );
            continue;
         }
         i++;
      }
   }

   // If we have no items left then the drag was
   // completely invalid... so barf.
   if ( sourceItems.IsEmpty() )
   {
      SelectItems( m_DragItems );
      tsBellEx( wxICON_INFORMATION );
      return;
   }

   // Popup a menu and get the result inline.
   int result = 0; 
   {
      tsMenu* menu = new tsMenu;
      menu->Append( tsID_MOVE, _T( "Move Here" ) );
      menu->Append( tsID_COPY, _T( "Copy Here" ) );
      menu->AppendSeparator();
      menu->Append( wxID_CANCEL, _T( "Cancel" ) );
      result = tsTrackPopupMenu( menu, true, event.GetPosition(), this );
      delete menu;
   }

   if ( result == 0 || result == wxID_CANCEL )
   {
      SelectItems( m_DragItems );
      return;
   }

   // Ok build the source string array.
   wxArrayString sourceArray;
   for ( int i=0; i < sourceItems.GetCount(); i++ )
   {
      wxString path = GetItemPath( sourceItems[i] );
      wxASSERT( !path.IsEmpty() );
      sourceArray.Add( path );
   }

   if ( result == tsID_COPY )
      tsCopyFiles( sourceArray, dest.GetFullPath() );

   else
   {
      wxASSERT( result == tsID_MOVE );
      tsMoveFiles( sourceArray, dest.GetFullPath() );
   }

   // Let the watcher update the tree items.
   CheckWatcher();

   // Go ahead and expand the dest if it's not already so.
   if ( !IsExpanded( destItem ) ) 
   {
      // Gotta force it to have children to 
      // get it to expand.
      SetItemHasChildren( destItem, true );
      Expand( destItem );
   }
}

void ProjectCtrl::OnOpenItem( wxTreeEvent& Event )
{
   if ( Event.GetItem().IsOk() )
   {
      OpenItem( Event.GetItem() );
      return;
   }

   // Open all selected items.
   wxArrayTreeItemIds selected;
   GetSelections( selected );
   for ( int i=0; i < selected.GetCount(); i++ )
      OpenItem( selected[i] );
}

void ProjectCtrl::OnItemMenu( wxTreeEvent& Event )
{
   // We're about to force the focus to ourselves
   // so remove any focus view override.
   m_FocusView = NULL;

   // Make sure we have the focus and the
   // active document.
   SetFocus();
   wxASSERT( this == FindFocus() );
   wxASSERT( m_ProjectDoc );
   wxASSERT( m_ProjectDoc->GetFirstView() );
   m_ProjectDoc->GetFirstView()->Activate( true );
   wxASSERT( this == FindFocus() );
   wxASSERT( m_ProjectDoc->GetDocumentManager()->GetCurrentView() == m_ProjectDoc->GetFirstView() );

   // TODO: Add icons to context menu!

   // Get the current selection set.
   wxArrayTreeItemIds selected;
   GetSelections( selected );

   // If this is only the root then have a very
   // simple menu... just do it and return.
   if ( selected.GetCount() == 1 && selected[0] == GetRootItem() ) 
   {
      tsMenu* menu = new tsMenu;

      menu->AppendIconItem( tsID_EXPLORE, _T( "&Explore" ), ts_explorer16 );
      menu->AppendSeparator();
      menu->Append( tsID_NEW_FILE, _T( "New Script" ) );
      menu->Append( tsID_NEW_FOLDER, _T( "New Folder" ) );
      menu->AppendSeparator();
      menu->Append( tsID_CLEARDSOS, _T( "&Delete DSOs" ) );
      menu->AppendSeparator();
      menu->Append( tsID_PROJECT_PROPERTIES, _T( "&Properties" ) );

      // Pop them menu... doesn't return till the menu is
      // canceled or it executes a command.
      PopupMenu( menu, Event.GetPoint() );
      delete menu;

      return;
   }

   // If we have multiple selections and one is the 
   // root item... remove it.
   const wxTreeItemId rootId = GetRootItem();
   if ( selected.GetCount() > 1 && selected.Index( rootId.m_pItem ) != wxNOT_FOUND )
   {
      SelectItem( rootId, false );
      selected.Empty();
      GetSelections( selected );
      wxASSERT( selected.Index( rootId.m_pItem ) == wxNOT_FOUND );
   }

   // Get some counts on the number of files and folder selected.
   size_t DSOs = 0; size_t files = 0; size_t folders = 0;
   {
      wxString path;
      for ( int i=0; i < selected.GetCount(); i++ )
      {
         path = GetItemPath( selected[i] );
         if ( wxFileName::DirExists( path ) )
            ++folders;
         else
         {
            ++files;
            if ( tsGetPrefs().IsScriptFile( path ) )
            {
               if ( !tsGetPrefs().GetDSOForScript( path ).IsEmpty() )
                  ++DSOs;
            }
         }
      }
   }

   tsMenu* menu = new tsMenu;
   if ( files == selected.GetCount() ) 
   {
      menu->AppendIconItem( tsID_PROJECT_OPEN, _T( "&Open" ), ts_open_document16 );

      if ( files == 1 )
         menu->Append( tsID_PROJECT_OPENWITH, _T( "Open Wit&h..." ) );

      if ( DSOs )
         menu->Append( tsID_CLEARDSO, _T( "Delete DSO" ) );

      if ( files == 1 )
         menu->AppendIconItem( tsID_EXPLORE, _T( "E&xplore" ), ts_explorer16 );

      menu->AppendSeparator();
   } 
   else if ( folders == 1 && files == 0 )
   {
      menu->AppendIconItem( tsID_EXPLORE, _T( "E&xplore" ), ts_explorer16 );
      menu->AppendSeparator();
      menu->Append( tsID_NEW_FILE, _T( "&New Script" ) );
      menu->Append( tsID_NEW_FOLDER, _T( "&New Folder" ) );
      menu->AppendSeparator();
      menu->AppendIconItem( tsID_FINDINFILES, _T( "F&ind in Files" ), ts_findinfiles16 );
      menu->AppendSeparator();
   }

   #ifdef __WXMSW__
   if ( selected.GetCount() == 1 )
   {
      wxString file = GetItemPath( selected[0] );
      m_OsMenu = new ShellMenu( file );
      menu->Append( wxID_ANY, _T( "&Explorer" ), m_OsMenu );
      menu->AppendSeparator();
   }
   #endif

   menu->AppendIconItem( wxID_DELETE, _T( "&Delete" ), ts_delete16 );

   if ( selected.GetCount() == 1 )
   {
      menu->Append( tsID_RENAME, _T( "Rena&me" ) );
      menu->AppendSeparator();
      menu->Append( wxID_PROPERTIES, _T( "P&roperties" ) );
   }

   // Pop them menu... doesn't return till the menu is
   // canceled or it executes a command.
   PopupMenu( menu, Event.GetPoint() );
   delete menu;

   #ifdef __WXMSW__
      m_OsMenu = NULL;
   #endif
}

#ifdef __WXMSW__
WXLRESULT ProjectCtrl::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
   if (  m_OsMenu && 
         m_OsMenu->MSWWindowProc(nMsg, wParam, lParam) == 0 )
      return 0;

   return wxTreeCtrl::MSWWindowProc(nMsg, wParam, lParam);
}
#endif

void ProjectCtrl::OpenItem( const wxTreeItemId& item )
{
   wxFileName file( GetItemPath( item ) );
   if ( file.IsDir() ) {
      return;
   }

   if (  tsGetPrefs().IsScriptFile( file.GetFullPath() ) ||
         tsGetPrefs().IsTextFile( file.GetFullPath() ) ) {

      // While OpenFile() will set focus to the editor control
      // it is stomped on by a mystery WM_FOCUS message on this
      // control.  Store the focus view which we redirect the
      // focus message to when we recieve it.
      wxASSERT( tsGetMainFrame() );
      m_FocusView = tsGetMainFrame()->OpenFile( file.GetFullPath() );

   } else {

      // Launch the associated application.
      wxString command;
      wxFileType* type = wxTheMimeTypesManager->GetFileTypeFromExtension( file.GetExt() );
      if ( type ) {
         
         wxString command = type->GetOpenCommand( file.GetFullPath() );
         if ( !command.IsEmpty() ) {
            if ( wxExecute( command, wxEXEC_ASYNC, NULL ) != 0 ) {
               return;
            }
         }
      }

      OpenItemWith( item );
   }
}

void ProjectCtrl::OpenItemWith( const wxTreeItemId& item )
{
   wxFileName file( GetItemPath( item ) );
   if ( file.IsDir() ) {
      return;
   }

   // If we got here then let the shell handle it!
   tsShellOpenAs( file.GetFullPath() );
}

wxTreeItemId ProjectCtrl::Select( const wxString& path )
{
   // Before we select do an idle tick, so
   // than any pending changes in the project
   // tree are dealt with.
   CheckWatcher();

   wxTreeItemId item = FindIdFromPath( path, true );
   if ( item.IsOk() )
   {
      UnselectAll();
      EnsureVisible( item );
      SelectItem( item );
   }

   return item;
}  

wxString ProjectCtrl::GetItemPath( const wxTreeItemId& item ) const
{
   if ( !item.IsOk() )
      return wxEmptyString;

   ProjectCtrlItemData* data = (ProjectCtrlItemData*)GetItemData( item );
   if ( !data )
      return wxEmptyString;

   return data->GetPath();
}

void ProjectCtrl::OnToolTip( wxTreeEvent& Event )
{
   wxTreeItemId item = Event.GetItem();
   if ( item.IsOk() ) {

      ProjectCtrlItemData* data = (ProjectCtrlItemData*)GetItemData( item );
      if ( data ) {

         /*
         wxFileName file( data->m_FullPath );
         wxDateTime mod = file.GetModificationTime();
         
         wxFileName::();
         Event.SetToolTip(  );
         */
      }
   }
}

int ProjectCtrl::OnCompareItems( const wxTreeItemId& item1, const wxTreeItemId& item2 )
{
   wxFileName file1( GetItemPath( item1 ) );
   wxFileName file2( GetItemPath( item2 ) );

   // Directories always go first!
   if ( file1.IsDir() && !file2.IsDir() ) 
      return -1;
   if ( file2.IsDir() && !file1.IsDir() ) 
      return 1;

   return file1.GetFullPath().CmpNoCase( file2.GetFullPath() );
}

void ProjectCtrl::NewFolder( wxTreeItemId item )
{
   wxFileName parent = GetItemPath( item );

   // Get a folder name!
   wxFileName folder;
   int i = 1;
   do
   {
      wxString name;
      name << "NewFolder" << i++;
      folder.AssignDir( parent.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) + name );

   } while ( folder.DirExists() );

   // Create the folder.
   if ( !wxMkdir( folder.GetFullPath() ) )
   {
      wxMessageDialog dlg( this, "The folder could not be created!", "Torsion", wxOK | wxICON_ERROR );
      dlg.ShowModal();
      return;
   }

   // Select the folder in the view.
   item = Select( folder.GetFullPath() );
   EditLabel( item );
}

void ProjectCtrl::NewFile( wxTreeItemId item )
{
   wxFileName parent = GetItemPath( item );

   // Get a file name!
   wxFileName file;
   const wxString ext = tsGetPrefs().GetDefaultScriptExtension();
   int i = 1;
   do
   {
      wxString name;
      name << parent.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) << "NewScript" << i++ << "." << ext;
      file.Assign( name );

   } while ( file.FileExists() );

   // Create the file.
   wxFile dummy( file.GetFullPath(), wxFile::write_excl );
   if ( !dummy.IsOpened() )
   {
      wxMessageDialog dlg( this, "The file could not be created!", "Torsion", wxOK | wxICON_ERROR );
      dlg.ShowModal();
      return;
   }

   // Make sure the parent is expanded!
   if ( !IsExpanded( item ) )
   {
      SetItemHasChildren( item, true );
      Expand( item );
   }

   // Select the file in the view and edit it's name.
   item = Select( file.GetFullPath() );
   EditLabel( item );
}

void ProjectCtrl::UnnestSelections()
{
   wxArrayTreeItemIds selected;
   GetSelections( selected );

   // Remove any id that is a child of another id
   // in the list.  This removes children that may
   // be selected under a folder.
   for ( int i=0; i < selected.GetCount(); i++ )
   {
      wxTreeItemId id = selected[i];
      int found = wxNOT_FOUND;

      while ( (id = GetItemParent(id)).IsOk() )
      {
         found = selected.Index( id.m_pItem );
         if ( found != wxNOT_FOUND )
         {
            SelectItem( selected[i], false );
            break;
         }
      }
   }
}

void ProjectCtrl::SelectItems( const wxArrayTreeItemIds& items )
{
   UnselectAll();

   for ( int i=0; i < items.GetCount(); i++ )
      SelectItem( items[i] );
}


