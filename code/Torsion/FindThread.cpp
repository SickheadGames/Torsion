// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "FindThread.h"

//#include "FindResultsCtrl.h"
//#include "MainFrame.h"

#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

DEFINE_LOCAL_EVENT_TYPE(tsEVT_FIND_STATUS)
DEFINE_LOCAL_EVENT_TYPE(tsEVT_FIND_OUTPUT)

class FindThreadInternal : public wxThread
{
   friend class FindThread;

public:

   FindThreadInternal()
      :  wxThread( wxTHREAD_JOINABLE ),
         m_EvtHandler( NULL )
   {
   }

protected:

   virtual void *Entry();

   void WriteLine( const wxString& text );

   bool Traverse( const wxString& path, const wxArrayString& filespecs );

   bool OnFile( const wxString& filename );

   bool IsType( const wxString& file ) const;

   wxEvtHandler* m_EvtHandler;

   wxString       m_What;
   wxString       m_WhatLower;
   wxArrayString  m_Paths;   
   wxString       m_Types;
   bool           m_MatchCase;
   bool           m_MatchWord;

   int m_Results;
   int m_Infiles;
   int m_TotalFiles;
};


FindThread::FindThread() 
   : m_Thread( NULL )
{
}

bool FindThread::Find( wxEvtHandler* handler, const wxString& what, const wxArrayString& paths, const wxString& types, bool matchCase, bool matchWord )
{
   wxASSERT( handler );

   // If we're created... then we're running!
   if ( IsSearching() )
      return false;

   // Delete any existing thread object... stupid
   // wxThread doesn't allow reuse of the object.
   if ( m_Thread )
   {
      m_Thread->Delete();
      delete m_Thread;
   }

   m_Thread = new FindThreadInternal();
   if ( m_Thread->Create() != wxTHREAD_NO_ERROR ) 
   {
      m_Thread->Delete();
      delete m_Thread;
      m_Thread = NULL;
      return false;
   }

   m_Thread->m_EvtHandler  = handler;
   m_Thread->m_What        = what;
   m_Thread->m_WhatLower   = what.Lower();
   m_Thread->m_Paths       = paths;
   m_Thread->m_Types       = types;
   m_Thread->m_MatchCase   = matchCase;
   m_Thread->m_MatchWord   = matchWord;

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

bool FindThread::IsSearching()
{
   return m_Thread && m_Thread->IsRunning();
}

void FindThread::Cancel()
{
   if ( !m_Thread )
      return;

   wxThread* thread = m_Thread;
   m_Thread = NULL;

   // This call can pump messages... so in order
   // to not recusively call delete, we cleared
   // the m_Thread pointer first.
   thread->Delete();
   delete thread;
}


void* FindThreadInternal::Entry()
{
   if ( m_Paths.IsEmpty() || m_What.IsEmpty() || m_Types.IsEmpty() ) 
   {
      WriteLine( "Invalid find parameters!" );
      return NULL;
   }

   wxString output;
   output << "Find \"" << m_What << "\" in files of type(s) \"" << m_Types << "\" in \"" << m_Paths[0] << "\"";
   WriteLine( output );

   m_Results = 0;
   m_Infiles = 0;
   m_TotalFiles = 0;

   // Grab the list of filespecs.
   wxArrayString Filespecs;
   wxStringTokenizer toker( m_Types, ";," );
   while ( toker.HasMoreTokens() ) {

      wxString filespec = toker.NextToken();
      filespec.Trim( false );
      filespec.Trim( true );
      Filespecs.Add( filespec );      
   }

   wxCommandEvent event( tsEVT_FIND_STATUS );
   event.SetString( "Scanning..." );
   wxPostEvent( m_EvtHandler, event );

   // Do the enumeration on each path!
   bool done = true;
   for ( int i=0; i < m_Paths.GetCount(); i++ )
   {
      wxString path = m_Paths[i];
      path.Trim( false );
      path.Trim( true );

      if ( !Traverse( path, Filespecs ) ) 
      {
         done = false;
         break;
      }
   }

   if ( done ) {

      output.Clear();
      output << "Results: " << m_Results << "    In Files: " << m_Infiles << "    Searched Files: " << m_TotalFiles;
      WriteLine( output );

   } else {

      WriteLine( "Operation cancled!" );
   }

   event.SetString( "" );
   wxPostEvent( m_EvtHandler, event );

   return NULL;
}

void FindThreadInternal::WriteLine( const wxString& text )
{
   wxCommandEvent event( tsEVT_FIND_OUTPUT );
   event.SetString( text + "\r\n" );
   wxPostEvent( m_EvtHandler, event );
}

bool FindThreadInternal::Traverse( const wxString& path, const wxArrayString& filespecs )
{
	wxDir dir( path );
   if ( !dir.IsOpened() ) {
      return true;
   }

   // The name of this dir with path delimiter at the end
   wxString prefix = dir.GetName();
   prefix += wxFILE_SEP_PATH;

   // First process the files in this folder... do one 
   // pass per filespec.
   for ( int f=0; f < filespecs.GetCount(); f++ ) {

      wxString filename;
      bool cont = dir.GetFirst( &filename, filespecs[f], wxDIR_FILES );
      while ( cont )
      {
         if ( TestDestroy() )
            return false;

         if ( !OnFile( prefix + filename ) ) {
            return false;
         }

         cont = dir.GetNext( &filename );
      }
   }

   // Now recurse into the sub directories.
   wxString dirname;
   bool cont = dir.GetFirst( &dirname, wxEmptyString, wxDIR_DIRS );
   while ( cont )
   {
      if ( TestDestroy() )
         return false;

      if ( !Traverse( prefix + dirname, filespecs ) ) {
         return false;
      }
      cont = dir.GetNext( &dirname );
   }

   return true;
}

bool FindThreadInternal::OnFile( const wxString& filename )
{
   ++m_TotalFiles;

   bool Found = false;

   wxCommandEvent event( tsEVT_FIND_STATUS );
   event.SetString( wxString( "Scanning " ) << filename << "..." );
   wxPostEvent( m_EvtHandler, event );

   // Search for a match in this file.
   wxFileInputStream input( filename );
   if ( !input.Ok() ) {
      return true;
   }
   wxTextInputStream text( input );
   
   const wxString Seps = " \t.,\"'\\/|[]{}:;<>()-+*^";
   int LineCount = 1;
   int Pos, Next;
   wxString Line;
   wxString Output;
   wxString MatchWord;
   Line.Alloc( 512 );
   Output.Alloc( 512 );

   while ( !input.Eof() ) {

      if ( TestDestroy() )
         return false;

      Line = text.ReadLine();

      if ( !m_MatchCase )
         Pos = Line.Lower().Find( m_WhatLower );
      else
         Pos = Line.Find( m_What );

      if ( m_MatchWord ) {

         Next = Pos + m_What.Len();
         if ( Pos > 0 && Seps.Find( Line[Pos-1] ) == -1 )
            Pos = -1;
         else if ( Next < Line.Len() && Seps.Find( Line[ Next ] ) == -1 )
            Pos = -1;
      }

      if ( Pos != -1 ) {

         Found = true;
         Output.Clear();
         Output << filename << "(" << LineCount << "):   " << Line;
         WriteLine( Output );
         ++m_Results;
      }

      ++LineCount;
   }

   if ( Found )
      ++m_Infiles;

   return true;
}

bool FindThreadInternal::IsType( const wxString& file ) const
{
   return true;
}

