// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCRIPTFRAME_H
#define TORSION_SCRIPTFRAME_H
#pragma once

#include <wx/docmdi.h>

class ScriptCtrl;
class shNotebook;
class shNotebookEvent;
class ScriptDoc;
class ScriptView;

WX_DECLARE_LIST(ScriptView, ScriptViewList);


class ScriptFrame : public wxDocMDIChildFrame
{
   DECLARE_CLASS(ScriptFrame)

      ScriptFrame();
      virtual ~ScriptFrame();

   public:

      virtual bool Destroy();

      void SetActive( wxView* view );
      void SetActive( int index );

      virtual bool ProcessEvent(wxEvent& event);

      wxWindow* GetViewControlParent() const;

      void AddScriptPage( ScriptView* scriptView, wxWindow* ctrl );
      void RemoveScriptPage( wxView* view );

      void OnScriptPageClose( shNotebookEvent& event );
      void OnScriptPageChanged( shNotebookEvent& event );
      void OnScriptPageChanging( shNotebookEvent& event );
      void OnContextMenu( wxContextMenuEvent& event );
      void OnActivate( wxActivateEvent& event );
   
      int GetViewIndex( ScriptView* view ) const;

      int GetPageCount() const;

      static ScriptFrame* GetFrame( bool create = false );

      ScriptView* GetSelectedView();

      ScriptView* GetScriptView( int page );
      ScriptDoc* GetScriptDoc( int page );

      void SetTabState( ScriptView* view, bool modified, bool readonly );

      void NextView();
      void PreviousView();

      bool  m_Tabbing;

   protected:

      static ScriptFrame* s_ScriptFrame;

      wxFileHistory     m_WindowList;

      ScriptViewList    m_PageViews;
      wxArrayInt        m_PageOrder;
      shNotebook*       m_ScriptNotebook;

      DECLARE_EVENT_TABLE()
};

#endif // TORSION_SCRIPTFRAME_H