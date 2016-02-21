// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "VSMenuItem.h"

#include "ColourHLS.h"
#include "ImageUtil.h"

#include <wx/image.h>
#include <wx/rawbmp.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


VSMenuItem::VSMenuItem( wxMenu* parentMenu, int id, const wxString& text, const wxString& helpString, wxItemKind kind, wxMenu* subMenu )
   : wxMenuItem( parentMenu, id, text, helpString, kind, subMenu )
{
   NONCLIENTMETRICS nm;
   nm.cbSize = sizeof(NONCLIENTMETRICS);
   SystemParametersInfo(SPI_GETNONCLIENTMETRICS,0,&nm,0);
   m_nMinHeight = nm.iMenuHeight;

   SetOwnerDrawn( true );
}

VSMenuItem::~VSMenuItem()
{
}

bool VSMenuItem::OnMeasureItem( size_t *pwidth, size_t *pheight )
{
   // Fixup our margin width... it's always 16.
   SetMarginWidth( 16 );

   if ( IsSeparator() )
   {
      *pheight = 2;
      *pwidth = 114;
   }
   else
   {
      wxMemoryDC dc;
      wxString strMenuText = GetItemLabelText();
      wxString str = wxStripMenuCodes(strMenuText);

	  wxFont fnt;
	  GetFontToUse(fnt);
      dc.SetFont(fnt);
      wxSize wh = dc.GetTextExtent(str);
	  pwidth = (size_t*)wh.GetWidth();
	  pheight = (size_t*)wh.GetHeight();
      dc.SetFont( wxNullFont );

      // If we have an accelerator then measure 
      // it seperately.
	  wxString m_strAccel = GetItemLabelText().AfterFirst('\t');
      if ( !m_strAccel.empty() )
      {
         long w, h;
         wxSize wh = dc.GetTextExtent( m_strAccel );
		 w = wh.GetWidth();
		 h = wh.GetHeight();

         if ( *pheight < h )
            *pheight = h;

         *pwidth += 16 + w;
      }

      // add space at the end of the menu for the submenu expansion arrow
      // this will also allow offsetting the accel string from the right edge
      *pwidth += GetMarginWidth() + 16 + 4;

      // Adjust the height.
      size_t adjustedHeight = GetMarginWidth() +
                                 (2*wxSystemSettings::GetMetric(wxSYS_EDGE_Y)) + 3;
      if (*pheight < adjustedHeight)
         *pheight = adjustedHeight;

      // Maybe the font was small?
      if ( *pheight < m_nMinHeight )
          *pheight = m_nMinHeight;
   }

   // Never let the menu get smaller than this.
   if ( *pwidth < 114 )
      *pwidth = 114;

   // remember height for use in OnDrawItem
   m_nHeight = *pheight;

   return true;
}

bool VSMenuItem::OnDrawItem( wxDC& dc, const wxRect& rc, wxODAction act, wxODStatus st )
{
    // we do nothing on focus change
    if ( act == wxODFocusChanged )
        return true;

    // To mimick VC++ style... if disabled never 
    // draw it selected or in any other style.
    if ( st & wxODDisabled && st & wxODSelected )
       st = wxODDisabled;
    else if ( IsSeparator() )
    {
       st = wxODStatus( 0 ); // does nothing!
    }
    
   // Setup the colors depending on the style.
   DWORD colBack, colSel, colSelBorder, colText;
   {
      wxColour from = wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE );
      wxImage::HSVValue hsv = wxImage::RGBtoHSV( wxImage::RGBValue( from.Red(), from.Green(), from.Blue() ) );
      hsv.saturation *= 0.08f;
      hsv.value = wxMin( hsv.value * 1.86, 1.0 );
      wxImage::RGBValue to = wxImage::HSVtoRGB( hsv );
      colBack = RGB( to.red, to.green, to.blue );
   }

   if ( st & wxODSelected || ( st & wxODChecked && !(st & wxODDisabled) ) )
   {
      wxASSERT( !(st & wxODDisabled) );
      ColourHLS back( GetSysColor( COLOR_HIGHLIGHT ) );
      ColourHLS border( back );

      const ColourHLS from( wxColour( 49, 106, 197 ) );
      //wxASSERT( from.ToRGB() == wxColour( 49, 106, 197 ) );
      const ColourHLS to( wxColour( 193, 210, 238 ) );
      //wxASSERT( to.ToRGB() == wxColour( 193, 210, 238 ) );
      const float satScale = to.m_Saturation / from.m_Saturation;
      const float lumScale = to.m_Luminance / from.m_Luminance;
      back.m_Saturation = wxMin( back.m_Saturation * satScale, 1.0f );
      back.m_Luminance = wxMin( back.m_Luminance * lumScale, 0.98 );

      colSelBorder = border.ToDWORD();
      colSel = back.ToDWORD();
      colText = GetSysColor(COLOR_MENUTEXT);
   }
   else
   {
      if ( st & wxODDisabled )
         colText = GetSysColor(COLOR_GRAYTEXT);
      else
         colText = GetSysColor(COLOR_MENUTEXT);
   }
   DWORD colMargin = GetSysColor(COLOR_BTNFACE);

   HDC hdc = dc.GetHDC();
   COLORREF colOldText = ::SetTextColor(hdc, colText),
            colOldBack = ::SetBkColor(hdc, colBack);

   // *2, as in wxSYS_EDGE_Y
   int margin = GetMarginWidth() + ( 2 * wxSystemSettings::GetMetric(wxSYS_EDGE_X) ) + 4;

   // select the font and draw the text
   // ---------------------------------

   // determine where to draw and leave space for a check-mark.
   // + 1 pixel to separate the edge from the highlight rectangle
   int xMargin   = rc.x + margin;
   int xText     = xMargin + 8;

   // Draw the item background.
   {
      //AutoHBRUSH hMarginBrush( colMargin );
	   HBRUSH hMarginBrush = CreateSolidBrush(colMargin);
      //TODO ? SelectInHDC marBrush( hdc, hMarginBrush );
      RECT marginRect = { 
                           rc.GetLeft()-1, 
                           rc.GetTop()-1,
                           xMargin, 
                           rc.GetBottom() + 2 };

      FillRect( hdc, &marginRect, hMarginBrush );

      // Draw the background color.
     // AutoHBRUSH menuBrush( colBack );
	  HBRUSH menuBrush = CreateSolidBrush(colBack);
      //TODO ? SelectInHDC bakBrush2( hdc, menuBrush );
      RECT rectFill = { 
                        xMargin, 
                        rc.GetTop() - 1,
                        rc.GetRight() + 1,
                        rc.GetBottom() + 1 };

      FillRect( hdc, &rectFill, menuBrush );

      // Do we have a selection?
      if ( st & wxODSelected )
      {
         //AutoHBRUSH menuBrush( colSel );
		 HBRUSH menuBrush = CreateSolidBrush(colSel);
         //TODO ? SelectInHDC selBrush2( hdc, menuBrush );

         RECT rectSel = {
                        rc.GetLeft(), 
                        rc.GetTop(),
                        rc.GetRight(),
                        rc.GetBottom() };

         FillRect( hdc, &rectSel, menuBrush );

         // Render the border.
         //AutoHBRUSH borderBrush( colSelBorder );
		 HBRUSH borderBrush = CreateSolidBrush(colSelBorder);
         //TODO ? SelectInHDC selBrush3( hdc, borderBrush );
         FrameRect( hdc, &rectSel, borderBrush );
      }
   }


   // Draw the seperator or the text...
   if ( IsSeparator() )
   {
      wxPen sepPen( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
      dc.SetPen( sepPen );
      dc.DrawLine( xText, rc.GetTop(), rc.GetRight() + 1, rc.GetTop() );
      dc.SetPen( wxNullPen );
   }
   else
   {
      int nPrevMode = SetBkMode(hdc, TRANSPARENT);

      // Set the font.
	  wxFont fontToUse;// = GetFontToUse();
	  GetFontToUse(fontToUse);
      //TODO ? SelectInHDC selFont(hdc, GetHfontOf(fontToUse));

	  wxString strMenuText = GetItemLabelText();

      SIZE sizeRect;
      ::GetTextExtentPoint32(hdc, strMenuText.c_str(), strMenuText.Length(), &sizeRect);
      RECT textRect = {
                        xText,
                        rc.GetTop() + ( ( rc.GetHeight() / 2 ) - ( sizeRect.cy / 2 ) ) - 1,
                        xText + sizeRect.cx,
                        rc.GetBottom() };
      ::DrawText( hdc, strMenuText.c_str(), strMenuText.length(), &textRect, DT_LEFT | DT_TOP );

      // De have an accelerator to draw?
      if ( !GetItemLabel().AfterFirst('\t').empty() )
      {
		  wxString m_strAccel = GetItemLabel().AfterFirst('\t');
            int accel_width, accel_height;
            dc.GetTextExtent(m_strAccel, &accel_width, &accel_height);

            RECT accRect = {
                              rc.GetRight() - accel_width - 16,
                              rc.GetTop() + ( ( rc.GetHeight() / 2 ) - ( sizeRect.cy / 2 ) ) - 1,
                              rc.GetRight() - 16,
                              rc.GetBottom() };
            ::DrawText( hdc, m_strAccel.c_str(), m_strAccel.length(), &accRect, DT_RIGHT | DT_TOP );

            // right align accel string with right edge of menu ( offset by the
            // margin width )
            /*
            ::DrawState(hdc, NULL, NULL,
                  (LPARAM)m_strAccel.c_str(), m_strAccel.length(),
                  rc.GetWidth()-16-accel_width, rc.y+(int) ((rc.GetHeight()-sizeRect.cy)/2.0),
                  0, 0,
                  DST_TEXT |
                  (((st & wxODDisabled) && !(st & wxODSelected)) ? DSS_DISABLED : 0));
            */
      }

      (void)SetBkMode(hdc, nPrevMode);
   }

   // Draw the icon/bitmap
   if ( !IsSeparator() )
   {
      wxBitmap bmp;

      if ( st & wxODDisabled )
      {
         bmp = GetDisabledBitmap();
         if ( !bmp.Ok() /*&& !IsCheckable()*/ )
         {
            bmp = GetBitmap();
            if ( bmp.Ok() )
            {
               wxImage image = bmp.ConvertToImage();
               tsColorizeImage( image, wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );
               SetDisabledBitmap( wxBitmap( image ) );
               bmp = GetDisabledBitmap();
            }
         }
      }

      // Do we still not have a bitmap?
      if ( !bmp.Ok() )
         bmp = GetBitmap( true );

      // If we're checked then draw the checked active box.
      if ( st & wxODChecked && !(st & wxODDisabled))
      {
		  HBRUSH menuBrush = CreateSolidBrush(colSel);
         //TODO ? SelectInHDC selBrush2( hdc, menuBrush );

         RECT rectSel = {
                        rc.GetLeft()+1, 
                        rc.GetTop()+1,
                        xMargin-2,
                        rc.GetBottom()-1 };

         FillRect( hdc, &rectSel, menuBrush );

         // Render the border.
         //AutoHBRUSH borderBrush( colSelBorder );
		 HBRUSH borderBrush = CreateSolidBrush(colSelBorder);
         //TODO ? SelectInHDC selBrush3( hdc, borderBrush );
         FrameRect( hdc, &rectSel, borderBrush );
      }

      if ( bmp.Ok() )
      {
         // center bitmap
         int nBmpWidth = bmp.GetWidth(),
               nBmpHeight = bmp.GetHeight();

         // there should be enough space!
         wxASSERT((nBmpWidth <= rc.GetWidth()) && (nBmpHeight <= rc.GetHeight()));

         int heightDiff = m_nHeight - nBmpHeight;

         dc.DrawBitmap( bmp, 
                        rc.x + (margin - nBmpWidth) / 2,
                        rc.y + heightDiff / 2,
                        true );
      }

      else if ( st & wxODChecked )
      {
         // We have to draw the check to an offscreen
         // bitmap then blt it to the menu.
         HDC hdcMem = CreateCompatibleDC( hdc );
         const int cxCheck = GetSystemMetrics(SM_CXMENUCHECK);
         const int cyCheck = GetSystemMetrics(SM_CYMENUCHECK);
         HBITMAP hbmMono = CreateBitmap(cxCheck, cyCheck, 1, 1, NULL);
         HBITMAP hbmPrev = (HBITMAP)SelectObject(hdcMem, hbmMono);
         RECT rect = { 0, 0, cxCheck, cyCheck };
         DrawFrameControl(hdcMem, &rect, DFC_MENU, DFCS_MENUCHECK);
         COLORREF clrTextPrev = SetTextColor( hdc, colText );
         COLORREF clrBkPrev = SetBkColor( hdc, /*st & wxODSelected ?*/ colSel /*: colMargin*/ );

         BitBlt( hdc, rc.x + (margin - cxCheck) / 2, 
               rc.GetTop() + ( ( rc.GetHeight() / 2 ) - ( cyCheck / 2 )), cxCheck, cyCheck, hdcMem, 0, 0, SRCCOPY );

         SetBkColor( hdc, clrBkPrev );
         SetTextColor( hdc, clrTextPrev );
         SelectObject( hdcMem, hbmPrev );
         DeleteObject(hbmMono);
         DeleteDC(hdcMem);
      }

      // Do we need an submenu arrow?
      if ( IsSubMenu() )
      {
         // We have to draw the arrow to an offscreen
         // bitmap then blt it to the menu.
         HDC hdcMem = CreateCompatibleDC( hdc );
         const int cxCheck = GetSystemMetrics(SM_CXMENUCHECK);
         const int cyCheck = GetSystemMetrics(SM_CYMENUCHECK);
         HBITMAP hbmMono = CreateBitmap(cxCheck, cyCheck, 1, 1, NULL);
         HBITMAP hbmPrev = (HBITMAP)SelectObject(hdcMem, hbmMono);
         RECT rect = { 0, 0, cxCheck, cyCheck };
         DrawFrameControl(hdcMem, &rect, DFC_MENU, DFCS_MENUARROW);
         COLORREF clrTextPrev = SetTextColor( hdc, colText );
         COLORREF clrBkPrev = SetBkColor( hdc, st & wxODSelected ? colSel : colBack );

         BitBlt( hdc,   rc.GetRight() - cxCheck, 
                        rc.GetTop() + ( ( rc.GetHeight() / 2 ) - ( cyCheck / 2 )), cxCheck-1, cyCheck, hdcMem, 0, 0, SRCCOPY );

         SetBkColor( hdc, clrBkPrev );
         SetTextColor( hdc, clrTextPrev );
         SelectObject( hdcMem, hbmPrev );
         DeleteObject(hbmMono);
         DeleteDC(hdcMem);
      }
   }

   ::SetTextColor(hdc, colOldText);
   ::SetBkColor(hdc, colOldBack);

   // Trick... this keeps the default submenu arrow
   // from being drawn by the OS...
   ::ExcludeClipRect( hdc, rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom() );

   return true;
}

