// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_CLIENTWINDOW_H
#define TORSION_CLIENTWINDOW_H
#pragma once

#include <wx/docmdi.h>


class ClientWindow : public wxMDIClientWindow
{
   DECLARE_CLASS(ClientWindow)

   public:

      ClientWindow();
      
      bool CreateClient( wxMDIParentFrame* parent, long style );

      void OnPaint( wxPaintEvent& event );
      void OnSize( wxSizeEvent& event );

   protected:

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_CLIENTWINDOW_H
