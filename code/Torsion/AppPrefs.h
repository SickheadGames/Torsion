// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_APPPREFS_H
#define TORSION_APPPREFS_H
#pragma once

#include "ToolCommand.h"


class wxFileHistory;


class AppPrefs
{
public:
    
	AppPrefs();

	bool Load( const wxString& Path );
	bool Save( const wxString& Path );
	bool SaveIfDirty( const wxString& Path );

	const bool IsDirty() const { return m_bDirty; }
	
	const wxFont& GetDefaultFont() const { return m_DefaultFont; }
   void SetDefaultFont( const wxFont& font ) { SetAndDirtyProp( m_DefaultFont, font ); }

	const wxString& GetReservedWords() const		   { return m_Reserved; }

	const wxColour& GetDefaultColor() const { return m_DefaultColor; }
   void SetDefaultColor( const wxColour& color ) { SetAndDirtyProp( m_DefaultColor, color ); }

   const wxColour& GetBgColor() const { return m_BgColor; }
   void SetBgColor( const wxColour& color ) { SetAndDirtyProp( m_BgColor, color ); }

   const wxColour& GetCommentColor() const { return m_CommentColor; }
   void SetCommentColor( const wxColour& color ) { SetAndDirtyProp( m_CommentColor, color ); }

	const wxColour&   GetReservedColor() const { return m_ReservedColor; }
   void SetReservedColor( const wxColour& color ) { SetAndDirtyProp( m_ReservedColor, color ); }

	const wxColour& GetExportsColor() const { return m_ExportsColor; }
   void SetExportsColor( const wxColour& color ) { SetAndDirtyProp( m_ExportsColor, color ); }
 
	const wxColour& GetNumberColor() const { return m_NumberColor; }
   void SetNumberColor( const wxColour& color ) { SetAndDirtyProp( m_NumberColor, color ); }

	const wxColour& GetStringColor() const { return m_StringColor; }
   void SetStringColor( const wxColour& color ) { SetAndDirtyProp( m_StringColor, color ); }

   const wxColour& GetLocalsColor() const { return m_LocalsColor; }
   void SetLocalsColor( const wxColour& color ) { SetAndDirtyProp( m_LocalsColor, color ); }

   const wxColour& GetGlobalsColor() const { return m_GlobalsColor; }
   void SetGlobalsColor( const wxColour& color ) { SetAndDirtyProp( m_GlobalsColor, color ); }

	const wxColour& GetOperatorsColor() const { return m_OperatorsColor; }
   void SetOperatorsColor( const wxColour& color ) { SetAndDirtyProp( m_OperatorsColor, color ); }

   const wxColour& GetMarginColor() const { return m_MarginColor; }
   void SetMarginColor( const wxColour& color ) { SetAndDirtyProp( m_MarginColor, color ); }

   const wxColour& GetMarginTextColor() const { return m_MarginTextColor; }
   void SetMarginTextColor( const wxColour& color ) { SetAndDirtyProp( m_MarginTextColor, color ); }

   const wxColour& GetSelColor() const { return m_SelColor; }
   void SetSelColor( const wxColour& color ) { SetAndDirtyProp( m_SelColor, color ); }

   const wxColour& GetSelBgColor() const { return m_SelBgColor; }
   void SetSelBgColor( const wxColour& color ) { SetAndDirtyProp( m_SelBgColor, color ); }

   const wxColour& GetBraceMatchColor() const { return m_BraceMatchColor; }
   void SetBraceMatchColor( const wxColour& color ) { SetAndDirtyProp( m_BraceMatchColor, color ); }
   const wxColour& GetBraceMatchBgColor() const { return m_BraceMatchBgColor; }
   void SetBraceMatchBgColor( const wxColour& color ) { SetAndDirtyProp( m_BraceMatchBgColor, color ); }
   const wxColour& GetBraceMatchErrColor() const { return m_BraceMatchErrColor; }
   void SetBraceMatchErrColor( const wxColour& color ) { SetAndDirtyProp( m_BraceMatchErrColor, color ); }

   const wxColour& GetCalltipColor() const { return m_CalltipColor; }
   void SetCalltipColor( const wxColour& color ) { SetAndDirtyProp( m_CalltipColor, color ); }
   const wxColour& GetCalltipBgColor() const { return m_CalltipBgColor; }
   void SetCalltipBgColor( const wxColour& color ) { SetAndDirtyProp( m_CalltipBgColor, color ); }
   const wxColour& GetCalltipHiColor() const { return m_CalltipHiColor; }
   void SetCalltipHiColor( const wxColour& color ) { SetAndDirtyProp( m_CalltipHiColor, color ); }

   bool GetTabsAndSpaces() const { return m_TabsAndSpaces; }
   void ShowTabsAndSpaces( bool bShow ) { SetAndDirtyProp( m_TabsAndSpaces, bShow ); }

   bool GetLineBreaks() const { return m_LineBreaks; }
   void ShowLineBreaks( bool bShow ) { SetAndDirtyProp( m_LineBreaks, bShow ); }

   bool GetLineNumbers() const { return m_LineNumbers; }
   void ShowLineNumbers( bool bShow ) { SetAndDirtyProp( m_LineNumbers, bShow ); }

   bool GetUseTabs() const { return m_UseTabs; }
   void SetUseTabs( bool tabs ) { SetAndDirtyProp( m_UseTabs, tabs ); }

   int GetTabWidth() const { return m_TabWidth; }
   void SetTabWidth( int width ) { SetAndDirtyProp( m_TabWidth, width ); }

   bool GetLineWrap() const { return m_LineWrap; }
   void SetLineWrap( bool Wrap ) { SetAndDirtyProp( m_LineWrap, Wrap ); }

   bool GetEdgeMarker() const { return m_UseEdgeMarker; }
   void SetEdgeMarker( bool enabled ) { SetAndDirtyProp( m_UseEdgeMarker, enabled ); }
   int GetEdgeMarkerCol() const { return m_EdgeColumn; }
   void SetEdgeMarkerCol( int col ) { SetAndDirtyProp( m_EdgeColumn, col ); }
   const wxColour& GetEdgeMarkerColor() const { return m_EdgeMarkerColor; }
   void SetEdgeMarkerColor( const wxColour& color ) { SetAndDirtyProp( m_EdgeMarkerColor, color ); }
 
   bool GetCodeFolding() const { return m_CodeFolding; }
   void SetCodeFolding( bool folding ) { SetAndDirtyProp( m_CodeFolding, folding ); }

	const wxColour& GetFoldingColor() const { return m_CodeFoldingColor; }
   void SetFoldingColor( const wxColour& color ) { SetAndDirtyProp( m_CodeFoldingColor, color ); }

   bool GetCodeCompletion() const { return m_CodeCompletion; }
   void SetCodeCompletion( bool enabled ) { SetAndDirtyProp( m_CodeCompletion, enabled ); }

   bool GetEnhancedCompletion() const { return m_EnhancedCompletion; }
   void SetEnhancedCompletion( bool enabled ) { SetAndDirtyProp( m_EnhancedCompletion, enabled ); }

   const wxRect& GetPosition() const { return m_Position; }
   void SetPosition( const wxRect& pos ) { SetAndDirtyProp( m_Position, pos ); }

   bool GetMaximized() const { return m_Maximized; }
   void SetMaximized( bool value ) { SetAndDirtyProp( m_Maximized, value ); }

   bool  GetFindMatchCase() const { return m_FindMatchCase; }
   void  SetFindMatchCase( bool value ) { SetAndDirtyProp( m_FindMatchCase, value ); }

   bool  GetFindMatchWord() const { return m_FindMatchWord; }
   void  SetFindMatchWord( bool value ) { SetAndDirtyProp( m_FindMatchWord, value ); }

   bool  GetFindSearchUp() const { return m_FindSearchUp; }
   void  SetFindSearchUp( bool value ) { SetAndDirtyProp( m_FindSearchUp, value ); }

   bool  GetCheckForUpdates() const { return m_CheckForUpdates; }
   void  SetCheckForUpdates( bool value ) { SetAndDirtyProp( m_CheckForUpdates, value ); }

   bool  GetShowAllMods() const { return m_ShowAllMods; }
   void  SetShowAllMods( bool value ) { SetAndDirtyProp( m_ShowAllMods, value ); }

   bool  GetShowAllFiles() const { return m_ShowAllFiles; }
   void  SetShowAllFiles( bool value ) { SetAndDirtyProp( m_ShowAllFiles, value ); }

   bool  GetSyncProjectTree() const { return m_SyncProjectTree; }
   void  SetSyncProjectTree( bool value ) { SetAndDirtyProp( m_SyncProjectTree, value ); }

   int  GetProjectSashWidth() const { return m_ProjectSashWidth; }
   void  SetProjectSashWidth( int value ) { SetAndDirtyProp( m_ProjectSashWidth, value ); }

   int  GetBottomSashHeight() const { return m_BottomSashHeight; }
   void  SetBottomSashHeight( int value ) { SetAndDirtyProp( m_BottomSashHeight, value ); }

   bool GetLoadLastProject() const { return m_LoadLastProject; }
   void SetLoadLastProject( bool value ) { SetAndDirtyProp( m_LoadLastProject, value ); }

   const wxString& GetLastProject() const { return m_LastProject; }
   void  SetLastProject( const wxString& value ) { SetAndDirtyProp( m_LastProject, value ); }

   //
   // The more complex settings.
   // 

   void GetFileHistory( wxFileHistory* history ) const;
   void SetFileHistory( wxFileHistory* history );

   void GetProjectHistory( wxFileHistory* history ) const;
   void SetProjectHistory( wxFileHistory* history );

   const wxArrayString& GetScriptExtensions() const { return m_ScriptExts; }
   wxString GetDefaultScriptExtension() const;
   wxString GetScriptExtsString() const;
   bool SetScriptExtsString( const wxString& exts );

   const wxArrayString& GetDSOExts() const { return m_DSOExts; }
   wxString GetDSOExtsString() const;
   bool SetDSOExtsString( const wxString& exts );
   bool IsDSOExt( const wxString& ext ) const;
   bool IsDSO( const wxString& file ) const;
   wxString GetDSOForScript( const wxString& script ) const;

   bool IsExcludedFile( const wxString& name ) const;
   bool IsExcludedFolder( const wxString& name ) const;
   const wxArrayString& GetExcludedFolders() const { return m_ExcludedFolders; }
   const wxArrayString& GetExcludedFiles() const { return m_ExcludedFiles; }
   bool SetExcludedFiles( const wxString& exclude );
   wxString GetExcludedFilesString() const;
   bool SetExcludedFolders( const wxString& exclude );
   wxString GetExcludedFoldersString() const;

   static int  GetStringsFromCombo( wxComboBox* combo, wxArrayString& output );
   static void AddStringsToCombo( wxComboBox* combo, const wxArrayString& output );

   void  SetFindStrings( const wxArrayString& strings );
   const wxArrayString& GetFindStrings() const { return m_FindStrings; }
   void  SetFindTypes( const wxArrayString& types );
   const wxArrayString& GetFindTypes() const { return m_FindTypes; }
   void  SetFindPaths( const wxArrayString& paths );
   const wxArrayString& GetFindPaths() const { return m_FindPaths; }
   void  SetFindPos( const wxPoint& point );
   const wxPoint& GetFindPos() const { return m_FindPos; }

   void  SetFindSymbols( const wxArrayString& strings );
   const wxArrayString& GetFindSymbols() const { return m_FindSymbols; }

   static void FixupFindPaths( wxArrayString& paths );

   bool IsScriptFile( const wxString& file ) const;
   bool IsTextFile( const wxString& file ) const;

   const ToolCommandArray& GetToolCommands() const { return m_ToolCommands; }
   bool  SetToolCommands( const ToolCommandArray& cmds );

protected:

   template <class T> void SetAndDirtyProp( T& old, const T& new_ )
   {
      if ( new_ != old )
      {
         old = new_;
         m_bDirty = true;
      }
   }

   void LoadFromString( const wxChar* Buffer );

   static const wxChar* s_ReservedWords;
   static const wxChar* s_LibraryWords;
   static const wxChar* s_GlobalWords;

	static wxString	ColorToString( const wxColour& Color );
	static wxColour	StringToColor( const wxString& Color );
	static wxString	RectToString( const wxRect& Rect );
   static wxRect     StringToRect( const wxString& Rect );

   static void ReformatWords( wxString& Words );

	bool		m_bDirty;

	wxFont	m_DefaultFont;

   wxColour m_BgColor;

   wxColour	m_DefaultColor;
	wxColour	m_CommentColor;
	wxColour	m_NumberColor;
	wxColour	m_StringColor;
	wxColour	m_OperatorsColor;
	wxColour	m_LocalsColor;
   wxColour m_GlobalsColor;

	wxColour	m_ReservedColor;
	wxColour	m_ExportsColor;

   wxColour	m_MarginColor;
   wxColour	m_MarginTextColor;

   wxColour	m_SelColor;
   wxColour	m_SelBgColor;

   wxColour	m_BraceMatchColor;
   wxColour	m_BraceMatchBgColor;
   wxColour	m_BraceMatchErrColor;

   wxColour	m_CalltipColor;
   wxColour	m_CalltipBgColor;
   wxColour	m_CalltipHiColor;

   bool     m_CodeFolding;
   wxColour	m_CodeFoldingColor;

   bool     m_CodeCompletion;
   bool     m_EnhancedCompletion;

	wxString	m_Reserved;

   wxArrayString m_DSOExts;
   wxArrayString m_ScriptExts;
   wxArrayString m_TextExts;
   wxArrayString m_ExcludedFiles;
   wxArrayString m_ExcludedFolders;

   wxArrayString  m_FindStrings;
   wxArrayString  m_FindTypes;
   wxArrayString  m_FindPaths;
   bool           m_FindMatchCase;
   bool           m_FindMatchWord;
   bool           m_FindSearchUp;
   wxPoint        m_FindPos;
   wxArrayString  m_FindSymbols;

   wxArrayString  m_FileHistory;
   wxArrayString  m_ProjectHistory;

   ToolCommandArray m_ToolCommands;

   wxRect         m_Position;
   bool           m_Maximized;
   int            m_ProjectSashWidth;
   int            m_BottomSashHeight;

   bool  m_UseTabs;
   int   m_TabWidth;

   bool  m_LineWrap;

   bool     m_LoadLastProject;
   wxString m_LastProject;

   bool     m_UseEdgeMarker;
   wxColour m_EdgeMarkerColor;
   int      m_EdgeColumn;

   bool m_ShowAllMods;
   bool m_ShowAllFiles;
   bool m_SyncProjectTree;

   bool m_TabsAndSpaces;
   bool m_LineBreaks;
   bool m_LineNumbers;

   bool m_CheckForUpdates;
};

// This is only used with UpdateAllViews to
// identifiy changes to the prefs need to be
// reloaded.
class tsPrefsUpdateHint : public wxObject 
{
   DECLARE_CLASS(tsPrefsUpdateHint)
};


#endif // TORSION_APPPREFS_H
