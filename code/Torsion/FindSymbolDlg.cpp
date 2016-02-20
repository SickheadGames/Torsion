// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "FindSymbolDlg.cpp"
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

#include "FindSymbolDlg.h"
#include "TorsionApp.h"

////@begin XPM images
////@end XPM images

/*!
 * FindSymbolDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( FindSymbolDlg, wxDialog )

/*!
 * FindSymbolDlg event table definition
 */

BEGIN_EVENT_TABLE( FindSymbolDlg, wxDialog )

////@begin FindSymbolDlg event table entries
   EVT_BUTTON( wxID_OK, FindSymbolDlg::OnFindClick )
   EVT_UPDATE_UI( wxID_OK, FindSymbolDlg::OnFindUpdate )

////@end FindSymbolDlg event table entries

END_EVENT_TABLE()

/*!
 * FindSymbolDlg constructors
 */

FindSymbolDlg::FindSymbolDlg( )
{
   Init();
}

FindSymbolDlg::FindSymbolDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
   Init();
   Create(parent, id, caption, pos, size, style);
}

/*!
 * FindSymbolDlg creator
 */

bool FindSymbolDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin FindSymbolDlg creation
   SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
   wxDialog::Create( parent, id, caption, pos, size, style );

   CreateControls();
   if (GetSizer())
   {
      GetSizer()->SetSizeHints(this);
   }
   Centre();
////@end FindSymbolDlg creation
   return true;
}

/*!
 * Member initialisation 
 */

void FindSymbolDlg::Init()
{
////@begin FindSymbolDlg member initialisation
   m_WhatCtrl = NULL;
////@end FindSymbolDlg member initialisation
}
/*!
 * Control creation for FindSymbolDlg
 */

void FindSymbolDlg::CreateControls()
{    
////@begin FindSymbolDlg content construction
   FindSymbolDlg* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
   itemDialog1->SetSizer(itemBoxSizer2);

   wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_TOP|wxALL, 7);

   wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 2);

   wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Fi&nd what:"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
   itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxADJUST_MINSIZE, 7);

   itemBoxSizer4->Add(0, 5, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 14);

   wxString* m_WhatCtrlStrings = NULL;
   m_WhatCtrl = new wxComboBox( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxSize(210, -1), 0, m_WhatCtrlStrings, wxCB_DROPDOWN );
   itemBoxSizer4->Add(m_WhatCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

   wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_TOP|wxRIGHT|wxTOP|wxBOTTOM, 7);

   wxButton* itemButton9 = new wxButton( itemDialog1, wxID_OK, _("&Find"), wxDefaultPosition, wxDefaultSize, 0 );
   itemButton9->SetDefault();
   itemBoxSizer8->Add(itemButton9, 0, wxALIGN_RIGHT|wxBOTTOM, 3);

   wxButton* itemButton10 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
   itemBoxSizer8->Add(itemButton10, 0, wxALIGN_RIGHT|wxALL, 0);

   itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 7);

////@end FindSymbolDlg content construction
   m_WhatCtrl->Clear();
   AppPrefs::AddStringsToCombo( m_WhatCtrl, tsGetPrefs().GetFindSymbols() );

   m_WhatCtrl->SetFocus();
}

/*!
 * Should we show tooltips?
 */

bool FindSymbolDlg::ShowToolTips()
{
   return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap FindSymbolDlg::GetBitmapResource( const wxString& name )
{
   // Bitmap retrieval
////@begin FindSymbolDlg bitmap retrieval
   wxUnusedVar(name);
   return wxNullBitmap;
////@end FindSymbolDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon FindSymbolDlg::GetIconResource( const wxString& name )
{
   // Icon retrieval
////@begin FindSymbolDlg icon retrieval
   wxUnusedVar(name);
   return wxNullIcon;
////@end FindSymbolDlg icon retrieval
}

void FindSymbolDlg::AddWhat( const wxString& what )
{
   wxASSERT( m_WhatCtrl );
   
   if ( what.IsEmpty() ) 
      return;

   // First remove it if it is already in the list.
   int Index = m_WhatCtrl->FindString( what );
   if ( Index != wxNOT_FOUND )
      m_WhatCtrl->Delete( Index );

   // Insert it into the top of the list.
   m_WhatCtrl->Insert( what, 0 );

   // Make sure it's the active item.
   m_WhatCtrl->SetValue( what );
   m_What = what;
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_OK
 */

void FindSymbolDlg::OnFindUpdate( wxUpdateUIEvent& event )
{
   event.Enable( m_WhatCtrl && !m_WhatCtrl->GetValue().IsEmpty() );
}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void FindSymbolDlg::OnFindClick( wxCommandEvent& event )
{
   // Add the new value to the list... this
   // makes sure it is only in the list once.
   Hide();
   AddWhat( m_WhatCtrl->GetValue() );

   // Store the new symbol parameters into the app prefs.
   wxArrayString strings;
   AppPrefs::GetStringsFromCombo( m_WhatCtrl, strings );
   if ( strings.GetCount() > 20 )
      strings.SetCount( 20 );
   tsGetPrefs().SetFindSymbols( strings );

   event.Skip();
}


