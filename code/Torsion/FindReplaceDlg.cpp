// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "FindReplaceDlg.h"
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

#include "FindReplaceDlg.h"
#include "TorsionApp.h"
#include <wx/fdrepdlg.h>
#include "MainFrame.h"

////@begin XPM images
////@end XPM images

/*!
 * FindReplaceDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( FindReplaceDlg, wxDialog )

/*!
 * FindReplaceDlg event table definition
 */

BEGIN_EVENT_TABLE( FindReplaceDlg, wxDialog )

////@begin FindReplaceDlg event table entries
   EVT_BUTTON( wxID_FIND, FindReplaceDlg::OnFindClick )
   EVT_UPDATE_UI( wxID_FIND, FindReplaceDlg::OnFindUpdate )

   EVT_BUTTON( wxID_REPLACE, FindReplaceDlg::OnReplaceClick )

   EVT_BUTTON( wxID_REPLACE_ALL, FindReplaceDlg::OnReplaceAllClick )

   EVT_BUTTON( wxID_CANCEL, FindReplaceDlg::OnCloseClick )

////@end FindReplaceDlg event table entries

END_EVENT_TABLE()

/*!
 * FindReplaceDlg constructors
 */

FindReplaceDlg::FindReplaceDlg( )
{
}

FindReplaceDlg::FindReplaceDlg( wxWindow* parent, wxFindReplaceData* data, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, data, id, caption, pos, size, style);
}

/*!
 * FindReplaceDlg creator
 */

bool FindReplaceDlg::Create( wxWindow* parent, wxFindReplaceData* data, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin FindReplaceDlg member initialisation
   m_WhatCtrl = NULL;
   m_WithSizer = NULL;
   m_WithCtrl = NULL;
   m_MatchCaseCtrl = NULL;
   m_WholeWordCtrl = NULL;
   m_SearchUpCtrl = NULL;
   m_ButtonSizer = NULL;
////@end FindReplaceDlg member initialisation

    wxASSERT( data );
    m_Data = data;

////@begin FindReplaceDlg creation
   SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
   wxDialog::Create( parent, id, caption, pos, size, style );

   CreateControls();
   if (GetSizer())
   {
      GetSizer()->SetSizeHints(this);
   }
   Centre();
////@end FindReplaceDlg creation

    // Move the window to the last know position.
    wxPoint movePos = tsGetPrefs().GetFindPos();
    Move( movePos );

    return true;
}

/*!
 * Control creation for FindReplaceDlg
 */

void FindReplaceDlg::CreateControls()
{    
////@begin FindReplaceDlg content construction
   FindReplaceDlg* itemDialog1 = this;

   wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
   itemDialog1->SetSizer(itemBoxSizer2);

   wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_TOP|wxALL, 7);

   wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxTOP|wxBOTTOM, 2);

   wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Fi&nd what:"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
   itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 7);

   itemBoxSizer4->Add(0, 5, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 14);

   wxString* m_WhatCtrlStrings = NULL;
   m_WhatCtrl = new wxComboBox( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxSize(210, -1), 0, m_WhatCtrlStrings, wxCB_DROPDOWN );
   itemBoxSizer4->Add(m_WhatCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

   m_WithSizer = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(m_WithSizer, 0, wxALIGN_LEFT, 0);

   wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Re&place with:"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
   m_WithSizer->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 7);

   wxString* m_WithCtrlStrings = NULL;
   m_WithCtrl = new wxComboBox( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxSize(210, -1), 0, m_WithCtrlStrings, wxCB_DROPDOWN );
   m_WithSizer->Add(m_WithCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 2);

   wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
   itemBoxSizer3->Add(itemBoxSizer11, 0, wxALIGN_LEFT|wxRIGHT|wxTOP|wxBOTTOM, 5);

   wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer11->Add(itemBoxSizer12, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5);

   m_MatchCaseCtrl = new wxCheckBox( itemDialog1, wxID_ANY, _("Match &case"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
   m_MatchCaseCtrl->SetValue(false);
   itemBoxSizer12->Add(m_MatchCaseCtrl, 0, wxALIGN_LEFT|wxBOTTOM, 3);

   m_WholeWordCtrl = new wxCheckBox( itemDialog1, wxID_ANY, _("Match &whole word"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
   m_WholeWordCtrl->SetValue(false);
   itemBoxSizer12->Add(m_WholeWordCtrl, 0, wxALIGN_RIGHT|wxTOP|wxBOTTOM, 3);

   m_SearchUpCtrl = new wxCheckBox( itemDialog1, wxID_ANY, _("Search &up"), wxDefaultPosition, wxDefaultSize, 0 );
   m_SearchUpCtrl->SetValue(false);
   itemBoxSizer12->Add(m_SearchUpCtrl, 0, wxALIGN_LEFT|wxTOP, 3);

   wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer11->Add(itemBoxSizer16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

   m_ButtonSizer = new wxBoxSizer(wxVERTICAL);
   itemBoxSizer2->Add(m_ButtonSizer, 0, wxALIGN_TOP|wxRIGHT|wxTOP, 7);

   wxButton* itemButton18 = new wxButton( itemDialog1, wxID_FIND, _("&Find Next"), wxDefaultPosition, wxDefaultSize, 0 );
   itemButton18->SetDefault();
   m_ButtonSizer->Add(itemButton18, 0, wxALIGN_RIGHT|wxBOTTOM, 3);

   wxButton* itemButton19 = new wxButton( itemDialog1, wxID_REPLACE, _("&Replace"), wxDefaultPosition, wxDefaultSize, 0 );
   m_ButtonSizer->Add(itemButton19, 0, wxALIGN_RIGHT|wxBOTTOM, 3);

   wxButton* itemButton20 = new wxButton( itemDialog1, wxID_REPLACE_ALL, _("Replace &All"), wxDefaultPosition, wxDefaultSize, 0 );
   m_ButtonSizer->Add(itemButton20, 0, wxALIGN_RIGHT|wxBOTTOM, 3);

   wxButton* itemButton21 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
   m_ButtonSizer->Add(itemButton21, 0, wxALIGN_RIGHT|wxALL, 0);

////@end FindReplaceDlg content construction

   m_WhatCtrl->Clear();
   AppPrefs::AddStringsToCombo( m_WhatCtrl, tsGetPrefs().GetFindStrings() );
   m_WhatCtrl->SetValue( m_Data->GetFindString() );

   m_MatchCaseCtrl->SetValue( tsGetPrefs().GetFindMatchCase() );
   m_WholeWordCtrl->SetValue( tsGetPrefs().GetFindMatchWord() );
   m_SearchUpCtrl->SetValue( tsGetPrefs().GetFindSearchUp() );
}


bool FindReplaceDlg::Destroy()
{
   // Store the window position.
   tsGetPrefs().SetFindPos( GetPosition() );

   // Store the checkbox settings.
   tsGetPrefs().SetFindMatchCase( m_MatchCaseCtrl->GetValue() );
   tsGetPrefs().SetFindMatchWord( m_WholeWordCtrl->GetValue() );
   tsGetPrefs().SetFindSearchUp( m_SearchUpCtrl->GetValue() );

   return wxDialog::Destroy();
}

bool FindReplaceDlg::Show( bool show )
{
   return ShowReplace( show );
}

bool FindReplaceDlg::ShowFind( bool show )
{
   if ( show )
   {
      SetTitle( "Find" );
      m_WithSizer->Show( (size_t)0, false );
      m_WithSizer->Show( (size_t)1, false );
      m_ButtonSizer->Show( (size_t)1, false );
      m_ButtonSizer->Show( (size_t)2, false );
      Layout();
      Fit();
      Refresh( false );
      m_WhatCtrl->SetFocus();
   }

   return wxDialog::Show( show );
}

bool FindReplaceDlg::ShowReplace( bool show )
{
   if ( show )
   {
      SetTitle( "Replace" );
      m_WithSizer->Show( (size_t)0, true );
      m_WithSizer->Show( (size_t)1, true );
      m_ButtonSizer->Show( (size_t)1, true );
      m_ButtonSizer->Show( (size_t)2, true );
      Layout();
      Fit();
      Refresh( false );
      m_WhatCtrl->SetFocus();
   }

   return wxDialog::Show( show );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FIND
 */

void FindReplaceDlg::OnFindClick( wxCommandEvent& event )
{
   wxString what = m_WhatCtrl->GetValue();

   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_NEXT );
   fevent.SetFindString( what );
   fevent.SetFlags(  ( m_SearchUpCtrl->GetValue() ? 0 : wxFR_DOWN ) |  
                     ( m_MatchCaseCtrl->GetValue() ? wxFR_MATCHCASE : 0 ) |
                     ( m_WholeWordCtrl->GetValue() ? wxFR_WHOLEWORD : 0 ) );

   wxASSERT( GetParent() );
   reinterpret_cast<MainFrame*>(GetParent())->ProcessEvent(fevent);
}

/*!
 * wxEVT_UPDATE_UI event handler for wxID_FIND
 */

void FindReplaceDlg::OnFindUpdate( wxUpdateUIEvent& event )
{
   event.Enable( !m_WhatCtrl->GetValue().IsEmpty() ); 
}

/*!
 * Should we show tooltips?
 */

bool FindReplaceDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap FindReplaceDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin FindReplaceDlg bitmap retrieval
   wxUnusedVar(name);
   return wxNullBitmap;
////@end FindReplaceDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon FindReplaceDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin FindReplaceDlg icon retrieval
   wxUnusedVar(name);
   return wxNullIcon;
////@end FindReplaceDlg icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REPLACE
 */

void FindReplaceDlg::OnReplaceClick( wxCommandEvent& event )
{
   wxString what = m_WhatCtrl->GetValue();
   wxString with = m_WithCtrl->GetValue();

   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_REPLACE );
   fevent.SetFindString( what );
   fevent.SetReplaceString( with );
   fevent.SetFlags(  ( m_SearchUpCtrl->GetValue() ? 0 : wxFR_DOWN ) |  
                     ( m_MatchCaseCtrl->GetValue() ? wxFR_MATCHCASE : 0 ) |
                     ( m_WholeWordCtrl->GetValue() ? wxFR_WHOLEWORD : 0 ) );

   wxASSERT( GetParent() );
   reinterpret_cast<MainFrame*>(GetParent())->ProcessEvent(fevent);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REPLACE_ALL
 */

void FindReplaceDlg::OnReplaceAllClick( wxCommandEvent& event )
{
   wxString what = m_WhatCtrl->GetValue();
   wxString with = m_WithCtrl->GetValue();

   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_REPLACE_ALL );
   fevent.SetFindString( what );
   fevent.SetReplaceString( with );
   fevent.SetFlags(  ( m_SearchUpCtrl->GetValue() ? 0 : wxFR_DOWN ) |  
                     ( m_MatchCaseCtrl->GetValue() ? wxFR_MATCHCASE : 0 ) |
                     ( m_WholeWordCtrl->GetValue() ? wxFR_WHOLEWORD : 0 ) );

   wxASSERT( GetParent() );
   reinterpret_cast<MainFrame*>(GetParent())->ProcessEvent(fevent);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void FindReplaceDlg::OnCloseClick( wxCommandEvent& event )
{
   // Let the frame deal with closing us!
   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_CLOSE );
   wxASSERT( GetParent() );
   reinterpret_cast<MainFrame*>(GetParent())->ProcessEvent(fevent);
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for wxID_ANY
 */

void FindReplaceDlg::OnCloseWindow( wxCloseEvent& event )
{
   event.Veto();

   // Let the frame deal with closing us!
   wxFindDialogEvent fevent( wxEVT_COMMAND_FIND_CLOSE );
   wxASSERT( GetParent() );
   reinterpret_cast<MainFrame*>(GetParent())->ProcessEvent(fevent);
}
