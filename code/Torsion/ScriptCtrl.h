// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCRIPTCTRL_H
#define TORSION_SCRIPTCTRL_H
#pragma once

#include <wx/stc/stc.h>
#include <wx/fdrepdlg.h>

#include "ScriptView.h"
#include "ScriptDoc.h"

class BreakpointEvent;

WX_DEFINE_ARRAY( wxStyledTextEvent*, wxStyledTextEventArray );


class ScriptCtrl : public wxStyledTextCtrl  
{
   public:

      ScriptCtrl();
      virtual ~ScriptCtrl();

      bool Create( wxWindow* parent, ScriptView* view, const wxPoint& pos, const wxSize& size );

      void OnNew();

      bool LoadFile( const wxString& filename );
      bool SaveFile( const wxString& filename );

      void OnKeyDown( wxKeyEvent& event );
      void OnKeyUp( wxKeyEvent& event );

      void OnFindNext( wxFindDialogEvent& event );
      void OnFindReplace( wxFindDialogEvent& event );
      void OnFindReplaceAll( wxFindDialogEvent& event );

      //void UpdateWindowUI(long flags = wxUPDATE_UI_NONE);

      void OnContextMenu( wxContextMenuEvent& event );

      void OnMarginClick( wxStyledTextEvent& event );
      void OnNeedUnfold( wxStyledTextEvent& event );

      void OnTextChanged( wxStyledTextEvent& event );
      void OnUpdateUI( wxStyledTextEvent& event );
      void OnChar( wxKeyEvent& event );
      void OnPosChanged( wxStyledTextEvent& event );

      //void OnAutoCompSel( wxStyledTextEvent& event );

      void OnHoverStart( wxStyledTextEvent& event );
      void OnHoverEnd( wxStyledTextEvent& event );

      void OnOpenExec( wxCommandEvent& event );
      void OnOpenDef( wxCommandEvent& event );
      void OnAddWatch( wxCommandEvent& event );
      void OnRemoveBreakpoint( wxCommandEvent& event );
      void OnToggleBreakpoint( wxCommandEvent& event );
      void OnInsertBreakpoint( wxCommandEvent& event );
      void OnBreakpointProperties( wxCommandEvent& event );

      void OnFolding( wxCommandEvent& event );
      void OnFoldingUpdateUI( wxUpdateUIEvent& event );

      void OnProperties( wxCommandEvent& event );

      void OnGoto(wxCommandEvent& event);

      void OnUpdateEnable( wxUpdateUIEvent& event );

      void SetLineVisible( int line );
      bool SetLineSelected( int line );

      void ClearBreakline();
      void SetBreakline( int Line );
      void UpdateBreakpoints();

      void UpdateBookmarks();

      void OnSetFocus( wxFocusEvent& event );
      void OnKillFocus( wxFocusEvent& event );

      void OnMouseWheel( wxMouseEvent& evt );
      void OnMouseLeftDown( wxMouseEvent& evt );
      //void OnMouseLeftUp( wxMouseEvent& evt );

      void OnDebugCallTip( const wxString& Expression, const wxString& Value );

      void OnBreakpointEvent( BreakpointEvent& event );

      void UpdatePrefs( bool refresh );

      void CommentSel( bool comment );
      bool MatchBrace();

      int GetLineMarginWidth();
      void ShowLineNumbers( bool enabled );

      void ShowCaretAtCurrentPosition();

      void Cut();

   protected:

      void OnListMembers( wxCommandEvent& event );
      bool ListMembers( bool forced );

      void OnParameterInfo( wxCommandEvent& event );
      bool ParameterInfo( bool forced );

      wxString GetLineIndentString( int line = -1 );
      wxChar GetLastNonWhitespaceChar( int pos = -1 ); 
      int FindBlockStart( int pos, wxChar blockStart, wxChar blockEnd, bool skipNested = true );

      void GetWordsAtPos( int pos, wxString* prevWord, wxString* sep, wxString* currWord );

      //int IdentifierStartPos( int pos );
      wxString GetIdentifierAt( int pos, bool* isFunction = NULL );

      //void OnInternalIdle();

      ScriptDoc*  GetDocument() const { return wxStaticCast( m_ScriptView->GetDocument(), ScriptDoc ); }

      bool HasWord( wxString word, const wxString& wordlist );

      ScriptView* m_ScriptView;

      wxString m_FilePath;

      wxString m_ExecPath;

      wxArrayString  m_DefPaths;
      wxArrayString  m_DefSymbols;
      wxArrayInt     m_DefLines;
      
      wxString m_WatchText;

      int   m_StartFindPos;
      int   m_LastFindPos;

      int   m_LastCallTipPos;

      bool  m_LastModify;
      bool  m_Loading;
      bool  m_DropInEditor;

      wxString m_AutoCompFillups;
      wxString m_AutoCompGlobalFillups;
      bool m_IsGlobalFullups;

      static const wxString sm_WordChars;

      wxTimer m_AutoCompTimer;
      void OnUpdateAutoCompText( wxTimerEvent& event );

      wxStyledTextEventArray  m_ScriptEvents;

      //int GetNearestLine( int Line );

      //void OnInternalIdle();

      void SetStyle( int Style, const wxColour& Fore, const wxColour& Back = wxColour(255,255,255) );

      bool InitializePrefs();

      //int FindBreakline( int Line );

      enum {
	      MARKER_BOOKMARK=1,
	      MARKER_BREAKPOINT,
	      MARKER_CONDITIONAL_BREAKPOINT,
	      MARKER_BREAKPOINT_DISABLED,
	      MARKER_BREAKLINE,
      };

      // The margins are set from 0 to 2... and 0
      // is always for line numbers.
      enum {
	      MARGIN_MARKERS = 0,
	      MARGIN_LINE_NUMBERS,
	      MARGIN_FOLDING,
      };

      // lanugage properties
      //LanguageInfo const* m_language;

      DECLARE_EVENT_TABLE()
};


#endif // TORSION_SCRIPTCTRL_H