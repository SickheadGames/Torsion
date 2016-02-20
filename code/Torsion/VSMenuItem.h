// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_VSMENUITEM_H
#define TORSION_VSMENUITEM_H
#pragma once


class VSMenuItem : public wxMenuItem
{
   public:
      VSMenuItem( wxMenu* parentMenu = NULL, int id = wxID_SEPARATOR, const wxString& text = "", const wxString& helpString = "", wxItemKind kind = wxITEM_NORMAL, wxMenu* subMenu = NULL );
      virtual ~VSMenuItem();

   protected:

      virtual bool OnMeasureItem(size_t *pwidth, size_t *pheight);
      virtual bool OnDrawItem(wxDC& dc, const wxRect& rc, wxODAction act, wxODStatus stat);

      size_t   m_nHeight,
               m_nMinHeight;
};

#endif // TORSION_VSMENUITEM_H