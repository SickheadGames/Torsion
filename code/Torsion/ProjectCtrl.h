// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PROJECTCTRL_H
#define TORSION_PROJECTCTRL_H
#pragma once

#include "DirWatcher.h"

#include <wx/treectrl.h>
#include <wx/docmdi.h>


class ProjectDoc;
class wxDragImage;
class FileTypeImageList;
class ShellMenu;
class ScriptView;


WX_DECLARE_OBJARRAY( wxTreeItemId, TreeItemIdArray );
 

class ProjectCtrl : public wxTreeCtrl
{
   friend class ProjectDirTraverser;

   public:

      ProjectCtrl( wxWindow* parent );
      virtual ~ProjectCtrl();

   public:

      void Refresh( ProjectDoc* project,  bool showAllMods, bool showAllFiles, bool clearFirst );

	   void OnOpenItem( wxTreeEvent& Event );
      void OnItemMenu( wxTreeEvent& Event );
      void OnItemExpanding( wxTreeEvent& Event );
      void OnItemExpanded( wxTreeEvent& Event );
      void OnItemCollapsing( wxTreeEvent& Event );
      void OnItemCollapsed( wxTreeEvent& Event );

      void OnBeginRename( wxTreeEvent& Event );
      void OnEndRename( wxTreeEvent& Event );
  
      wxTreeItemId Select( const wxString& path );

      void OpenItem( const wxTreeItemId& item );
      void OpenItemWith( const wxTreeItemId& item );

      void OnSetFocus( wxFocusEvent& event );
      void OnKillFocus( wxFocusEvent& event );

      void OnToolTip( wxTreeEvent& Event );

      wxString GetItemPath( const wxTreeItemId& item ) const;

      size_t   GetSelectedCount();
      void     UnnestSelections();
      wxString GetSelectedPath() const;

      void OnInternalIdle();

      virtual int OnCompareItems( const wxTreeItemId& item1, const wxTreeItemId& item2 );

      void OnDragBegin( wxTreeEvent& Event );
      void OnDragMove( wxMouseEvent& event );
      void OnDragExpandTimer( wxTimerEvent& event );
      void OnDragKey( wxKeyEvent& event );
      void OnDragEnd( wxMouseEvent& event );

      ProjectDoc* GetProjectDoc() const { return m_ProjectDoc; }

      int GetFileIcon( const wxString& file );

      void NewFolder( wxTreeItemId item );
      void NewFile( wxTreeItemId item );

      void SelectItems( const wxArrayTreeItemIds& items );

   protected:

      wxTreeItemId FindIdFromPath( const wxString& path, bool expand );
      void CheckWatcher();
      bool RefreshFolder( wxTreeItemId item );
      void FixupFolders( wxTreeItemId item );

      #ifdef __WXMSW__
         ShellMenu*  m_OsMenu;
         WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
      #endif

      // To fix flicker!
      bool m_DonePaint;
      #ifdef __WXMSW__
         void OnEraseBackground( wxEraseEvent& event );
         void OnPaint( wxPaintEvent& event );
      #endif // __WXMSW__

      FileTypeImageList* m_ImageList;

      ProjectDoc*       m_ProjectDoc;
      DirWatcher        m_DirWatcher;
      bool              m_ShowAllMods;
      bool              m_ShowAllFiles;

      wxTreeItemId         m_DragItem;
      wxTreeItemId         m_SelStart;
      wxPoint              m_DragPt;
      bool                 m_IsDragging;
      wxTimer              m_DragExpandTimer;
      wxCursor             m_DragCursor;
      wxArrayTreeItemIds   m_DragItems;
      wxTreeItemId         m_LastHighlight;

      ScriptView *m_FocusView;

      DECLARE_CLASS( ProjectCtrl )
      DECLARE_EVENT_TABLE()
};

#endif // TORSION_PROJECTCTRL_H