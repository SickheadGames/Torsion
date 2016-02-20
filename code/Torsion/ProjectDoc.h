// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PROJECTDOC_H
#define TORSION_PROJECTDOC_H
#pragma once

#include "Breakpoint.h"
#include "Bookmark.h"
#include "ProjectConfig.h"


class FileInfo
{
public:
   wxString Name;
   wxPoint  Offset;
   int      Index;

   static int CmpIndex( FileInfo** f1, FileInfo** f2 )
   {
      return (*f1)->Index < (*f2)->Index ? -1 : (*f1)->Index > (*f2)->Index ? 1 : 0;
   }
};

WX_DECLARE_OBJARRAY(FileInfo, FileInfoArray);


class ProjectDoc : public wxDocument
{
   DECLARE_DYNAMIC_CLASS(ProjectDoc)

   public:

      ProjectDoc();
      virtual ~ProjectDoc();

      virtual bool OnSaveDocument( const wxString& filename );
      virtual bool OnOpenDocument( const wxString& filename );
      virtual bool OnNewDocument();

      void SaveOptions();
      void LoadOptions();

      virtual bool Close();

      void CopyDocument( const ProjectDoc* doc );

	   void SetName( const wxString& name );
	   const wxString& GetName() const { return m_Name; }
	   void SetWorkingDir( const wxString& dir );
      const wxString& GetWorkingDir() const { return m_WorkingDir; }

	   wxString GetExportsFilePath() const;

      const wxString&   GetAddress() const   { return m_Address; }
      const wxString&   GetPassword() const  { return m_Password; }
      int               GetPort() const      { return m_Port; }

      void SetAddress( const wxString& address ) { m_Address = address; }
      void SetPassword( const wxString& password ) { m_Password = password; }
      void SetPort( int port ) { m_Port = port; }

      void SetEntryScript( const wxString& script );
      const wxString& GetEntryScript() const { return m_EntryScript; }

      void SetDebugHook( const wxString& hook );
      const wxString& GetDebugHook() const { return m_DebugHook; }

      void SetMods( const wxArrayString& mods );
      const wxArrayString& GetMods() const { return m_Mods; }
      void SetModsString( const wxString& mods );
      wxString GetModsString() const;
      bool IsMod( const wxString& mod ) const;
      wxString GetModPath( const wxString& FullPath ) const;

      void SetScannerExts( const wxArrayString& exts );
      void SetScannerExtsString( const wxString& exts );
      wxString GetScannerExtsString() const;


	   const wxString GetExecuteCommand( const wxString& config, bool withArgs = true ) const;

      const wxString GetEntryScriptPath() const;

      wxString GetSearchUrl( const wxString& name ) const;
      static wxString GetDefaultSearchUrl( const wxString& name );

      const wxString& GetSearchUrl() const { return m_SearchURL; }
      const wxString& GetSearchProduct() const { return m_SearchProduct; }
      const wxString& GetSearchVersion() const { return m_SearchVersion; }
      
      void SetSearchUrl( const wxString& url );
      void SetSearchProduct( const wxString& product );
      void SetSearchVersion( const wxString& version );

      const wxString MakeReleativeTo( const wxString& FilePath ) const;
      const wxString MakeAbsoluteTo( const wxString& FilePath ) const;

      bool HasExports() const;

      void SetExecModifiedScripts( bool exec );
      bool ExecModifiedScripts() const { return m_ExecModifiedScripts; }

      const wxString& GetLastConfigName() const { return m_LastConfig; }
      void SetLastConfigName( const wxString& config ) { m_LastConfig = config; }
      const ProjectConfig* GetConfig( const wxString& config ) const;
      const ProjectConfigArray& GetConfigs() const { return m_Configs; }
      bool SetConfigs( const ProjectConfigArray& configs );

   	Breakpoint* FindBreakpoint( const wxString& File, int Line );
   	void        AddBreakpoint( const wxString& File, int Line, int Pass, const wxString& Condition, bool Enabled = true );
      void        ChangeBreakpoint( Breakpoint* old, Breakpoint* bp );
      void        DeleteBreakpoint( Breakpoint* Bp );
      void        MoveBreakpoint( Breakpoint* Bp, int Line );
	   bool        ToggleBreakpoint( const wxString& File, int Line, bool Disable );
      void        EnableBreakpoint( Breakpoint* Bp, bool Enable = true );
	   bool        GetBreakpoints( const wxString& File, BreakpointArray& Breakpoints );
      void        DeleteAllBreakpoints();
      void        EnableAllBreakpoints( bool enable );
      bool        GetBreakpointsEnabled() const;

      const BreakpointArray& GetBreakpoints() const { return m_Breakpoints; }

   	Bookmark*   FindBookmark( const wxString& File, int Line );
   	void        AddBookmark( const wxString& File, int Line );
	   bool        GetBookmarks( const wxString& File, BookmarkArray& Bookmarks );
      void        DeleteBookmark( Bookmark* bm );
      void        DeleteAllBookmarks();

      const BookmarkArray& GetBookmarks() const { return m_Bookmarks; }

      bool        GetSavedYet() const  { return m_savedYet; }


      bool SetDebugHook( int port, wxString& password );
      void RemoveDebugHook();

      void BuildExportsDB() const;

   protected:

      void UpdateOpenFilesArray();

      void BreakpointNotify( BreakpointEvent& event );

      static const wxString sm_SearchURL;
      static const wxString sm_DebugHookBegin;
      static const wxString sm_DebugHookEnd;
      static const wxString sm_DefaultHook;

      wxString	   m_Name;
	   wxString	   m_WorkingDir;

      wxString    m_LastConfig;

      wxArrayString m_Mods;
      wxArrayString m_ScannerExts;

      bool        m_ExecModifiedScripts;

      wxString	   m_SearchURL;
      wxString	   m_SearchProduct;
      wxString	   m_SearchVersion;

      wxString	   m_Address;
	   int			m_Port;
	   wxString	   m_Password;

      wxString    m_EntryScript;
	   wxString	   m_DebugHook;

      ProjectConfigArray   m_Configs;

   	BreakpointArray   m_Breakpoints;

      BookmarkArray     m_Bookmarks;

      int            m_ActiveOpenFile;
      FileInfoArray  m_OpenFiles;
};

#endif // TORSION_PROJECTDOC_H
