// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_CONNECTDLG_H
#define TORSION_CONNECTDLG_H
#pragma once

#include <wx/dialog.h>


class ConnectDlg : public wxDialog
{
   protected:

      wxSocketClient*   m_Server;
      wxIPV4address*    m_Address;
      wxString          m_Password;
      bool              m_Connecting;

   public:

      ConnectDlg( wxWindow* parent, wxSocketClient* server, wxIPV4address* address, const wxString& password, bool connectNow );

      enum {
         ID_CONNECT = wxID_HIGHEST,
         ID_SOCKET_EVENT,
      };

      void OnInitDialog( wxInitDialogEvent& event );
      void OnConnect( wxCommandEvent& event );
      void OnCancel( wxCommandEvent& event );

      void OnSocketEvent( wxSocketEvent& event );

      const wxString& GetPassword() const { return m_Password; }

   protected:

      bool                 m_ConnectNow;
      wxTextCtrl*          m_AddressCtrl;
      wxTextCtrl*          m_PortCtrl;
      wxTextCtrl*          m_PasswordCtrl;

      DECLARE_EVENT_TABLE()
};


#endif // TORSION_CONNECTDLG_H