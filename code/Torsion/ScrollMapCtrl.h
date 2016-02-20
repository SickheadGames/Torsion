// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCROLLMAPCTRL_H
#define TORSION_SCROLLMAPCTRL_H
#pragma once

#include <wx/control.h>

#include <vector>


class ScrollMapCtrl : public wxControl
{
   public:

      ScrollMapCtrl( wxWindow* parent, wxWindowID id, 
                     const wxPoint& pos, const wxSize& size, long style );
      virtual ~ScrollMapCtrl();

   public:

      void SetRange( int range );
      void AddItem( int line, const wxColor& color, const wxString& tip );
      void Clear();

      void SetScrollWindow( wxWindow* win ) { m_ScrollWin = win; }

   protected:

      struct Item
      {
         int line;
         wxColor color;
         wxString tip;
      };

      typedef std::vector<Item*> ItemArray;

      void CalcPad( int& top, int& bottom ) const; 

      void OnClick( wxMouseEvent& event );
      void OnRelease( wxMouseEvent& event );
      void OnLeave( wxMouseEvent& event );
      void OnDrag( wxMouseEvent& event );
      void OnScroll( wxMouseEvent& event );
      void OnLoseCapture( wxMouseEvent& event );

      void OnErase( wxEraseEvent& event );
      void OnPaint( wxPaintEvent& event );

      int   m_TopPad, m_BottomPad;
      wxWindow* m_ScrollWin;

      bool        m_HasDrawn;
      int         m_Range;
      ItemArray   m_Items;

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_SCROLLMAPCTRL_H