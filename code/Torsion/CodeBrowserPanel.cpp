// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "CodeBrowserPanel.h"
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

#include "CodeBrowserPanel.h"
#include "CodeBrowserCtrl.h"
#include "tsToolBar.h"

////@begin XPM images
#include "icons/class16.xpm"
#include "icons/datablock16.xpm"
#include "icons/functions16.xpm"
#include "icons/globals16.xpm"
#include "icons/namespace16.xpm"
////@end XPM images

/*!
 * CodeBrowserPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CodeBrowserPanel, wxPanel )

/*!
 * CodeBrowserPanel event table definition
 */

BEGIN_EVENT_TABLE( CodeBrowserPanel, wxPanel )

////@begin CodeBrowserPanel event table entries
   EVT_MENU( ID_CLASSES, CodeBrowserPanel::OnFilterClick )
   EVT_UPDATE_UI( ID_CLASSES, CodeBrowserPanel::OnFilterUpdate )

   EVT_MENU( ID_DATABLOCKS, CodeBrowserPanel::OnFilterClick )
   EVT_UPDATE_UI( ID_DATABLOCKS, CodeBrowserPanel::OnFilterUpdate )

   EVT_MENU( ID_FUNCTIONS, CodeBrowserPanel::OnFilterClick )
   EVT_UPDATE_UI( ID_FUNCTIONS, CodeBrowserPanel::OnFilterUpdate )

   EVT_MENU( ID_GLOBALS, CodeBrowserPanel::OnFilterClick )
   EVT_UPDATE_UI( ID_GLOBALS, CodeBrowserPanel::OnFilterUpdate )

   EVT_MENU( ID_NAMESPACES, CodeBrowserPanel::OnFilterClick )
   EVT_UPDATE_UI( ID_NAMESPACES, CodeBrowserPanel::OnFilterUpdate )

////@end CodeBrowserPanel event table entries

END_EVENT_TABLE()

/*!
 * CodeBrowserPanel constructors
 */

CodeBrowserPanel::CodeBrowserPanel( )
{
}

CodeBrowserPanel::CodeBrowserPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * CodeBrowserPanel creator
 */

bool CodeBrowserPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CodeBrowserPanel member initialisation
   m_CodeTree = NULL;
   m_DescCtrl = NULL;
////@end CodeBrowserPanel member initialisation

////@begin CodeBrowserPanel creation
   SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
   wxPanel::Create( parent, id, pos, size, style );

   CreateControls();
   if (GetSizer())
   {
      GetSizer()->SetSizeHints(this);
   }
////@end CodeBrowserPanel creation

    SetLabel( SYMBOL_CODEBROWSERPANEL_TITLE );

    return true;
}

/*!
 * Control creation for CodeBrowserPanel
 */

void CodeBrowserPanel::CreateControls()
{    
////@begin CodeBrowserPanel content construction
   CodeBrowserPanel* itemPanel1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
   itemPanel1->SetSizer(itemBoxSizer2);

   wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW, 0);

   tsToolBar* itemToolBar4 = new tsToolBar( itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER|wxNO_BORDER );
   wxBitmap itemtool5Bitmap(itemPanel1->GetBitmapResource(wxT("icons/class16.xpm")));
   wxBitmap itemtool5BitmapDisabled;
   itemToolBar4->AddTool(ID_CLASSES, _("Classes"), itemtool5Bitmap, itemtool5BitmapDisabled, wxITEM_CHECK, _("Show classes"), wxEmptyString);
   wxBitmap itemtool6Bitmap(itemPanel1->GetBitmapResource(wxT("icons/datablock16.xpm")));
   wxBitmap itemtool6BitmapDisabled;
   itemToolBar4->AddTool(ID_DATABLOCKS, _("Datablocks"), itemtool6Bitmap, itemtool6BitmapDisabled, wxITEM_CHECK, _("Show datablocks"), wxEmptyString);
   wxBitmap itemtool7Bitmap(itemPanel1->GetBitmapResource(wxT("icons/functions16.xpm")));
   wxBitmap itemtool7BitmapDisabled;
   itemToolBar4->AddTool(ID_FUNCTIONS, _("Functions"), itemtool7Bitmap, itemtool7BitmapDisabled, wxITEM_CHECK, _("Show global functions"), wxEmptyString);
   wxBitmap itemtool8Bitmap(itemPanel1->GetBitmapResource(wxT("icons/globals16.xpm")));
   wxBitmap itemtool8BitmapDisabled;
   itemToolBar4->AddTool(ID_GLOBALS, _("Globals"), itemtool8Bitmap, itemtool8BitmapDisabled, wxITEM_CHECK, _("Show global variables and named objects"), wxEmptyString);
   wxBitmap itemtool9Bitmap(itemPanel1->GetBitmapResource(wxT("icons/namespace16.xpm")));
   wxBitmap itemtool9BitmapDisabled;
   itemToolBar4->AddTool(ID_NAMESPACES, _("Namespaces"), itemtool9Bitmap, itemtool9BitmapDisabled, wxITEM_CHECK, _("Show namespaces"), wxEmptyString);
   itemToolBar4->Realize();
   itemBoxSizer3->Add(itemToolBar4, 0, wxGROW|wxBOTTOM, 2);

   m_CodeTree = new CodeBrowserCtrl( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(100, 100) );
   itemBoxSizer3->Add(m_CodeTree, 1, wxGROW, 0);

   m_DescCtrl = new wxTextCtrl( itemPanel1, wxID_ANY, _T(""), wxDefaultPosition, wxSize(-1, 120), wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxNO_BORDER );
   itemBoxSizer3->Add(m_DescCtrl, 0, wxGROW|wxALL, 5);

////@end CodeBrowserPanel content construction

    m_DescCtrl->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
    m_CodeTree->SetDescCtrl( m_DescCtrl );
}

/*!
 * Should we show tooltips?
 */

bool CodeBrowserPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CodeBrowserPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CodeBrowserPanel bitmap retrieval
   wxUnusedVar(name);
   if (name == _T("icons/class16.xpm"))
   {
      wxBitmap bitmap( class16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/datablock16.xpm"))
   {
      wxBitmap bitmap( datablock16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/functions16.xpm"))
   {
      wxBitmap bitmap( functions16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/globals16.xpm"))
   {
      wxBitmap bitmap( globals16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/namespace16.xpm"))
   {
      wxBitmap bitmap( namespace16_xpm);
      return bitmap;
   }
   return wxNullBitmap;
////@end CodeBrowserPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CodeBrowserPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CodeBrowserPanel icon retrieval
   wxUnusedVar(name);
   return wxNullIcon;
////@end CodeBrowserPanel icon retrieval
}
/*!
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_CLASSES
 */

void CodeBrowserPanel::OnFilterClick( wxCommandEvent& event )
{
   unsigned int bit = event.GetId() - ID_CLASSES;
   m_CodeTree->SetFilterValue( event.IsChecked(), (1<<bit) );
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_CLASSES
 */

void CodeBrowserPanel::OnFilterUpdate( wxUpdateUIEvent& event )
{
   unsigned int bit = event.GetId() - ID_CLASSES;
   event.Check( m_CodeTree->GetFilterValue() & (1<<bit) );
}


