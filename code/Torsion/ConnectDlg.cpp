// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ConnectDlg.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


ConnectDlg::ConnectDlg( wxWindow* parent, wxSocketClient* server, 
                        wxIPV4address* address, const wxString& password, 
                        bool connectNow )
   :  wxDialog( (wxWindow*)parent, (wxWindowID)wxID_ANY, wxString( "Connecting" ) ),
      m_Server( server ),
      m_Address( address ),
      m_Password( password ),
      m_Connecting( false ),
      m_ConnectNow( connectNow )
{
}

BEGIN_EVENT_TABLE( ConnectDlg, wxDialog )
   EVT_INIT_DIALOG( OnInitDialog )   
   EVT_BUTTON( ID_CONNECT, OnConnect )
   EVT_BUTTON( wxID_CANCEL, OnCancel )
	EVT_SOCKET( ID_SOCKET_EVENT, OnSocketEvent )
END_EVENT_TABLE()

void ConnectDlg::OnInitDialog( wxInitDialogEvent& event )
{
   wxASSERT( m_Server );
   wxASSERT( m_Address );

   CentreOnParent();

   wxBoxSizer* content = new wxBoxSizer( wxVERTICAL );

   // The address
   content->Add( new wxStaticText( this, wxID_ANY, "&Address:" ) );
   content->AddSpacer( 5 );
   m_AddressCtrl = new wxTextCtrl( this, wxID_ANY, m_Address->IPAddress() );
   content->Add( m_AddressCtrl, 0, wxEXPAND | wxLEFT, 10 );
   content->AddSpacer( 7 );

   // The port
   content->Add( new wxStaticText( this, wxID_ANY, "&Port:" ) );
   content->AddSpacer( 5 );
   m_PortCtrl = new wxTextCtrl( this, wxID_ANY, wxString() << m_Address->Service() );
   content->Add( m_PortCtrl, 0, wxEXPAND | wxLEFT, 10 );
   content->AddSpacer( 7 );

   // The port
   content->Add( new wxStaticText( this, wxID_ANY, "Pass&word:" ) );
   content->AddSpacer( 5 );
   m_PasswordCtrl = new wxTextCtrl( this, wxID_ANY, m_Password );
   content->Add( m_PasswordCtrl, 0, wxEXPAND | wxLEFT, 10 );

   // Add the animated icon on the left.
   //wxBoxSizer* content2 = new wxBoxSizer( wxHORIZONTAL );
   //content->Add( m_PasswordCtrl, 0, wxEXPAND | wxLEFT, 10 );

   // the cancel button
   content->AddSpacer( 14 );
   wxBoxSizer* bsizer = new wxBoxSizer( wxHORIZONTAL );
   bsizer->Add( new wxButton( this, ID_CONNECT, "Connect" ) );
   bsizer->AddSpacer( 5 );
   bsizer->Add( new wxButton( this, wxID_CANCEL, "&Cancel" ) );
   content->Add( bsizer, 0, wxALIGN_CENTER_HORIZONTAL );

   // Create a sizer for the borders around the entire
   // content... then add it all to the dialog.
   wxBoxSizer* border = new wxBoxSizer( wxVERTICAL );
   border->Add( content, 0, wxALL, 10 );
   SetSizerAndFit( border );

   if ( m_ConnectNow )
      OnConnect( wxCommandEvent() );
}

void ConnectDlg::OnConnect( wxCommandEvent& event )
{
   wxLogDebug( "ConnectDlg::OnConnect!\n" );

   m_Connecting = true;

   wxASSERT( m_Address );
   m_Address->Hostname( m_AddressCtrl->GetValue() );
   m_Address->Service( m_PortCtrl->GetValue() );
   m_Password = m_PasswordCtrl->GetValue();

   m_AddressCtrl->Enable( false );
   m_PortCtrl->Enable( false );
   m_PasswordCtrl->Enable( false );

   wxASSERT( FindWindow( ID_CONNECT ) );
   FindWindow( ID_CONNECT )->Enable( false );

   // We do a connection with the wait disabled which means
   // that it will return a 'lost' event shortly after.  This
   // lost connection is retried in the event handler.
   wxASSERT( m_Server );
   m_Server->Discard();
   m_Server->Close();
   m_Server->SetEventHandler( *this, ID_SOCKET_EVENT );
   m_Server->SetNotify( wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG );
	m_Server->Notify( true );
   m_Server->Connect( *m_Address, false );
}

void ConnectDlg::OnCancel( wxCommandEvent& event )
{
   // Stop the server connection.
   wxASSERT( m_Server );
   m_Server->Notify( false );
   m_Server->Discard();
   m_Server->Close();

   if ( m_Connecting && !m_ConnectNow ) 
   {
      // Reset the dialog waiting for 
      // the next connection.

      m_Connecting = false;
      m_AddressCtrl->Enable( true );
      m_PortCtrl->Enable( true );
      m_PasswordCtrl->Enable( true );

      wxASSERT( FindWindow( ID_CONNECT ) );
      FindWindow( ID_CONNECT )->Enable( true );
   } 
   else 
   {
      // Close the dialog.
      //wxDialog::OnCancel( event );
	  EndModal(wxID_CANCEL);
   }
}

void ConnectDlg::OnSocketEvent( wxSocketEvent& event )
{
   wxLogDebug( "ConnectDlg::OnSocketEvent!\n" );

   wxASSERT( m_Server );
   wxASSERT( m_Server == event.GetSocket() );

	if ( event.GetSocketEvent() == wxSOCKET_LOST ) 
   {      
   	// The connection timed out... try to connect again.
      m_Server->Close();
      m_Server->Connect( *m_Address, false );
	} 
   else if ( event.GetSocketEvent() == wxSOCKET_CONNECTION ) 
   {
      // We connected... disable notifications and 
      // shutdown this dialog.
      m_Server->Notify( false );
      EndModal( wxID_OK );
   }
   else
   {
      // If we got here we got some unexpected event!
      wxASSERT( false );
   }
}




