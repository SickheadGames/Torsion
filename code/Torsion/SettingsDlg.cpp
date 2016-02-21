// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "SettingsDlg.h"
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

#include "SettingsDlg.h"
#include "TorsionApp.h"

#include <wx/propgrid/props.h>
#include <wx/propgrid/advprops.h>
//WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(TSColourProperty)

// Colour labels. Last (before NULL, if any) must be Custom.
static const wxChar* mycolprop_labels[] = {
   wxT("White"),
   wxT("Black"),
   wxT("Blue"),
   wxT("Dark Blue"),
   wxT("Green"),
   wxT("Dark Green"),
   wxT("Red"),
   wxT("Dark Red"),
   wxT("Magenta"),
   wxT("Purple"),
   wxT("Olive"),
   wxT("Aquamarine"),
   wxT("Light Grey"),
   wxT("Dark Grey"),
   wxT("Custom"),
   (const wxChar*) NULL
};

// Relevant colour values as unsigned longs.
static unsigned long mycolprop_colours[] = {
   wxPG_COLOUR(255,255,255),
   wxPG_COLOUR(0,0,0),
   wxPG_COLOUR(0,0,255),
   wxPG_COLOUR(0,0,128),
   wxPG_COLOUR(0,255,0),
   wxPG_COLOUR(0,128,0),
   wxPG_COLOUR(255,0,0),
   wxPG_COLOUR(128,0,0),
   wxPG_COLOUR(255,0,255),
   wxPG_COLOUR(128,0,128),
   wxPG_COLOUR(128,128,0),
   wxPG_COLOUR(0,128,128),
   wxPG_COLOUR(192,192,192),
   wxPG_COLOUR(128,128,128),
   wxPG_COLOUR(0,0,0)
};

// Implement property class. Third argument is optional values array,
// but in this example we are only interested in creating a shortcut
// for user to access the colour values.
//WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(TSColourProperty,
//                                                      mycolprop_labels,
//                                                      (long*)NULL,
//                                                      mycolprop_colours)

////@begin XPM images
////@end XPM images

/*!
 * SettingsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SettingsDlg, wxDialog )

/*!
 * SettingsDlg event table definition
 */

BEGIN_EVENT_TABLE( SettingsDlg, wxDialog )

////@begin SettingsDlg event table entries
    EVT_BUTTON( wxID_OK, SettingsDlg::OnOkClick )

////@end SettingsDlg event table entries

END_EVENT_TABLE()

/*!
 * SettingsDlg constructors
 */

SettingsDlg::SettingsDlg( )
{
}

SettingsDlg::SettingsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * SettingsDlg creator
 */

bool SettingsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin SettingsDlg member initialisation
    m_PropGrid = NULL;
////@end SettingsDlg member initialisation

////@begin SettingsDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
////@end SettingsDlg creation
    return true;
}

/*!
 * Control creation for SettingsDlg
 */

void SettingsDlg::CreateControls()
{    
////@begin SettingsDlg content construction
    SettingsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 7);

    m_PropGrid = new wxPropertyGridManager( itemDialog1, ID_CUSTOM, wxDefaultPosition, wxSize(100, 100), wxNO_BORDER|wxPG_BOLD_MODIFIED | wxPG_DESCRIPTION | wxPG_TOOLTIPS | wxTAB_TRAVERSAL | wxPG_STATIC_LAYOUT );
    itemBoxSizer3->Add(m_PropGrid, 1, wxGROW, 0);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer3->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxTOP, 5);
    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    itemStdDialogButtonSizer5->Realize();

////@end SettingsDlg content construction

   wxASSERT( m_PropGrid );
   wxASSERT( m_PropGrid->GetGrid() );

   // TODO: Fix to refresh colors on system color change!
   wxColor active( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
   wxFont font( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Tahoma" );
   m_PropGrid->GetGrid()->SetFont( font );
   m_PropGrid->GetGrid()->SetMarginColour( active );
   m_PropGrid->GetGrid()->SetLineColour( active );
   m_PropGrid->GetGrid()->SetCaptionBackgroundColour( active );
   //m_PropGrid->GetGrid()->SetCaptionForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_APPWORKSPACE ) );
   m_PropGrid->SetDescBoxHeight( 60 );


   // Fill the grid control.
   wxASSERT( m_PropGrid );
   m_PropGrid->AddPage();

   m_PropGrid->Append(new wxPropertyCategory(wxT("File System")) );
   wxPGProperty *id = m_PropGrid->Append(new wxStringProperty(wxT("Script Extensions"), wxPG_LABEL, tsGetPrefs().GetScriptExtsString()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("A semicolon seperated list of extensions of TorqueScript files to associate and open with Torsion.") );
   id = m_PropGrid->Append(new wxStringProperty(wxT("DSO Extensions"), wxPG_LABEL, tsGetPrefs().GetDSOExtsString()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Set the extensions used for compiled script files.") );
   id = m_PropGrid->Append(new wxStringProperty(wxT("Excluded Files"), wxPG_LABEL, tsGetPrefs().GetExcludedFilesString()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("A semicolon separated list of the files and file extensions which are always excluded from the project tree view.") );
   id = m_PropGrid->Append(new wxStringProperty(wxT("Excluded Folders"), wxPG_LABEL, tsGetPrefs().GetExcludedFoldersString()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("A semicolon separated list of the folders which are always excluded from the project tree view.") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Syntax Highlighting"),wxT("Editor")) );
   id = m_PropGrid->Append(new wxFontProperty(wxT("Font"), wxPG_LABEL, tsGetPrefs().GetDefaultFont()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The font used for all text in the script editor window.") );
   m_PropGrid->Collapse( id );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Background Color"), wxPG_LABEL, tsGetPrefs().GetBgColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The background color of the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Text Color"), wxPG_LABEL, tsGetPrefs().GetDefaultColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The default color for text in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Reserved Words Color"), wxPG_LABEL, tsGetPrefs().GetReservedColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color for reserved words such as FOR, IF, or FUNCTION in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Comment Color"), wxPG_LABEL, tsGetPrefs().GetCommentColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color for block and single line comments in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Margin Color"), wxPG_LABEL, tsGetPrefs().GetMarginColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The background color used in the left margin in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Margin Text Color"), wxPG_LABEL, tsGetPrefs().GetMarginTextColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The text color used in the left margin in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Exported Color"), wxPG_LABEL, tsGetPrefs().GetExportsColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for functions, object and datablock types, and variables exported from the executable.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Number Color"), wxPG_LABEL, tsGetPrefs().GetNumberColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for numbers in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("String Color"), wxPG_LABEL, tsGetPrefs().GetStringColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for quoted and tagged strings in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Variable Color"), wxPG_LABEL, tsGetPrefs().GetLocalsColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for %local variables in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Global Variable Color"), wxPG_LABEL, tsGetPrefs().GetGlobalsColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for $global variables in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Operators Color"), wxPG_LABEL, tsGetPrefs().GetOperatorsColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for operators such as *, ==, or SPC in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Selection Color"), wxPG_LABEL, tsGetPrefs().GetSelColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color for selected text in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Selection Background Color"), wxPG_LABEL, tsGetPrefs().GetSelBgColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The background color for selected text in the script editor window.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Brace Match Color"), wxPG_LABEL, tsGetPrefs().GetBraceMatchColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The forground color when highlighting properly matched braces.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Brace Match Background Color"), wxPG_LABEL, tsGetPrefs().GetBraceMatchBgColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The background color when highlighting properly matched braces.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Brace Match Error Color"), wxPG_LABEL, tsGetPrefs().GetBraceMatchErrColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The forground color when highlighting bad brace matches.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Calltip Text Color"), wxPG_LABEL, tsGetPrefs().GetCalltipColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The text color for calltips.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Calltip Background Color"), wxPG_LABEL, tsGetPrefs().GetCalltipBgColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The background color for calltips.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Calltip Highlight Color"), wxPG_LABEL, tsGetPrefs().GetCalltipHiColor() ) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The text highlight color for calltips") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Tabs")) );
   id = m_PropGrid->Append(new wxIntProperty(wxT("Size"), wxPG_LABEL, tsGetPrefs().GetTabWidth()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Specifies the number of spaces represented by a TAB character.") );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Insert Spaces"), wxPG_LABEL, !tsGetPrefs().GetUseTabs()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Insert spaces instead of TAB characters when editing.") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Edge Marker")) );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Enabled"), wxPG_LABEL, tsGetPrefs().GetEdgeMarker()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Toggles the visibility of column edge marker in the editor.") );
   id = m_PropGrid->Append(new wxIntProperty(wxT("Column"), wxPG_LABEL, tsGetPrefs().GetEdgeMarkerCol()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Specifies the column where the edge marker will be drawn.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Color"), wxPG_LABEL, tsGetPrefs().GetEdgeMarkerColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for the column edge marker.") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Code Folding")) );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Enabled"), wxPG_LABEL, tsGetPrefs().GetCodeFolding()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Toggles the UI elements which allow you to collapse and expand sections of code.") );
   id = m_PropGrid->Append(new wxColourProperty(wxT("Color"), wxPG_LABEL, tsGetPrefs().GetFoldingColor()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("The color used for all the code folding elements in the script editor window.") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Code Completion")) );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Enabled"), wxPG_LABEL, tsGetPrefs().GetCodeCompletion()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Toggle the generation and display of code completion data including completion lists, informational tool tips, and the code browser.") );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Enhanced Completion Keys"), wxPG_LABEL, tsGetPrefs().GetEnhancedCompletion()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Enables statement completion when any non-underscore non-alphanumeric character is entered.") );

   m_PropGrid->Append(new wxPropertyCategory(wxT("Misc")) );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Load Last Project"), wxPG_LABEL, tsGetPrefs().GetLoadLastProject()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Open the last loaded project on Torsion startup.") );
   id = m_PropGrid->Append(new wxBoolProperty(wxT("Check For Updates"), wxPG_LABEL, tsGetPrefs().GetCheckForUpdates()) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Disable startup checking for new releases of Torsion.") );

   /*
   id = m_PropGrid->Append( wxStringProperty(wxT("Auto Indent"), wxPG_LABEL, true) );
   m_PropGrid->SetPropertyHelpString( id, wxT("Enables automatic indenting on newlines within when entering { spaces instead of TAB characters when editing.") );
   */
   
   // Let's use checkboxes instead of a choice list.
   m_PropGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);

   wxASSERT( m_PropGrid->GetGrid() );
   m_PropGrid->GetGrid()->SetSplitterLeft();
   m_PropGrid->SelectProperty( wxT("Editor.Font") );
}

/*!
 * Should we show tooltips?
 */

bool SettingsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap SettingsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin SettingsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end SettingsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon SettingsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin SettingsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end SettingsDlg icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void SettingsDlg::OnOkClick( wxCommandEvent& event )
{
   wxASSERT( m_PropGrid );

   // If nothing in the grid changed then
   // return canceled.
   if ( !m_PropGrid->IsAnyModified() )
   {
      EndModal( wxID_CANCEL );
      return;
   }

   // Pull the values from grid and push it back into
   // the app preferences collection.
   //
   // TODO: This mess shows that we need a better
   // generic way to access and change prefs.  Look
   // at using wxVariant in some way.
   //

   tsGetPrefs().SetDefaultFont( m_PropGrid->GetPropertyValue( "Editor.Font" ).GetAny().As<wxFont>() );

   tsGetPrefs().SetTabWidth( m_PropGrid->GetPropertyValue( "Tabs.Size" ).GetLong() );
   tsGetPrefs().SetUseTabs( !m_PropGrid->GetPropertyValue( "Tabs.Insert Spaces" ).GetBool() );

   tsGetPrefs().SetBgColor( m_PropGrid->GetPropertyValue( "Editor.Background Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetBgColor( m_PropGrid->GetPropertyValue( "Editor.Background Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetDefaultColor( m_PropGrid->GetPropertyValue( "Editor.Text Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetReservedColor( m_PropGrid->GetPropertyValue( "Editor.Reserved Words Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetCommentColor( m_PropGrid->GetPropertyValue( "Editor.Comment Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetMarginColor( m_PropGrid->GetPropertyValue( "Editor.Margin Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetMarginTextColor(  m_PropGrid->GetPropertyValue( "Editor.Margin Text Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetExportsColor(  m_PropGrid->GetPropertyValue( "Editor.Exported Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetNumberColor(  m_PropGrid->GetPropertyValue( "Editor.Number Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetStringColor(  m_PropGrid->GetPropertyValue( "Editor.String Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetLocalsColor(  m_PropGrid->GetPropertyValue( "Editor.Variable Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetGlobalsColor(  m_PropGrid->GetPropertyValue( "Editor.Global Variable Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetOperatorsColor(  m_PropGrid->GetPropertyValue( "Editor.Operators Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetSelColor(  m_PropGrid->GetPropertyValue( "Editor.Selection Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetSelBgColor(  m_PropGrid->GetPropertyValue( "Editor.Selection Background Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetBraceMatchColor(  m_PropGrid->GetPropertyValue( "Editor.Brace Match Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetBraceMatchBgColor(  m_PropGrid->GetPropertyValue( "Editor.Brace Match Background Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetBraceMatchErrColor(  m_PropGrid->GetPropertyValue( "Editor.Brace Match Error Color" ).GetAny().As<wxColour>() );

   tsGetPrefs().SetCalltipColor( m_PropGrid->GetPropertyValue( "Editor.Calltip Text Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetCalltipBgColor( m_PropGrid->GetPropertyValue( "Editor.Calltip Background Color" ).GetAny().As<wxColour>() );
   tsGetPrefs().SetCalltipHiColor( m_PropGrid->GetPropertyValue( "Editor.Calltip Highlight Color" ).GetAny().As<wxColour>() );

   tsGetPrefs().SetEdgeMarker( m_PropGrid->GetPropertyValue( "Edge Marker.Enabled" ).GetBool() );
   tsGetPrefs().SetEdgeMarkerCol( m_PropGrid->GetPropertyValue( "Edge Marker.Column" ).GetLong() );
   tsGetPrefs().SetEdgeMarkerColor( m_PropGrid->GetPropertyValue( "Edge Marker.Color" ).GetAny().As<wxColour>() );

   tsGetPrefs().SetCodeFolding( m_PropGrid->GetPropertyValue( "Code Folding.Enabled" ).GetBool() );
   tsGetPrefs().SetFoldingColor( m_PropGrid->GetPropertyValue( "Code Folding.Color" ).GetAny().As<wxColour>() );

   tsGetPrefs().SetCodeCompletion( m_PropGrid->GetPropertyValue( "Code Completion.Enabled" ).GetBool() );
   tsGetPrefs().SetEnhancedCompletion( m_PropGrid->GetPropertyValue( "Code Completion.Enhanced Completion Keys" ).GetBool() ); 

   tsGetPrefs().SetLoadLastProject( m_PropGrid->GetPropertyValue( "Misc.Load Last Project" ).GetBool() );
   tsGetPrefs().SetCheckForUpdates( m_PropGrid->GetPropertyValue( "Misc.Check For Updates" ).GetBool() );

   tsGetPrefs().SetScriptExtsString( m_PropGrid->GetPropertyValue( "File System.Script Extensions" ) );
   tsGetPrefs().SetDSOExtsString( m_PropGrid->GetPropertyValue( "File System.DSO Extensions" ) );
   tsGetPrefs().SetExcludedFiles( m_PropGrid->GetPropertyValue( "File System.Excluded Files" ) );
   tsGetPrefs().SetExcludedFolders( m_PropGrid->GetPropertyValue( "File System.Excluded Folders" ) );

   //wxDialog::OnOK( event );
}


