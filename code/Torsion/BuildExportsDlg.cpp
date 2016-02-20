// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "BuildExportsDlg.h"
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
#include "BuildExportsDlg.h"
#include "BuildExportsThread.h"

////@begin XPM images
////@end XPM images

/*!
 * BuildExportsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( BuildExportsDlg, wxDialog )

/*!
 * BuildExportsDlg event table definition
 */

BEGIN_EVENT_TABLE( BuildExportsDlg, wxDialog )

   EVT_TIMER(wxID_ANY, BuildExportsDlg::OnTimer)

////@begin BuildExportsDlg event table entries
    EVT_IDLE( BuildExportsDlg::OnIdle )

    EVT_BUTTON( wxID_CANCEL, BuildExportsDlg::OnCancel )

////@end BuildExportsDlg event table entries

END_EVENT_TABLE()

/*!
 * BuildExportsDlg constructors
 */

BuildExportsDlg::BuildExportsDlg( )
   :  m_Thread( NULL ),
      m_Exports( NULL )
{
}

BuildExportsDlg::BuildExportsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
   :  m_Thread( NULL ),
      m_Exports( NULL )
{
    Create(parent, id, caption, pos, size, style);
}

BuildExportsDlg::~BuildExportsDlg()
{
   if ( m_Thread && m_Thread->IsRunning() )
   {
      m_Thread->Pause();
      m_Thread->Delete();
      wxDELETE( m_Thread );
   }
   wxDELETE( m_Exports );
}

/*!
 * BuildExportsDlg creator
 */

bool BuildExportsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin BuildExportsDlg member initialisation
    m_Progress = NULL;
////@end BuildExportsDlg member initialisation

////@begin BuildExportsDlg creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end BuildExportsDlg creation
    return true;
}

/*!
 * Control creation for BuildExportsDlg
 */

void BuildExportsDlg::CreateControls()
{    
////@begin BuildExportsDlg content construction
    BuildExportsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 7);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Torsion is executing project configurations to generate a database\nof engine exports.  This may take up to a minute to complete."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxADJUST_MINSIZE, 14);

    m_Progress = new wxGauge( itemDialog1, ID_GAUGE, 100, wxDefaultPosition, wxSize(200, 20), 0 );
    m_Progress->SetValue(0);
    itemBoxSizer3->Add(m_Progress, 0, wxGROW|wxLEFT|wxRIGHT, 14);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 7);

////@end BuildExportsDlg content construction

    // Setup the timer to check the thread status.
    m_Timer.SetOwner( this );
    m_Timer.Start( 500 );
}

int BuildExportsDlg::ShowModal(  const wxString& exportScriptPath,
                                 const wxString& workingDir,
                                 const ProjectConfigArray& configs )
{
   m_Thread = 
      new BuildExportsThread( exportScriptPath, 
                              workingDir,
                              configs );
   m_Thread->Create();
   m_Thread->SetPriority( 100 );
   m_Thread->Run();

   return wxDialog::ShowModal();
}

/*!
 * Should we show tooltips?
 */

bool BuildExportsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap BuildExportsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin BuildExportsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end BuildExportsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon BuildExportsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin BuildExportsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end BuildExportsDlg icon retrieval
}

void BuildExportsDlg::OnTimer( wxTimerEvent& event )
{
   wxASSERT( m_Thread );

   // Update the progress.
   m_Progress->SetValue( (int)m_Thread->GetProgressPct() );

   // If the thread is done!
   if ( !m_Thread->IsRunning() )
   {
      m_Timer.Stop();

      m_Exports = m_Thread->TakeExports();
      m_Thread->Delete();
      wxDELETE( m_Thread );

      // End the dialog!
      EndModal( m_Exports ? wxID_OK : wxID_CANCEL );
      return;
   }
}

AutoCompExports* BuildExportsDlg::TakeExports()
{
   AutoCompExports* exports = m_Exports;
   m_Exports = NULL;
   return exports;
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void BuildExportsDlg::OnCancel( wxCommandEvent& event )
{
   m_Timer.Stop();

   if ( m_Thread && m_Thread->IsRunning() )
   {
      m_Thread->Pause();
      m_Thread->Delete();
      wxDELETE( m_Thread );
   }

   EndModal( wxID_CANCEL );
}


/*!
 * wxEVT_IDLE event handler for ID_DIALOG
 */

void BuildExportsDlg::OnIdle( wxIdleEvent& event )
{
////@begin wxEVT_IDLE event handler for ID_DIALOG in BuildExportsDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_IDLE event handler for ID_DIALOG in BuildExportsDlg. 
}


