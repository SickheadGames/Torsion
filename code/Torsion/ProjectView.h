// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PROJECTVIEW_H
#define TORSION_PROJECTVIEW_H
#pragma once

#include <wx/docview.h>

class ProjectCtrl;
class CodeBrowserPanel;
class ProjectPanel;
class Bookmark;


class ProjectView : public wxView
{
   DECLARE_DYNAMIC_CLASS(ProjectView)

   protected:

      ProjectPanel*        m_ProjectPanel;
      ProjectCtrl*         m_ProjectCtrl;
      CodeBrowserPanel*    m_BrowserPanel;

      bool                 m_ShowAllMods;
      bool                 m_ShowAllFiles;
      bool                 m_SyncTree;

      Bookmark*            m_LastBookmark;

      void DoBookmarkNav( bool prev, int fromIndex = wxNOT_FOUND );

   public:

      ProjectView();
      virtual ~ProjectView();

      wxString GetSelectedPath() const;

      virtual bool OnCreate( wxDocument *doc, long flags );
      virtual void OnDraw( wxDC* dc );
      virtual void OnUpdate( wxView *sender, wxObject* hint = NULL );
      virtual bool OnClose( bool deleteWindow = true );
      virtual void OnClosingDoocument();

      void OnOpenSel( wxCommandEvent& event );
      void OnOpenWith( wxCommandEvent& event );
      void OnExplore( wxCommandEvent& event );
      void OnProperties( wxCommandEvent& event );

      void OnDelete( wxCommandEvent& event );
      void OnUpdateDelete( wxUpdateUIEvent& event );

      void OnRename( wxCommandEvent& event );
      void OnUpdateRename( wxUpdateUIEvent& event );

      void OnRefresh( wxCommandEvent& event );
      void OnNewFolder( wxCommandEvent& event );
      void OnNewFile( wxCommandEvent& event );

      void OnShowAllMods( wxCommandEvent& event );
      void OnUpdateShowAllMods( wxUpdateUIEvent& event );

      void OnShowAllFiles( wxCommandEvent& event );
      void OnUpdateShowAllFiles( wxUpdateUIEvent& event );

      void OnSyncTree( wxCommandEvent& event );
      void OnUpdateSyncTree( wxUpdateUIEvent& event );

      void OnUpdateDisable( wxUpdateUIEvent& event );
      void OnUpdateSelected( wxUpdateUIEvent& event );
      void OnUpdateMultSelected( wxUpdateUIEvent& event );
      
      void OnUpdateGotActiveScriptView( wxUpdateUIEvent& event );

      void Select( const wxString& path );

      void OnBookmarkToggle( wxCommandEvent& event );
      void OnBookmarkClearAll( wxCommandEvent& event );
      void OnUpdateBookmarkClearAll( wxUpdateUIEvent& event );
      void OnBookmarkNext( wxCommandEvent& event );
      void OnBookmarkPrev( wxCommandEvent& event );

   protected:

      DECLARE_EVENT_TABLE()
};

#endif // TORSION_PROJECTVIEW_H