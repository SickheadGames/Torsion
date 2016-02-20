// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "ProjectPanel.h"
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

#include "ProjectPanel.h"
#include "Identifiers.h"
#include "tsToolBar.h"

////@begin XPM images
#include "icons/filter16.xpm"
#include "icons/filter_nonscript16.xpm"
#include "icons/deletedso16.xpm"
#include "icons/sync_tree16.xpm"
#include "icons/refresh16.xpm"
////@end XPM images

/*!
 * ProjectPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( ProjectPanel, wxPanel )

/*!
 * ProjectPanel event table definition
 */

BEGIN_EVENT_TABLE( ProjectPanel, wxPanel )

////@begin ProjectPanel event table entries
////@end ProjectPanel event table entries

END_EVENT_TABLE()

/*!
 * ProjectPanel constructors
 */

ProjectPanel::ProjectPanel( )
{
}

ProjectPanel::ProjectPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * ProjectPanel creator
 */

bool ProjectPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ProjectPanel member initialisation
   m_ProjectCtrl = NULL;
////@end ProjectPanel member initialisation

////@begin ProjectPanel creation
   wxPanel::Create( parent, id, pos, size, style );

   CreateControls();
   if (GetSizer())
   {
      GetSizer()->SetSizeHints(this);
   }
////@end ProjectPanel creation
   
   SetLabel( SYMBOL_PROJECTPANEL_TITLE );

   return true;
}

/*!
 * Control creation for ProjectPanel
 */

void ProjectPanel::CreateControls()
{    
////@begin ProjectPanel content construction
   ProjectPanel* itemPanel1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
   itemPanel1->SetSizer(itemBoxSizer2);

   wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW, 0);

   tsToolBar* itemToolBar4 = new tsToolBar( itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER|wxNO_BORDER );
   itemToolBar4->SetToolBitmapSize(wxSize(16, 16));
   wxBitmap itemtool5Bitmap(itemPanel1->GetBitmapResource(wxT("icons/filter16.xpm")));
   wxBitmap itemtool5BitmapDisabled;
   itemToolBar4->AddTool(tsID_PROJECT_SHOWALLMODS, _T(""), itemtool5Bitmap, itemtool5BitmapDisabled, wxITEM_CHECK, _("Show All Mods"), wxEmptyString);
   wxBitmap itemtool6Bitmap(itemPanel1->GetBitmapResource(wxT("icons/filter_nonscript16.xpm")));
   wxBitmap itemtool6BitmapDisabled;
   itemToolBar4->AddTool(tsID_PROJECT_SHOWALLFILES, _T(""), itemtool6Bitmap, itemtool6BitmapDisabled, wxITEM_CHECK, _("Show All Files"), wxEmptyString);
   wxBitmap itemtool7Bitmap(itemPanel1->GetBitmapResource(wxT("icons/deletedso16.xpm")));
   wxBitmap itemtool7BitmapDisabled;
   itemToolBar4->AddTool(tsID_CLEARDSOS, _T(""), itemtool7Bitmap, itemtool7BitmapDisabled, wxITEM_NORMAL, _("Delete DSOs"), wxEmptyString);
   wxBitmap itemtool8Bitmap(itemPanel1->GetBitmapResource(wxT("icons/sync_tree16.xpm")));
   wxBitmap itemtool8BitmapDisabled;
   itemToolBar4->AddTool(tsID_PROJECT_SYNCTREE, _T(""), itemtool8Bitmap, itemtool8BitmapDisabled, wxITEM_CHECK, _("Sync With Editor"), wxEmptyString);
   wxBitmap itemtool9Bitmap(itemPanel1->GetBitmapResource(wxT("icons/refresh16.xpm")));
   wxBitmap itemtool9BitmapDisabled;
   itemToolBar4->AddTool(tsID_PROJECT_REFRESH, _T(""), itemtool9Bitmap, itemtool9BitmapDisabled, wxITEM_NORMAL, _("Refresh"), wxEmptyString);
   itemToolBar4->Realize();
   itemBoxSizer3->Add(itemToolBar4, 0, wxGROW|wxBOTTOM, 2);

   m_ProjectCtrl = new ProjectCtrl( itemPanel1 );
   itemBoxSizer3->Add(m_ProjectCtrl, 1, wxGROW, 0);

////@end ProjectPanel content construction
}

/*!
 * Should we show tooltips?
 */

bool ProjectPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap ProjectPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ProjectPanel bitmap retrieval
   wxUnusedVar(name);
   if (name == _T("icons/filter16.xpm"))
   {
      wxBitmap bitmap( filter16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/filter_nonscript16.xpm"))
   {
      wxBitmap bitmap( filter_nonscript16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/deletedso16.xpm"))
   {
      wxBitmap bitmap( deletedso16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/sync_tree16.xpm"))
   {
      wxBitmap bitmap( sync_tree16_xpm);
      return bitmap;
   }
   else if (name == _T("icons/refresh16.xpm"))
   {
      wxBitmap bitmap( refresh16_xpm);
      return bitmap;
   }
   return wxNullBitmap;
////@end ProjectPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon ProjectPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ProjectPanel icon retrieval
   wxUnusedVar(name);
   return wxNullIcon;
////@end ProjectPanel icon retrieval
}
