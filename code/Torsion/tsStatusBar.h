// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_TSSTATUSBAR_H
#define TORSION_TSSTATUSBAR_H
#pragma once

#include <wx/statusbr.h>

class tsStatusBar : public wxStatusBar
{
public:

    tsStatusBar(  wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  long style = wxST_SIZEGRIP,
                  const wxString& name = wxEmptyString);

protected:

   #ifdef __WXMSW__
   void OnEraseBackground( wxEraseEvent& event );
   void OnPaint( wxPaintEvent& event );
   //WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
   #endif

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_TSSTATUSBAR_H