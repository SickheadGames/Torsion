// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsShellExecute.h"

#include "Platform.h"


#ifdef __WXMSW__
   #include <process.h>
#else
   #error Not Implemented!
#endif

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


class tsShellExecuteThread : public wxThread
{
   friend class tsShellExecute;

protected:

   HANDLE m_Process;
   wxEvtHandler*  m_EvtHandler;
   bool m_SendEvent;

public:

   tsShellExecuteThread( wxEvtHandler* handler )
      :  wxThread( wxTHREAD_JOINABLE ),
         m_EvtHandler( handler )
   {
   }

protected:

   virtual ExitCode Entry();
   virtual void OnExit();

};


wxThread::ExitCode tsShellExecuteThread::Entry()
{
   m_SendEvent = true;

   // Wait till the process ends or we're 
   // shutdown from outside.
   while ( WaitForSingleObject(m_Process, 500) != WAIT_OBJECT_0 )
   {
      if ( TestDestroy() )
      {
         m_SendEvent = false;
         return (ExitCode)0;
      }
   }

   return (ExitCode)1;
}

void tsShellExecuteThread::OnExit()
{
   if ( WaitForSingleObject( m_Process, 0 ) == WAIT_OBJECT_0 && m_SendEvent )
   {
      wxProcessEvent event( wxID_HIGHEST, 0, 0 );
      ::wxPostEvent( m_EvtHandler, event );
   }
}


tsShellExecute::tsShellExecute( wxEvtHandler* handler, int eventId )
   :  m_EvtHandler( handler ),
      m_EventId( eventId ),
      m_Pid( 0 ),
      m_Monitor( NULL )
{
   memset(&m_SE, 0, sizeof(m_SE)); 
   m_SE.cbSize = sizeof(m_SE);
}

tsShellExecute::~tsShellExecute()
{
   kill();
}

BEGIN_EVENT_TABLE( tsShellExecute, wxEvtHandler )
	EVT_END_PROCESS( wxID_HIGHEST, tsShellExecute::onProcessStopped )
END_EVENT_TABLE()

bool tsShellExecute::isRunning() const
{
   return   m_SE.hProcess > 0 && 
            WaitForSingleObject( m_SE.hProcess, 0 ) != WAIT_OBJECT_0;
}

bool tsShellExecute::execDeatched(  const wxString& verb, 
                                    const wxString& dir, 
                                    const wxString& file, 
                                    const wxString& args,
                                    int show )
{
   SHELLEXECUTEINFO se;
   memset(&se, 0, sizeof(se)); 
   se.cbSize = sizeof(se);

   se.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI; 
   se.lpVerb = verb.c_str();
   se.lpFile = file.c_str();
   se.lpDirectory = dir.c_str();
   se.lpParameters = args.c_str(); 
   se.nShow = show;
   if ( !ShellExecuteEx( &se ) )
      return false;

   return true;
}

bool tsShellExecute::exec( const wxString& verb, 
                           const wxString& dir, 
                           const wxString& file, 
                           const wxString& args,
                           int show )
{
   memset(&m_SE, 0, sizeof(m_SE)); 
   m_SE.cbSize = sizeof(m_SE);
   m_SE.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI; 
   m_SE.lpVerb = verb.c_str();
   m_SE.lpFile = file.c_str();
   m_SE.lpDirectory = dir.c_str();
   m_SE.lpParameters = args.c_str(); 
   m_SE.nShow = show;
   if ( !ShellExecuteEx( &m_SE ) || m_SE.hProcess == 0 )
      return false;

   m_Pid = tsMswGetProcessId( m_SE.hProcess );

   // Create the monitor thread...
   m_Monitor = new tsShellExecuteThread( this );
   m_Monitor->Create();
   m_Monitor->m_Process = m_SE.hProcess;
   m_Monitor->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
   m_Monitor->Run();

   return true;
}

bool tsShellExecute::setPriorityClass( tsPRIORITY_CLASS pc )
{
   if ( m_SE.hProcess == 0 )
      return false;
   
   DWORD wpc;
   int tpc;
   switch ( pc ) {

      case tsPRIORITY_CLASS_IDLE:

         wpc = IDLE_PRIORITY_CLASS;
         tpc = THREAD_PRIORITY_IDLE;
         break;

      case tsPRIORITY_CLASS_NORMAL:
      default:

         wpc = NORMAL_PRIORITY_CLASS;
         tpc = THREAD_PRIORITY_NORMAL;
         break;
   };

   return SetPriorityClass( m_SE.hProcess, wpc ) == TRUE;
}

void tsShellExecute::detach()
{
   m_SE.hProcess = 0;
   m_Pid = 0;

   if ( m_Monitor )
   {
      // The reason we reassign the monitor to a local
      // temporary is because Delete() will process events
      // and can call onProcessStopped() deleting the m_Monitor
      // out from under the Delete() processing causing a 
      // crash.  By taking ownership of m_Monitor we avoid
      // this potential crash situation.
      tsShellExecuteThread* monitor = m_Monitor;
      m_Monitor = NULL;
      monitor->Delete();
      wxDELETE( monitor );
   }
}

void tsShellExecute::kill()
{
   if ( m_SE.hProcess == 0 )
      return;

   HANDLE process = m_SE.hProcess;

   // First detach... we don't want the monitor
   // to capture the terminate event.
   detach();

   // Now terminate the process. 
   TerminateProcess( process, 0 );
}

void tsShellExecute::onProcessStopped( wxProcessEvent& event )
{
   wxProcessEvent newEvent( m_EventId, m_Pid, event.GetExitCode() );

   // Cleanup now.
   m_SE.hProcess = 0;
   m_Pid = 0;

   // Only send the even if the monitor was not
   // taken by detach() above.  This avoids sending
   // the event from the Delete() message pump.
   if ( m_Monitor )
   {
      m_Monitor->Wait();
      wxDELETE( m_Monitor );

      // Forward the event to whoever wanted it.
      if ( m_EvtHandler )
         m_EvtHandler->ProcessEvent( newEvent );
   }
}
