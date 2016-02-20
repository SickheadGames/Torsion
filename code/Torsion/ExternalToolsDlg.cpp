// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "ExternalToolsDlg.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "PreCompiled.h"
#include "ExternalToolsDlg.h"
#include "Platform.h"
#include "tsMenu.h"

////@begin XPM images
////@end XPM images

/*!
 * ExternalToolsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ExternalToolsDlg, wxDialog )

/*!
 * ExternalToolsDlg event table definition
 */

BEGIN_EVENT_TABLE( ExternalToolsDlg, wxDialog )

////@begin ExternalToolsDlg event table entries
   EVT_LISTBOX( etID_TOOLLIST, ExternalToolsDlg::OnToolSelected )

   EVT_BUTTON( wxID_ADD, ExternalToolsDlg::OnAddClick )

   EVT_BUTTON( wxID_DELETE, ExternalToolsDlg::OnDeleteClick )
   EVT_UPDATE_UI( wxID_DELETE, ExternalToolsDlg::OnDeleteUpdate )

   EVT_BUTTON( wxID_UP, ExternalToolsDlg::OnUpClick )
   EVT_UPDATE_UI( wxID_UP, ExternalToolsDlg::OnUpUpdate )

   EVT_BUTTON( wxID_DOWN, ExternalToolsDlg::OnDownClick )
   EVT_UPDATE_UI( wxID_DOWN, ExternalToolsDlg::OnDownUpdate )

   EVT_BUTTON( etID_CMDMENU, ExternalToolsDlg::OnCmdMenuClick )

   EVT_BUTTON( etID_ARGMENU, ExternalToolsDlg::OnArgMenuClick )

   EVT_BUTTON( etID_DIRMENU, ExternalToolsDlg::OnDirMenuClick )

   EVT_BUTTON( wxID_OK, ExternalToolsDlg::OnOkClick )

////@end ExternalToolsDlg event table entries

END_EVENT_TABLE()

/*!
 * ExternalToolsDlg constructors
 */

ExternalToolsDlg::ExternalToolsDlg( )
{
}

ExternalToolsDlg::ExternalToolsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
   Create(parent, id, caption, pos, size, style);
}

/*!
 * ExternalToolsDlg creator
 */

bool ExternalToolsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ExternalToolsDlg member initialisation
   m_ToolList = NULL;
   m_TitleCtrl = NULL;
   m_CmdCtrl = NULL;
   m_ArgCtrl = NULL;
   m_DirCtrl = NULL;
////@end ExternalToolsDlg member initialisation

////@begin ExternalToolsDlg creation
   SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
   wxDialog::Create( parent, id, caption, pos, size, style );

   CreateControls();
   if (GetSizer())
   {
      GetSizer()->SetSizeHints(this);
   }
   Centre();
////@end ExternalToolsDlg creation
   return true;
}

/*!
 * Control creation for ExternalToolsDlg
 */

void ExternalToolsDlg::CreateControls()
{    
////@begin ExternalToolsDlg content construction
   ExternalToolsDlg* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
   itemDialog1->SetSizer(itemBoxSizer2);

   wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 7);

   wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxBOTTOM, 5);

   wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Me&nu contents:"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_TOP|wxLEFT|wxTOP|wxBOTTOM|wxADJUST_MINSIZE, 0);

   wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer6, 1, wxGROW|wxALL, 0);

   wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer6->Add(itemBoxSizer7, 1, wxGROW|wxRIGHT, 7);

   wxString* m_ToolListStrings = NULL;
   m_ToolList = new wxListBox( itemDialog1, etID_TOOLLIST, wxDefaultPosition, wxSize(275, -1), 0, m_ToolListStrings, wxLB_SINGLE|wxLB_NEEDED_SB );
   itemBoxSizer7->Add(m_ToolList, 1, wxGROW|wxALL, 0);

   wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer6->Add(itemBoxSizer9, 0, wxGROW, 5);

   wxButton* itemButton10 = new wxButton( itemDialog1, wxID_ADD, _("&Add"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer9->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 3);

   wxButton* itemButton11 = new wxButton( itemDialog1, wxID_DELETE, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer9->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM, 3);

   itemBoxSizer9->Add(5, 5, 1, wxGROW|wxALL, 5);

   wxButton* itemButton13 = new wxButton( itemDialog1, wxID_UP, _("Move &Up"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer9->Add(itemButton13, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM, 3);

   wxButton* itemButton14 = new wxButton( itemDialog1, wxID_DOWN, _("Move Do&wn"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer9->Add(itemButton14, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 3);

   wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer15, 0, wxGROW|wxTOP, 10);

   wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer15->Add(itemBoxSizer16, 1, wxGROW, 5);

   wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer16->Add(itemBoxSizer17, 0, wxGROW, 0);

   wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("&Title:"), wxDefaultPosition, wxSize(100, -1), wxALIGN_LEFT );
   itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

   m_TitleCtrl = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer17->Add(m_TitleCtrl, 3, wxALIGN_CENTER_VERTICAL|wxALL, 0);

   wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer16->Add(itemBoxSizer20, 0, wxGROW|wxTOP, 5);

   wxStaticText* itemStaticText21 = new wxStaticText( itemDialog1, wxID_STATIC, _("&Command:"), wxDefaultPosition, wxSize(100, -1), wxALIGN_LEFT );
   itemBoxSizer20->Add(itemStaticText21, 0, wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

   m_CmdCtrl = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer20->Add(m_CmdCtrl, 3, wxALIGN_CENTER_VERTICAL|wxALL, 0);

   wxButton* itemButton23 = new wxButton( itemDialog1, etID_CMDMENU, _(">"), wxDefaultPosition, wxSize(26, -1), wxBU_EXACTFIT );
   itemBoxSizer20->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 7);

   wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer16->Add(itemBoxSizer24, 0, wxGROW|wxTOP, 5);

   wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, _("A&rguments:"), wxDefaultPosition, wxSize(100, -1), wxALIGN_LEFT );
   itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

   m_ArgCtrl = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer24->Add(m_ArgCtrl, 3, wxALIGN_CENTER_VERTICAL|wxALL, 0);

   wxButton* itemButton27 = new wxButton( itemDialog1, etID_ARGMENU, _(">"), wxDefaultPosition, wxSize(26, -1), wxBU_EXACTFIT );
   itemBoxSizer24->Add(itemButton27, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 7);

   wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer16->Add(itemBoxSizer28, 0, wxGROW|wxTOP, 5);

   wxStaticText* itemStaticText29 = new wxStaticText( itemDialog1, wxID_STATIC, _("&Initial Directory:"), wxDefaultPosition, wxSize(100, -1), wxALIGN_LEFT );
   itemBoxSizer28->Add(itemStaticText29, 0, wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

   m_DirCtrl = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer28->Add(m_DirCtrl, 3, wxALIGN_CENTER_VERTICAL|wxALL, 0);

   wxButton* itemButton31 = new wxButton( itemDialog1, etID_DIRMENU, _(">"), wxDefaultPosition, wxSize(26, -1), wxBU_EXACTFIT );
   itemBoxSizer28->Add(itemButton31, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 7);

   wxStdDialogButtonSizer* itemStdDialogButtonSizer32 = new wxStdDialogButtonSizer;

   itemBoxSizer3->Add(itemStdDialogButtonSizer32, 0, wxALIGN_RIGHT|wxTOP, 10);
   wxButton* itemButton33 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
   itemStdDialogButtonSizer32->AddButton(itemButton33);

   wxButton* itemButton34 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
   itemStdDialogButtonSizer32->AddButton(itemButton34);

   itemStdDialogButtonSizer32->Realize();

////@end ExternalToolsDlg content construction

   m_LastSel = wxNOT_FOUND;
   UpdateToolList();
}

void ExternalToolsDlg::UpdateToolList()
{
   int sel = m_ToolList->GetSelection(); 
   m_ToolList->Clear();

   for ( size_t i=0; i < m_ToolCmds.GetCount(); i++ )
   {
      wxString title = m_ToolCmds[i].GetTitle();
      m_ToolList->Append( title );
   }

   if ( sel >= 0 && sel < m_ToolList->GetCount() )
      m_ToolList->SetSelection( sel );
   else if ( m_ToolList->GetCount() > 0 )
      m_ToolList->SetSelection( 0 );

   OnToolSelected( wxCommandEvent() );
}

/*!
 * Should we show tooltips?
 */

bool ExternalToolsDlg::ShowToolTips()
{
   return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ExternalToolsDlg::GetBitmapResource( const wxString& name )
{
   // Bitmap retrieval
////@begin ExternalToolsDlg bitmap retrieval
   wxUnusedVar(name);
   return wxNullBitmap;
////@end ExternalToolsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ExternalToolsDlg::GetIconResource( const wxString& name )
{
   // Icon retrieval
////@begin ExternalToolsDlg icon retrieval
   wxUnusedVar(name);
   return wxNullIcon;
////@end ExternalToolsDlg icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_ADD
 */

void ExternalToolsDlg::OnAddClick( wxCommandEvent& event )
{
   // Do a select event so that any current changes to
   // the selected tool are stored first!
   OnToolSelected( wxCommandEvent() );
   m_LastSel = wxNOT_FOUND;

   wxASSERT( m_ToolList );
   ToolCommand cmd;
   cmd.SetTitle( "&New Tool" );
   m_ToolCmds.Add( cmd );
   int sel = m_ToolList->Append( cmd.GetTitle() );
   m_ToolList->SetSelection( sel );
   OnToolSelected( wxCommandEvent() );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
 */

void ExternalToolsDlg::OnDeleteClick( wxCommandEvent& event )
{
   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   
   wxASSERT( sel >= 0 && sel < m_ToolCmds.GetCount() );
   m_ToolCmds.RemoveAt( sel );
   m_ToolList->Delete( sel );

   // Restore the selection.
   if ( sel >= m_ToolList->GetCount() )
      sel = m_ToolList->GetCount() - 1;
   if ( sel >= 0 )
      m_ToolList->SetSelection( sel );

   // Reset the last sel... it was deleted!
   m_LastSel = wxNOT_FOUND;
   OnToolSelected( wxCommandEvent() );
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_DELETE
 */

void ExternalToolsDlg::OnDeleteUpdate( wxUpdateUIEvent& event )
{
   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   event.Enable( sel != wxNOT_FOUND );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_UP
 */

void ExternalToolsDlg::OnUpClick( wxCommandEvent& event )
{
   // Do a select event so that any current changes to
   // the selected tool are stored first!
   OnToolSelected( wxCommandEvent() );
   m_LastSel = wxNOT_FOUND;

   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   
   wxASSERT( sel > 0 && sel < m_ToolCmds.GetCount() );
   wxASSERT( m_ToolList->GetCount() == m_ToolCmds.GetCount() );

   // Swap stuff!
   ToolCommand cmd = m_ToolCmds[ sel ];
   m_ToolCmds[ sel ] = m_ToolCmds[ sel - 1 ];
   m_ToolCmds[ sel - 1 ] = cmd;
   m_ToolList->SetString( sel, m_ToolCmds[ sel ].GetTitle() );
   m_ToolList->SetString( sel-1, m_ToolCmds[ sel-1 ].GetTitle() );

   m_ToolList->SetSelection( sel - 1 );
   OnToolSelected( wxCommandEvent() );
}


/*!
 * wxEVT_UPDATE_UI event handler for wxID_UP
 */

void ExternalToolsDlg::OnUpUpdate( wxUpdateUIEvent& event )
{
   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   event.Enable( sel != wxNOT_FOUND && sel != 0 );
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DOWN
 */

void ExternalToolsDlg::OnDownClick( wxCommandEvent& event )
{
   // Do a select event so that any current changes to
   // the selected tool are stored first!
   OnToolSelected( wxCommandEvent() );
   m_LastSel = wxNOT_FOUND;

   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   
   wxASSERT( sel >= 0 && sel < m_ToolCmds.GetCount()-1 );
   wxASSERT( m_ToolList->GetCount() == m_ToolCmds.GetCount() );

   // Swap stuff!
   ToolCommand cmd = m_ToolCmds[ sel ];
   m_ToolCmds[ sel ] = m_ToolCmds[ sel + 1 ];
   m_ToolCmds[ sel + 1 ] = cmd;
   m_ToolList->SetString( sel, m_ToolCmds[ sel ].GetTitle() );
   m_ToolList->SetString( sel+1, m_ToolCmds[ sel+1 ].GetTitle() );

   m_ToolList->SetSelection( sel + 1 );
   OnToolSelected( wxCommandEvent() );
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_DOWN
 */

void ExternalToolsDlg::OnDownUpdate( wxUpdateUIEvent& event )
{
   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();
   event.Enable( sel != wxNOT_FOUND && sel != m_ToolList->GetCount()-1 );
}

/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler
 */

void ExternalToolsDlg::OnToolSelected( wxCommandEvent& event )
{
   wxASSERT( m_ToolList );
   int sel = m_ToolList->GetSelection();

   // Save changes to any existing selection.
   if ( m_LastSel != wxNOT_FOUND )
   {
      ToolCommand& tool = m_ToolCmds[ m_LastSel ];

      tool.SetTitle( m_TitleCtrl->GetLabel() );
      tool.SetCmd( m_CmdCtrl->GetLabel() );
      tool.SetArgs( m_ArgCtrl->GetLabel() );
      tool.SetDir( m_DirCtrl->GetLabel() );

      m_ToolList->SetString( m_LastSel, tool.GetTitle() );
   }

   // Store the new selection for saving later.
   m_LastSel = sel;

   // If we don't have a new selection then
   // disabled and empty the controls.
   if ( sel == wxNOT_FOUND )
   {
      m_TitleCtrl->SetLabel( "" );
      m_CmdCtrl->SetLabel( "" );
      m_ArgCtrl->SetLabel( "" );
      m_DirCtrl->SetLabel( "" );

      m_TitleCtrl->Enable( false );
      m_CmdCtrl->Enable( false );
      m_ArgCtrl->Enable( false );
      m_DirCtrl->Enable( false );
      return;
   }

   // Make sure the controls are enabled!
   m_TitleCtrl->Enable( true );
   m_CmdCtrl->Enable( true );
   m_ArgCtrl->Enable( true );
   m_DirCtrl->Enable( true );

   // Fill the controls with the new selection.
   const ToolCommand& tool = m_ToolCmds[ sel ];
   m_TitleCtrl->SetLabel( tool.GetTitle() );
   m_CmdCtrl->SetLabel( tool.GetCmd() );
   m_ArgCtrl->SetLabel( tool.GetArgs() );
   m_DirCtrl->SetLabel( tool.GetDir() );
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void ExternalToolsDlg::OnOkClick( wxCommandEvent& event )
{
   // Do a select event so that any current changes to
   // the selected tool are stored first!
   OnToolSelected( wxCommandEvent() );

   // Fall thru and let the base process it now!
   event.Skip();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OPEN
 */

void ExternalToolsDlg::OnCmdMenuClick( wxCommandEvent& event )
{
   // Get the button for positioning.
   wxWindow* button = FindWindow( event.GetId() );
   wxASSERT( button );
   
   // Setup the menu.
   tsMenu* menu = new tsMenu;
   menu->Append( ToolCommand::VAR_CONFIG_EXE, ToolCommand::GetVarName( ToolCommand::VAR_CONFIG_EXE ) );
   menu->Append( ToolCommand::VAR_MAX, "Browse..." );

   // Show it!
   int result = tsTrackPopupMenu( menu, true, button->GetPosition(), this );
   delete menu;

   // Validate it!
   if ( result == ToolCommand::VAR_CONFIG_EXE )
   {
      wxString var( ToolCommand::GetVar( ToolCommand::VAR_CONFIG_EXE ) );
      m_CmdCtrl->SetLabel( var );
      return;
   }
   else if ( result == ToolCommand::VAR_MAX )
   {
      wxASSERT( m_CmdCtrl );
      wxString cmd = m_CmdCtrl->GetLabel();
   
      wxString path = wxFileSelector( "Choose a command executable", 
         cmd, wxEmptyString, wxEmptyString, 
         "All Executables (*.exe;*.com;*.pif;*.bat;*.cmd)|*.exe;*.com;*.pif;*.bat;*.cmd|All Files (*.*)|*.*",
         wxOPEN | wxFILE_MUST_EXIST, this );
      
      if ( path.IsEmpty() )
         return;

      m_CmdCtrl->SetLabel( path );
   }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for etID_ARGMENU
 */

void ExternalToolsDlg::OnArgMenuClick( wxCommandEvent& event )
{
   // Get the button for positioning.
   wxWindow* button = FindWindow( event.GetId() );
   wxASSERT( button );
   
   // Setup the menu.
   tsMenu* menu = new tsMenu;
   menu->Append( ToolCommand::VAR_FILE_PATH, ToolCommand::GetVarName( ToolCommand::VAR_FILE_PATH ) );
   menu->Append( ToolCommand::VAR_FILE_DIR, ToolCommand::GetVarName( ToolCommand::VAR_FILE_DIR ) );
   menu->Append( ToolCommand::VAR_FILE_NAME, ToolCommand::GetVarName( ToolCommand::VAR_FILE_NAME ) );
   menu->Append( ToolCommand::VAR_FILE_EXT, ToolCommand::GetVarName( ToolCommand::VAR_FILE_EXT ) );
   menu->AppendSeparator();
   menu->Append( ToolCommand::VAR_PROJECT_NAME, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_NAME ) );
   menu->Append( ToolCommand::VAR_PROJECT_DIR, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_DIR ) );
   menu->Append( ToolCommand::VAR_PROJECT_FILE_NAME, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_FILE_NAME ) );
   menu->Append( ToolCommand::VAR_PROJECT_WORKING_DIR, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_WORKING_DIR ) );
   menu->AppendSeparator();
   menu->Append( ToolCommand::VAR_CONFIG_NAME, ToolCommand::GetVarName( ToolCommand::VAR_CONFIG_NAME ) );
   menu->Append( ToolCommand::VAR_CONFIG_EXE, ToolCommand::GetVarName( ToolCommand::VAR_CONFIG_EXE ) );
   menu->Append( ToolCommand::VAR_CONFIG_ARGS, ToolCommand::GetVarName( ToolCommand::VAR_CONFIG_ARGS ) );

   // Show it!
   int result = tsTrackPopupMenu( menu, true, button->GetPosition(), this );
   delete menu;

   // Validate it!
   if ( result <= ToolCommand::VAR_NULL || result > ToolCommand::VAR_MAX )
      return;

   wxString var( ToolCommand::GetVar( (ToolCommand::VAR)result ) );
   m_ArgCtrl->WriteText( var );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for etID_DIRMENU
 */

void ExternalToolsDlg::OnDirMenuClick( wxCommandEvent& event )
{
   // Get the button for positioning.
   wxWindow* button = FindWindow( event.GetId() );
   wxASSERT( button );
   
   // Setup the menu.
   tsMenu* menu = new tsMenu;
   menu->Append( ToolCommand::VAR_FILE_DIR, ToolCommand::GetVarName( ToolCommand::VAR_FILE_DIR ) );
   menu->Append( ToolCommand::VAR_PROJECT_DIR, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_DIR ) );
   menu->Append( ToolCommand::VAR_PROJECT_WORKING_DIR, ToolCommand::GetVarName( ToolCommand::VAR_PROJECT_WORKING_DIR ) );
   
   // Show it!
   int result = tsTrackPopupMenu( menu, true, button->GetPosition(), this );
   delete menu;

   // Validate it!
   if ( result <= ToolCommand::VAR_NULL || result > ToolCommand::VAR_MAX )
      return;

   wxString var( ToolCommand::GetVar( (ToolCommand::VAR)result ) );
   m_DirCtrl->WriteText( var );
}


