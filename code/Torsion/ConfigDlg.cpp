// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "ConfigDlg.h"
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
#include "ConfigDlg.h"
#include "ProjectDoc.h"


////@begin XPM images
////@end XPM images

/*!
 * ConfigDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ConfigDlg, wxDialog )

/*!
 * ConfigDlg event table definition
 */

BEGIN_EVENT_TABLE( ConfigDlg, wxDialog )

////@begin ConfigDlg event table entries
    EVT_BUTTON( wxID_OK, ConfigDlg::OnOkClick )

////@end ConfigDlg event table entries

END_EVENT_TABLE()

/*!
 * ConfigDlg constructors
 */

ConfigDlg::ConfigDlg( )
{
}

ConfigDlg::ConfigDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * ConfigDlg creator
 */

bool ConfigDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ConfigDlg member initialisation
    m_PropGrid = NULL;
////@end ConfigDlg member initialisation

////@begin ConfigDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end ConfigDlg creation
    return true;
}

/*!
 * Control creation for ConfigDlg
 */

void ConfigDlg::CreateControls()
{    
////@begin ConfigDlg content construction
    ConfigDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 7);

    m_PropGrid = new wxPropertyGridManager( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxPG_BOLD_MODIFIED | wxPG_DESCRIPTION | wxPG_TOOLTIPS | wxTAB_TRAVERSAL | wxPG_STATIC_LAYOUT );
    itemBoxSizer3->Add(m_PropGrid, 1, wxGROW, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer3->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxTOP, 14);
    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end ConfigDlg content construction

   wxASSERT( m_PropGrid );
   wxASSERT( m_PropGrid->GetGrid() );

   // TODO: Fix to refresh colors on system color change!
   wxColor active( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
   wxFont font( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Tahoma" );
   m_PropGrid->GetGrid()->SetFont( font );
   m_PropGrid->GetGrid()->SetMarginColour( active );
   m_PropGrid->GetGrid()->SetLineColour( active );
   m_PropGrid->GetGrid()->SetCaptionBackgroundColour( active );
//   m_PropGrid->GetGrid()->SetCaptionForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE ) );
   m_PropGrid->SetDescBoxHeight( 60 );
}

int ConfigDlg::ShowModal( ProjectConfig* config, ProjectConfigArray* configs, const wxString& baseDir, bool forceModified )
{
   m_Config = config;
   m_Configs = configs;
   m_BaseDir = baseDir;

   // Fill the grid control.
   wxASSERT( m_PropGrid );
   m_PropGrid->AddPage();

   // Add the general properties.
   m_PropGrid->Append( new wxPropertyCategory(wxT("General")) );
   wxPGProperty* id = m_PropGrid->Append(new wxStringProperty(wxT("Name"), wxPG_LABEL, m_Config->GetName()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The unique configuration name.") );
   
   wxString exe = m_Config->GetExe();
   id = m_PropGrid->Append(new wxFileProperty(wxT("Executable"), wxPG_LABEL, exe ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The game executable for this configuration.") );
   m_PropGrid->SetPropertyAttribute(wxT("Executable"), wxPG_FILE_INITIAL_PATH, m_BaseDir );
   m_PropGrid->SetPropertyAttribute(wxT("Executable"), wxPG_FILE_WILDCARD,
                                 wxT("Executable files (*.exe)|*.exe"));

   id = m_PropGrid->Append(new wxStringProperty(wxT("Arguments"), wxPG_LABEL, m_Config->GetArgs()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The optional arguments to pass to the executable on launch.") );
   m_PropGrid->Append(new  wxPropertyCategory(wxT("Debugging")) );
   id = m_PropGrid->Append(new  wxBoolProperty(wxT("Enable Precompile"), wxPG_LABEL, m_Config->Precompile()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("This toggles the precompile step before execution of the configuration.") );
   id = m_PropGrid->Append(new  wxBoolProperty(wxT("Enable setModPaths"), wxPG_LABEL, m_Config->UseSetModPaths()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("This will slow down the precompile step, but it is needed for some versions of Torque or when TORQUE_SHIPPING is defined.") );
   id = m_PropGrid->Append(new  wxBoolProperty(wxT("OneClick Debugging"), wxPG_LABEL, m_Config->InjectDebugger()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Enables automatic injection of the debugger startup hook in to the main entry script.") );
   m_PropGrid->Append(new  wxPropertyCategory(wxT("ScriptSense")) );
   id = m_PropGrid->Append(new  wxBoolProperty(wxT("Build Exports"), wxPG_LABEL, m_Config->HasExports()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("This enables extraction of the engine exports from this executable.") );

   // Let's use checkboxes instead of a choice list.
   m_PropGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

   wxASSERT( m_PropGrid->GetGrid() );
   m_PropGrid->GetGrid()->SetSplitterLeft();
   m_PropGrid->SelectProperty( wxT("General.Name") );

   return wxDialog::ShowModal();
}

/*!
 * Should we show tooltips?
 */

bool ConfigDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ConfigDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ConfigDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ConfigDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ConfigDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ConfigDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ConfigDlg icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for cdID_BROWSE
 */

/*
void ConfigDlg::OnBrowseClick( wxCommandEvent& event )
{
wxASSERT( m_Exe );

   wxFileName exe( m_Exe->GetValue() );
   if ( !exe.GetFullPath().IsEmpty() && !m_BaseDir.IsEmpty() ) {

      if ( !exe.IsAbsolute() )
         exe.MakeAbsolute( m_BaseDir );

      exe.MakeRelativeTo(  m_BaseDir );
   }

   wxString tmp = wxFileSelector( "Select your Torque executable",
         exe.GetFullPath().IsEmpty() ? m_BaseDir : exe.GetFullPath(),
         "", "exe", "Executable files (*.exe; *.bat)|*.exe;*.bat",
         wxOPEN | wxFILE_MUST_EXIST,
         this );

   if ( !tmp.IsEmpty() ) {

      wxFileName relative( tmp );
      wxASSERT( relative.IsAbsolute() );

      if ( !m_BaseDir.IsEmpty() )
         relative.MakeRelativeTo( m_BaseDir );

      m_Exe->SetValue( relative.GetFullPath() );
      return;
   }
}
*/

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void ConfigDlg::OnOkClick( wxCommandEvent& event )
{
   // Verify we got a name.
   wxString name = m_PropGrid->GetPropertyValue( "General.Name" );
   if ( name.IsEmpty() ) {
      wxMessageDialog dlg( this, "You must enter a unique name for this config!", GetTitle(), wxOK );
      dlg.ShowModal();
      return;
   }

   // Make sure the config name is unique.
   wxASSERT( m_Configs );
   for ( int i=0; i < m_Configs->GetCount(); i++ ) 
   {
      if (  &m_Configs->Item( i ) != m_Config &&
            m_Configs->Item( i ).GetName() == name ) 
      {
         wxMessageDialog dlg( this, "You already have a config with this name!", GetTitle(), wxOK );
         dlg.ShowModal();
         return;
      }
   }

   wxString exec = m_PropGrid->GetPropertyValue( "General.Executable" );
   {
      wxFileName absolute( exec );

      if ( !absolute.IsAbsolute() )
      {
         absolute.MakeAbsolute( m_BaseDir );

         // If the file doesn't exist then reassign
         // the original string.... this makes things
         // work with global shortcuts for TGB 1.5.
         if ( !absolute.FileExists() ) 
            absolute.Assign( exec );
      }

      exec = absolute.GetFullPath();
   }

   // If nothing in the grid changed then
   // return canceled.
   wxASSERT( m_PropGrid );
   if ( !m_PropGrid->IsAnyModified() )
   {
      EndModal( wxID_CANCEL );
      return;
   }

   // Apply all the settings now.
   wxASSERT( m_Config );
   m_Config->SetName( name );
   m_Config->SetExe( exec );
   m_Config->SetArgs( m_PropGrid->GetPropertyValue( "General.Arguments" ) );
   m_Config->SetPrecompile( m_PropGrid->GetPropertyValueAsBool( "Debugging.Enable Precompile" ) );
   m_Config->SetUseSetModPaths( m_PropGrid->GetPropertyValueAsBool( "Debugging.Enable setModPaths" ) );
   m_Config->SetInjectDebugger( m_PropGrid->GetPropertyValueAsBool( "Debugging.OneClick Debugging" ) );
   m_Config->SetExports( m_PropGrid->GetPropertyValueAsBool( "ScriptSense.Build Exports" ) );

   //wxDialog::OnOK( event );
   event.Skip();
}


