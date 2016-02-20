// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "PreCompiler.h"

#include "TorsionApp.h"
#include "MainFrame.h"
#include "OutputPanel.h"
#include "Debugger.h"
#include "Platform.h"

#include <wx/textfile.h>
#include <wx/ffile.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


DEFINE_EVENT_TYPE(tsEVT_PRECOMPILER_OUTPUT)
DEFINE_EVENT_TYPE(tsEVT_PRECOMPILER_DONE)


class PreCompilerThread : public wxThread
{
   friend class PreCompiler;
   friend class ScriptCollector;

public:

   PreCompilerThread()
      :  wxThread( wxTHREAD_JOINABLE ),
         m_OutputWnd( NULL )
   {
   }

protected:

   void UpdateOutput( const wxString& file, wxFileOffset& lastReadPos );
   void WriteOutput( const wxString& text );

   virtual ExitCode Entry();
   virtual void OnExit();

   wxString m_WorkingDir;
   wxString m_Exec;
   wxString m_ActiveScript;
   wxArrayString m_Mods;
   bool     m_ClearDSOs;
   bool     m_SetModPaths;
   wxEvtHandler*  m_OutputWnd;

   wxFileName m_Script;

   wxArrayString m_DSOExts;
   wxArrayString m_Scripts;
   wxArrayString m_ExcludedFolders;
   wxArrayString m_ScriptExts;
};


class ScriptCollector : protected wxDirTraverser
{
   friend class PreCompilerThread;

public:

   ScriptCollector( PreCompilerThread& pc )
      :  wxDirTraverser(), 
         m_PC( pc )
   {
   }

   virtual wxDirTraverseResult OnFile( const wxString& name );
   virtual wxDirTraverseResult OnDir( const wxString& name );

   PreCompilerThread& m_PC;
};

wxDirTraverseResult ScriptCollector::OnFile( const wxString& name )
{
   // Were we canceled?
   if ( m_PC.TestDestroy() )
      return wxDIR_STOP;

   // Is this a script?
   wxFileName script( name );
   if ( m_PC.m_ScriptExts.Index( script.GetExt(), wxFileName::IsCaseSensitive() ) == wxNOT_FOUND )
      return wxDIR_CONTINUE;

   // Skip empty files.
   wxStructStat stat;
   if ( wxStat( name, &stat ) == 0 && stat.st_size <= 0 )
      return wxDIR_CONTINUE;

   // If we're rebuilding all then delete the DSOs.
   if ( m_PC.m_ClearDSOs )
   {
      for ( int i=0; i < m_PC.m_DSOExts.GetCount(); i++ )
      {
         wxString dsoName;
         dsoName << name << '.' << m_PC.m_DSOExts[i];
         wxRemoveFile( dsoName );
      }
   }
   else
   {
      // else skip scripts where the DSO exists and is newer.
      for ( int i=0; i < m_PC.m_DSOExts.GetCount(); i++ )
      {
         wxFileName dsoName( name + '.' + m_PC.m_DSOExts[i] );
         if (  dsoName.FileExists() &&
               dsoName.GetModificationTime().IsLaterThan( script.GetModificationTime() ))
            return wxDIR_CONTINUE;
      }
   }

   script.MakeRelativeTo( m_PC.m_WorkingDir );
   m_PC.m_Scripts.Add( script.GetFullPath() );

   return wxDIR_CONTINUE;
}

wxDirTraverseResult ScriptCollector::OnDir( const wxString& name )
{
   // Were we canceled?
   if ( m_PC.TestDestroy() )
      return wxDIR_STOP;

   wxFileName folder;
   folder.AssignDir( name );
   folder.MakeRelativeTo( m_PC.m_WorkingDir );
   if ( folder.GetDirCount() == 0 )
      return wxDIR_CONTINUE;

   // Is this folder name excluded?
   wxString folderName( folder.GetDirs().Last() );
   if ( m_PC.m_ExcludedFolders.Index( folderName, wxFileName::IsCaseSensitive() ) != wxNOT_FOUND )
      return wxDIR_IGNORE;

   // If we have specific mods to process.
   if ( m_PC.m_Mods.GetCount() )
   {
      wxString mod = folder.GetDirs()[0];
      if ( m_PC.m_Mods.Index( mod, wxFileName::IsCaseSensitive() ) == wxNOT_FOUND )
         return wxDIR_IGNORE;
   }

   return wxDIR_CONTINUE;
}


PreCompiler::PreCompiler() 
   : m_Thread( NULL )
{
}

PreCompiler::~PreCompiler()
{
   if ( m_Thread )
   {
      m_Thread->Delete();
      delete m_Thread;
   }
}

bool PreCompiler::Run( const wxString& workingDir, const wxString& exec, const wxArrayString& mods, const wxString& activeScript, bool clearDSOs, bool setModPaths, wxEvtHandler* outputWnd )
{
   wxASSERT( !m_Thread );

   // Create the thread..
   m_Thread = new PreCompilerThread();
   if ( m_Thread->Create() != wxTHREAD_NO_ERROR ) 
   {
      m_Thread->Delete();
      delete m_Thread;
      m_Thread = NULL;
      return false;
   }

   // Init the data we'll need in the thread.
   m_Thread->m_WorkingDir = workingDir;
   m_Thread->m_Exec = exec;
   m_Thread->m_ClearDSOs = clearDSOs;
   m_Thread->m_ScriptExts = tsGetPrefs().GetScriptExtensions();
   m_Thread->m_ActiveScript = activeScript;
   m_Thread->m_Mods = mods;
   m_Thread->m_ExcludedFolders = tsGetPrefs().GetExcludedFolders();
   m_Thread->m_DSOExts = tsGetPrefs().GetDSOExts();
   m_Thread->m_OutputWnd = outputWnd;
   m_Thread->m_SetModPaths = setModPaths;

   m_Thread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );

   // Start running it!
   if ( m_Thread->Run() != wxTHREAD_NO_ERROR )
   {
      m_Thread->Delete();
      delete m_Thread;
      m_Thread = NULL;
      return false;
   }

   return true;
}

void PreCompiler::Stop()
{
   if ( !m_Thread )
      return;

   m_Thread->Delete();
   delete m_Thread;
   m_Thread = NULL;
}

void PreCompiler::WaitForFinish( bool* success, int* scripts, int* errors )
{
   wxThread::ExitCode result = 0;
   if ( m_Thread )
   {
      result = m_Thread->Wait();

      if ( scripts )
         *scripts = m_Thread->m_Scripts.Count();

      m_Thread->Delete();
      delete m_Thread;
      m_Thread = NULL;
   }

   if ( success )
      *success = result ? true : false;

   // TODO: We need to move this out of the debugger!
   if ( errors )
   {
      wxASSERT( tsGetMainFrame() );
      wxASSERT( tsGetMainFrame()->GetOutputPanel() );
      *errors = tsGetMainFrame()->GetOutputPanel()->GetErrors().size();
   }
}

wxThread::ExitCode PreCompilerThread::Entry()
{
   // Enum all the script files to be compiled.
   m_Scripts.Clear();
   if ( m_ActiveScript.IsEmpty() )
   {
	   wxDir dir( m_WorkingDir );
      if ( !dir.IsOpened() )
         return 0;
      ScriptCollector sink( *this );
      dir.Traverse( sink, "*.*", wxDIR_DIRS | wxDIR_FILES );
   }
   else
   {
      m_Scripts.Add( m_ActiveScript );
   }

   // Were we canceled?
   if ( TestDestroy() )
      return (ExitCode)0;

   // Do we have scripts to compile?
   if ( m_Scripts.IsEmpty() )
      return (ExitCode)1;

   // Copy the precompiler script.
   wxFileName sourceScript( wxGetApp().GetAppPath() );
   sourceScript.SetFullName( "torsion_precompile.cs" );
   m_Script.Clear();
   m_Script.AssignDir( m_WorkingDir );
   m_Script.SetFullName( "torsion_precompile.tmp" );
   if ( !wxCopyFile( sourceScript.GetFullPath(), m_Script.GetFullPath() ) )
      return 0;

   // Add all the scripts into the precompiler script.
   wxTextFile scriptFile;
   if ( !scriptFile.Open( m_Script.GetFullPath() ) )
      return 0;

   // Find the starting line!
   int insert = 0;
   for ( ; insert < scriptFile.GetLineCount(); insert++ )
   {
      if ( TestDestroy() )
         return (ExitCode)0;

      if ( scriptFile.GetLine( insert ) == "// PRECOMPILE_START" )
         break;
   }

   // Set the mod paths!
   if ( m_SetModPaths )
   {
      wxArrayString found;
      wxString paths;
      for ( int i=0; i < m_Scripts.GetCount(); i++ )
      {
         if ( TestDestroy() )
            return (ExitCode)0;

         wxString path = m_Scripts[i].BeforeFirst( '\\' );
         if ( path.Len() == m_Scripts[i].Len() || found.Index( path ) != -1 )
            continue;
         found.Add( path );
         paths << path << ';';
      }
      wxString cmd;
      cmd << "setModPaths( \"" << paths << "\" );";
      ++insert;
      scriptFile.InsertLine( cmd, insert );
   }
   
   // Add the scripts!
   for ( int i=0; i < m_Scripts.GetCount(); i++ )
   {
      if ( TestDestroy() )
         return (ExitCode)0;

      wxString path( m_Scripts[i] );
      path.Replace( "\\", "/" );
      wxString line;
      line << "compile( \"" << path << "\" );";
      scriptFile.InsertLine( line, insert + i + 1 );
   }

   scriptFile.Write();
   scriptFile.Close();

   // Empty the console log to clear it.
   wxFileName output;
   output.AssignDir( m_WorkingDir );
   output.SetFullName( "console.log" );
   {
      wxFile file( output.GetFullPath(), wxFile::write );
      file.Close();
   }

   // TODO: wxExecute doesn't work with threads... so 
   // here is my win32 specific hack until we need a 
   // real cross-platform solution.
   wxString args( m_Script.GetFullName() );
   SHELLEXECUTEINFO se;
   memset(&se, 0, sizeof(se)); 
   se.cbSize = sizeof(se); 
   se.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI; 
   se.lpVerb = "open";
   se.lpFile = m_Exec.c_str();
   se.lpDirectory = m_WorkingDir.c_str();
   se.lpParameters = args.c_str(); 
   se.nShow = SW_SHOWMINNOACTIVE;
   if ( !ShellExecuteEx(&se) )
      return 0;

   // Wait for the executable to finish.
   wxFileOffset lastReadPos = 0;
   long pid = tsMswGetProcessId( se.hProcess );
   while ( WaitForSingleObject(se.hProcess, 500) != WAIT_OBJECT_0 )
   {
      if ( TestDestroy() || tsProcessHasNonConsoleWindow( pid ) )
      {
         // Kill nicely... then get serious!
			if ( ::wxKill( pid, wxSIGTERM  ) != 0 )
            ::wxKill( pid, wxSIGKILL );

         // Get the last bit of output.
         UpdateOutput( output.GetFullPath(), lastReadPos );

         return (ExitCode)0;
      }

      UpdateOutput( output.GetFullPath(), lastReadPos );
   }

   UpdateOutput( output.GetFullPath(), lastReadPos );

   /*
   // Were we canceled?
   while ( true )
   {
      if ( TestDestroy() )
         return (ExitCode)0;
   }
   */

   return (ExitCode)1;
}

void PreCompilerThread::WriteOutput( const wxString& text )
{
   wxCommandEvent event( tsEVT_PRECOMPILER_OUTPUT, wxID_ANY );
   event.SetString( text );

   wxASSERT( m_OutputWnd );
   ::wxPostEvent( m_OutputWnd, event );
}

void PreCompilerThread::UpdateOutput( const wxString& path, wxFileOffset& lastReadPos )
{
   wxFFile file( path );
   if ( !file.IsOpened() )
      return;

   file.Seek( lastReadPos );
   wxFileOffset size = file.Length() - lastReadPos;
   if ( size <= 0 )
      return;

   wxString buffer;
   wxChar* log = buffer.GetWriteBuf( size );
   memset( log, 0, size );
   file.Read( log, size );
   buffer.UngetWriteBuf( size );

   lastReadPos += size;
   WriteOutput( buffer );
}

void PreCompilerThread::OnExit()
{
   // Cleanup...
   if ( m_Script.FileExists() )
      wxRemoveFile( m_Script.GetFullPath() );

   // Let the main thread know we're done!
   wxCommandEvent event( tsEVT_PRECOMPILER_DONE, wxID_ANY );
   wxASSERT( m_OutputWnd );
   ::wxPostEvent( m_OutputWnd, event );
}

