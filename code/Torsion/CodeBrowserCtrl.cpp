// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "CodeBrowserCtrl.h"

//#include "ProjectDoc.h"
#include "TorsionApp.h"
#include "AutoComp.h"
#include "AutoCompPage.h"
#include "MainFrame.h"
#include "GotoDefDialog.h"

#include "Icons.h"

#include <algorithm>



#ifdef __WXMSW__
   #include <commctrl.h>
#endif
#include <wx/dcbuffer.h>

//#include <wx/arrimpl.cpp>
//WX_DEFINE_OBJARRAY(TreeItemIdArray);

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_CLASS( CodeBrowserCtrl, wxTreeCtrl )

BEGIN_EVENT_TABLE( CodeBrowserCtrl, wxTreeCtrl )

   EVT_TREE_ITEM_EXPANDING( wxID_ANY, CodeBrowserCtrl::OnItemExpanding )
   //EVT_TREE_ITEM_COLLAPSED( wxID_ANY, CodeBrowserCtrl::OnItemCollapsed )
   EVT_TREE_ITEM_RIGHT_CLICK( wxID_ANY, CodeBrowserCtrl::OnRightClickItem )
   EVT_TREE_ITEM_MENU( wxID_ANY, CodeBrowserCtrl::OnRightClickItem )
   EVT_TREE_ITEM_ACTIVATED( wxID_ANY, CodeBrowserCtrl::OnItemActivated )

   EVT_COMMAND(wxID_ANY, tsEVT_AUTOCOMPDATA_UPDATED, CodeBrowserCtrl::OnDataUpdated )

   //EVT_SET_FOCUS( CodeBrowserCtrl::OnSetFocus )
   //EVT_KILL_FOCUS( CodeBrowserCtrl::OnKillFocus )

   EVT_TREE_SEL_CHANGED( wxID_ANY, CodeBrowserCtrl::OnSelChanged )
   //EVT_TREE_ITEM_GETTOOLTIP( wxID_ANY, OnToolTip )

   // To fix flicker!
   #ifdef __WXMSW__
      EVT_ERASE_BACKGROUND( CodeBrowserCtrl::OnEraseBackground )
      EVT_PAINT( CodeBrowserCtrl::OnPaint ) 
   #endif // __WXMSW__

END_EVENT_TABLE()


CodeBrowserCtrl::CodeBrowserCtrl( wxWindow* parent, wxWindowID id, 
         const wxPoint& pos, const wxSize& size )
   :  wxTreeCtrl( parent, id, pos, size, wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_LINES_AT_ROOT | wxSTATIC_BORDER ),
      m_Filter( FILTER_ALL ),
      m_Data( NULL ),
      m_ExportColor( 0, 0, 128 ),
      m_IsActive( false ),
      m_NeedUpdate( false ),
      m_IsActivate( false ),
      m_DescCtrl( NULL ),
      m_DonePaint( false )
{
   // NOTE: This must match the ITYPE enum in the header!
   wxImageList* TreeImages = new wxImageList( 16, 16, true );
   TreeImages->Add( ts_globals16 );           // ITYPE_VAR
	TreeImages->Add( ts_functions16 );         // ITYPE_FUNC
	TreeImages->Add( ts_class16 );             // ITYPE_CLASS
	TreeImages->Add( ts_datablock16 );         // ITYPE_DATABLOCK
	TreeImages->Add( ts_object16 );            // ITYPE_OBJECT
	TreeImages->Add( ts_datablock_object16 );  // ITYPE_DATABLOCKOBJECT
   TreeImages->Add( ts_namespace16 );         // ITYPE_NAMESPACE
	TreeImages->Add( ts_package16 );           // ITYPE_PACKAGE
	AssignImageList( TreeImages );

   // Add the root now... we won't need to change it.
   AddRoot( "", -1, -1, new ItemData );

   // Add ourselves to the autocomp notification list.
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->AddNotify( this );

   // Set the initial enabled state.
   Enable( tsGetPrefs().GetCodeCompletion() );
}

CodeBrowserCtrl::~CodeBrowserCtrl()
{
   // Remove ourselves from the autocomp notification list.
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->RemoveNotify( this );
}


#ifdef __WXMSW__

   void CodeBrowserCtrl::OnEraseBackground( wxEraseEvent& event )
   {
      // Let the native control draw the background up
      // until we get our first real paint.
      if ( !m_DonePaint )
         event.Skip();
   }

   void CodeBrowserCtrl::OnPaint( wxPaintEvent& event )
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
      ::SendMessage( GetHandle(), WM_PAINT, (WPARAM)dc.GetHDC(), 0 );
      SetEvtHandlerEnabled( true );
   }

#endif // __WXMSW__

bool CodeBrowserCtrl::Enable( bool enable )
{
   if ( enable )
      SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
   else 
      SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

   Refresh( true );

   return wxTreeCtrl::Enable( enable );
}

void CodeBrowserCtrl::SetFilterValue( bool enabled, unsigned int bit )
{
   unsigned int filter = m_Filter;

   if ( enabled )
      filter |= bit;
   else
      filter ^= bit;

   if ( filter != 0 && m_Filter != filter )
   {
      m_Filter = filter; 
      UpdateData();
   }
}

wxTreeItemId CodeBrowserCtrl::ReviveItemFromCache( const wxString& sid )
{
   // First check if we already have this
   // in the tree using the previous map.
   StringToIdMap::iterator iter = m_CachedIds.find( sid );
   if ( iter != m_CachedIds.end() && (*iter).second.IsOk() ) {

      wxTreeItemId result = (*iter).second;
      m_CurrentIds[ sid ] = (*iter).second;
      m_CachedIds.erase( iter );
      return result;
   }

   // Ok... it's not in the old list, but it might
   // be in the current list as there could be 
   // duplicate info in the pages.
   iter = m_CurrentIds.find( sid );
   if ( iter != m_CurrentIds.end() ) {

      // Just continue... nothing to add.
      return (*iter).second;;
   }

   return wxTreeItemId();
}

wxTreeItemId CodeBrowserCtrl::AppendItem( const wxString& sid, wxTreeItemId parent, const wxString& name, ITYPE type )
{
   ItemData* data = new ItemData;
   data->name = name;
   data->type = type;
   data->sid = sid;

   wxTreeItemId result = wxTreeCtrl::AppendItem( parent, name, type, type, data );
   m_CurrentIds[ sid ] = result;
   return result;
}

void CodeBrowserCtrl::OnInternalIdle()
{
   m_IsActive =   FindFocus() == this ||
                  IsEnabled() != tsGetPrefs().GetCodeCompletion(); // IsShown() && GetParent()->IsShown();

   if ( m_IsActive && m_NeedUpdate )
      UpdateData();

   wxTreeCtrl::OnInternalIdle();
}

void CodeBrowserCtrl::OnDataUpdated( wxCommandEvent& event )
{
   m_NeedUpdate = true;
}

void CodeBrowserCtrl::UpdateData()
{
   m_NeedUpdate = false;

   // Grab the autocomp pages... note we're getting an
   // exclusive lock on them.  We need to work quick.
   wxASSERT( tsGetAutoComp() );
   m_Data = tsGetAutoComp()->Lock();

   // Copy the current over to the old list.
   m_CachedIds = m_CurrentIds;
   m_CurrentIds.clear();

   Freeze();

   // If we have the data page then merge it into the tree root.
   if ( m_Data )
   {
      const AutoCompClassArray& classes = m_Data->GetClasses();
      for ( int i=0; i < classes.GetCount(); i++ )
      {
         if ( classes[i]->IsDatablock() )
         {
            if ( m_Filter & FILTER_DATABLOCKS )
               MergeClass( GetRootItem(), classes[i], ITYPE_DATABLOCK );
         }
         else if ( m_Filter & FILTER_CLASSES )
            MergeClass( GetRootItem(), classes[i], ITYPE_CLASS );
      }

      const AutoCompClassArray& objects = m_Data->GetObjects();
      for ( int i=0; i < objects.GetCount(); i++ )
      {
         // Skip unnamed objects.
         if ( objects[i]->GetName().IsEmpty() )
            continue;

         if ( objects[i]->IsNamespace() )
         {
            if ( m_Filter & FILTER_NAMESPACES )
               MergeClass( GetRootItem(), objects[i], ITYPE_NAMESPACE );
         }
         else if ( objects[i]->IsDatablock() )
         {
            if ( (m_Filter & FILTER_DATABLOCKOBJECT) == FILTER_DATABLOCKOBJECT )
               MergeClass( GetRootItem(), objects[i], ITYPE_DATABLOCKOBJECT );
         }
         else if ( (m_Filter & FILTER_OBJECT) == FILTER_OBJECT )
            MergeClass( GetRootItem(), objects[i], ITYPE_OBJECT );
      }

      if ( m_Filter & FILTER_FUNCTIONS )
         MergeFunctions( GetRootItem(), m_Data->GetFunctions(), wxEmptyString );

      if ( m_Filter & FILTER_GLOBALS )
         MergeVars( GetRootItem(), m_Data->GetVars(), true );
   }

   m_Data = NULL;
   tsGetAutoComp()->Unlock();

   // Anything left in the cached id list is a
   // dead id... remove them before we do anything else.
   StringToIdMap::iterator iter = m_CachedIds.begin();
   for ( ; iter != m_CachedIds.end(); iter++ )
   {
      if ( (*iter).second.IsOk() )
         Delete( (*iter).second );
   }
   m_CachedIds.clear();

   SortChildren( GetRootItem() );

   Thaw();

   Enable( tsGetPrefs().GetCodeCompletion() && GetChildrenCount( GetRootItem() ) > 0 );
}

void CodeBrowserCtrl::MergeFunctions( wxTreeItemId parent, const AutoCompFunctionArray& functions, const wxString& namespace_ )
{
   wxString sid;
   wxString parentSid = GetSid( parent );
   wxString name;

   for ( int i=0; i < functions.GetCount(); i++ ) {

      wxASSERT( functions[i] );
      name = functions[i]->GetName();

      MakeSid( &sid, parentSid, name, ITYPE_FUNC );
      wxTreeItemId id = ReviveItemFromCache( sid );
      if ( !id.IsOk() )
      {
         wxString fullName = name;

         if ( !namespace_.IsEmpty() )
            fullName = namespace_ + "::" + name;

         id = AppendItem( sid, parent, fullName, ITYPE_FUNC );
         if ( functions[i]->IsExport() )
            SetItemTextColour( id, m_ExportColor );
      }

      // Set the name with arguments.
      name << "(" << functions[i]->GetArgs() << ")";
      SetItemText( id, name );
   }
}

void CodeBrowserCtrl::MergeVars( wxTreeItemId parent, const AutoCompVarArray& vars, bool globalsOnly )
{
   for ( int j=0; j < vars.GetCount(); j++ ) {

      const AutoCompVar* var = vars[j];
      wxASSERT( var );

      if ( globalsOnly && !var->IsGlobal() )
         continue;

      MergeVar( parent, var );
   }
}

void CodeBrowserCtrl::MergeVar( wxTreeItemId parent, const AutoCompVar* var )
{
   wxASSERT( var );
   wxString name = var->m_Name;

   ItemData* item = (ItemData*)GetItemData( parent );
   wxASSERT( item );
   wxString parentSid = item->sid;
   wxString parentName;
   if ( item->type == ITYPE_VAR )
   {
      //wxASSERT( name.StartsWith( item->name ) );
      name.Remove( 0, item->name.Len() + 2 );
      parentSid += "/";
      parentName = item->name;
   }

   ITYPE type = ITYPE_VAR;
   wxTreeItemId id;
   wxString fixed;
   wxString sid;

   // Break out the namespaces!
   for ( ;; ) 
   {
      int sep = name.Find( "::" );
      if ( sep != -1 ) 
      {
         MakeSid( &sid, parentSid, name.Left( sep ), type );
         id = ReviveItemFromCache( sid );
         if ( !id.IsOk() ) 
         {
            fixed = name.Left( sep );

            wxString fullName = fixed;

            if ( !parentName.IsEmpty() )
               fullName = parentName + "::" + fixed;

            id = AppendItem( sid, parent, fullName, type );
            if ( var->IsExport() )
               SetItemTextColour( id, m_ExportColor );

            SetItemText( id, fixed );
         }

         parent = id;
         name = name.Mid( sep + 2 );
         parentSid = sid + "/";

         if ( IsExpanded( id ) )
            continue;
         else
         {
            SetItemHasChildren( id, true );
            return;
         }
      }

      break;
   }

   if ( name.IsEmpty() )
      return;

   type = ITYPE_VAR;
   MakeSid( &sid, parentSid, name, type );
   if ( !ReviveItemFromCache( sid ).IsOk() ) {

      wxString fullName = name;

      if ( !parentName.IsEmpty() )
         fullName = parentName + "::" + name;

      id = AppendItem( sid, parent, fullName, type );
      if ( var->IsExport() )
         SetItemTextColour( id, m_ExportColor );

      if ( !parentName.IsEmpty() )
         SetItemText( id, name );
   }
}

wxTreeItemId CodeBrowserCtrl::MergeClass( wxTreeItemId parent, const AutoCompClass* class_, ITYPE type )
{
   wxASSERT( class_ );

   wxString parentSid = GetSid( parent );
   wxString sid;
   MakeSid( &sid, parentSid, class_->GetName(), type );

   wxTreeItemId id = ReviveItemFromCache( sid );
   if ( !id.IsOk() )
      id = AppendItem( sid, parent, class_->GetName(), type );

   if ( class_->IsExport() )
      SetItemTextColour( id, m_ExportColor );

   // Is it an export?
   //if ( class_->m_Page.IsEmpty() )
   //   SetItemFont( classId, exportFont );

   // Only merge children if the item is expanded.
   wxASSERT( id.IsOk() );
   if ( IsExpanded( id ) )
      MergeClassMembers( id, class_ ); 
   else if ( class_->HasBase() || class_->HasMembers() )
      SetItemHasChildren( id, true );

   return id;
}


void CodeBrowserCtrl::MergeClassMembers( wxTreeItemId parent, const AutoCompClass* class_ )
{
   // Do we have a base?
   if ( !class_->GetBase().IsEmpty() ) {
      
      // Look up the parent.
      wxASSERT( m_Data );
      
      const AutoCompClass* found = AutoCompClass::Find( class_->GetBase(), m_Data->GetClasses() );
      if ( found )
      {
         ITYPE type = ITYPE_CLASS;
         if ( found->IsDatablock() )
            type = ITYPE_DATABLOCK;

         wxTreeItemId id = MergeClass( parent, found, type );
         ItemData* data = (ItemData*)GetItemData( id );
         wxASSERT( data );
         data->base = true;
      }
      else
      {
         // TODO: what if we don't find a base?
      }
   }

   MergeFunctions( parent, class_->GetFunctions(), class_->GetName() );
   MergeVars( parent, class_->GetVars(), false );
}

int CodeBrowserCtrl::OnCompareItems( const wxTreeItemId& item1, const wxTreeItemId& item2 )
{
   // Is either item a base class?
   ItemData* data = (ItemData*)GetItemData( item1 );
   wxASSERT( data );
   if ( data->base )
      return -1;
   data = (ItemData*)GetItemData( item2 );
   wxASSERT( data );
   if ( data->base )
      return 1;

   wxString text1 = GetItemText( item1 );
   wxString text2 = GetItemText( item2 );

   // Remove the $ when we compare items.
   if ( text1[0] == '$' )
      text1 = text1.Mid( 1 );
   if ( text2[0] == '$' )
      text2 = text2.Mid( 1 );

   return text1.CmpNoCase( text2 );
}


/*
void CodeBrowserCtrl::OnSetFocus( wxFocusEvent& event )
{
   wxASSERT( m_ProjectDoc );
   wxASSERT( m_ProjectDoc->GetFirstView() );
   m_ProjectDoc->GetFirstView()->Activate( true );

   // If we don't skip it the TreeCtrl base
   // will never get to handle it.
   event.Skip();
}

void CodeBrowserCtrl::OnKillFocus( wxFocusEvent& event )
{
   wxASSERT( m_ProjectDoc );
   wxASSERT( m_ProjectDoc->GetFirstView() );
   m_ProjectDoc->GetFirstView()->Activate( false );

   // If we don't skip it the TreeCtrl base
   // will never get to handle it.
   event.Skip();
}
*/

/*
void CodeBrowserCtrl::OnItemCollapsed( wxTreeEvent& Event )
{
   wxTreeItemId item = Event.GetItem();

   // Never let the root collapse.
   if ( item == GetRootItem() ) {
      Event.Veto();
      return;
   }

   wxASSERT( ItemHasChildren( item ) );
   DeleteChildren( item );
   SetItemHasChildren( item );
}
*/

void CodeBrowserCtrl::OnItemExpanding( wxTreeEvent& Event )
{
   if ( m_IsActivate )
   {
      Event.Veto();
      return;
   }

   // Grab the data...
   ItemData* item = (ItemData*)GetItemData( Event.GetItem() );
   wxASSERT( item );

   wxASSERT( tsGetAutoComp() );
   m_Data = tsGetAutoComp()->Lock();
   if ( !m_Data )
   {
      m_Data = NULL;
      tsGetAutoComp()->Unlock();
      return;
   }

   Freeze();

   if ( item->type == ITYPE_CLASS || item->type == ITYPE_DATABLOCK ) 
   {
      AutoCompClass* found = AutoCompClass::Find( item->name, m_Data->GetClasses() );
      if ( found )
         MergeClassMembers( Event.GetItem(), found );
   } 
   else if ( item->type == ITYPE_VAR )
   {
      AutoCompVarArray vars( CmpNameNoCase );
      if ( AutoCompVar::FindScoped( item->name, m_Data->GetVars(), vars ) > 0 )
         MergeVars( Event.GetItem(), vars, false );
   }
   else
   {
      wxASSERT(   item->type == ITYPE_OBJECT ||
                  item->type == ITYPE_DATABLOCKOBJECT ||
                  item->type == ITYPE_NAMESPACE ||                  
                  item->type == ITYPE_PACKAGE );

      AutoCompClass* found = AutoCompClass::Find( item->name, m_Data->GetObjects() );
      if ( found )
         MergeClassMembers( Event.GetItem(), found );
   } 

   // Unlock the pages or we'll stall the autocomp thread!
   m_Data = NULL;
   tsGetAutoComp()->Unlock();

   SortChildren( Event.GetItem() );

   Thaw();
}

void CodeBrowserCtrl::OnRightClickItem( wxTreeEvent& Event )
{
   /*
   // Make sure we have the focus and
   // the active document.
   SetFocus();
   wxASSERT( this == FindFocus() );
   wxASSERT( m_ProjectDoc );
   wxASSERT( m_ProjectDoc->GetFirstView() );
   m_ProjectDoc->GetFirstView()->Activate( true );
   wxASSERT( this == FindFocus() );
   wxASSERT( m_ProjectDoc->GetDocumentManager()->GetCurrentView() == m_ProjectDoc->GetFirstView() );

   // If this is the root then have some special options.
   if ( Event.GetItem() == GetRootItem() ) {

      wxMenu* menu = new wxMenu;
   
      AppendWithIcon( menu, tsID_EXPLORE, _T( "&Explore" ) );
      menu->AppendSeparator();
      AppendWithIcon( menu, tsID_PROJECT_REFRESH, _T( "&Refresh" ) );
      AppendWithIcon( menu, tsID_CLEARDSOS, _T( "&Delete DSOs" ) );
      menu->AppendSeparator();
      AppendWithIcon( menu, tsID_PROJECT_PROPERTIES, _T( "&Properties" ) );

      PopupMenu( menu, Event.GetPoint() );
      delete menu;

      return;
   }

   ProjectCtrlItemData* Data = (ProjectCtrlItemData*)GetItemData( Event.GetItem() );
   if ( Data ) {

      wxFileName file( Data->GetPath() );

      wxMenu* menu = new wxMenu;
      if ( !file.IsDir() ) {
         AppendWithIcon( menu, tsID_PROJECT_OPEN, _T( "&Open" ), open_document16_xpm );
         AppendWithIcon( menu, tsID_PROJECT_OPENWITH, _T( "Open Wit&h..." ) );
         AppendWithIcon( menu, tsID_EXPLORE, _T( "Open &Folder" ) );
         if ( tsGetPrefs().IsScriptFile( file.GetFullPath() ) ) {
            AppendWithIcon( menu, tsID_CLEARDSO, _T( "D&elete DSO" ) );
         }
         menu->AppendSeparator();
      } else {
         AppendWithIcon( menu, tsID_EXPLORE, _T( "&Explore" ) );
         menu->AppendSeparator();
      }
      AppendWithIcon( menu, wxID_CUT, _T( "Cu&t" ), cut_clipboard16_xpm );
      AppendWithIcon( menu, wxID_COPY, _T( "&Copy" ), copy_clipboard16_xpm );
      if ( file.IsDir() ) {
         AppendWithIcon( menu, wxID_PASTE, _T( "&Paste" ), paste_clipboard16_xpm );
      }
      AppendWithIcon( menu, wxID_DELETE, _T( "&Delete" ), delete_x16_xpm );
      menu->AppendSeparator();
      AppendWithIcon( menu, wxID_PROPERTIES, _T( "P&roperties" ) );

      PopupMenu( menu, Event.GetPoint() );
      delete menu;
   }
   */
}

#ifdef __WXMSW__

bool CodeBrowserCtrl::MSWOnNotify( int idCtrl, WXLPARAM lParam, WXLPARAM* result )
{
   // This is all a hack for keeping double clicks (item 
   // activation) from expanding items.

   NMHDR *hdr = (NMHDR*)lParam;
   if ( hdr->code == NM_DBLCLK ) 
      m_IsActivate = false;

   bool ret = wxTreeCtrl::MSWOnNotify( idCtrl, lParam, result );

   if ( hdr->code == NM_DBLCLK && m_IsActivate )
   {
      *result = true;
      m_IsActivate = false;
   }

   return ret;
}

#endif

void CodeBrowserCtrl::OnItemActivated( wxTreeEvent& event )
{
   m_IsActivate = true;
   // If we're expanded then don't launch anything!
   //if ( IsExpanded( event.GetItem() ) )
   //   return;

   ItemData* item = (ItemData*)GetItemData( event.GetItem() );
   wxASSERT( item );

   wxArrayString paths;
   wxArrayInt lines;
   wxArrayString symbols;

   wxASSERT( tsGetAutoComp() );
   const AutoCompData* data = tsGetAutoComp()->Lock();
   if ( data )
   {
      data->GetDefinitionFileAndLine( item->name, &paths, &lines, &symbols );
      tsGetAutoComp()->Unlock();
   }

   // Did we get anything?
   wxASSERT( paths.GetCount() == lines.GetCount() );
   if ( paths.IsEmpty() )
   {
      // TODO: Ring a bell maybe?
      return;
   }

   wxString path = paths[0];
   int line = lines[0];

   // Did we get more than one?
   if ( paths.GetCount() > 1 )
   {
      GotoDefDialog dlg( this );
      if ( dlg.ShowModal( paths, lines, symbols ) != wxID_OK )
         return;

      path = dlg.GetPath();
      line = dlg.GetLine();     
   }

   wxASSERT( tsGetMainFrame() );
   tsGetMainFrame()->OpenFile( path, line );
}

/*
void ProjectCtrl::OnOpenItem( wxTreeEvent& Event )
{
   OpenItem( Event.GetItem() );
}

void ProjectCtrl::OpenItemWith( const wxTreeItemId& item )
{
	ProjectCtrlItemData* Data = (ProjectCtrlItemData*)GetItemData( item );
   if ( !Data ) {
      return;
   }

   wxFileName file( Data->GetPath() );
   if ( file.IsDir() ) {
      return;
   }

   // If we got here then let the shell handle it!
   wxShellOpenAs( file.GetFullPath() );
}

void ProjectCtrl::Select( const wxString& path )
{
   wxFileName file( path.Lower() );
   if ( !file.IsAbsolute() || file.IsDir() || !file.FileExists() ) {
      return;
   }

   // Make sure it's under this root.  We do this by making it
   // relative and testing it.
   wxASSERT( m_ProjectDoc );
   file = m_ProjectDoc->MakeReleativeTo( file.GetFullPath() );
   if (  file.IsAbsolute() || 
      ( file.GetDirCount() != 0 && file.GetDirs().Item( 0 ) == ".." ) ) {
      return;
   }

   // It's under our root... so use a special dir traversal.
   wxTreeItemId item = GetRootItem();
   wxTreeItemIdValue dummy;
   while ( item.IsOk() && file.GetDirCount() > 0 ) {

      if ( !IsExpanded( item ) ) {
         Expand( item );
      }

      // Get the next directory.
      wxFileName dir;
      dir.AppendDir( file.GetDirs().Item( 0 ) );
      file.RemoveDir( 0 );
      
      // Look at the child elements for it.
      wxTreeItemId child = GetFirstChild( item, dummy );
      while ( child.IsOk() ) {
         
         // Is this child our dir?
         if ( wxFileName::DirName( GetItemText( child ) ) == dir ) {
            break;
         }

         child = GetNextChild( item, dummy );
      }

      item = child;
   }

   // If we still have a valid item them expand 
   // it and select the file.
   if ( item.IsOk() ) {

      if ( !IsExpanded( item ) ) {
         Expand( item );
      }

      // Look at the file name as a child.
      wxFileName filename( file.GetFullName() );
      wxTreeItemId child = GetFirstChild( item, dummy );
      while ( child.IsOk() ) {
         
         // Is this child our dir?
         if ( wxFileName( GetItemText( child ) ) == filename ) {

            SelectItem( child );
            return;
         }

         child = GetNextChild( item, dummy );
      }
   }

   // If we got here we somehow expanded
   // down the tree and the file or some
   // directory componet didn't exist...
   // this must be timing issue and someone
   // deleted the file between the start of
   // this function and here.
}  

wxString ProjectCtrl::GetSelectedPath() const
{
   wxTreeItemId item = GetSelection();
   if ( item.IsOk() ) {
      
      ProjectCtrlItemData* data = (ProjectCtrlItemData*)GetItemData( item );
      if ( data ) {
         return data->GetPath();
      }
   }

   return "";
}
*/

//#include <wx/tipwin.h>

void CodeBrowserCtrl::OnSelChanged( wxTreeEvent& event )
{
   if ( !m_DescCtrl )
      return;

   m_DescCtrl->SetValue( "" );

   if ( !event.GetItem().IsOk() )
      return;

   ItemData* item = (ItemData*)GetItemData( event.GetItem() );
   wxASSERT( item );

   //wxArrayString paths;
   //wxArrayInt lines;
   //wxArrayString symbols;

   wxASSERT( tsGetAutoComp() );
   const AutoCompData* data = tsGetAutoComp()->Lock();
   if ( !data )
      return;

   CallTipInfo calltip;
   data->GetCallTip( item->name, calltip );
   //data->GetDefinitionFileAndLine( item->name, &paths, &lines, &symbols );
   tsGetAutoComp()->Unlock();

   if ( !calltip.IsEmpty() )
      m_DescCtrl->SetValue( calltip.GetTip() );
}

/*
void CodeBrowserCtrl::OnToolTip( wxTreeEvent& event )
{
   //event.Veto();
   //return;

   //SetToolTip( NULL );

   if ( !event.GetItem().IsOk() )
      return;

   ItemData* item = (ItemData*)GetItemData( event.GetItem() );
   wxASSERT( item );

   wxArrayString paths;
   wxArrayInt lines;
   wxArrayString symbols;

   wxASSERT( tsGetAutoComp() );
   const AutoCompData* data = tsGetAutoComp()->Lock();
   if ( !data )
      return;

   CallTipInfo calltip;
   data->GetCallTip( item->name, calltip );
   //data->GetDefinitionFileAndLine( item->name, &paths, &lines, &symbols );
   tsGetAutoComp()->Unlock();

   //if ( !calltip.IsEmpty() )
      //wxTipWindow( this, calltip.GetTip(), 100, NULL );     
}
*/


