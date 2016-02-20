// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_TSSASHLAYOUTWINDOW_H
#define TORSION_TSSASHLAYOUTWINDOW_H
#pragma once

#include <wx/laywin.h>


class tsSashLayoutWindow : public wxSashLayoutWindow
{
public:

   tsSashLayoutWindow(  wxWindow *parent, 
                        wxWindowID id = wxID_ANY, 
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize, 
                        long style = wxSW_3D|wxCLIP_CHILDREN, 
                        const wxString& name = wxT("layoutWindow") )
   {
      m_HasPainted = false;
      Create(parent, id, pos, size, style, name);
   }

protected:

   void OnPaint( wxPaintEvent& event );
   void OnEraseBackground( wxEraseEvent& event );

   bool m_HasPainted;

   DECLARE_EVENT_TABLE()
};


#endif // TORSION_TSSASHLAYOUTWINDOW_H