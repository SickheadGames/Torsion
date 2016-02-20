// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_MAINFRAME_H
#define TORSION_MAINFRAME_H
#pragma once

#include <wx/docmdi.h>
#include <wx/fdrepdlg.h>
#include <wx/help.h>

#include "Identifiers.h"
#include "DocManager.h"


class wxDocument;
class wxView;
class shNotebook;
class wxSashLayoutWindow;
class wxSashEvent;
class wxTextCtrl;
class wxListView;
class wxListEvent;
class ScriptFrame;
class ScriptView;
class ScriptDoc;
class ProjectDoc;
class ProjectView;
class WatchCtrl;
class FunctionCallArray;
class FindResultsCtrl;
class FindThread;
class FindReplaceDlg;
class BreakpointsPanel;
class OutputPanel;
class PreCompiler;
class ScriptDocArray;
class tsMenu;


class MainFrame : public wxDocMDIParentFrame
{
   DECLARE_CLASS(MainFrame)

   friend class ClientWindow;

   public:

      MainFrame();
      virtual ~MainFrame();

      bool Create( DocManager* manager, wxFrame* frame, const wxString& title, const wxPoint& pos, const wxSize& size );

      virtual bool Destroy();

      DocManager *GetDocumentManager() const { return m_DocManager; }

      ScriptView* OpenFile( const wxString& FullPath, int ZeroBasedLine = -1 );
      ScriptDoc* GetOpenDoc( const wxString& FullPath );
      ScriptView* GetOpenView( const wxString& FullPath );
      ScriptView* GetActiveView() const;

      int GetChangedScripts( ScriptDocArray& scripts );

      void OnNewProject( wxCommandEvent& event );
      void OnOpenProject( wxCommandEvent& event );
      void OnProjectClose( wxCommandEvent& event );
      void OnUpdateProjectClose( wxUpdateUIEvent& event );
      void OnNewProjectUpdateUI( wxUpdateUIEvent& event );
      void OnProjectRefresh( wxCommandEvent& event );
      void OnMRUProject( wxCommandEvent& event );

      void OnWindowList( wxCommandEvent& event );

      bool OpenProject( const wxString& path );

      void OnFileClose( wxCommandEvent& event );
      void OnUpdateFileClose( wxUpdateUIEvent& event );
      void OnFileCloseAll( wxCommandEvent& event );
      void OnUpdateFileCloseAll( wxUpdateUIEvent& event );      

      void OnFileCloseOthers( wxCommandEvent& event );
      void OnUpdateFileCloseOthers( wxUpdateUIEvent& event );

      void OnSaveAll( wxCommandEvent& event );

      void OnAbout( wxCommandEvent& event );
   	void OnSize( wxSizeEvent& event );
	   void OnSashDrag( wxSashEvent& event );

      void OnHelp( wxCommandEvent& event );
      void OnHelp( wxHelpEvent& event );

      void OnUpdateCheck( wxCommandEvent& event );
      void OnBugTracker( wxCommandEvent& event );

      void OnDropFiles( wxDropFilesEvent& event );

      void OnCloseWindow( wxCloseEvent& event );

      void OnNextView( wxCommandEvent& event );
      void OnPreviousView( wxCommandEvent& event );
      void OnUpdateNextView( wxUpdateUIEvent& event );

      void FindDialogShow( wxCommandEvent& event );
      void FindDialogClose( wxFindDialogEvent& event );

      void OnUpdateScriptFrame( wxUpdateUIEvent& event );
      void OnUpdateScriptOrProject( wxUpdateUIEvent& event );
      void OnUpdateDisabled( wxUpdateUIEvent& event ); 

      void OnFindInFiles( wxCommandEvent& event );
      void OnFindOutput( wxCommandEvent& event );

      void OnFindSymbol( wxCommandEvent& event );
      void OnUpdateFindSymbol( wxUpdateUIEvent& event ); 

      void OnFindComboChanged( wxCommandEvent& event );
      wxFindReplaceData&   GetFindData() { return m_FindData; }

      void StartDebug( const wxString& command, bool doPrecompile );
      void StartRun( bool doPrecompile );

      void OnDebugStart( wxCommandEvent& event );
      void OnDebugStartWithoutDebugging( wxCommandEvent& event );
	   void OnDebugConnect( wxCommandEvent& event );
	   void OnDebugStop( wxCommandEvent& event );
	   void OnDebugRestart( wxCommandEvent& event );
	   void OnDebugBreak( wxCommandEvent& event );
      void OnDebugStartBreak( wxCommandEvent& event );
	   void OnDebugStep( wxCommandEvent& event );
	   void OnDebugStepOver( wxCommandEvent& event );
	   void OnDebugStepOut( wxCommandEvent& event );
	   void OnDebugUpdateUI( wxUpdateUIEvent& event );
      void UpdateDebugMenu();

      void OnReloadScripts( wxCommandEvent& event );
	   void OnUpdateReloadScripts( wxUpdateUIEvent& event );

      void OnNewBreakpoint( wxCommandEvent& event );
      void OnClearAllBreakpoints( wxCommandEvent& event );
      void OnDisableAllBreakpoints( wxCommandEvent& event );
      void OnBreakpointUpdateUI( wxUpdateUIEvent& event );

      void OnUpdateBookmarks( wxUpdateUIEvent& event );
      
      void OnCallStackActivated( wxListEvent& event );

      shNotebook* ShowProjectSash( bool show );

      void OnProjectProperties( wxCommandEvent& event );
      void OnProjectPrecompile( wxCommandEvent& event );
      void UpdatePrecompileMenu();

      void OnProjectRebuildExports( wxCommandEvent& event );
      void OnUpdateRebuildExports( wxUpdateUIEvent& event );

      void OnExecTool( wxCommandEvent& event );
      void OnUpdateTools( wxUpdateUIEvent& event );
      void OnExternalTools( wxCommandEvent& event );
      void OnPreferences( wxCommandEvent& event );
      void UpdateToolsMenu();

      void OnPrecompileDone( wxCommandEvent& event );
      void OnPrecompileOutput( wxCommandEvent& event );
      void OnPrecompileStop( wxCommandEvent& event );
      void OnUpdatePrecompile( wxUpdateUIEvent& event );

      void DoPrecompile( int eventId );

      void OnClearDSOs( wxCommandEvent& event );
      void OnClearDSO( wxCommandEvent& event );
      void OnUpdateHasDSO( wxUpdateUIEvent& event );

      void OnShowLeftPane( wxCommandEvent& event );
      void OnUpdateShowLeftPane( wxUpdateUIEvent& event );
      void OnShowBottomPane( wxCommandEvent& event );
      void OnUpdateShowBottomPane( wxUpdateUIEvent& event );

      wxMenu* GetEditMenu() { return (wxMenu*)m_EditMenu; }
      
      ProjectDoc* GetProjectDoc() { return m_ProjectDoc; }
      ProjectView* GetProjectView();

      void OnDebugCallTip( const wxString& Expression, const wxString& Value );

      void SetCallStack( const FunctionCallArray& CallStack, int Level );
	   void SetBreakline( const wxString& Filename, int Line );
	   void ClearBreaklines();
	   void OnDebuggerStop();

      OutputPanel* GetOutputPanel() { return m_OutputPanel; }
      WatchCtrl* GetWatchWindow() { return m_WatchWindow; }
      FindResultsCtrl* GetFindWindow() { return m_FindWindow; }
      BreakpointsPanel* GetBreakpointsPanel() { return m_Breakpoints; }

      void OnSysColourChanged( wxSysColourChangedEvent& event );

      virtual void DoMenuUpdates(wxMenu* menu);
      virtual bool ProcessEvent( wxEvent& event );
      virtual bool MSWTranslateMessage( WXMSG* msg );
      virtual bool MSWProcessMessage( WXMSG* pMsg );

      virtual wxMDIClientWindow* OnCreateClient();

      void UpdateTitle();

      void AddFindString( wxString& text );

      bool        SetActiveConfig( const wxString& config );
      wxString    GetActiveConfigName() const;

      void SendHintToAllViews( wxObject* hint, bool scriptViewsOnly );

      tsMenu* GetWindowMenu() const { return m_WindowMenu; }

   protected:

      wxString GetSelectedText( bool AtCursor = false );
      
      wxHelpController     m_HelpController;

      wxComboBox*          m_FindComboBox;
         
      wxToolBarToolBase*   m_StartToolButton;

      ProjectDoc*          m_ProjectDoc;
      wxComboBox*          m_ConfigComboBox;

      wxFindReplaceData    m_FindData;
      FindReplaceDlg*      m_FindReplaceDialog;
      FindThread*          m_FindThread;

      wxSashLayoutWindow*  m_ProjectSash;
      shNotebook*          m_ProjectNotebook;

   	wxSashLayoutWindow*	m_BottomSash;
	   shNotebook*			   m_BottomNotebook;
	   OutputPanel*			m_OutputPanel;
      FindResultsCtrl*     m_FindWindow;
	   wxListView*			   m_CallStack;
	   BreakpointsPanel*		m_Breakpoints;
	   //WatchCtrl*			   m_LocalsWindow;
	   WatchCtrl*			   m_WatchWindow;

      tsMenu*              m_EditMenu;
      tsMenu*              m_DebugMenu;
      tsMenu*              m_ProjectMenu;
      tsMenu*              m_WindowMenu;
      tsMenu*              m_ToolsMenu;

      DocManager*          m_DocManager;

      PreCompiler*         m_PreCompiler;
      bool                 m_PreCompilerStopping;
      wxString             m_DebugCommand;
      enum 
      { 
         START_NOTHING, 
         START_DEBUG, 
         START_RUN

      } m_AfterPrecompile;

      DECLARE_EVENT_TABLE()
};

extern MainFrame* tsGetMainFrame();

#endif // TORSION_MAINFRAME_H
