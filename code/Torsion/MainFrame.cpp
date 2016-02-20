// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#if !wxUSE_MDI_ARCHITECTURE
#error You must set wxUSE_MDI_ARCHITECTURE to 1 in setup.h!
#endif

#include "TorsionApp.h"
#include "AppPrefs.h"
#include "MainFrame.h"
#include "ClientWindow.h"
#include "ScriptDoc.h"
#include "ScriptView.h"
#include "ScriptFrame.h"
#include "ProjectDoc.h"
#include "ProjectView.h"
#include "Debugger.h"
#include "WatchCtrl.h"
#include "ProjectDlg.h"
#include "AboutDlg.h"
#include "FindInFilesDlg.h"
#include "FindThread.h"
#include "FindResultsCtrl.h"
#include "OutputPanel.h"
#include "CodeBrowserCtrl.h"
#include "shNotebook.h"
#include "BreakpointsPanel.h"
#include "BreakpointPropertyDlg.h"
#include "AutoComp.h"
#include "Platform.h"
#include "PreCompiler.h"
#include "SettingsDlg.h"
#include "tsMenu.h"
#include "FindReplaceDlg.h"
#include "tsToolBar.h"
#include "tsMenuBar.h"
#include "tsStatusBar.h"
#include "tsSashLayoutWindow.h"
#include "ExternalToolsDlg.h"
#include "FindSymbolDlg.h"
#include "GotoDefDialog.h"
#include "tsShellExecute.h"

#ifndef __WXMSW__
   #include "icons\torsion16.xpm"
   #include "icons\torsion32.xpm"
#endif

#include "Icons.h"

#include <wx/laywin.h>
//#include <wx/toolbar.h>
#include <wx/sysopt.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_CLASS( MainFrame, wxDocMDIParentFrame )

BEGIN_EVENT_TABLE( MainFrame, wxDocMDIParentFrame )

   EVT_MENU( tsID_NEWPROJECT, OnNewProject )
   EVT_MENU( tsID_OPENPROJECT, OnOpenProject )
   EVT_MENU( tsID_CLOSEPROJECT, OnProjectClose )
   EVT_UPDATE_UI( tsID_CLOSEPROJECT, OnUpdateProjectClose )

   EVT_HELP( wxID_ANY, OnHelp )
   EVT_MENU( wxID_HELP, OnHelp )
   EVT_MENU( tsID_UPDATECHECK, OnUpdateCheck )
   
   EVT_MENU( tsID_FEATUREREQUEST, OnBugTracker )
   EVT_MENU( tsID_BUGTRACKER, OnBugTracker )

   EVT_UPDATE_UI( tsID_PROJECT_OPENWITH, OnUpdateDisabled )
   EVT_UPDATE_UI( tsID_EXPLORE, OnUpdateDisabled )

   EVT_MENU( tsID_SHOW_LEFTPANE, OnShowLeftPane )
   EVT_UPDATE_UI( tsID_SHOW_LEFTPANE, OnUpdateShowLeftPane )
   
   EVT_MENU( tsID_SHOW_BOTTOMPANE, OnShowBottomPane )
   EVT_UPDATE_UI( tsID_SHOW_BOTTOMPANE, OnUpdateShowBottomPane )
   
   EVT_MENU( tsID_PROJECT_REFRESH, OnProjectRefresh )
   EVT_UPDATE_UI( tsID_PROJECT_REFRESH, OnUpdateProjectClose )

   EVT_MENU( tsID_PROJECT_REBUILDEXPORTS, OnProjectRebuildExports )
   EVT_UPDATE_UI( tsID_PROJECT_REBUILDEXPORTS, OnUpdateRebuildExports )

   
   EVT_MENU( tsID_PROJECT_PRECOMPILE, OnProjectPrecompile )
   EVT_MENU( tsID_PROJECT_PRECOMPILE_ACTIVE, OnProjectPrecompile )
   EVT_MENU( tsID_PROJECT_PRECOMPILEALL, OnProjectPrecompile )
   EVT_UPDATE_UI( tsID_PROJECT_PRECOMPILE, OnUpdatePrecompile )
   EVT_UPDATE_UI( tsID_PROJECT_PRECOMPILE_ACTIVE, OnUpdatePrecompile )
   EVT_UPDATE_UI( tsID_PROJECT_PRECOMPILEALL, OnUpdatePrecompile )

   EVT_MENU_RANGE( tsID_PROJECT1, tsID_PROJECT5, OnMRUProject )

   EVT_MENU( tsID_CLEARDSO, OnClearDSO )
   EVT_UPDATE_UI( tsID_CLEARDSO, OnUpdateHasDSO )
   EVT_MENU( tsID_CLEARDSOS, OnClearDSOs )
   EVT_UPDATE_UI( tsID_CLEARDSOS, OnUpdateProjectClose )

   EVT_MENU( wxID_CLOSE, OnFileClose )
   EVT_UPDATE_UI( wxID_CLOSE, OnUpdateFileClose )

   EVT_MENU( wxID_CLOSE_ALL, OnFileCloseAll )
   EVT_UPDATE_UI( wxID_CLOSE_ALL, OnUpdateFileCloseAll )

   EVT_MENU( tsID_CLOSE_OTHERS, OnFileCloseOthers )
   EVT_UPDATE_UI( tsID_CLOSE_OTHERS, OnUpdateFileCloseOthers )

   EVT_MENU( tsID_SAVEALL, OnSaveAll )
   EVT_UPDATE_UI( tsID_SAVEALL, OnUpdateScriptOrProject )

   EVT_MENU( wxID_ABOUT, OnAbout )

   EVT_MENU( wxID_FIND, FindDialogShow )
   EVT_MENU( wxID_REPLACE, FindDialogShow )
   EVT_FIND_CLOSE( wxID_ANY, FindDialogClose )
   EVT_UPDATE_UI(wxID_FIND, OnUpdateScriptFrame)

   // Make sure we only activate printing when a
   // script doc is active!
   EVT_UPDATE_UI(wxID_PRINT, OnUpdateScriptFrame)

   EVT_MENU( tsID_FINDSYMBOL, OnFindSymbol )
   EVT_UPDATE_UI( tsID_FINDSYMBOL, OnUpdateFindSymbol )

   EVT_MENU( tsID_FINDINFILES, MainFrame::OnFindInFiles )
   EVT_COMMAND(wxID_ANY, tsEVT_FIND_OUTPUT, MainFrame::OnFindOutput )
   EVT_COMMAND(wxID_ANY, tsEVT_FIND_STATUS, MainFrame::OnFindOutput )

   EVT_UPDATE_UI(tsID_GOTO, OnUpdateScriptFrame)
   EVT_UPDATE_UI(tsID_TABSANDSPACES, OnUpdateScriptFrame)
   EVT_UPDATE_UI(tsID_LINEBREAKS, OnUpdateScriptFrame)
   EVT_UPDATE_UI(tsID_LINENUMBERS, OnUpdateScriptFrame)
   EVT_UPDATE_UI(tsID_LINEWRAP, OnUpdateScriptFrame)
   EVT_UPDATE_UI(tsID_NEXTVIEW, OnUpdateNextView)
   EVT_UPDATE_UI(tsID_PREVIOUSVIEW, OnUpdateNextView)

   // HACK: I'm assuming these will stay contiguous.
   EVT_UPDATE_UI_RANGE(wxID_CUT, wxID_REPLACE_ALL, OnUpdateScriptFrame)

	EVT_UPDATE_UI_RANGE(tsID_FOLDING_FIRST, tsID_FOLDING_LAST, OnUpdateScriptFrame )
	EVT_UPDATE_UI_RANGE(tsID_ADVANCED_FIRST, tsID_ADVANCED_LAST, OnUpdateScriptFrame )
	EVT_UPDATE_UI_RANGE(tsID_SCRIPTSENSE_FIRST, tsID_SCRIPTSENSE_LAST, OnUpdateScriptFrame )

   EVT_SASH_DRAGGED_RANGE( tsID_PROJECTSASH, tsID_BOTTOMSASH, OnSashDrag )

	EVT_MENU( tsID_DEBUG_START, OnDebugStart )
	EVT_MENU( tsID_DEBUG_START_WITHOUT_DEBUGGING, OnDebugStartWithoutDebugging )
	EVT_MENU( tsID_DEBUG_CONNECT, OnDebugConnect )
	EVT_MENU( tsID_DEBUG_STOP, OnDebugStop )
	EVT_MENU( tsID_DEBUG_RESTART, OnDebugRestart )
	EVT_MENU( tsID_DEBUG_BREAK, OnDebugBreak )
	EVT_MENU( tsID_DEBUG_STARTBREAK, OnDebugStartBreak )
   EVT_MENU( tsID_DEBUG_STEP, OnDebugStep )
	EVT_MENU( tsID_DEBUG_STEP_OVER, OnDebugStepOver )
	EVT_MENU( tsID_DEBUG_STEP_OUT, OnDebugStepOut )
	EVT_UPDATE_UI_RANGE( tsID_DEBUG_START, tsID_DEBUG_RESTART, OnDebugUpdateUI )

	EVT_MENU( tsID_RELOAD_SCRIPTS, OnReloadScripts )
   EVT_UPDATE_UI( tsID_RELOAD_SCRIPTS, OnUpdateReloadScripts )

   EVT_MENU( tsID_DEBUG_NEWBREAKPOINT, OnNewBreakpoint ) 
   EVT_MENU( tsID_DEBUG_CLEARALLBREAKPOINTS, OnClearAllBreakpoints ) 
   EVT_MENU( tsID_DEBUG_DISABLEALLBREAKPOINTS, OnDisableAllBreakpoints )
	EVT_UPDATE_UI_RANGE( tsID_DEBUG_NEWBREAKPOINT, tsID_DEBUG_DISABLEALLBREAKPOINTS, OnBreakpointUpdateUI )

   EVT_MENU(tsID_NEXTVIEW, OnNextView)
   EVT_MENU(tsID_PREVIOUSVIEW, OnPreviousView)

   EVT_TEXT(tsID_FINDCOMBO, OnFindComboChanged)
   //EVT_UPDATE_UI(tsID_FINDCOMBO,OnUpdateScriptFrame)

   EVT_UPDATE_UI( tsID_CONFIGCOMBO, OnUpdateProjectClose )
   //EVT_COMBOBOX( tsID_CONFIGCOMBO, OnConfigChanged )

   EVT_MENU(tsID_PROJECT_PROPERTIES, OnProjectProperties)
   EVT_UPDATE_UI(tsID_PROJECT_PROPERTIES, OnUpdateProjectClose)

   EVT_MENU(tsID_TOOLS_PREFERENCES, OnPreferences)
   EVT_MENU_RANGE( tsID_TOOLS_FIRST, tsID_TOOLS_LAST, OnExecTool )
   EVT_UPDATE_UI_RANGE( tsID_TOOLS_FIRST, tsID_TOOLS_LAST, OnUpdateTools )
   EVT_MENU(tsID_TOOLS_EXTERNAL_TOOLS, OnExternalTools)

   EVT_SIZE( OnSize )
   EVT_DROP_FILES( OnDropFiles )
   EVT_CLOSE( OnCloseWindow )

   EVT_SYS_COLOUR_CHANGED( OnSysColourChanged ) 

   //EVT_LIST_ITEM_ACTIVATED( tsID_FINDWINDOW, OnFindActivated )
   EVT_LIST_ITEM_ACTIVATED( tsID_CALLSTACK, OnCallStackActivated )

   EVT_COMMAND(wxID_ANY, tsEVT_PRECOMPILER_OUTPUT, MainFrame::OnPrecompileOutput )
   EVT_COMMAND(wxID_ANY, tsEVT_PRECOMPILER_DONE, MainFrame::OnPrecompileDone )

	EVT_UPDATE_UI_RANGE( tsID_BOOKMARK_FIRST, tsID_BOOKMARK_LAST, MainFrame::OnUpdateBookmarks )

   //EVT_MENU_OPEN( MainFrame::OnMenuOpen )

   EVT_MENU_RANGE( tsID_WINDOW_FIRST, tsID_WINDOW_LAST, MainFrame::OnWindowList )


END_EVENT_TABLE()


MainFrame::MainFrame()
    : m_FindReplaceDialog( NULL ),
      m_ProjectSash( NULL ),
      m_ProjectNotebook( NULL ),
      m_EditMenu( NULL ),
      m_ProjectDoc( NULL ),
      m_FindComboBox( NULL ),
      m_PreCompiler( NULL ),
      m_PreCompilerStopping( false ),
      m_ProjectMenu( NULL ),
      m_DebugMenu( NULL ),
      m_ToolsMenu( NULL ),
      m_FindThread( NULL ),
      m_DocManager( NULL )
{
}

MainFrame::~MainFrame()
{
   wxASSERT( !m_ProjectDoc );
   wxASSERT( !m_PreCompiler );
}

bool MainFrame::Create( DocManager* manager, wxFrame* frame, const wxString& title, const wxPoint& pos, const wxSize& size )
{
   if ( !wxDocMDIParentFrame::Create( manager, frame, wxID_ANY, title, pos, size, 
      wxDEFAULT_FRAME_STYLE | wxFRAME_NO_WINDOW_MENU | wxCLIP_CHILDREN, _T( "TorsionMainFrame" ) ) )
   {
      return false;
   }

   // Store it so we don't have to cast it later.
   m_DocManager = manager;

   SetMinSize( wxSize( 160, 100 ) );

   DragAcceptFiles( true );

   wxIconBundle icons;
#ifdef __WXMSW__
   icons.AddIcon( wxIcon( "\"AAAAAAIDI_MAINFRAME\"", wxBITMAP_TYPE_ICO_RESOURCE ) );
#else
   icons.AddIcon( wxIcon( torsion16_xpm ) );
   icons.AddIcon( wxIcon( torsion32_xpm ) );
#endif
	SetIcons( icons );

   tsMenu *fileMenu = new tsMenu;
   {
      tsMenu* newMenu = new tsMenu;
      newMenu->AppendIconItem( tsID_NEWPROJECT,    _T( "&Project...\tCtrl+Shift+N" ), ts_new_project16 );
      newMenu->AppendIconItem( wxID_NEW,           _T( "&Script\tCtrl+N" ), ts_new_document16 );
      fileMenu->Append( wxID_ANY, _T( "&New" ), newMenu );

      tsMenu* openMenu = new tsMenu;
      openMenu->AppendIconItem( tsID_OPENPROJECT,  _T( "&Project...\tCtrl+Shift+O" ), ts_open_project16 );
      openMenu->AppendIconItem( wxID_OPEN,         _T( "&Script...\tCtrl+O" ), ts_open_document16 );
      fileMenu->Append( wxID_ANY, _T( "&Open" ), openMenu );

      fileMenu->Append( wxID_CLOSE,        _T( "&Close\tCtrl+F4" ) );
      fileMenu->Append( tsID_CLOSEPROJECT, _T( "Close P&roject" ) );
      fileMenu->AppendSeparator();
      fileMenu->AppendIconItem( wxID_SAVE,         _T( "&Save\tCtrl+S" ), ts_save16 );
      fileMenu->Append( wxID_SAVEAS,       _T( "Save &As...\tCtrl+Alt+S" ) );
      fileMenu->AppendIconItem( tsID_SAVEALL,      _T( "Save A&ll\tCtrl+Shift+S" ), ts_save_all16 );
      fileMenu->AppendSeparator();
      fileMenu->AppendIconItem( wxID_PRINT,        _T( "&Print...\tCtrl+P" ), ts_print16 );
      //fileMenu->Append( wxID_PRINT_SETUP,  _T( "Print Set&up..." ) );
      fileMenu->AppendSeparator();

      tsMenu *recent_menu = new tsMenu;
      manager->FileHistoryUseMenu( recent_menu );
      manager->FileHistoryAddFilesToMenu( recent_menu );
      fileMenu->Append( wxID_ANY,          _T( "Recent &Files" ), recent_menu );

      tsMenu *recent_projects_menu = new tsMenu;
      wxASSERT( manager->GetProjectHistory() );
      manager->GetProjectHistory()->UseMenu( recent_projects_menu );
      manager->GetProjectHistory()->AddFilesToMenu( recent_projects_menu );
      fileMenu->Append( wxID_ANY,          _T( "Recent Pro&jects" ), recent_projects_menu );
      fileMenu->AppendSeparator();
      fileMenu->Append( wxID_EXIT,         _T( "E&xit" ) );
   }

   m_EditMenu = new tsMenu;
   {
      m_EditMenu->AppendIconItem( wxID_UNDO,   _T( "&Undo\tCtrl+Z" ), ts_undo16 );
      m_EditMenu->AppendIconItem( wxID_REDO,   _T( "&Redo\tCtrl+Y" ), ts_redo16 );

      m_EditMenu->AppendSeparator();
      m_EditMenu->AppendIconItem( wxID_CUT,     _T( "Cu&t\tCtrl+X" ), ts_cut_clipboard16 );
      m_EditMenu->AppendIconItem( wxID_COPY,    _T( "&Copy\tCtrl+C" ), ts_copy_clipboard16 );
      m_EditMenu->AppendIconItem( wxID_PASTE,   _T( "&Paste\tCtrl+V" ), ts_paste_clipboard16 );
      m_EditMenu->AppendIconItem( wxID_DELETE,  _T( "&Delete\tDel" ), ts_delete16 );

      m_EditMenu->AppendSeparator();
      m_EditMenu->Append( wxID_SELECTALL,   _T( "Select &All\tCtrl+A" ) );
      m_EditMenu->AppendSeparator();
      tsMenu* findMenu = new tsMenu;
      findMenu->AppendIconItem( wxID_FIND,       _T( "&Find\tCtrl+F" ), ts_search16 );
      findMenu->AppendIconItem( wxID_REPLACE,    _T( "R&eplace\tCtrl+H" ), ts_findreplace16 );
      findMenu->AppendIconItem( tsID_FINDINFILES, _T( "F&ind in Files\tCtrl+Shift+F" ), ts_findinfiles16 );
      findMenu->AppendIconItem( tsID_FINDSYMBOL, _T( "Find S&ymbol\tCtrl+Shift+Y" ), ts_searchblock16 );
      m_EditMenu->Append( wxID_ANY, _T( "&Find and Replace" ), findMenu );
      m_EditMenu->Append( tsID_GOTO,       _T( "&Goto...\tCtrl+G" ) );
      m_EditMenu->AppendSeparator();

      tsMenu* advancedMenu = new tsMenu;
      advancedMenu->Append( tsID_UPPER_SELECTION,  _T( "Make &Uppercase\tCtrl+Shift+U" ) );
      advancedMenu->Append( tsID_LOWER_SELECTION,  _T( "Make &Lowecase\tCtrl+U" ) );
      advancedMenu->Append( tsID_COMMENT_SELECTION,  _T( "&Comment Selection\tCtrl+K" ) );
      advancedMenu->Append( tsID_UNCOMMENT_SELECTION,  _T( "Uncomment Selection\tCtrl+Shift+K" ) );
      advancedMenu->Append( tsID_INDENT_SELECTION,  _T( "&Indent Selection" ) );
      advancedMenu->Append( tsID_UNINDENT_SELECTION,  _T( "Unindent Selection" ) );
      advancedMenu->Append( tsID_MATCH_BRACE,  _T( "Match Brace\tCtrl+]" ) );
      m_EditMenu->Append( wxID_ANY, _T( "Ad&vanced" ), advancedMenu );

      tsMenu* bookmarkMenu = new tsMenu;
      bookmarkMenu->Append( tsID_BOOKMARK_TOGGLE,  _T( "&Toggle Bookmark\tCtrl+F2" ) );
      bookmarkMenu->Append( tsID_BOOKMARK_NEXT,  _T( "&Next Bookmark\tF2" ) );
      bookmarkMenu->Append( tsID_BOOKMARK_PREV,  _T( "&Previous Bookmark\tShift+F2" ) );
      bookmarkMenu->Append( tsID_BOOKMARK_CLEARALL,  _T( "&Clear All" ) );
      m_EditMenu->Append( wxID_ANY, _T( "Boo&kmarks" ), bookmarkMenu );

      tsMenu* foldingMenu = new tsMenu;
      foldingMenu->Append( tsID_FOLDING_TOGGLEBLOCK, "&Collapse Block" );
      foldingMenu->Append( tsID_FOLDING_TOGGLEALLINBLOCK, "Collapse &All In Block" );
      foldingMenu->Append( tsID_FOLDING_TOGGLEALL, "E&xpand All" );
      foldingMenu->Append( tsID_FOLDING_COLLAPSETODEFS, "Collapse To &Definitions" );
      m_EditMenu->Append( wxID_ANY, _T( "Foldi&ng" ), foldingMenu );

      tsMenu* scriptSenseMenu = new tsMenu;
      scriptSenseMenu->Append( tsID_LIST_MEMBERS,  _T( "List Mem&bers\tCtrl+J" ) );
      scriptSenseMenu->Append( tsID_PARAMETER_INFO,  _T( "Parameter Inf&o\tCtrl+Shift+Space" ) );
      scriptSenseMenu->Append( tsID_QUICK_INFO,  _T( "&Quick Info\tCtrl+Q" ) );
      scriptSenseMenu->Append( tsID_COMPLETE_WORD,  _T( "Complete &Word\tAlt+Right Arrow" ) );
      m_EditMenu->Append( wxID_ANY, _T( "Script&Sense" ), scriptSenseMenu );
   }

   tsMenu* viewMenu = new tsMenu;
   {
      viewMenu->Append( tsID_PROJECT_OPENWITH, _T( "Open Wit&h..." ) );
      viewMenu->AppendIconItem( tsID_EXPLORE, _T( "Open Containing &Folder" ), ts_explorer16 );
      viewMenu->AppendSeparator();
      viewMenu->AppendCheckIconItem( tsID_TABSANDSPACES,  _T( "Show Whi&tespace" ), ts_showspace16 );
      viewMenu->AppendCheckItem( tsID_LINEBREAKS,     _T( "Show Line B&reaks" ) );
      viewMenu->AppendCheckIconItem( tsID_LINENUMBERS,    _T( "Enable Line &Numbers" ), ts_showlinenumbers16 );
      viewMenu->AppendCheckIconItem( tsID_LINEWRAP,       _T( "Enable Line &Wrap" ), ts_togglewordwrap16 );
      viewMenu->AppendSeparator();
      viewMenu->AppendCheckItem( tsID_SHOW_LEFTPANE,    _T( "Show &Project Pane\tCtrl+Alt+P" ) );
      viewMenu->AppendCheckItem( tsID_SHOW_BOTTOMPANE,    _T( "Show &Output Pane\tCtrl+Alt+O" ) );
      //viewMenu->->AppendSeparator();
      //viewMenu->AppendWithIcon( wxID_ANY, _T( "Navigate &Backward" ) );
      //viewMenu->AppendWithIcon( wxID_ANY, _T( "Navigate &Forward" ) );
   }

   m_ProjectMenu = new tsMenu;
   {
      //AppendWithIcon( projectMenu, wxID_ANY,          _T( "&Add Script" ) );
      //projectMenu->AppendSeparator();
      m_ProjectMenu->Append( tsID_PROJECT_REBUILDEXPORTS, _T( "&Rebuild Exports" ) );
      m_ProjectMenu->AppendSeparator();
      m_ProjectMenu->AppendIconItem( tsID_PROJECT_PRECOMPILE, _T( "Pre&Compile\tF7" ), ts_precompile_changes16 );
      m_ProjectMenu->AppendIconItem( tsID_PROJECT_PRECOMPILE_ACTIVE, _T( "PreCompile Active\tCtrl+F7" ), ts_precompile_file16 );
      m_ProjectMenu->AppendIconItem( tsID_PROJECT_PRECOMPILEALL, _T( "PreCompile &All\tCtrl+Shift+F7" ), ts_precompile_all16 );
      m_ProjectMenu->AppendIconItem( tsID_CLEARDSOS,          _T( "&Delete DSOs" ), ts_deletedso16 );   
      m_ProjectMenu->AppendSeparator();
      m_ProjectMenu->Append( tsID_PROJECT_PROPERTIES, _T( "&Properties..." ) );
   }

   m_DebugMenu = new tsMenu; 
   {
      m_DebugMenu->AppendIconItem( tsID_DEBUG_START,       _T( "&Start\tF5" ), ts_start16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_START_WITHOUT_DEBUGGING, _T( "Start Without Debu&gging\tCtrl+F5" ), ts_startnodebug16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_CONNECT,     _T( "Connect" ), ts_debugconnect16 );
      m_DebugMenu->Append( tsID_DEBUG_RESTART,     _T( "&Restart\tCtrl+Shift+F5" ) );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_STOP,        _T( "&Stop\tShift+F5" ), ts_stop16 );
      m_DebugMenu->AppendSeparator();
      m_DebugMenu->AppendIconItem( tsID_DEBUG_BREAK,       _T( "&Break\tCtrl+Alt+Break" ), ts_break16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_STEP,        _T( "&Step\tF11" ), ts_stepin16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_STEP_OVER,   _T( "Step &Over\tF10" ), ts_stepover16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_STEP_OUT,    _T( "Step O&ut\tShift+F11" ), ts_stepout16 );
      m_DebugMenu->AppendSeparator();
      m_DebugMenu->Append( tsID_RELOAD_SCRIPTS,    _T( "Re&load Scripts\tCtrl+Shift+R" ) );
      m_DebugMenu->AppendSeparator();
      m_DebugMenu->AppendIconItem( tsID_DEBUG_NEWBREAKPOINT, _T( "New Breakpoint...\tCtrl+B" ), ts_newbreak16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_CLEARALLBREAKPOINTS, _T( "Delete All Breakpoints\tCtrl+Shift+F9" ), ts_breakclearall16 );
      m_DebugMenu->AppendIconItem( tsID_DEBUG_DISABLEALLBREAKPOINTS, _T( "Disable All Breakpoints" ), ts_breakdisableall16 );
   }

   m_ToolsMenu = new tsMenu;
   UpdateToolsMenu();

   m_WindowMenu = new tsMenu;
   {
      m_WindowMenu->Append( tsID_NEXTVIEW,      _T( "Next\tCtrl+Tab" ) );
      m_WindowMenu->Append( tsID_PREVIOUSVIEW,  _T( "Previous\tCtrl+Shift+Tab" ) );
      m_WindowMenu->AppendSeparator();
      m_WindowMenu->Append( wxID_CLOSE_ALL,  _T( "C&lose All Files\tCtrl+Shift+F4" ) );
      //m_WindowMenu->Append( tsID_WINDOW_FIRST, "", "", wxITEM_SEPARATOR ); 
      //m_WindowList.UseMenu( windowMenu );
   }

   tsMenu *helpMenu = new tsMenu;
   {
      helpMenu->AppendIconItem( wxID_HELP,         _T( "&Topics" ), ts_topics16 );
      helpMenu->AppendSeparator();
      helpMenu->Append( tsID_FEATUREREQUEST,  _T( "&Feature Request" ) );
      helpMenu->Append( tsID_BUGTRACKER,  _T( "&Bug Report" ) );
      helpMenu->Append( tsID_UPDATECHECK,  _T( "Check New &Version" ) );

      wxMenuItem* item = helpMenu->Append( wxID_ABOUT, _T( "&About Torsion" ) );
      #ifdef __WXMSW__
         wxBitmap bmp;
         bmp.CopyFromIcon( wxIcon( "\"AAAAAAIDI_MAINFRAME\"", wxBITMAP_TYPE_ICO_RESOURCE, 16, 16 ) );
         item->SetBitmap( bmp );
      #else
         item->SetBitmap( wxBitmap( torsion16_xpm ) );
      #endif
   }

   wxMenuBar *menu_bar = new tsMenuBar( wxNO_BORDER | wxCLIP_CHILDREN );
   menu_bar->Append(fileMenu, _T("&File"));
   menu_bar->Append(m_EditMenu, _T("&Edit"));
   menu_bar->Append(viewMenu, _T("&View"));
   menu_bar->Append(m_ProjectMenu, _T("&Project"));
   menu_bar->Append(m_DebugMenu, _T("&Debug"));
   menu_bar->Append(m_ToolsMenu, _T("&Tools"));
   menu_bar->Append(m_WindowMenu, _T("&Window"));
   menu_bar->Append(helpMenu, _T("&Help"));

   #ifdef __WXMAC__
      wxMenuBar::MacSetCommonMenuBar(menu_bar);
   #endif //def __WXMAC__

   // Associate the menu bar with the frame
   SetMenuBar( menu_bar );

   wxSystemOptions::SetOption(wxT("msw.remap"), 0);
   wxToolBar* toolBar = new tsToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_HORIZONTAL | wxTB_NODIVIDER | wxNO_BORDER );
   toolBar->SetMargins( 0, 0 );
   toolBar->SetToolBitmapSize( wxSize( 16, 16 ) );
   //toolBar->AddSeparator();
   toolBar->AddTool( tsID_NEWPROJECT, _T( "New Project" ), ts_new_project16, _T( "New Project (Ctrl+Shift+N)" ) );
   toolBar->AddTool( wxID_NEW, _T( "New Script" ), ts_new_document16, _T( "New Script (Ctrl+N)" ) );
   toolBar->AddTool( wxID_OPEN, _T( "Open Script" ), ts_open_document16, _T( "Open Script (Ctrl+O)" ) );
   toolBar->AddTool( wxID_SAVE, _T( "Save Script" ), ts_save16, _T( "Save Script (Ctrl+S)" ) );
   toolBar->AddTool( tsID_SAVEALL, _T( "Save All" ), ts_save_all16, _T( "Save All (Ctrl+Shift+S)" ) );
   toolBar->AddSeparator();
   toolBar->AddTool( wxID_CUT, _T( "Cut" ), ts_cut_clipboard16, _T( "Cut (Ctrl+X)" ) );
   toolBar->AddTool( wxID_COPY, _T( "Copy" ), ts_copy_clipboard16, _T( "Copy (Ctrl+C)" ) );
   toolBar->AddTool( wxID_PASTE, _T( "Paste" ), ts_paste_clipboard16, _T( "Paste (Ctrl+V)" ) );
   toolBar->AddSeparator();
   toolBar->AddTool( wxID_UNDO, _T( "Undo" ), ts_undo16, _T( "Undo (Ctrl+Z)" ) );
   toolBar->AddTool( wxID_REDO, _T( "Redo" ), ts_redo16, _T( "Redo (Ctrl+Y)" ) );
   toolBar->AddSeparator();

   m_StartToolButton = toolBar->AddTool( tsID_DEBUG_STARTBREAK, _T( "Start" ), ts_start16, _T( "Start (F5)" ) );
   
   /*
   m_BreakTool = toolBar->AddTool( tsID_DEBUG_BREAK, _T( "Break" ), wxBitmap( break16_xpm ), _T( "Break (Ctrl+Alt+Break)" ) );
   toolBar->Realize();
   toolBar->RemoveTool( tsID_DEBUG_BREAK );
   m_ContinueTool = toolBar->AddTool( tsID_DEBUG_BREAK, _T( "Continue" ), wxBitmap( start16_xpm ), _T( "Continue (F5)" ) );
   toolBar->Realize();
   toolBar->RemoveTool( tsID_DEBUG_BREAK );
   */

   toolBar->AddTool( tsID_DEBUG_STEP, _T( "Step" ), ts_stepin16, _T( "Step (F11)" ) );
   toolBar->AddTool( tsID_DEBUG_STEP_OVER, _T( "Step Over" ), ts_stepover16, _T( "Step Over (F10)" ) );
   toolBar->AddTool( tsID_DEBUG_STEP_OUT, _T( "Step Out" ), ts_stepout16, _T( "Step Out (Shift+F11)" ) );
   m_ConfigComboBox = new wxComboBox( toolBar, tsID_CONFIGCOMBO, "", wxDefaultPosition, wxSize( 160, wxDefaultCoord), 0, NULL, wxSTATIC_BORDER | wxCB_DROPDOWN | wxCB_READONLY | wxCB_SORT );
   toolBar->AddControl( m_ConfigComboBox );
   toolBar->AddSeparator();
   toolBar->AddTool( tsID_FINDINFILES, _T( "Find in Files" ), ts_findinfiles16, _T( "Find in Files (Ctrl+Shift+F)" ) );
   m_FindComboBox = new wxComboBox( toolBar, tsID_FINDCOMBO, "", wxDefaultPosition, wxSize( 175, wxDefaultCoord), 0, NULL, wxSTATIC_BORDER );
   AppPrefs::AddStringsToCombo( m_FindComboBox, tsGetPrefs().GetFindStrings() );
   toolBar->AddControl( m_FindComboBox );
   //toolBar->AddTool( tsID_FINDPREV, _T( "Find Previous" ), wxBitmap( search_prev16_xpm ), _T( "Find Previous (Shift+F3)" ) );
   //toolBar->AddTool( tsID_FINDNEXT, _T( "Find Next" ), wxBitmap( search_next16_xpm ), _T( "Find Next (F3)" ) );
   toolBar->Realize();
   SetToolBar( toolBar );
   //m_FindComboBox->SetSize( 175, 10 );

   // Make sure we initialize the find data with
   // the selected item in the combo box.
   wxASSERT( m_FindComboBox );
   m_FindData.SetFindString( m_FindComboBox->GetValue() );
   m_FindData.SetFlags( wxFR_DOWN );
   m_FindThread = new FindThread();

   // Setup the status bar...
   // TODO: We should size these based on font size at some point.
   // TODO: Consider adding icons insted of text for Ln, Col, Ins/Ovr, 
   // etc.  See statbar example!
   tsStatusBar *statusBar = new tsStatusBar(this, wxID_ANY, wxNO_BORDER | wxST_SIZEGRIP | wxCLIP_CHILDREN );
   statusBar->SetFieldsCount(7);
   SetStatusBar( statusBar );
   if ( statusBar ) {
      int widths[] = { -1, 64, 52, 75, 47, 27, 20 };
      //int styles[] = { wxSB_NORMAL, wxSB_FLAT, wxSB_FLAT, wxSB_RAISED, wxSB_NORMAL, wxSB_FLAT, wxSB_NORMAL }; 
      //bar->SetStatusStyles( 7, styles );
      statusBar->SetStatusWidths( 7, widths );
   }
   SetStatusText( _T( "Ready" ) );

   // Create the project sash and notebook, but
   // keep them hidden for now.
   m_ProjectSash = new tsSashLayoutWindow( this, tsID_PROJECTSASH,
      wxDefaultPosition, wxSize(1, 1), wxSW_3DSASH | wxCLIP_CHILDREN );
   wxSize sashSize( tsGetPrefs().GetProjectSashWidth(), size.GetHeight() );
   if ( sashSize.GetWidth() <= 4 )
      sashSize.SetWidth( size.GetWidth() / 4 );
   m_ProjectSash->SetDefaultSize( sashSize );
   m_ProjectSash->SetOrientation( wxLAYOUT_VERTICAL );
   m_ProjectSash->SetAlignment( wxLAYOUT_LEFT );
   m_ProjectSash->SetDefaultBorderSize( 4 );
   m_ProjectSash->SetExtraBorderSize( 0 );
   m_ProjectSash->SetSashVisible( wxSASH_RIGHT, true );
   m_ProjectSash->SetSashBorder( wxSASH_RIGHT, true ); 
   m_ProjectSash->Hide();
   m_ProjectNotebook = new shNotebook( m_ProjectSash, wxID_ANY,
      wxDefaultPosition, wxDefaultSize, shNB_NOBUTTONS );
   {
      wxImageList* images = new wxImageList( 16, 16, true );
	   images->Add( ts_new_project16 );
	   m_ProjectNotebook->AssignImageList( images );
   }

   // The bottom sash which will contain a notebook 
   // for find in files and different debug windows.
	m_BottomSash = new tsSashLayoutWindow( this, tsID_BOTTOMSASH,
      wxDefaultPosition, wxSize(1, 1), wxSW_3DSASH | wxCLIP_CHILDREN );
   sashSize.Set( size.GetWidth(), tsGetPrefs().GetBottomSashHeight() );
   if ( sashSize.GetHeight() <= 4 )
      sashSize.SetHeight( size.GetHeight() / 4 );
   m_BottomSash->SetDefaultSize( sashSize );
	m_BottomSash->SetOrientation( wxLAYOUT_HORIZONTAL );
	m_BottomSash->SetAlignment( wxLAYOUT_BOTTOM );
   m_BottomSash->SetDefaultBorderSize( 4 );
   m_BottomSash->SetExtraBorderSize( 0 );
   m_BottomSash->SetSashVisible( wxSASH_TOP, true );
   m_BottomSash->SetSashBorder( wxSASH_TOP, true ); 
   m_BottomNotebook = new shNotebook( m_BottomSash, wxID_ANY, 
	   wxDefaultPosition, wxDefaultSize, shNB_NOBUTTONS );
   /*
   {
      wxImageList* images = new wxImageList( 16, 16, true );
	   images->Add( wxIcon( output_window16_xpm ) );
	   images->Add( wxIcon( callstack_window16_xpm ) );
	   images->Add( wxIcon( watch_window16_xpm ) );
	   m_BottomNotebook->AssignImageList( images );
   }
   */

   // Setup the output panel.
   m_OutputPanel = new OutputPanel();
   m_OutputPanel->Hide();
   m_OutputPanel->Create( m_BottomNotebook );
	m_BottomNotebook->AddPage( m_OutputPanel, m_OutputPanel->GetLabel(), wxEmptyString, false, -1 );

   // Where you can spy on locals
	//m_LocalsWindow = new WatchCtrl( m_BottomNotebook );
	//m_BottomNotebook->AddPage( m_LocalsWindow, "Locals", false, 2 );

	// Where you can spy on anything.
	m_WatchWindow = new WatchCtrl( m_BottomNotebook, true );
	m_BottomNotebook->AddPage( m_WatchWindow, "Watch", wxEmptyString, false, 2 );

   // The current callstack...
	m_CallStack = new wxListView;
   m_CallStack->Hide();
   m_CallStack->Create( m_BottomNotebook, tsID_CALLSTACK,
      						wxDefaultPosition, wxDefaultSize,
		      				wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxSTATIC_BORDER );
	m_CallStack->InsertColumn( 0, "Function", wxLIST_FORMAT_LEFT, 300 );
	m_CallStack->InsertColumn( 1, "File", wxLIST_FORMAT_LEFT, 300 );
	m_CallStack->InsertColumn( 2, "Line", wxLIST_FORMAT_LEFT, 200 );
	m_BottomNotebook->AddPage( m_CallStack, "Callstack", wxEmptyString, false, 1 );

   // The "find" window.
	m_FindWindow = new FindResultsCtrl( m_BottomNotebook, tsID_FINDWINDOW, m_FindThread );
   m_BottomNotebook->AddPage( m_FindWindow, "Find Results", wxEmptyString, false, 0 );
   
   // The breakpoints...
   m_Breakpoints = new BreakpointsPanel();
   m_Breakpoints->Hide();
   m_Breakpoints->Create( m_BottomNotebook );
	m_BottomNotebook->AddPage( m_Breakpoints, m_Breakpoints->GetLabel(), wxEmptyString, false, -1 );

   // Select the output tab.
   m_BottomNotebook->SetSelection( 0 );

   wxFileName chmFile( wxGetApp().GetAppPath() );
   chmFile.SetFullName( "Torsion.chm" );
   m_HelpController.Initialize( chmFile.GetFullPath() );

   return true;
}

bool MainFrame::Destroy()
{
   // Clean up any open find/replace dialogs.
   FindDialogClose( wxFindDialogEvent() );

   {
      wxArrayString FindHistory;
      AppPrefs::GetStringsFromCombo( m_FindComboBox, FindHistory );
      tsGetPrefs().SetFindStrings( FindHistory );
   }

   // Stop and cleanup the find thread.
   if ( m_FindThread )
   {
      m_FindThread->Cancel();
      wxDELETE( m_FindThread );
   }

   // Restore the window before closing up, so
   // that we get the proper position and size.
   bool isMaximized = IsMaximized();
   if ( IsIconized() || isMaximized )
      Maximize( false );

   tsGetPrefs().SetPosition( wxRect( GetPosition(), GetSize() ) );
   tsGetPrefs().SetMaximized( isMaximized );
   tsGetPrefs().SetProjectSashWidth( m_ProjectSash->GetSize().GetWidth() );
   tsGetPrefs().SetBottomSashHeight( m_BottomSash->GetSize().GetHeight() );

   // This would have been cleared before here.
   wxASSERT( !m_ProjectDoc );

   return wxDocMDIParentFrame::Destroy();
}

wxMDIClientWindow* MainFrame::OnCreateClient()
{
    return new ClientWindow;
}

void MainFrame::OnCloseWindow( wxCloseEvent& event )
{
   // First clear the project if we have one.
   if ( m_ProjectDoc ) 
   {
      // Look to see if we need to stop the precompile.
      if ( m_PreCompiler ) 
      {
         wxMessageDialog dlg( this, "Do you want to stop the precompiler?", "Torsion", wxYES_NO | wxICON_QUESTION  );
         if ( dlg.ShowModal() != wxID_YES )
            return;

         OnPrecompileStop( wxCommandEvent() );
      }

   	wxASSERT( tsGetDebugger() );
      if ( tsGetDebugger()->IsRunning() ) 
      {
         wxMessageDialog dlg( this, "Do you want to stop debugging?", "Torsion", wxYES_NO | wxICON_QUESTION  );
         if ( dlg.ShowModal() != wxID_YES )
            return;

         OnDebugStop( wxCommandEvent() );
      }

      if ( m_ProjectDoc->Close() ) 
      {
         // Store the last project for later.
         tsGetPrefs().SetLastProject( m_ProjectDoc->GetFilename() );

         m_ProjectDoc->DeleteAllViews();
         m_ProjectDoc = NULL;

         wxASSERT( m_ConfigComboBox );
         m_ConfigComboBox->Clear();
      } 
      else 
      {
         // The user canceled the save on close
         // so halt the window close operation.
         event.Veto();
         return;
      }
   }
   else
   {
      // Store an empty last project.
      tsGetPrefs().SetLastProject( wxEmptyString );
   }

   // Free up the autocomp manager here while the main loop is
   // still alive.  If i don't wxThread will kill itself and
   // fail to clean up it's memory.
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->ClearExports();
   tsGetAutoComp()->SetEnable( false );

   // Now let the base clear the rest of the docs.
   wxDocMDIParentFrame::OnCloseWindow( event );
}

void MainFrame::OnNewProject( wxCommandEvent& event )
{
   // TODO: Consider adding a wizard here instead.
   ProjectDoc dummyDoc;
   ProjectDlg dlg( this );
   dlg.SetLabel( "New Project" );
   if ( dlg.ShowModal( &dummyDoc ) != wxID_OK ) {
      return;
   }

   DocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   // Close the existing document if we have one.
   if (  m_ProjectDoc && 
         !docMan->CloseDocument( m_ProjectDoc, false ) ) {
      return;
   }
   m_ProjectDoc = NULL;

   // The last project has been closed, so
   // update the title to remove it.
   UpdateTitle();

   // Ok create the new document and copy our dummy over.
   wxDocTemplate* temp = docMan->GetProjectTemplate();
   m_ProjectDoc = (ProjectDoc*)temp->CreateDocument( wxEmptyString, 0 );
   if ( m_ProjectDoc ) 
   {
      m_ProjectDoc->SetDocumentName( temp->GetDocumentName() );
      m_ProjectDoc->SetDocumentTemplate( temp );

      // Copy my settings from the other document over.
      m_ProjectDoc->CopyDocument( &dummyDoc );

      if (!m_ProjectDoc->OnNewDocument()) {

         m_ProjectDoc->DeleteAllViews();
         m_ProjectDoc = NULL;
         return;
      }

      // Update the config combo box.
      wxASSERT( m_ConfigComboBox );
      m_ConfigComboBox->Clear();
      const ProjectConfigArray& Configs = m_ProjectDoc->GetConfigs();

      for ( int i = 0; i < Configs.GetCount(); i++ )
         m_ConfigComboBox->Append( Configs[i].GetName() );

      m_ConfigComboBox->Select( 0 );
      
      // Update the title again to reflect the
      // new project name.
      UpdateTitle();
   }
}

void MainFrame::OnNewProjectUpdateUI( wxUpdateUIEvent& event )
{
	wxASSERT( tsGetDebugger() );
   event.Enable( !tsGetDebugger()->IsRunning() && !m_PreCompiler );
}

void MainFrame::OnOpenProject( wxCommandEvent& event )
{
   DocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxDocTemplate* temp = docMan->GetProjectTemplate();
   
   // Temporarily make the template visible.
   long flags = temp->GetFlags();
   temp->SetFlags( wxTEMPLATE_VISIBLE );
   wxString path;
   bool result = docMan->SelectDocumentPath( &temp, 1, path, 0 ) ? true : false;
   temp->SetFlags( flags );
   if ( !result )
      return;

   OpenProject( path ); 
}

bool MainFrame::OpenProject( const wxString& path )
{
   DocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   // Is this project already open?
   wxDocument* currentDoc = docMan->FindDocument( path );
   if (currentDoc) {
      wxASSERT( m_ProjectDoc == currentDoc );
      return true;
   }

   // Close the existing document if we have one.
   if (  m_ProjectDoc && !docMan->CloseDocument( m_ProjectDoc, false ) ) {
      return true;
   }
   m_ProjectDoc = NULL;

   // The last project has been closed, so
   // update the title to remove it.
   UpdateTitle();

   // Get the template for the project type.
   wxDocTemplate* temp = docMan->GetProjectTemplate();
   wxASSERT( temp );

   // Ok open the project.
   m_ProjectDoc = (ProjectDoc*)temp->CreateDocument(path, 0);
   if (m_ProjectDoc)
   {
      m_ProjectDoc->SetDocumentName(temp->GetDocumentName());
      m_ProjectDoc->SetDocumentTemplate(temp);
      if (!m_ProjectDoc->OnOpenDocument(path))
      {
            m_ProjectDoc->Modify(false);
            m_ProjectDoc->DeleteAllViews();
            m_ProjectDoc = NULL;

            // We only return false if the document
            // cannot be opened... this is used to 
            // figure out if we should remove it from
            // the MRU list.
            return false;
      }

      // Update the config combo box.
      wxASSERT( m_ConfigComboBox );
      m_ConfigComboBox->Clear();
      const ProjectConfigArray& Configs = m_ProjectDoc->GetConfigs();
      for ( int i = 0; i < Configs.GetCount(); i++ ) {
         m_ConfigComboBox->Append( Configs[i].GetName() );
      }
      m_ConfigComboBox->SetValue( m_ProjectDoc->GetLastConfigName() );
      if ( m_ConfigComboBox->GetSelection() == wxNOT_FOUND )
         m_ConfigComboBox->Select( 0 );

      // Update the title again to reflect the
      // new project name.
      UpdateTitle();

      // Add this project to the history.
      wxASSERT( docMan->GetProjectHistory() );
      docMan->GetProjectHistory()->AddFileToHistory( path );
   }

   return true;
}

void MainFrame::OnWindowList( wxCommandEvent& event )
{
   if ( !ScriptFrame::GetFrame() )
      return;

   ScriptFrame::GetFrame()->SetActive( event.GetId() - tsID_WINDOW_FIRST );
}

void MainFrame::OnMRUProject( wxCommandEvent& event )
{
   DocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );
   wxFileHistory* projectHistory = docMan->GetProjectHistory();
   wxASSERT( projectHistory );

   int n = event.GetId() - tsID_PROJECT1;
   wxString file( projectHistory->GetHistoryFile( n ) );
   if ( !file.empty() ) 
   {
      if ( !OpenProject( file ) )
         projectHistory->RemoveFileFromHistory( n );
   }
}

void MainFrame::OnShowLeftPane( wxCommandEvent& event )
{
   wxASSERT( m_ProjectSash );
   m_ProjectSash->Show( !m_ProjectSash->IsShown() );
	wxLayoutAlgorithm layout;
	layout.LayoutMDIFrame(this);
}

void MainFrame::OnUpdateShowLeftPane( wxUpdateUIEvent& event )
{
   wxASSERT( m_ProjectSash );
   event.Enable( GetProjectDoc() != NULL );
   event.Check( GetProjectDoc() && m_ProjectSash->IsShown() );
}

void MainFrame::OnShowBottomPane( wxCommandEvent& event )
{
   wxASSERT( m_BottomSash );
   m_BottomSash->Show( !m_BottomSash->IsShown() );
	wxLayoutAlgorithm layout;
	layout.LayoutMDIFrame(this);
}

void MainFrame::OnUpdateShowBottomPane( wxUpdateUIEvent& event )
{
   wxASSERT( m_BottomSash );
   event.Enable( m_BottomSash != NULL );
   event.Check( m_BottomSash && m_BottomSash->IsShown() );
}

void MainFrame::OnProjectClose( wxCommandEvent& event )
{
   wxASSERT( m_ProjectDoc );

   // If the user allows us to close it...
   if ( m_ProjectDoc->Close() ) 
   {
      DocManager* docMan = GetDocumentManager();
      wxASSERT( docMan );

      // If the project is saved then make sure it's in the MRU.
      if ( m_ProjectDoc->GetSavedYet() ) 
      {
         wxASSERT( docMan->GetProjectHistory() );
         docMan->GetProjectHistory()->AddFileToHistory( m_ProjectDoc->GetFilename() );
      }

      m_ProjectDoc->DeleteAllViews();
      m_ProjectDoc = NULL;

      wxASSERT( m_ConfigComboBox );
      m_ConfigComboBox->Clear();

      // Remove the project name from the title.
      UpdateTitle();
   }
}

void MainFrame::OnExternalTools( wxCommandEvent& event )
{
   ExternalToolsDlg dlg;
   dlg.m_ToolCmds = tsGetPrefs().GetToolCommands();
   dlg.Create( this );
   if ( dlg.ShowModal() == wxID_OK && tsGetPrefs().SetToolCommands( dlg.m_ToolCmds ) )
      UpdateToolsMenu();
}

void MainFrame::UpdateToolsMenu()
{
   // Remove all the current items.
   while ( m_ToolsMenu->GetMenuItemCount() > 0 )
      m_ToolsMenu->Destroy( m_ToolsMenu->FindItemByPosition( 0 ) );

   // Add the new items from the command array.
   const ToolCommandArray& cmds = tsGetPrefs().GetToolCommands();
   for ( int i=0; i < cmds.GetCount(); i++ )
   {
      int newId = tsID_TOOLS_FIRST+i;
      if ( newId > tsID_TOOLS_LAST )
         break;
      
      wxString title = cmds[i].GetTitle();
      title = title.BeforeFirst( '\t' );
      m_ToolsMenu->Append( newId, title );
   }

   // Add the remaining elements!
   m_ToolsMenu->Append( tsID_TOOLS_EXTERNAL_TOOLS,  _T( "&External Tools..." ) );
   m_ToolsMenu->AppendSeparator();
   m_ToolsMenu->Append( tsID_TOOLS_PREFERENCES,  _T( "&Preferences..." ) );
}

void MainFrame::OnExecTool( wxCommandEvent& event )
{
   // Get the command object.
   int tool = event.GetId() - tsID_TOOLS_FIRST;
   const ToolCommand& cmd = tsGetPrefs().GetToolCommands()[ tool ];

   // Get the input vars for the command.
   wxString projectName, projectPath, projectWorkingDir;
   wxString configName, configExe, configArgs;
   if ( GetProjectDoc() )
   {
      projectName = GetProjectDoc()->GetName();
      projectPath = GetProjectDoc()->GetFilename();
      projectWorkingDir = GetProjectDoc()->GetWorkingDir();

      wxASSERT( m_ConfigComboBox );
      wxString configName = m_ConfigComboBox->GetValue();
      const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );
      if ( config )
      {
         configName = config->GetName();
         configExe = config->GetExe();
         configArgs = config->GetArgs();
      }
   }
   wxString filePath;
   if ( GetActiveView() )
      filePath = GetActiveView()->GetDocument()->GetFilename();

   // Get the exec line for this command.
   wxString cwd;
   wxString command;
   wxString args;
   if ( !cmd.GetExec(   filePath,
                        projectName,
                        projectPath, 
                        projectWorkingDir,
                        configName,
                        configExe,
                        configArgs,
                        &cwd, 
                        &command,
                        &args ) )
   {
      tsBellEx( wxICON_ERROR );
      return;
   }

   // Launch the command!
   if ( !tsShellExecute::execDeatched( "open", cwd, command, args, tsSW_SHOWNORMAL ) )
      tsBellEx( wxICON_ERROR );
}

void MainFrame::OnUpdateTools( wxUpdateUIEvent& event )
{
   int tool = event.GetId() - tsID_TOOLS_FIRST;
   const ToolCommandArray& cmds = tsGetPrefs().GetToolCommands();
   event.Enable( tool >= 0 && tool < cmds.GetCount() && !cmds[ tool ].GetCmd().IsEmpty() );
}

void MainFrame::OnPreferences( wxCommandEvent& event )
{
   wxArrayString lastExts = tsGetPrefs().GetScriptExtensions();

   SettingsDlg dlg( this );
   if ( dlg.ShowModal() == wxID_OK )
   {
      // First... do we need to change the associations for
      // script extensions?
      if ( tsGetPrefs().GetScriptExtensions() != lastExts )
      {
         // Unregister all the old ones... register the new ones
         // this should not break anything that works currently.
         wxGetApp().UnregisterScriptExts( lastExts );
         wxGetApp().RegisterScriptExts();

         // Gotta update the file templates or you won't
         // be able to open any new script exts.
         wxASSERT( GetDocumentManager() );
         GetDocumentManager()->UpdateScriptFileTemplates();
      }

      // The settings changed... send the update 
      // hint to all the active views.
      tsPrefsUpdateHint hint;
      SendHintToAllViews( &hint, false );

      // Let the colors update on the find and output windows too.
      wxASSERT( GetOutputPanel() );
      GetOutputPanel()->m_Output->UpdatePrefs();
      GetFindWindow()->UpdatePrefs();

      // Toggle the autocomplete stuff.
      tsGetAutoComp()->SetEnable( tsGetPrefs().GetCodeCompletion() );

      // Store the prefs now... so that we don't loose
      // settings if they happen to crash out.
      wxFileName prefs( wxGetApp().GetAppPath() );
      prefs.SetFullName( "preferences.xml" );
      tsGetPrefs().SaveIfDirty( prefs.GetFullPath() );
   }
}

void MainFrame::OnProjectProperties( wxCommandEvent& event )
{
   wxASSERT( GetProjectDoc() );

   ProjectDlg dlg( this );
   if ( dlg.ShowModal( GetProjectDoc() ) == wxID_OK ) 
   {
      // Get the current focus.
      wxWindow* focused = FindFocus();

      // Do we need to rebuild the exports?
      if ( dlg.GetRebuildExports() )
         m_ProjectDoc->BuildExportsDB();

      // Update the config combo box.
      wxASSERT( m_ConfigComboBox );
      wxString sel = m_ConfigComboBox->GetStringSelection();
      m_ConfigComboBox->Clear();
      const ProjectConfigArray& Configs = m_ProjectDoc->GetConfigs();
      
      for ( int i = 0; i < Configs.GetCount(); i++ )
         m_ConfigComboBox->Append( Configs[i].GetName() );

      if ( !m_ConfigComboBox->SetStringSelection( sel ) )
         m_ConfigComboBox->SetSelection( 0 );

      GetProjectDoc()->UpdateAllViews();

      // Update the title again to reflect the
      // new project name.
      UpdateTitle();

      // Restore the focus.
      if ( focused )
         focused->SetFocus();
   }
}

void MainFrame::OnProjectRebuildExports( wxCommandEvent& event )
{
   wxASSERT( m_ProjectDoc );
   m_ProjectDoc->BuildExportsDB();
}

void MainFrame::OnUpdateRebuildExports( wxUpdateUIEvent& event )
{
	wxASSERT( tsGetDebugger() );
   event.Enable( GetProjectDoc() && GetProjectDoc()->HasExports() &&
                  !m_PreCompiler && !tsGetDebugger()->IsRunning() );
}

void MainFrame::UpdatePrecompileMenu()
{
   wxASSERT( m_ProjectMenu );

   if ( m_PreCompiler ) 
      m_ProjectMenu->SetLabel( tsID_PROJECT_PRECOMPILE, _T( "C&ancel PreCompile\tCtrl+Break" ) );
   else 
      m_ProjectMenu->SetLabel( tsID_PROJECT_PRECOMPILE, _T( "Pre&Compile\tF7" ) );

   // Now update the accel table.
   GetMenuBar()->RebuildAccelTable();
   SetAcceleratorTable( GetMenuBar()->GetAccelTable() );
}

void MainFrame::OnProjectPrecompile( wxCommandEvent& event )
{
   if ( m_PreCompiler )
   {
      OnPrecompileStop( wxCommandEvent() );
      return;
   }

   m_AfterPrecompile = START_NOTHING;
   DoPrecompile( event.GetId() );
}

void MainFrame::DoPrecompile( int eventId )
{
   // Force all the scripts to save before the compile.
   OnSaveAll( wxCommandEvent() );

   wxASSERT( m_ConfigComboBox );
   wxString config = m_ConfigComboBox->GetValue();

   wxASSERT( m_OutputPanel );
	m_OutputPanel->Clear();
   m_BottomNotebook->SetSelection( 0 );

   wxASSERT( m_ProjectDoc );
   const ProjectConfig* currentCfg = m_ProjectDoc->GetConfig( config );
   wxASSERT( currentCfg );

   // Print the startup message...
   m_OutputPanel->AppendText( "Precompiling...\r\n\r\n" );

   wxString activeScript = "";
   if ( eventId == tsID_PROJECT_PRECOMPILE_ACTIVE )
   {
      ScriptView* view = GetActiveView();
      //wxASSERT( view );
      
      // Bugfix... Although the OnUpdatePrecompile function 
      // checks to be sure that GetActiveView() returns a vaild
      // ScriptView, this has happened in release builds.  So
      // protect against it.
      if ( !view ) 
         return;

      activeScript = view->GetDocument()->GetFilename();
      activeScript = m_ProjectDoc->MakeReleativeTo( activeScript );
   }

   wxASSERT( m_ProjectDoc );
   wxASSERT( !m_PreCompiler );
   m_PreCompiler = new PreCompiler();
   m_PreCompiler->Run(  m_ProjectDoc->GetWorkingDir(), 
                        m_ProjectDoc->GetExecuteCommand( config, false ),
                        m_ProjectDoc->GetMods(),
                        activeScript,
                        eventId == tsID_PROJECT_PRECOMPILEALL ? true : false,
                        currentCfg->UseSetModPaths(),
                        this );

   UpdatePrecompileMenu();
}

void MainFrame::OnPrecompileDone( wxCommandEvent& event )
{
   // This can happen when we stop the precompiler... let
   // OnPrecompileStop deal with cleanup.
   if ( !m_PreCompiler || m_PreCompilerStopping )
      return;

   int errors, scripts;
   bool success;
   m_PreCompiler->WaitForFinish( &success, &scripts, &errors );
   delete m_PreCompiler;
   m_PreCompiler = NULL;
   UpdatePrecompileMenu();

   // Did the precompiler fail to finish?
   if ( !success )
   {
      wxASSERT( m_OutputPanel );
      m_OutputPanel->AppendText( "Precompile failed\r\n" ); 
   
      if ( m_AfterPrecompile != START_NOTHING )
      {
         wxMessageDialog MessageBox( this, "There are script errors. Continue?", "Torsion", wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION );
		   if ( MessageBox.ShowModal() != wxID_YES )
            return;
      }

   } 
   else 
   {
      // Write the output banner.
      wxASSERT( m_OutputPanel );
      wxString temp;
      temp << "\n" << m_ProjectDoc->GetName() << " - " << 
            scripts << " scripts(s), " <<
            errors << " error(s)\r\n";
      m_OutputPanel->AppendText( temp );

      // Do we need to warn?
      if ( errors > 0 && m_AfterPrecompile != START_NOTHING )
      {
         wxMessageDialog MessageBox( this, "There were script errors. Continue?", "Torsion", wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION );
		   if ( MessageBox.ShowModal() != wxID_YES )
            return;
      }
   }

   // What next?
   if ( m_AfterPrecompile == START_DEBUG )
      StartDebug( m_DebugCommand, false );
   else if ( m_AfterPrecompile == START_RUN )
      StartRun( false );
   else if ( m_AfterPrecompile == START_NOTHING )
   {
      // Just bell...
      if ( errors > 0 )
         tsBellEx( wxICON_EXCLAMATION );
      else
         tsBellEx( wxICON_INFORMATION );
   }
}

void MainFrame::OnPrecompileOutput( wxCommandEvent& event )
{
   wxASSERT( m_OutputPanel );
   m_OutputPanel->AppendText( event.GetString() );
}

void MainFrame::OnPrecompileStop( wxCommandEvent& event )
{
   if ( !m_PreCompiler )
      return;

   m_PreCompilerStopping = true;
   m_PreCompiler->Stop();
   delete m_PreCompiler;
   m_PreCompiler = NULL;
   UpdatePrecompileMenu();
   m_PreCompilerStopping = false;

   wxASSERT( m_OutputPanel );
   m_OutputPanel->AppendText( "\r\nPrecompile cancelled\r\n" );
}

void MainFrame::OnUpdatePrecompile( wxUpdateUIEvent& event )
{
   wxASSERT( m_ProjectMenu );

   if ( !GetProjectDoc() )
   {
      event.Enable( false );
      return;
   }

   // If the precompiler is active and this is the 
   // then keep it enabled.
   if ( m_PreCompiler && event.GetId() == tsID_PROJECT_PRECOMPILE )
   {
      event.Enable( true );
      return;
   }

   // Ok... should it be enabled?
   wxASSERT( m_ConfigComboBox );
   wxString configName = m_ConfigComboBox->GetValue();
   const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );
   wxASSERT( tsGetDebugger() );
   bool enabled = !m_PreCompiler && config->Precompile() && !tsGetDebugger()->IsRunning();

   // Special case for precompile active.
   if ( event.GetId() == tsID_PROJECT_PRECOMPILE_ACTIVE )
   {
      ScriptView* view = GetActiveView();
      if ( !view || !view->GetDocument())
         enabled = false;
   }

   event.Enable( enabled );
}

/*
void MainFrame::OnMenuOpen( wxMenuEvent& event )
{
   wxASSERT( m_ProjectMenu );

   event.Skip();

   if ( event.GetMenu() != m_ProjectMenu )
      return;
}
*/

void MainFrame::OnUpdateProjectClose( wxUpdateUIEvent& event )
{
	wxASSERT( tsGetDebugger() );
   event.Enable( m_ProjectDoc != NULL && !tsGetDebugger()->IsRunning() && !m_PreCompiler );
}

void MainFrame::OnProjectRefresh( wxCommandEvent& event )
{
   wxASSERT( m_ProjectDoc );
   m_ProjectDoc->UpdateAllViews();
}

void MainFrame::OnFileClose( wxCommandEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxDocument *doc = docMan->GetCurrentDocument();
   if (!doc || wxDynamicCast(doc,ProjectDoc))
      return;

   if (doc->Close())
   {
      doc->DeleteAllViews();
      if (docMan->GetDocuments().Member(doc))
         delete doc;
   }
}

void MainFrame::OnUpdateFileClose( wxUpdateUIEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxDocument *doc = docMan->GetCurrentDocument();
   event.Enable( doc != NULL && !wxDynamicCast(doc,ProjectDoc) );
}

void MainFrame::OnFileCloseAll(wxCommandEvent& WXUNUSED(event))
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxList& docs = docMan->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while (node)
   {
      wxDocument *doc = (wxDocument *)node->GetData();
      wxList::compatibility_iterator next = node->GetNext();

      if (doc != m_ProjectDoc) {

         if (!docMan->CloseDocument(doc, false))
            return;
      }

      node = next;
   }
}

void MainFrame::OnUpdateFileCloseAll( wxUpdateUIEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   int count = docMan->GetDocuments().GetCount();
   if ( m_ProjectDoc )
      --count;

   event.Enable( count > 0 );
}

void MainFrame::OnFileCloseOthers( wxCommandEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxDocument *activeDoc = docMan->GetCurrentDocument();
   if (!activeDoc || wxDynamicCast(activeDoc,ProjectDoc))
      return;

   wxList& docs = docMan->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while (node)
   {
      wxDocument *doc = (wxDocument *)node->GetData();
      wxList::compatibility_iterator next = node->GetNext();

      if (doc != m_ProjectDoc && doc != activeDoc) {

         if (!docMan->CloseDocument(doc, false))
            return;
      }

      node = next;
   }
}

void MainFrame::OnUpdateFileCloseOthers( wxUpdateUIEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   int count = docMan->GetDocuments().GetCount();
   if ( m_ProjectDoc )
      --count;

   event.Enable( count > 1 );
}

void MainFrame::OnSaveAll( wxCommandEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   // First save all the files.
   wxList& docs = docMan->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while (node)
   {
      wxDocument *doc = (wxDocument *)node->GetData();
      wxList::compatibility_iterator next = node->GetNext();

      if (doc != m_ProjectDoc) {

         if (!doc->Save())
            return;
      }

      node = next;
   }

   // Now save the project if we have one.
   if ( m_ProjectDoc )
      m_ProjectDoc->Save();
}

void MainFrame::OnUpdateScriptOrProject( wxUpdateUIEvent& event )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxDocument *doc = docMan->GetCurrentDocument();
   event.Enable( doc != NULL );
}

void MainFrame::OnUpdateDisabled( wxUpdateUIEvent& event )
{
   event.Enable( false );
}

// TODO: Move this to it's own implementation file.
class DeleteDSOsTraverser : public wxDirTraverser
{
public:

   DeleteDSOsTraverser( OutputPanel* ctrl ) 
      :  wxDirTraverser(), 
         m_Files( 0 ),
         m_Skipped( 0 ),
         m_Ctrl( ctrl )
   {
   }

   virtual wxDirTraverseResult OnFile( const wxString& filename )
   {
      if ( !tsGetPrefs().IsDSO( filename ) )
         return wxDIR_CONTINUE;

      ++m_Files;

      wxString script( filename.BeforeLast( '.' ) );
      wxString temp;
      if ( wxFileName::FileExists( script ) && wxRemoveFile( filename ) ) 
      {
         temp << "Deleted " << filename << "\r\n";
         m_Ctrl->AppendText( temp );
      }
      else
      {
         temp << "Skipped " << filename << "\r\n";
         m_Ctrl->AppendText( temp );
         ++m_Skipped;
      }

      return wxDIR_CONTINUE;
   }

   virtual wxDirTraverseResult OnDir( const wxString& ) 
   {
      return wxDIR_CONTINUE;
   }

   OutputPanel* m_Ctrl;
   int m_Files;
   int m_Skipped;
};

void MainFrame::OnClearDSOs( wxCommandEvent& event )
{
   wxASSERT( m_OutputPanel );
	m_OutputPanel->Clear();
   m_BottomNotebook->SetSelection( 0 );

   m_OutputPanel->AppendText( "Deleting compiled scripts...\r\n\r\n" );

   wxASSERT( GetProjectDoc() );
   DeleteDSOsTraverser sink( m_OutputPanel );
	wxDir dir( GetProjectDoc()->GetWorkingDir() );
	if ( dir.IsOpened() ) 
		dir.Traverse( sink, "*.*", wxDIR_DIRS | wxDIR_FILES );

   wxString temp;
   temp << "\n" << GetProjectDoc()->GetName() << " - " <<
         sink.m_Files << " compiled script(s), " << 
         ( sink.m_Files - sink.m_Skipped ) << " deleted, " <<
         sink.m_Skipped << " skipped\r\n";
   m_OutputPanel->AppendText( temp );
}

void MainFrame::OnClearDSO( wxCommandEvent& event )
{
   wxASSERT( GetDocumentManager() );
   wxASSERT( GetDocumentManager()->GetCurrentDocument() );

   wxString file( GetDocumentManager()->GetCurrentDocument()->GetFilename() );
   file = tsGetPrefs().GetDSOForScript( file );
   if ( file.IsEmpty() )
      return;

   wxASSERT( m_OutputPanel );
	m_OutputPanel->Clear();
   m_BottomNotebook->SetSelection( 0 );

   wxString temp;
   if ( wxRemoveFile( file ) ) 
   {
      temp << "Deleted " << file << "\r\n";
      m_OutputPanel->AppendText( temp );
   }
   else 
   {
      temp << "Failed Deleting " << file << "!\r\n";
      m_OutputPanel->AppendText( temp );
   }
}

void MainFrame::OnUpdateHasDSO( wxUpdateUIEvent& event )
{
	wxASSERT( tsGetDebugger() );
   wxASSERT( GetDocumentManager() );

   // Don't allow DSO delete if the debugger is running
   // or if we don't have an active document!
   if ( tsGetDebugger()->IsRunning() ||
      !GetDocumentManager()->GetCurrentDocument() ) 
   {
      event.Enable( false );
      return;
   }

   // TODO: Make sure the doc is a script document!

   event.Enable( !tsGetPrefs().GetDSOForScript( 
      GetDocumentManager()->GetCurrentDocument()->GetFilename() ).IsEmpty() );
}

shNotebook* MainFrame::ShowProjectSash( bool show )
{
   m_ProjectSash->Show( show );
	wxLayoutAlgorithm layout;
	layout.LayoutMDIFrame(this);
   return m_ProjectNotebook;
}

void MainFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
   AboutDlg dlg;
   dlg.Create( this );
   dlg.ShowModal();
}

void MainFrame::OnUpdateScriptFrame( wxUpdateUIEvent& event )
{
   wxASSERT( m_docManager );
   wxView* active = m_docManager->GetCurrentView();
   event.Enable( active && active->IsKindOf(CLASSINFO(ScriptView)) );
}

void MainFrame::OnFindInFiles( wxCommandEvent& event )
{
   wxString text = GetSelectedText();

   {
      wxASSERT( m_FindComboBox );
      wxArrayString strings;
      AppPrefs::GetStringsFromCombo( m_FindComboBox, strings );
      tsGetPrefs().SetFindStrings( strings );
   }

   FindInFilesDlg dlg( this );
   dlg.m_FindThread = m_FindThread;

   if ( !text.IsEmpty() )
      dlg.AddWhat( text );

   // If we get a tsMenu as the event object then
   // this came from the project tree.... so use
   // the currently selected path.
   if ( GetProjectDoc() && wxDynamicCast( event.GetEventObject(), wxMenu ) )
   {
      dlg.AddLookIn( GetProjectView()->GetSelectedPath() );
   }

   if ( dlg.ShowModal() == wxID_OK ) 
   {
      // Restore the strings in the find combo to 
      // match what we set in the find in files dialog.
      wxASSERT( m_FindComboBox );
      m_FindComboBox->Clear();
      AppPrefs::AddStringsToCombo( m_FindComboBox, tsGetPrefs().GetFindStrings() );

      wxASSERT( m_FindWindow );
      wxASSERT( m_BottomNotebook );
	   m_FindWindow->Clear();
      m_BottomNotebook->SetSelection( 3 );

      wxArrayString paths = dlg.GetPathsFromLookIn( dlg.GetLookIn() );
      m_FindThread->Find( this, dlg.GetWhat(), paths, dlg.GetTypes(), 
         tsGetPrefs().GetFindMatchCase(), tsGetPrefs().GetFindMatchWord() );
   }
}

void MainFrame::OnFindOutput( wxCommandEvent& event )
{
   if ( event.GetEventType() == tsEVT_FIND_STATUS )
      SetStatusText( event.GetString(), 0 );
   else
      m_FindWindow->AppendText( event.GetString() );
}

void MainFrame::FindDialogShow( wxCommandEvent& event )
{
   wxString text = GetSelectedText();

   {
      wxASSERT( m_FindComboBox );
      wxArrayString strings;
      AppPrefs::GetStringsFromCombo( m_FindComboBox, strings );
      tsGetPrefs().SetFindStrings( strings );
   }

   if ( !text.IsEmpty() )
      m_FindData.SetFindString( text );

   if ( !m_FindReplaceDialog )
      m_FindReplaceDialog = new FindReplaceDlg( this, &m_FindData );

   if ( event.GetId() == wxID_FIND )
      m_FindReplaceDialog->ShowFind();
   else
      m_FindReplaceDialog->Show();
}

void MainFrame::FindDialogClose( wxFindDialogEvent& event )
{
   if ( m_FindReplaceDialog )
      m_FindReplaceDialog->Destroy();
   m_FindReplaceDialog = NULL;
}

void MainFrame::AddFindString( wxString& text )
{
   m_FindData.SetFindString( text );

   wxASSERT( m_FindComboBox );
   int index = m_FindComboBox->FindString( m_FindData.GetFindString() );
   if ( index > 0 ) {
      m_FindComboBox->Delete( index );
   }

   while( m_FindComboBox->GetCount() > 19 ) {
      m_FindComboBox->Delete( m_FindComboBox->GetCount()-1 );
   }

   if ( index != 0 ) {
      m_FindComboBox->Insert( m_FindData.GetFindString(), 0 );
      m_FindComboBox->SetValue( m_FindData.GetFindString() );
   }
   if ( m_FindComboBox->GetValue() != m_FindData.GetFindString() ) {
      m_FindComboBox->SetValue( m_FindData.GetFindString() );
   }
}

void MainFrame::OnFindSymbol( wxCommandEvent& event )
{
   FindSymbolDlg dlg( this );

   wxString what = GetSelectedText();
   if ( !what.IsEmpty() )
      dlg.AddWhat( what );

   if ( dlg.ShowModal() != wxID_OK || dlg.m_What.IsEmpty() )
      return;

   wxArrayString paths;
   wxArrayInt lines;
   wxArrayString symbols;

   wxASSERT( tsGetAutoComp() );
   const AutoCompData* data = tsGetAutoComp()->Lock();
   if ( data )
   {
      data->GetDefinitionFileAndLine( dlg.m_What, &paths, &lines, &symbols );
      tsGetAutoComp()->Unlock();
   }

   // Did we get anything?
   wxASSERT( paths.GetCount() == lines.GetCount() );
   if ( paths.IsEmpty() )
   {
      tsBellEx( wxICON_INFORMATION );
      SetStatusText( "The specified symbol was not found." );
      return;
   }

   wxString path = paths[0];
   int line = lines[0];

   // Did we get more than one?
   if ( paths.GetCount() > 1 )
   {
      GotoDefDialog dlg( this );
      if ( dlg.ShowModal( paths, lines, symbols ) != wxID_OK )
         return;

      path = dlg.GetPath();
      line = dlg.GetLine();     
   }

   OpenFile( path, line );
}

void MainFrame::OnUpdateFindSymbol( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetAutoComp() );
   event.Enable( tsGetAutoComp()->HasData() );
}

wxString MainFrame::GetSelectedText( bool AtCursor )
{
   ScriptView* view = NULL;
   if ( ScriptFrame::GetFrame() )
      view = ScriptFrame::GetFrame()->GetSelectedView();

   wxString text;

   if ( view )
      text = view->GetSelectedText( true );

   return text;
}

void MainFrame::OnSize( wxSizeEvent& event )
{
	wxLayoutAlgorithm layout;
	layout.LayoutMDIFrame(this);
}

void MainFrame::OnSashDrag( wxSashEvent& event )
{
   if ( event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE )
      return;

   switch ( event.GetId() )
   {
      case tsID_PROJECTSASH:
      {
         m_ProjectSash->SetDefaultSize( wxSize( event.GetDragRect().width, m_ProjectSash->GetSize().GetHeight() ) );
         break;
      }

      case tsID_BOTTOMSASH:
      {
         m_BottomSash->SetDefaultSize( wxSize( m_BottomSash->GetSize().GetWidth(), event.GetDragRect().height ) );
         break;
      }
   }

   wxLayoutAlgorithm layout;
   layout.LayoutMDIFrame(this);

   // Leaves bits of itself behind sometimes
   //GetClientWindow()->Refresh();
}

void MainFrame::OnUpdateNextView( wxUpdateUIEvent& event )
{
   event.Enable( ScriptFrame::GetFrame() && ScriptFrame::GetFrame()->GetPageCount() > 1 );
}

void MainFrame::OnNextView( wxCommandEvent& event )
{
   if ( ScriptFrame::GetFrame() )
      ScriptFrame::GetFrame()->NextView();
}

void MainFrame::OnPreviousView( wxCommandEvent& event )
{
   if ( ScriptFrame::GetFrame() )
      ScriptFrame::GetFrame()->PreviousView();
}

ScriptDoc* MainFrame::GetOpenDoc( const wxString& FullPath )
{
   // Generate an absolute path if we need it.
   wxString absolutePath = FullPath;
   if ( GetProjectDoc() )
      absolutePath = GetProjectDoc()->MakeAbsoluteTo( absolutePath );

   // Check to see if we already have it open.
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );
   wxDocument* doc = docMan->FindDocument( absolutePath );
   if ( !doc ) 
   {
      // Do a title search!
      const wxList& docs = docMan->GetDocuments();
      const wxNode *node = docs.GetFirst();
      while ( node )
      {
         wxDocument* openDoc = wxDynamicCast( node->GetData(), wxDocument );
         if ( openDoc && openDoc->GetTitle() == absolutePath )
         {
            doc = openDoc;
            break;
         }

         node = node->GetNext();
      }
   }

   return wxDynamicCast( doc, ScriptDoc );
}

ScriptView* MainFrame::GetOpenView( const wxString& FullPath )
{
   ScriptDoc* doc = GetOpenDoc( FullPath );
   if ( !doc )
      return NULL;

   return wxDynamicCast( doc->GetFirstView(), ScriptView );
}

ScriptView* MainFrame::OpenFile( const wxString& FullPath, int ZeroBasedLine )
{
   // Did we find it yet?
   wxDocument* doc = GetOpenDoc( FullPath );
   if ( !doc ) 
   {
      wxDocManager* docMan = GetDocumentManager();
      wxASSERT( docMan );
 
      // Do a title search!
      const wxList& docs = docMan->GetDocuments();
      const wxNode *node = docs.GetFirst();
      while ( node )
      {
         wxDocument* openDoc = wxDynamicCast( node->GetData(), wxDocument );
         if ( openDoc && openDoc->GetTitle() == FullPath )
         {
            doc = openDoc;
            break;
         }

         node = node->GetNext();
      }
   }

   if ( !doc )
   {
      // Generate an absolute path.
      wxString absolutePath = FullPath;
      if ( GetProjectDoc() )
         absolutePath = GetProjectDoc()->MakeAbsoluteTo( absolutePath );

      // Check for existance first!
      if ( !wxFileName::FileExists( absolutePath ) ) 
      {
         wxString msg;
         msg << "The file '" << absolutePath << "' does not exist!";
         wxMessageDialog dlg( this, msg, "Torsion", wxOK | wxICON_STOP );
         dlg.ShowModal();
         return NULL;
      }

      // Make sure it's a valid format!
      wxDocManager* docMan = GetDocumentManager();
      wxASSERT( docMan );
      if ( !docMan->FindTemplateForPath( absolutePath ) ) 
      {
         // Let the shell open it or alternatively "Open With"....
         if (  tsExecuteVerb( absolutePath, "edit", true ) ||
               tsShellOpenAs( absolutePath ) )
            return NULL;

         wxString msg;
         msg << "The file '" << absolutePath << "' is not a script or text file type!";
         wxMessageDialog dlg( this, msg, "Torsion", wxOK | wxICON_STOP );
         dlg.ShowModal();
         return NULL;
      }

      doc = docMan->CreateDocument( absolutePath, wxDOC_SILENT );
      if ( !doc ) 
      {
         wxString msg;
         msg << "The file '" << absolutePath << "' could not be opened!";
         wxMessageDialog dlg( this, msg, "Torsion", wxOK | wxICON_STOP );
         dlg.ShowModal();
         return NULL;
      } 
   }

   // Sometimes the current Project file can be passed into
   // here... don't open it!
   if ( doc->IsKindOf( CLASSINFO( ProjectDoc ) ) )
      return NULL;

   // Make it the active view!
   wxASSERT( ScriptFrame::GetFrame() );
   wxASSERT( doc );
   ScriptFrame::GetFrame()->SetActive( doc->GetFirstView() );

   // Add it to the open file history.
   //m_WindowList.AddFileToHistory( absolutePath );

   // Do we want to scroll to an existing line?
   ScriptView* view = wxDynamicCast( doc->GetFirstView(), ScriptView );
   if ( ZeroBasedLine >= 0 )
      view->SetLineSelected( ZeroBasedLine );

   // Finally make sure the control is focused!
   if ( view )
      view->SetCtrlFocused();

   return view;
}

ScriptView* MainFrame::GetActiveView() const
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );
   return wxDynamicCast( docMan->GetCurrentView(), ScriptView );
}

void MainFrame::OnDropFiles( wxDropFilesEvent& event )
{
   wxString msg = "The following files were unrecongnized and not opened:\n";
   bool unopened = false;
   for ( int i=0; i < event.GetNumberOfFiles(); i++ ) {

      wxFileName file = event.GetFiles()[ i ];
      if (  tsGetPrefs().IsScriptFile( file.GetFullPath() ) ||
            tsGetPrefs().IsTextFile( file.GetFullPath() ) )
         OpenFile( file.GetFullPath() );
      else {
         msg += "\n\t" + file.GetFullName();
         unopened = true;
      }
   }

   if ( unopened ) {

      wxMessageDialog dlg( this, msg, "Drop Files", wxOK | wxICON_WARNING );
      dlg.ShowModal();
   }
}

void MainFrame::DoMenuUpdates(wxMenu* menu)
{
   // MSW wxMDIParentFrame::DoMenuUpdates() does something odd.  It lets
   // the active wxMDIChildFrame process the menu updates.  This was
   // causing it to miss handlers in the frame, project and script views.
   // 
   // So skip that crap...
   wxFrameBase::DoMenuUpdates(menu);
}

bool MainFrame::ProcessEvent(wxEvent& event)
{
   // HACK:  In order to override the behavior
   // in wxDocManager for close and close all
   // we're forcing those events to go thru us
   // only and not to the doc manager.
   //
   // TODO: Make the overloaded DocManager aware
   // of the ProjectDoc and let it deal with project
   // issues.
   //
   if (  event.GetId() == wxID_CLOSE ||
         event.GetId() == wxID_CLOSE_ALL ) 
   {
      return wxEvtHandler::ProcessEvent( event );
   }

   wxASSERT( m_docManager );
   
   // HACK: Find commands can be executed when the script view
   // isn't active.... so activate it before the event is processed.
   if (  ScriptFrame::GetFrame() &&
         event.GetEventType() != wxEVT_UPDATE_UI &&
         event.GetEventType() != wxEVT_MENU_HIGHLIGHT &&
         (  event.GetId() == tsID_FINDNEXT ||             
            event.GetId() == tsID_FINDPREV ||
            event.GetEventType() == wxEVT_COMMAND_FIND ||
            event.GetEventType() == wxEVT_COMMAND_FIND_NEXT ||
            event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE ||
            event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL ||
            (  event.GetId() == tsID_FINDCOMBO && 
               event.GetEventType() == wxEVT_COMMAND_TEXT_ENTER ) ) )
   {
      ScriptView* view = ScriptFrame::GetFrame()->GetSelectedView();
      wxASSERT( view );
      if ( m_docManager->GetCurrentView() != view )
         view->Activate( true );
   }

   // TODO: Once we switch to a true docking MDI framework
   // like wxAUI then we won't need to do special cases for
   // forwarding messages around to other windows.

   // First let the active view handle the message.
   wxView* view = m_docManager->GetCurrentView();
   if (  view && 
         view->ProcessEvent(event) )
      return true;

   // If we've got a copy command...  check to see if the find
   // or output control has focus.
   if ( event.GetId() == wxID_COPY )
   {
      if (  GetOutputPanel() && 
            GetOutputPanel()->m_Output == wxWindow::FindFocus() &&
            GetOutputPanel()->m_Output->ProcessEvent( event ) )
         return true;

      if (  GetFindWindow() && 
            GetFindWindow() == wxWindow::FindFocus() &&
            GetFindWindow()->ProcessEvent( event ) )
         return true;
   }

   // If the view didn't handle it then let the project try.
   if ( m_ProjectDoc )
   {
      wxView* projectView = m_ProjectDoc->GetFirstView();
      if (  projectView && 
            projectView != view &&
            projectView->ProcessEvent( event ) )
         return true;
   }

   // Neither the project or the active view handled
   // the message... let the doc manager get a crack.
   if ( m_docManager->ProcessEvent( event ) )
      return true;

   // SKip the wxDocMDIParentFrame handling as we've done it all above.
   return wxEvtHandler::ProcessEvent(event);
}

bool MainFrame::MSWTranslateMessage(WXMSG* msg)
{
   // This keeps the accellerators from fucking up
   // cut and paste in text based controls.
   if (  ( msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN ) ||
         ( msg->message == WM_KEYDOWN && msg->wParam == 'X' && msg->wParam & VK_CONTROL ) ||
         ( msg->message == WM_KEYDOWN && msg->wParam == 'C' && msg->wParam & VK_CONTROL ) ||
         ( msg->message == WM_KEYDOWN && msg->wParam == 'V' && msg->wParam & VK_CONTROL ) ||
         ( msg->message == WM_KEYDOWN && msg->wParam == 'A' && msg->wParam & VK_CONTROL ) ||
         ( msg->message == WM_KEYDOWN && msg->wParam == VK_DELETE ) ) 
   {
      wxWindow* focused = FindFocus();
      wxASSERT( focused );
      if (  focused->IsKindOf( CLASSINFO(wxComboBox) ) ||
            focused->IsKindOf( CLASSINFO(wxListCtrl) ) ||
            focused->IsKindOf( CLASSINFO(wxTextCtrl) ) )
      {
         return false;  
      }
   }
   
   return wxDocMDIParentFrame::MSWTranslateMessage( msg );
}

bool MainFrame::MSWProcessMessage( WXMSG* msg )
{
   /*
   if ( msg->message == WM_COPYDATA ) {
      
      COPYDATASTRUCT* cds = (COPYDATASTRUCT*)msg->lParam;
      if ( cds->dwData == COPYDATA_OPEN ) {

         wxString Files( (wxChar*)cds->lpData, cds->cbData );
      	wxStringTokenizer Tokenizer( Files, "\n", wxTOKEN_STRTOK );
         while( Tokenizer.HasMoreTokens() ) {
            wxString File = Tokenizer.GetNextToken();
            OpenFile( File );
         }
      }
   }
   */

   return wxDocMDIParentFrame::MSWProcessMessage( msg );
}

ProjectView* MainFrame::GetProjectView()
{
   if ( m_ProjectDoc ) {
      return (ProjectView*)m_ProjectDoc->GetFirstView();
   }
   return NULL; 
}

void MainFrame::OnFindComboChanged( wxCommandEvent& event )
{
   m_FindData.SetFindString( event.GetString() );
   
   /*
   wxASSERT( ScriptFrame::GetFrame() );
   wxASSERT( ScriptFrame::GetFrame()->GetSelectedView() );
   ScriptFrame::GetFrame()->GetSelectedView()->ClearSelectedText();
   */
}

void MainFrame::OnNewBreakpoint( wxCommandEvent& event )
{
	wxASSERT( GetProjectDoc() );
   
   Breakpoint bp( wxEmptyString, 1, 0, wxEmptyString );

   BreakpointPropertyDlg dlg;
   dlg.Create( this, &bp );
   if ( dlg.ShowModal() == wxID_OK )
      GetProjectDoc()->AddBreakpoint( bp.GetFile(), bp.GetLine(), bp.GetPass(), bp.GetCondition(), true );
}

void MainFrame::OnClearAllBreakpoints( wxCommandEvent& event )
{
	wxASSERT( GetProjectDoc() );
   GetProjectDoc()->DeleteAllBreakpoints();
}

void MainFrame::OnDisableAllBreakpoints( wxCommandEvent& event )
{
	wxASSERT( GetProjectDoc() );
   GetProjectDoc()->EnableAllBreakpoints( false );
}

void MainFrame::OnBreakpointUpdateUI( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetDebugger() );

   if ( event.GetId() == tsID_DEBUG_NEWBREAKPOINT ) {
      event.Enable( GetProjectDoc() != NULL );
      return;
   }
   
   event.Enable( GetProjectDoc() && GetProjectDoc()->GetBreakpoints().GetCount() > 0 );
}

void MainFrame::OnUpdateBookmarks( wxUpdateUIEvent& event )
{
   if ( m_ProjectDoc )
      event.Skip();
	
   event.Enable( false );
}

void MainFrame::UpdateDebugMenu()
{
   wxASSERT( m_DebugMenu );
   wxASSERT( GetToolBar() );
   wxASSERT( tsGetDebugger() );

   bool Realize = false;

   if ( tsGetDebugger()->IsAtBreakpoint() ) {

      m_DebugMenu->SetLabel( tsID_DEBUG_START, _T( "&Start" ) );
      m_DebugMenu->SetLabel( tsID_DEBUG_BREAK, _T( "&Continue\tF5" ) );

      if ( m_StartToolButton->GetLabel() == "Break" ) {

         Realize = true;
         m_StartToolButton->SetNormalBitmap( ts_start16 );
      }

      m_StartToolButton->SetLabel( _T( "Continue" ) );
      m_StartToolButton->SetShortHelp( _T( "Continue (F5)" ) );

   } else {

      if (  tsGetDebugger()->IsRunning() ) {

         if ( m_StartToolButton->GetLabel() != "Break" ) {

            Realize = true;
            m_StartToolButton->SetNormalBitmap( ts_break16 );
         }

         m_StartToolButton->SetLabel( _T( "Break" ) );
         m_StartToolButton->SetShortHelp( _T( "Break (Ctrl+Alt+Break)" ) );
         
      } else {

         if ( m_StartToolButton->GetLabel() == "Break" ) {
            Realize = true;
            m_StartToolButton->SetNormalBitmap( ts_start16 );
         }

         m_StartToolButton->SetLabel( _T( "Start" ) );
         m_StartToolButton->SetShortHelp( _T( "Start (F5)" ) );
      }

      m_DebugMenu->SetLabel( tsID_DEBUG_START, _T( "&Start\tF5" ) );
      m_DebugMenu->SetLabel( tsID_DEBUG_BREAK, _T( "&Break\tCtrl+Alt+Break" ) );
   }

   if ( Realize )
      GetToolBar()->Realize();

   // Now update the accel table.
   GetMenuBar()->RebuildAccelTable();
   SetAcceleratorTable( GetMenuBar()->GetAccelTable() );
}

void MainFrame::OnDebugStartBreak( wxCommandEvent& event )
{
   if ( tsGetDebugger()->IsAtBreakpoint() || tsGetDebugger()->IsRunning() )
      OnDebugBreak( event );
   else {
      OnDebugStart( event );
   }
}

void MainFrame::OnDebugStart( wxCommandEvent& event )
{
   wxASSERT( GetProjectDoc() );
   wxASSERT( m_ConfigComboBox );

   wxString configName = m_ConfigComboBox->GetValue();
   const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );

   StartDebug( "CONTINUE", config && config->Precompile() );
}

void MainFrame::StartDebug( const wxString& command, bool doPrecompile )
{
	wxASSERT( tsGetDebugger() );
	wxASSERT( !tsGetDebugger()->IsRunning() );

	wxASSERT( GetProjectDoc() );
	
   // Do we need to precompile the scripts?
   if ( doPrecompile )
   {
      // Fire off the precompile which when finished will
      // continue with the debug session.
      m_DebugCommand = command;
      m_AfterPrecompile = START_DEBUG;
      DoPrecompile( tsID_PROJECT_PRECOMPILE );
      return;
   }

   wxASSERT( m_OutputPanel );
	m_OutputPanel->Clear();

   // Save any changed files... 
   // TODO: We should ask and save in project options.
   OnSaveAll( wxCommandEvent() );

   // Launch the debugger.
   if ( !tsGetDebugger()->Start( m_ConfigComboBox->GetValue(), command ) ) {

        // TODO: Implement error return from debugger and do message.
	}

   UpdateDebugMenu();
	UpdateTitle();
}

void MainFrame::OnDebugStartWithoutDebugging( wxCommandEvent& event )
{
	wxASSERT( GetProjectDoc() );
   wxASSERT( m_ConfigComboBox );

   wxString configName = m_ConfigComboBox->GetValue();
   const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );

   StartRun( config && config->Precompile() );
}

void MainFrame::StartRun( bool doPrecompile )
{
	wxASSERT( tsGetDebugger() );
	wxASSERT( !tsGetDebugger()->IsRunning() );

   // Do we need to precompile the scripts?
   if ( doPrecompile )
   {
      // Fire off the precompile which when finished will
      // continue with the debug session.
      m_AfterPrecompile = START_RUN;
      DoPrecompile( tsID_PROJECT_PRECOMPILE );
      return;
   }

   // Save any changed files... 
   // TODO: We should ask and save in project options.
   OnSaveAll( wxCommandEvent() );

   ProjectDoc* project = GetProjectDoc();
   wxASSERT( project );

   const ProjectConfig* config = project->GetConfig( m_ConfigComboBox->GetValue() );
   wxASSERT( config );

   // Grab the cwd, exe, and arguments for launching the executable.
   wxString cwd   = project->GetWorkingDir();
	wxString exe   = config->GetExe();
   wxString args  = config->GetArgs();

   // Launch the executable!
   if ( !tsShellExecute::execDeatched( "open", cwd, exe, args, tsSW_SHOWNORMAL ) )
   {
		wxMessageDialog MessageBox( this, "Failed launching executable!", "Error", wxOK );
		MessageBox.ShowModal();
	}
}

void MainFrame::OnDebugConnect( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );
	wxASSERT( !tsGetDebugger()->IsRunning() );
	
	wxASSERT( GetProjectDoc() );
	
   wxASSERT( m_OutputPanel );
	m_OutputPanel->Clear();

   // Launch the debugger.
	if ( !tsGetDebugger()->Connect(  GetProjectDoc()->GetAddress(), 
                                    GetProjectDoc()->GetPort(), 
                                    GetProjectDoc()->GetPassword(), 
                                    wxEmptyString ) ) 
   {
         // TODO: Implement error return from debugger and do message.
	}

   UpdateDebugMenu();
	UpdateTitle();

}

void MainFrame::OnDebugStop( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );
   if ( tsGetDebugger()->IsRunning() )
   {
	   tsGetDebugger()->Stop();
      UpdateDebugMenu();
	   UpdateTitle();
   }
}

void MainFrame::OnDebugRestart( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );
   if ( tsGetDebugger()->IsRunning() )
   {
	   tsGetDebugger()->Stop();
      UpdateDebugMenu();
	   UpdateTitle();

      OnDebugStart( event );
   }
}

void MainFrame::OnDebugBreak( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );
	wxASSERT( tsGetDebugger()->IsRunning() );

	if ( tsGetDebugger()->IsAtBreakpoint() ) {
		tsGetDebugger()->Continue();
	} else {
		tsGetDebugger()->Break();
	}
   UpdateDebugMenu();
	UpdateTitle();
}

void MainFrame::OnDebugStep( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );

   if ( !tsGetDebugger()->IsRunning() ) {

      wxASSERT( GetProjectDoc() );
      wxASSERT( m_ConfigComboBox );

      wxString configName = m_ConfigComboBox->GetValue();
      const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );

      StartDebug( "STEPIN", config && config->Precompile() );
      return;
   }

   wxASSERT( tsGetDebugger()->IsAtBreakpoint() );
	tsGetDebugger()->Step();

   UpdateDebugMenu();
   UpdateTitle();
}

void MainFrame::OnDebugStepOver( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );

   if ( !tsGetDebugger()->IsRunning() ) {

      wxASSERT( GetProjectDoc() );
      wxASSERT( m_ConfigComboBox );

      wxString configName = m_ConfigComboBox->GetValue();
      const ProjectConfig* config = GetProjectDoc()->GetConfig( configName );

      StartDebug( "STEPIN", config && config->Precompile() );
      return;
   }

   wxASSERT( tsGetDebugger()->IsAtBreakpoint() );
   tsGetDebugger()->StepOver();

   UpdateDebugMenu();
   UpdateTitle();
}

void MainFrame::OnDebugStepOut( wxCommandEvent& event )
{
	wxASSERT( tsGetDebugger() );
	wxASSERT( tsGetDebugger()->IsRunning() );
	wxASSERT( tsGetDebugger()->IsAtBreakpoint() );
	tsGetDebugger()->StepOut();
   UpdateDebugMenu();
}

void MainFrame::OnDebugUpdateUI( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetDebugger() );

   if ( !GetProjectDoc() || m_PreCompiler )
   {
      event.Enable( false );
      return;
   }

   if (  event.GetId() == tsID_DEBUG_STARTBREAK )
   {
      event.Enable( true );
   }
   else if (   event.GetId() == tsID_DEBUG_START || 
               event.GetId() == tsID_DEBUG_START_WITHOUT_DEBUGGING ||
               event.GetId() == tsID_DEBUG_CONNECT ) 
   {
		event.Enable( !tsGetDebugger()->IsRunning() );
	} 
   else if (   event.GetId() == tsID_DEBUG_STOP || 
               event.GetId() == tsID_DEBUG_RESTART ||
               event.GetId() == tsID_DEBUG_BREAK ) 
   {
      event.Enable( tsGetDebugger()->IsRunning() );
   } 
   else if (   event.GetId() == tsID_DEBUG_STEP ||
               event.GetId() == tsID_DEBUG_STEP_OVER ) 
   {
		event.Enable( !tsGetDebugger()->IsRunning() || tsGetDebugger()->IsAtBreakpoint() );
   } 
   else if ( event.GetId() == tsID_DEBUG_STEP_OUT ) 
   {
		event.Enable( tsGetDebugger()->IsRunning() && tsGetDebugger()->IsAtBreakpoint() );
	}
}

void MainFrame::OnDebuggerStop()
{
	ClearBreaklines();

   wxASSERT( m_CallStack );
	m_CallStack->DeleteAllItems();

	// Update the locals window.
	//wxASSERT( m_LocalsWindow );
   //m_LocalsWindow->ClearAll();

	// Update the globals window.
	wxASSERT( m_WatchWindow );
	m_WatchWindow->UpdateWatch();

   UpdateDebugMenu();
	UpdateTitle();
}

void MainFrame::OnReloadScripts( wxCommandEvent& event )
{
   wxASSERT( tsGetDebugger() );

   ScriptDocArray scripts;
   GetChangedScripts( scripts );
   
   wxArrayString files;
   for ( int i=0; i < scripts.GetCount(); i++ )
   {
      if ( !scripts[i]->Save() )
         return;

      files.Add( scripts[i]->GetFilename() );
   }

   if ( files.GetCount() > 0 )
      tsGetDebugger()->ReloadScripts( files );
}

void MainFrame::OnUpdateReloadScripts( wxUpdateUIEvent& event )
{
   wxASSERT( tsGetDebugger() );
   ScriptDocArray scripts;
   event.Enable( tsGetDebugger()->IsRunning() && GetChangedScripts( scripts ) > 0 );
}

void MainFrame::SetCallStack( const FunctionCallArray& CallStack, int Level )
{
	wxASSERT( CallStack.GetCount() > 0 );

	wxASSERT( m_CallStack );
	m_CallStack->Freeze();
	long Sel = m_CallStack->GetFirstSelected();
	m_CallStack->DeleteAllItems();
	for ( int i=0; i < CallStack.GetCount(); i++ ) {
		
		wxASSERT( CallStack[i] );

		long Item = m_CallStack->InsertItem( m_CallStack->GetItemCount(), CallStack[i]->GetFunction() );
		m_CallStack->SetItem( Item, 1, CallStack[i]->GetFile() );
		wxString Line;
		Line << CallStack[i]->GetLine();
		m_CallStack->SetItem( Item, 2, Line );
	}
	m_CallStack->Select( Sel );
	m_CallStack->Thaw();

	// Update the locals window.
   //wxASSERT( m_LocalsWindow );
	//m_LocalsWindow->SetFunctionCall( CallStack[Level] );

	// Update the globals window.
	wxASSERT( m_WatchWindow );
	m_WatchWindow->UpdateWatch();

   UpdateDebugMenu();
   UpdateTitle();

	SetBreakline( CallStack[Level]->GetFile(), CallStack[Level]->GetLine() );
}


void MainFrame::ClearBreaklines()
{
   wxList& docs = GetDocumentManager()->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while ( node ) {

      ScriptDoc* doc = wxDynamicCast( node->GetData(), ScriptDoc );
      if ( doc ) {
         
         wxASSERT( wxDynamicCast( doc->GetFirstView(), ScriptView ) );
         ScriptView* view = (ScriptView*)doc->GetFirstView();
         view->ClearBreakline();
      }

      node = node->GetNext();
   }

   UpdateDebugMenu();
   UpdateTitle();
}

void MainFrame::SetBreakline( const wxString& Filename, int Line )
{
   // We shouldn't have any, but clear any existing breaklines.
	ClearBreaklines();

   // Bring us to the forground.
	Raise();

	// Well try just opening it up... it will return an existing view.
	ScriptView* view = OpenFile( Filename );

   // If we got a frame activate and set the breakline!
	if ( view ) {

		view->SetBreakline( Line );

   } else {

      // TODO: If we couldn't find the file we gotta do 
      // something... but what?  Show disassembly?

   }
}

void MainFrame::UpdateTitle()
{
	wxString Title;

   // Add the project title!
	ProjectDoc* Project = GetProjectDoc();
	if ( Project ) {
  		Title << Project->GetName() << " - Torsion";
	}
   else
   {
      Title << "Torsion";
   }

   // Add the mode.
	if ( Project ) {

      wxASSERT( tsGetDebugger() );

		if ( tsGetDebugger()->IsAtBreakpoint() ) {

			Title << " [breakpoint]";

		} else if ( tsGetDebugger()->IsRunning() ) {
			
			Title << " [running]";

		} else {

			//Title << " [stopped]";
		}
	}

	SetTitle( Title );
}

void MainFrame::OnCallStackActivated( wxListEvent& event )
{
   if ( !tsGetDebugger()->IsRunning() )
      return;

   if ( tsGetDebugger()->IsAtBreakpoint() ) {
		tsGetDebugger()->SetStackLevel( event.GetIndex() );
	}
}

void MainFrame::OnHelp( wxHelpEvent& event )
{
   wxString text = GetSelectedText( true );
   if ( text.IsEmpty() )
      m_HelpController.DisplayContents();

   wxURI uri;
   if ( GetProjectDoc() )
      uri = GetProjectDoc()->GetSearchUrl( text );
   else
      uri = ProjectDoc::GetDefaultSearchUrl( text );

   wxString query( uri.BuildURI() );
   if ( query.IsEmpty() )
      wxBell();
   else
      wxLaunchDefaultBrowser( uri.BuildURI() );
}

void MainFrame::OnHelp( wxCommandEvent& event )
{
   m_HelpController.DisplayContents();
}

void MainFrame::OnUpdateCheck( wxCommandEvent& event )
{
   wxGetApp().CheckForUpdate( true );
}

void MainFrame::OnBugTracker( wxCommandEvent& event )
{
   if ( event.GetId() == tsID_FEATUREREQUEST )
      wxLaunchDefaultBrowser( "http://mantis.sickheadgames.com/bug_report_page.php?project_id=6" );
   else
      wxLaunchDefaultBrowser( "http://mantis.sickheadgames.com/bug_report_page.php?project_id=2" );
}

void MainFrame::OnDebugCallTip( const wxString& Expression, const wxString& Value )
{
   ScriptView* view = ScriptFrame::GetFrame()->GetSelectedView();
   if ( !view )
      return;

   view->OnDebugCallTip( Expression, Value );
}

int MainFrame::GetChangedScripts( ScriptDocArray& scripts )
{
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );

   wxList& docs = docMan->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while (node)
   {
      ScriptDoc *doc = wxDynamicCast( node->GetData(), ScriptDoc );
      wxList::compatibility_iterator next = node->GetNext();

      if (doc && doc->IsModified() && !doc->GetFilename().IsEmpty() )
         scripts.Add( doc );

      node = next;
   }

   return scripts.GetCount();
}

bool MainFrame::SetActiveConfig( const wxString& config )
{
   wxASSERT( m_ConfigComboBox );

   int sel = m_ConfigComboBox->FindString( config );
   if ( sel != wxNOT_FOUND )
   {
      m_ConfigComboBox->SetSelection( sel );
      return true;
   }

   return false;
}

wxString MainFrame::GetActiveConfigName() const
{
   wxASSERT( m_ConfigComboBox );
   return m_ConfigComboBox->GetStringSelection();
}

void MainFrame::OnSysColourChanged( wxSysColourChangedEvent& event )
{
   // The menus need to know when color changes
   // occur, but they are not real windows... so
   // we have to pass the event to them.
   tsMenu::OnSysColourChanged();

   // Let the base handle it as well.
   event.Skip();
}

void MainFrame::SendHintToAllViews( wxObject* hint, bool scriptViewsOnly )
{
   wxASSERT( GetDocumentManager() );

   wxList& docs = GetDocumentManager()->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while ( node )
   {
      wxDocument* doc = (wxDocument*)node->GetData();
      wxASSERT( doc );
      node = node->GetNext();

      if ( scriptViewsOnly && !wxDynamicCast( doc, ScriptDoc ) )
         continue;

      doc->UpdateAllViews( NULL, hint );
   }
}

