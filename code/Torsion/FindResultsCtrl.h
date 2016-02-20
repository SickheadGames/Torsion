// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_FINDRESULTSCTRL_H
#define TORSION_FINDRESULTSCTRL_H
#pragma once

#include <wx/stc/stc.h>

class FindThread;


class FindResultsCtrl : public wxStyledTextCtrl
{
   public:

      FindResultsCtrl( wxWindow* parent, wxWindowID id, FindThread* findThread );
      virtual ~FindResultsCtrl();

   public:

      void AppendText( const wxString& text );
      void Clear();
      //void ShowPosition( int pos );

      //long GetFirstVisibleLine() const;
      void ShowLine( int line );

      void OnSetCursor( wxSetCursorEvent& event );

      void OnDblClick( wxMouseEvent& event );
      void OnContextMenu( wxContextMenuEvent& event );

      void OnGotoFile( wxCommandEvent& event );
      void OnUpdateGotoFile( wxUpdateUIEvent& event );
      void OnClearAll( wxCommandEvent& event );
      void OnUpdateClearAll( wxUpdateUIEvent& event );
      void OnCopyLines( wxCommandEvent& event );
      void OnUpdateCopyLines( wxUpdateUIEvent& event );
      void OnStopFind( wxCommandEvent& event );
      void OnUpdateStopFind( wxUpdateUIEvent& event );

      void UpdatePrefs();

      void ActivateLine( int line, bool openFile );

   protected:


      //virtual WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const;
      FindThread* m_FindThread;
      int m_Selected;

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_FINDRESULTSCTRL_H