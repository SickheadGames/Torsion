// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScriptFrame.h"

#include "ScriptCtrl.h"
#include "MainFrame.h"
#include "shNotebook.h"
#include "ScriptDoc.h"
#include "TorsionApp.h"
#include "Debugger.h"
#include "tsMenu.h"
#include "Icons.h"

#include <wx/cmdproc.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_CLASS( ScriptFrame, wxDocMDIChildFrame )

BEGIN_EVENT_TABLE( ScriptFrame, wxDocMDIChildFrame )
   EVT_SH_NOTEBOOK_PAGE_CHANGING( wxID_ANY, OnScriptPageChanging )
   EVT_SH_NOTEBOOK_PAGE_CHANGED( wxID_ANY, OnScriptPageChanged )
   EVT_SH_NOTEBOOK_PAGE_CLOSE( wxID_ANY, OnScriptPageClose )
   EVT_CONTEXT_MENU( OnContextMenu )
   EVT_ACTIVATE( OnActivate )
END_EVENT_TABLE()

ScriptFrame* ScriptFrame::s_ScriptFrame = NULL;

ScriptFrame* ScriptFrame::GetFrame( bool create )
{
   if ( !create )
      return s_ScriptFrame;

   ScriptFrame* frame = s_ScriptFrame;
   if ( !frame ) 
      frame = new ScriptFrame();
   frame->Show( true );
   s_ScriptFrame = frame;
   return s_ScriptFrame;
}

ScriptFrame::ScriptFrame()
   : wxDocMDIChildFrame( NULL, NULL, tsGetMainFrame(), wxID_ANY, _T( "" ),
      wxDefaultPosition, wxDefaultSize, wxMAXIMIZE | wxBORDER_NONE | wxCLIP_CHILDREN ),
      m_ScriptNotebook( NULL ),
      m_Tabbing( false ),
      m_WindowList( tsID_WINDOW_LAST - tsID_WINDOW_FIRST, tsID_WINDOW_FIRST )
{
   wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
   SetSizer(sizer);
   m_ScriptNotebook = new shNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize );
   sizer->Add( m_ScriptNotebook, 1, wxEXPAND ); 

   //m_WindowList.UseMenu( tsGetMainFrame()->GetWindowMenu() );

   /*
   wxImageList* PageImages = new wxImageList( 16, 16, true );
	PageImages->Add( wxIcon( new_document16_xpm ) );
	PageImages->Add( wxIcon( text_document16_xpm ) );
	m_ScriptNotebook->AssignImageList( PageImages );
   */
}

ScriptFrame::~ScriptFrame()
{
}

wxWindow* ScriptFrame::GetViewControlParent() const
{ 
   wxASSERT( wxDynamicCast( m_ScriptNotebook, wxWindow ) );
   return m_ScriptNotebook;
}

bool ScriptFrame::Destroy()
{
   while ( m_WindowList.GetCount() > 0 )
      m_WindowList.RemoveFileFromHistory( 0 );
   m_WindowList.RemoveMenu( tsGetMainFrame()->GetWindowMenu() );

   return wxDocMDIChildFrame::Destroy();
}

void ScriptFrame::AddScriptPage( ScriptView* scriptView, wxWindow* ctrl )
{
   m_PageOrder.Add( (int)m_PageViews.GetCount() );
   m_PageViews.Append( scriptView );

   wxDocument* doc = scriptView->GetDocument();
   wxASSERT( doc );
   wxString title;
   doc->GetPrintableName(title);
   wxASSERT( ctrl->GetParent() == m_ScriptNotebook );
   m_ScriptNotebook->AddPage( ctrl, title, doc->GetFilename(), true, 0 );

   wxASSERT( ( m_PageViews.GetCount() == m_ScriptNotebook->GetPageCount() ) &&
             ( m_PageOrder.GetCount() == m_ScriptNotebook->GetPageCount() ) );
}

void ScriptFrame::RemoveScriptPage( wxView* view )
{
   int index = 0;
   ScriptViewList::Node* node = m_PageViews.GetFirst();
   while ( node ) {
      wxASSERT( node->GetData() );
      if ( node->GetData() == view ) {

         SetView( NULL );
         SetDocument( NULL );
         view->GetDocument()->GetCommandProcessor()->SetEditMenu( NULL );
         view->SetFrame( NULL );

         m_ScriptNotebook->RemovePage( index );
         m_PageViews.Erase( node );

         // Clear this index from the page order.
         m_PageOrder.Remove( index );
         for ( size_t i=0; i < m_PageOrder.GetCount(); i++ ) {
            if ( m_PageOrder[i] > index ) {
               m_PageOrder[i] = m_PageOrder[i] - 1;
            }
         }

         wxASSERT( ( m_PageViews.GetCount() == m_ScriptNotebook->GetPageCount() ) &&
                  ( m_PageOrder.GetCount() == m_ScriptNotebook->GetPageCount() ) );
         break;
      }
      node = node->GetNext();
      ++index;
   }

   // If we're out of pages hide this frame.
   if ( m_ScriptNotebook->GetPageCount() == 0 ) {

      ScriptFrame* frame = s_ScriptFrame;
      s_ScriptFrame = NULL;

      // Usually you would call Destroy() here, but
      // we need the frame to go away immediately.  If
      // not when switching projects the delayed deletion
      // of the old frame keeps the new frame from maximizing.
      delete frame;
   }
}

int ScriptFrame::GetPageCount() const
{
   wxASSERT( m_ScriptNotebook );
   return m_ScriptNotebook->GetPageCount();
}

void ScriptFrame::OnScriptPageClose( shNotebookEvent& event )
{
   event.Veto();
   
   // Make sure we're the active page!
   ScriptView* view = m_PageViews.Item( event.GetSelection() )->GetData();
   wxASSERT( view );
   view->Activate( true );

   wxCommandEvent closeEvent( wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE );
   closeEvent.SetEventObject(this);
   tsGetMainFrame()->ProcessEvent( closeEvent );
}


void ScriptFrame::OnScriptPageChanging( shNotebookEvent& event )
{
   if ( event.GetOldSelection() == -1 )
      return;

   ScriptView* lastView = m_PageViews.Item( event.GetOldSelection() )->GetData();
   lastView->GetDocument()->GetCommandProcessor()->SetEditMenu( NULL );
   //lastView->Activate( false );
   //lastView->SetFrame( NULL );
}

void ScriptFrame::OnScriptPageChanged( shNotebookEvent& event )
{
   if ( event.GetSelection() < 0 )
      return;

   ScriptView* view = m_PageViews.Item( event.GetSelection() )->GetData();
   SetView( view );
   SetDocument( view->GetDocument() );

   // Make the active page the first in the order array.
   if ( !m_Tabbing ) 
   {
      m_PageOrder.Remove( event.GetSelection() );
      m_PageOrder.Insert( event.GetSelection(), 0 );
      m_WindowList.AddFileToHistory( view->GetDocument()->GetFilename() );
   }

   // TODO: What does this really do?
   view->GetDocument()->GetCommandProcessor()->SetEditMenu( tsGetMainFrame()->GetEditMenu() );

   wxASSERT( m_ScriptNotebook );
   SetTitle( m_ScriptNotebook->GetPageText( event.GetSelection() ) );

   // Write the full path in the status bar.
   wxASSERT( tsGetMainFrame() );
   tsGetMainFrame()->SetStatusText( view->GetDocument()->GetFilename() );
}

void ScriptFrame::NextView()
{
   int sel = m_ScriptNotebook->GetSelection();
   if ( sel == -1 )
      sel = 0;

   // Is this the first tab?
   if ( !m_Tabbing )
   {
      // Make the active page the first in the array.
      m_PageOrder.Remove( sel );
      m_PageOrder.Insert( sel, 0 );
      wxASSERT( m_PageOrder.Index( sel ) == 0 );
   }

   m_Tabbing = true;

   // What is the next page in the order array?
   int index = m_PageOrder.Index( sel ) + 1;
   if ( index >= (int)m_PageOrder.GetCount() ) {
      index = 0;
   }

   sel = m_PageOrder[ index ];
   m_ScriptNotebook->SetSelection( sel );
}

void ScriptFrame::PreviousView()
{
   int sel = m_ScriptNotebook->GetSelection();
   if ( sel == -1 ) {
      sel = 0;
   }

   // Is this the first tab?
   if ( !m_Tabbing ) {

      // Make the active page the first in the array.
      m_PageOrder.Remove( sel );
      m_PageOrder.Insert( sel, 0 );
      wxASSERT( m_PageOrder.Index( sel ) == 0 );
   }

   m_Tabbing = true;

   // What is the next page in the order array?
   int index = m_PageOrder.Index( sel ) - 1;
   if ( index < 0 ) {
      index = (int)m_PageOrder.GetCount() - 1;
   }

   sel = m_PageOrder[ index ];
   m_ScriptNotebook->SetSelection( sel );
}

void ScriptFrame::SetActive( int index )
{
   m_ScriptNotebook->SetSelection( index );
}

void ScriptFrame::SetActive( wxView* view )
{
   int sel = 0;
   ScriptViewList::Node* node = m_PageViews.GetFirst();
   while ( node ) {
      wxASSERT( node->GetData() );
      if ( node->GetData() == view ) {
         
         m_ScriptNotebook->SetSelection( sel );
         return;
      }
      node = node->GetNext();
      ++sel;
   }
}

void ScriptFrame::OnContextMenu( wxContextMenuEvent& event )
{
   wxASSERT( m_ScriptNotebook );
   
   long flags;
   wxPoint pt = ScreenToClient( event.GetPosition() );
   int page = m_ScriptNotebook->HitTest( pt, &flags );
   if ( !( flags & shNB_HITTEST_ONITEM ) || page == -1 ) {
      return;
   }
   
   m_ScriptNotebook->SetSelection( page );

   ScriptDoc* doc = GetScriptDoc( page );
   GetScriptView( page )->Activate( true );

   wxASSERT( doc );
   
   wxString save;
   save << "&Save " << doc->GetTitle();;

   tsMenu* menu = new tsMenu;
   menu->AppendIconItem( wxID_SAVE, save, ts_save16 );
   menu->Append( wxID_CLOSE,   _T( "&Close" ) );
   menu->Append( wxID_CLOSE_ALL,   _T( "Close &All" ) );
   menu->Append( tsID_CLOSE_OTHERS,   _T( "Close All But &This" ) );
   menu->AppendSeparator();
   menu->Append( tsID_COPY_FULL_PATH,   _T( "Copy &Full Path" ) );
   menu->Append( tsID_PROJECT_OPENWITH, _T( "Open Wit&h..." ) );
   menu->AppendIconItem( tsID_EXPLORE,   _T( "&Open Containing Folder" ), ts_explorer16 );
   menu->AppendSeparator();
   menu->AppendIconItem( tsID_CLEARDSO, _T( "&Delete DSO" ), ts_delete16 );
   menu->AppendIconItem( tsID_RELOAD_SCRIPT, _T( "&Reload Script" ), ts_refreshpage16 );
   PopupMenu( menu, pt );   
   delete menu;
}

ScriptView* ScriptFrame::GetSelectedView()
{
   wxASSERT( m_ScriptNotebook );
   int sel = m_ScriptNotebook->GetSelection();
   if ( sel < 0 )
      return NULL;

   return GetScriptView( sel );
}

ScriptView* ScriptFrame::GetScriptView( int page )
{
   wxASSERT( page > -1 );
   wxASSERT( page < m_ScriptNotebook->GetPageCount() );
   wxASSERT( page < m_PageViews.GetCount() );
   wxASSERT( m_PageViews.Item( page ) );
   wxASSERT( m_PageViews.Item( page )->GetData() );
   return m_PageViews.Item( page )->GetData();
}

ScriptDoc* ScriptFrame::GetScriptDoc( int page )
{
   wxASSERT( GetScriptView( page ) );
   return wxDynamicCast( GetScriptView( page )->GetDocument(), ScriptDoc );
}

void ScriptFrame::SetTabState( ScriptView* view, bool modified, bool readonly )
{
   wxASSERT( view );

   int page = m_PageViews.IndexOf( view );
   wxASSERT( page != wxNOT_FOUND );

   wxString title, tabTitle, toolTip;
   wxASSERT( view->GetDocument() );
   view->GetDocument()->GetPrintableName(title);
   toolTip = view->GetDocument()->GetFilename();
   tabTitle = title;

   if ( modified ) {
      tabTitle << "*";
      title << "*";
   }

   if ( readonly ) {
      tabTitle << " (ro)";
   }

   wxASSERT( m_ScriptNotebook );
   wxASSERT( page < m_ScriptNotebook->GetPageCount() );
   m_ScriptNotebook->SetPageText( page, tabTitle, toolTip );

   if ( page == m_ScriptNotebook->GetSelection() )
      SetTitle( title );
}

void ScriptFrame::OnActivate( wxActivateEvent& event )
{
   if ( !event.GetActive() )
      return;
   
   // We can sometimes get called a second time while we're
   // in the middle of script reloading... so avoid recursion.
   static bool inActivateLoop = false;
   if ( inActivateLoop )
      return;      
   inActivateLoop = true;

   // Go thru all the attached views and request that they reload
   // themselves if they've changed.
   int lastResult = 0;
   wxScriptViewListNode* node = m_PageViews.GetFirst();
   while ( node ) {
   
      ScriptView* view = node->GetData();
      wxASSERT( view );
      
      lastResult = view->ReloadFile( lastResult, false );
      if ( lastResult == wxID_NOTOALL )
         break;

      node = node->GetNext();
   }

   inActivateLoop = false;
}

int ScriptFrame::GetViewIndex( ScriptView* view ) const
{
   return m_PageViews.IndexOf( view );
   /*
   if ( index != wxNOT_FOUND )
      return m_PageOrder[index];

   return -1;
   */
}

bool ScriptFrame::ProcessEvent(wxEvent& event)
{

   return wxDocMDIChildFrame::ProcessEvent( event );
}
