// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "BuildExportsThread.h"

#include "ProjectDoc.h"
#include "AutoComp.h"
#include "Platform.h"

#include <process.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


BuildExportsThread::BuildExportsThread(   const wxString& exportScriptPath,
                                          const wxString& workingDir,
                                          const ProjectConfigArray& Configs )
   :  wxThread( wxTHREAD_JOINABLE ),
      m_SourceExportScript( exportScriptPath ),
      m_WorkingDir( workingDir ),
      m_Configs( Configs ),
      m_Exports( NULL ),
      m_Progress( 0.0f )
{
}

BuildExportsThread::~BuildExportsThread()
{
   wxDELETE( m_Exports );
}

float BuildExportsThread::GetProgressPct()
{
   wxCriticalSectionLocker lock( m_DataLock );
   return m_Progress;
}

AutoCompExports* BuildExportsThread::TakeExports()
{
   wxASSERT( IsMain() );
   wxASSERT( !IsRunning() );
   AutoCompExports* exports = m_Exports;
   m_Exports = NULL;
   return exports;
}

void BuildExportsThread::_AddProgress( float inc )
{
   wxCriticalSectionLocker lock( m_DataLock );
   m_Progress += inc;
   if ( m_Progress > 100.0f )
      m_Progress = 100.0f;
}

wxThread::ExitCode BuildExportsThread::Entry()
{
   //m_LastCwd.Clear();
   m_ExportScript.Clear();
   
   // Figure out how big the progress increment should be.
   float pinc = 100.0f / ( ( m_Configs.GetCount() * 2 ) + 2 );

   // Look for reasons to immediatly close
   if ( TestDestroy() || !m_SourceExportScript.FileExists() )
      return 0;

   // First step is to generate an autocomp page for
   // each project config.  We do this by querying
   // each executable for a dump of it's classes and
   // functions.

   // Copy the script to the working path.
   m_ExportScript.AssignDir( m_WorkingDir );
   m_ExportScript.SetFullName( "torsion_exports.tmp" );
   if ( !wxCopyFile( m_SourceExportScript.GetFullPath(), m_ExportScript.GetFullPath() ) )
      return 0;

   wxFileName output;
   output.AssignDir( m_WorkingDir );
   output.SetFullName( "console.log" );
   
   _AddProgress( pinc );

   for ( int i=0; i < m_Configs.GetCount(); i++ ) { 

      if ( TestDestroy() )
         return 0;

      const ProjectConfig& config = m_Configs[i];
      if ( !config.HasExports() )
      {
         _AddProgress( pinc * 2 );
         continue;
      }

      // TODO: wxExecute doesn't work with threads... so 
      // here is my win32 specific hack until we need a 
      // real cross-platform solution.
      wxString exe( config.GetExe().c_str() );
      if ( exe.IsEmpty() )
         continue;
      wxString args( m_ExportScript.GetFullName().c_str() );

      // Delete any existing console log first, so that
      // we never mistakenly include the same exports twice.
      wxRemoveFile( output.GetFullPath() );

   	//wxString lastCwd = wxFileName::GetCwd();
		//wxFileName::SetCwd( m_WorkingDir );

      SHELLEXECUTEINFO se;
      memset(&se, 0, sizeof(se)); 
      se.cbSize = sizeof(se); 
      se.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI; 
      se.lpVerb = L"open";
      se.lpFile = exe.c_str();
      se.lpDirectory = m_WorkingDir.c_str();
      se.lpParameters = args.c_str(); 
      se.nShow = SW_SHOWMINNOACTIVE;
      if ( !ShellExecuteEx(&se) )
         return 0;

		//wxFileName::SetCwd( lastCwd );

      // Wait for the executable to finish.
      long pid = tsMswGetProcessId( se.hProcess );
      bool skip = false;
      while ( WaitForSingleObject(se.hProcess, 1000) != WAIT_OBJECT_0 )
      {
         if ( TestDestroy() )
         {
            // Kill nicely... then get serious!
			   if ( ::wxKill( pid, wxSIGTERM  ) != 0 ) {
               ::wxKill( pid, wxSIGKILL );
			   }

            return 0;
         }

         if ( tsProcessHasNonConsoleWindow( pid ) )
         {
            // This executable launched fully... kill 
            // it and skip processing the output.
            if ( ::wxKill( pid, wxSIGTERM  ) != 0 ) {
               ::wxKill( pid, wxSIGKILL );
			   }

            skip = true;
            break;
         }
      }

      if ( skip )
      {
         _AddProgress( pinc * 2 );
         continue;
      }

      _AddProgress( pinc );

      AutoCompExports* exports = new AutoCompExports( config.GetName() );
      if ( exports->LoadFromDoxygen( output.GetFullPath() ) )
         m_ConfigExports.Add( exports );
      else
         wxDELETE( exports ); 

      _AddProgress( pinc );

      if ( TestDestroy() )
         return 0;
   }

   // Generate a single page of exports from the array
   // to be returned to the caller.
   m_Exports = new AutoCompExports();
   m_Exports->Merge( m_ConfigExports );
   WX_CLEAR_ARRAY( m_ConfigExports );

   _AddProgress( pinc );

   // Save the one page of pre-compiled autocomplete 
   // data to disk in an xml format.
   //m_Exports->SaveXml( m_ExportsFile );
   //wxDELETE( m_Exports );

   return (ExitCode)1;
}

void BuildExportsThread::OnExit()
{
   if ( m_ExportScript.FileExists() )
      wxRemoveFile( m_ExportScript.GetFullPath() );

   WX_CLEAR_ARRAY( m_ConfigExports );
}
