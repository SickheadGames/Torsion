// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"

#include "ScriptPrintout.h"
#include "ScriptCtrl.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


ScriptPrintout::ScriptPrintout( const wxString& title, ScriptCtrl* ctrl )
   :  wxPrintout( title ),
      m_ScriptCtrl( ctrl )
{
}

void ScriptPrintout::OnPreparePrinting()
{
   m_PagePos.Clear();

   wxDC* dc = GetDC();
   if ( !dc )
      return;

   // Setup the page for printing.
   SetPrintScaling( dc );
   m_ScriptCtrl->SetPrintColourMode( wxSTC_PRINT_BLACKONWHITE );
   m_ScriptCtrl->SetPrintMagnification( 0 );

   // Figure out the total number of pages
   // and the start and end position of each.
   const int posEnd = m_ScriptCtrl->GetLength();
   m_PagePos.Add( 0 );
   for ( int pagePos = 0; pagePos < posEnd; )
   {
      pagePos = m_ScriptCtrl->FormatRange( false, pagePos, posEnd, dc, dc, m_PageRect, m_PageRect );
      m_PagePos.Add( pagePos );
   }
}

void ScriptPrintout::GetPageInfo(int *minPage, int *maxPage, int *pageFrom, int *pageTo)
{
   *minPage = 1;
   *maxPage = m_PagePos.Count()-1;  
}

bool ScriptPrintout::HasPage(int page)
{
   return page > 0 && page < m_PagePos.Count();
}

bool ScriptPrintout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if ( !dc )
       return false;

   SetPrintScaling( dc );

   // Get the position range for this page.
   int pageStart  = m_PagePos[page-1];
   int pageEnd    = m_PagePos[page];

   // Print it!
   m_ScriptCtrl->FormatRange( true, pageStart, pageEnd, dc, dc, m_PageRect, m_PageRect );

   return true;
}

void ScriptPrintout::SetPrintScaling( wxDC *dc )
{
    // get print page informations and convert to printer pixels
    wxSize ppiScr;
    GetPPIScreen(&ppiScr.x, &ppiScr.y);

    wxSize ppiPrn;
    GetPPIPrinter(&ppiPrn.x, &ppiPrn.y);

    wxSize ppi = IsPreview() ? ppiScr : ppiPrn;
    //wxSize page = ;
    //page.x = int(page.x * ppi.x / 25.4);
    //page.y = int(page.y * ppi.y / 25.4);
    m_PageRect = wxRect( dc->GetSize() ); //0, 0, page.x, page.y);
    
    // get margins informations and convert to printer pixels
    wxPoint topLeft( 1, 1 );
    wxPoint bottomRight( 1, 1 );

    int top    = int(topLeft.y     * ppi.y);
    int bottom = int(bottomRight.y * ppi.y);
    int left   = int(topLeft.x     * ppi.x);
    int right  = int(bottomRight.x * ppi.x);

    m_PrintRect = wxRect(left, top, m_PageRect.width-(left+right), m_PageRect.height-(top+bottom));

    int dc_width=1, dc_height=1;
    dc->GetSize(&dc_width, &dc_height);
    int page_width_pix=1, page_height_pix=1;
    GetPageSizePixels(&page_width_pix, &page_height_pix); //# pixels in page

    // FIXME print scaling is really busted
#ifdef __WXMSW__
    float scale_x = float(ppiPrn.x * dc_width )/float(ppiScr.x * page_width_pix);
    float scale_y = float(ppiPrn.y * dc_height)/float(ppiScr.y * page_height_pix);
#else // this works in GTK at least...
    float scale_x = float(dc_width) / float(page_width_pix);
    float scale_y = float(dc_height) / float(page_height_pix);
#endif

    dc->SetUserScale(scale_x, scale_y);

    m_PageRect.x = (float)m_PageRect.x / scale_x;
    m_PageRect.y = (float)m_PageRect.y / scale_y;
    m_PageRect.width = (float)m_PrintRect.width / scale_x;
    m_PageRect.height = (float)m_PrintRect.height / scale_y;
}



