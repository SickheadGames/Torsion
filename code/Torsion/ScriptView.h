// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCRIPTVIEW_H
#define TORSION_SCRIPTVIEW_H
#pragma once

#include "ScriptFrame.h"
#include "FileWatcher.h"
#include "tsComboBox.h"

#include <wx/docview.h>

class ScriptCtrl;
class wxFindDialogEvent;
class BreakpointEvent;
class tsComboBox;


class ScriptView : public wxView
{
   DECLARE_DYNAMIC_CLASS(ScriptView)

   protected:

      static ScriptViewList s_ScriptViews;

      ScriptCtrl*    m_ScriptCtrl;

      FileWatcher    m_FileWatcher;

      tsComboBox*    m_Types;
      tsComboBox*    m_Members;

      wxArrayString  m_Functions;
      wxArrayInt     m_Lines;
      wxArrayInt     m_EndLines;

      int            m_LastLine;

      bool m_AutoCompUpdated;
      wxTimer m_UpdateCombosTimer;
      void OnUpdateCombos( wxTimerEvent& event );

      void UpdateTypeList();

   public:

      static const ScriptViewList& GetList() { return s_ScriptViews; }
         
      ScriptView();
      virtual ~ScriptView();

      virtual bool ProcessEvent( wxEvent& event );

      bool OnSaveFile( const wxString& filename );
      bool OnLoadFile( const wxString& filename );

      void SetCtrlFocused();

      ScriptCtrl* GetCtrl() { return m_ScriptCtrl; }

      ScriptFrame* GetFrame() { return wxDynamicCast( m_viewFrame, ScriptFrame ); }

      void ClearBreakline();
      void SetBreakline( int Line );
      bool SetLineSelected( int Line );

      virtual bool OnCreate( wxDocument *doc, long flags );
      virtual void OnDraw( wxDC* dc );
      virtual void OnUpdate( wxView *sender, wxObject* hint = NULL );
      virtual bool OnClose( bool deleteWindow = true );
      virtual wxPrintout* OnCreatePrintout();

      int ReloadFile( int lastResult, bool force );
      
      virtual void OnChangeFilename();

      void OnDebugCallTip( const wxString& Expression, const wxString& Value );

      void OnAutoCompUpdated( wxCommandEvent& event );

      virtual void OnActivateView( bool activate, wxView *activeView, wxView *deactiveView );

      void OnUpdateMembers( wxCommandEvent& event );
      void OnSelectMember( wxCommandEvent& event );

      void OnExplore( wxCommandEvent& event );
      void OnOpenWith( wxCommandEvent& event );
      void OnFindNext( wxCommandEvent& event );
      void OnCtrlModifyed();

      void OnUpdateEnable(wxUpdateUIEvent& event);
      void OnUpdateHasSelection(wxUpdateUIEvent& event);

      void OnMoveInstPtr( wxCommandEvent& event );
      void OnUpdateMoveInstPtr( wxUpdateUIEvent& event );

      void OnReloadScript( wxCommandEvent& event );
      void OnUpdateReloadScript( wxUpdateUIEvent& event );

      void OnCopyFullPath( wxCommandEvent& event );
      void OnUpdateEnabled( wxUpdateUIEvent& event );

      void OnUpdateFileExists( wxUpdateUIEvent& event );
      
      void OnLineBreaks( wxCommandEvent& event );
      void OnUpdateLineBreaks( wxUpdateUIEvent& event );
      void OnTabsAndSpaces( wxCommandEvent& event );
      void OnUpdateTabsAndSpaces( wxUpdateUIEvent& event );
      void OnLineNumbers( wxCommandEvent& event );
      void OnUpdateLineNumbers( wxUpdateUIEvent& event );
      void OnLineWrap( wxCommandEvent& event );
      void OnUpdateLineWrap( wxUpdateUIEvent& event );

      void OnUpperSel(wxCommandEvent& event);
      void OnLowerSel(wxCommandEvent& event);
      void OnCommentSel(wxCommandEvent& event);
      void OnUncommentSel(wxCommandEvent& event);
      void OnIndentSel(wxCommandEvent& event);
      void OnUnindentSel(wxCommandEvent& event);

      void OnCut(wxCommandEvent& event);
      void OnCopy(wxCommandEvent& event);
      void OnPaste(wxCommandEvent& event);
      void OnUndo(wxCommandEvent& event);
      void OnRedo(wxCommandEvent& event);
      void OnDelete(wxCommandEvent& event);
      void OnSelectAll(wxCommandEvent& event);

      void OnUpdateUndo(wxUpdateUIEvent& event);
      void OnUpdateRedo(wxUpdateUIEvent& event);
      void OnUpdateSelectAll(wxUpdateUIEvent& event);

      void OnBreakpointEvent( BreakpointEvent& event );

      void OnMatchBrace(wxCommandEvent& event);

      int GetSelectionLine() const;
      wxString GetSelectedText( bool AtCursor = false ) const; 
      void ClearSelectedText();

   protected:

      DECLARE_EVENT_TABLE()
};

/// This hint is sent when a file has been renamed in the project tree.
class tsFileRenameHint : public wxObject 
{
   DECLARE_CLASS(tsFileRenameHint)

public:
   wxString oldPath;
   wxString newPath;
};

#endif // TORSION_SCRIPTVIEW_H