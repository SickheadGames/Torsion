// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "GotoDefDialog.h"
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

#include "GotoDefDialog.h"

#include "MainFrame.h"
#include "ProjectDoc.h"

#include <wx/txtstrm.h>
#include <wx/wfstream.h>

////@begin XPM images
////@end XPM images

/*!
 * GotoDefDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( GotoDefDialog, wxDialog )

/*!
 * GotoDefDialog event table definition
 */

BEGIN_EVENT_TABLE( GotoDefDialog, wxDialog )

////@begin GotoDefDialog event table entries
    EVT_LIST_ITEM_ACTIVATED( ID_SYMBOLSLIST, GotoDefDialog::OnSymbolDoubleClicked )

    EVT_BUTTON( wxID_OK, GotoDefDialog::OnOkClick )
    EVT_UPDATE_UI( wxID_OK, GotoDefDialog::OnOkUpdate )

////@end GotoDefDialog event table entries

END_EVENT_TABLE()

/*!
 * GotoDefDialog constructors
 */

GotoDefDialog::GotoDefDialog( )
   : m_Selected( -1 )
{
}

GotoDefDialog::GotoDefDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
   : m_Selected( -1 )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * GotoDefDialog creator
 */

bool GotoDefDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin GotoDefDialog member initialisation
    m_SymbolList = NULL;
////@end GotoDefDialog member initialisation

////@begin GotoDefDialog creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end GotoDefDialog creation
    return true;
}

/*!
 * Control creation for GotoDefDialog
 */

void GotoDefDialog::CreateControls()
{    
////@begin GotoDefDialog content construction
    GotoDefDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 7);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("The symbol is defined in multiple locations:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxBOTTOM|wxADJUST_MINSIZE, 2);

    m_SymbolList = new wxListView( itemDialog1, ID_SYMBOLSLIST, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT|wxLC_SINGLE_SEL );
    itemBoxSizer3->Add(m_SymbolList, 1, wxGROW, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer6 = new wxStdDialogButtonSizer;

    itemBoxSizer3->Add(itemStdDialogButtonSizer6, 0, wxALIGN_RIGHT|wxTOP, 7);
    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("&Go To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer6->AddButton(itemButton7);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer6->AddButton(itemButton8);

    itemStdDialogButtonSizer6->Realize();

////@end GotoDefDialog content construction

    m_SymbolList->InsertColumn( 0, "Symbol", wxLIST_FORMAT_LEFT, 200 );
    m_SymbolList->InsertColumn( 1, "File", wxLIST_FORMAT_LEFT, 210 );
    m_SymbolList->InsertColumn( 2, "Line", wxLIST_FORMAT_LEFT, 39 );
}

int GotoDefDialog::ShowModal( const wxArrayString& paths, const wxArrayInt& lines, const wxArrayString& symbols )
{
   m_Selected = -1;
  
   wxASSERT( paths.GetCount() == lines.GetCount() );

   m_Paths = paths;
   m_Lines = lines;
   
   /*
   // Read in each line and stick it into the symbol list.
   m_Symbols.Insert( "", 0, m_Paths.GetCount() );
   for ( int i=0; i < m_Paths.GetCount(); i++ )
   {
      wxFFileInputStream input( m_Paths[i], "rt" );
      wxTextInputStream text( input );
      wxString line;

      for ( int l=0; l <= m_Lines[i]; l++ )
         line = text.ReadLine();

      line.Trim();
      line.Trim( false );
      m_Symbols[i] = line;
   }
   */

   // Convert the paths to be relative to the project
   // if we have an active project.
   const ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   if ( project )
   {
      for ( int i=0; i < m_Paths.GetCount(); i++ )
         m_Paths[i] = project->MakeReleativeTo( m_Paths[i] );
   }

   wxASSERT( m_SymbolList );
   m_SymbolList->DeleteAllItems();

   for ( int i=0; i < m_Paths.GetCount(); i++ )
   {
      m_SymbolList->InsertItem( i, symbols[i] );
      m_SymbolList->SetItem( i, 1, m_Paths[i] );
      //m_SymbolList->InsertItem( i, m_Paths[i] );
      wxString line;
      line << (m_Lines[i] + 1); // lines are zero based
      m_SymbolList->SetItem( i, 2, line );
   }

   return wxDialog::ShowModal();
}

wxString GotoDefDialog::GetPath() const
{
   if ( m_Selected < 0 || m_Selected >= m_Paths.GetCount() )
      return wxEmptyString;

   wxASSERT( m_Paths.GetCount() == m_Lines.GetCount() );
   return m_Paths[ m_Selected ];
}

int GotoDefDialog::GetLine() const
{
   if ( m_Selected < 0 || m_Selected >= m_Paths.GetCount() )
      return 0;

   wxASSERT( m_Paths.GetCount() == m_Lines.GetCount() );
   return m_Lines[ m_Selected ];
}

/*!
 * Should we show tooltips?
 */

bool GotoDefDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap GotoDefDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin GotoDefDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end GotoDefDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon GotoDefDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin GotoDefDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end GotoDefDialog icon retrieval
}
/*!
 * wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for wxID_ANY
 */

void GotoDefDialog::OnSymbolDoubleClicked( wxListEvent& event )
{
   m_Selected = event.GetIndex();
   wxDialog::OnOK( wxCommandEvent() );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void GotoDefDialog::OnOkClick( wxCommandEvent& event )
{
   wxASSERT( m_SymbolList );
   wxASSERT( m_SymbolList->GetSelectedItemCount() == 1 );
   m_Selected = m_SymbolList->GetFirstSelected();
   wxDialog::OnOK( event );
}


/*!
 * wxEVT_UPDATE_UI event handler for wxID_OK
 */

void GotoDefDialog::OnOkUpdate( wxUpdateUIEvent& event )
{
   wxASSERT( m_SymbolList );
   event.Enable( m_SymbolList->GetSelectedItemCount() > 0 );
}


