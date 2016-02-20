// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCRIPTPRINTOUT_H
#define TORSION_SCRIPTPRINTOUT_H
#pragma once

#include <wx/print.h>

//class wxPrintData;
class ScriptCtrl;


class ScriptPrintout : public wxPrintout
{
   public:

      ScriptPrintout( const wxString& title, ScriptCtrl* ctrl );
      
      void OnPreparePrinting();
      void GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo);
      bool HasPage(int page);
      bool OnPrintPage(int page);

   protected:

      void SetPrintScaling( wxDC *dc );

      wxRect         m_PageRect;
      wxRect         m_PrintRect;
      ScriptCtrl*    m_ScriptCtrl;
      wxArrayInt     m_PagePos;
};

#endif // TORSION_SCRIPTPRINTOUT_H
