// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_TSSHELLEXECUTE_H
#define TORSION_TSSHELLEXECUTE_H
#pragma once

enum
{
   tsSW_SHOWNORMAL = 1,
};

enum tsPRIORITY_CLASS 
{
   tsPRIORITY_CLASS_NORMAL,
   tsPRIORITY_CLASS_IDLE,
};

class tsShellExecuteThread;


class tsShellExecute : public wxEvtHandler
{
protected:

   #ifdef __WXMSW__
      SHELLEXECUTEINFO m_SE;
   #else
      #error Not Implemented!
   #endif

   tsShellExecuteThread* m_Monitor;

   long m_Pid;

   int m_EventId;
   wxEvtHandler* m_EvtHandler;

   void onProcessStopped( wxProcessEvent& event );

public:

   tsShellExecute( wxEvtHandler* handler = NULL, int eventId = -1 );
   virtual ~tsShellExecute();

public:

   static bool execDeatched(  const wxString& verb, 
                              const wxString& dir, 
                              const wxString& file, 
                              const wxString& args,
                              int show );

   bool exec(  const wxString& verb, 
               const wxString& dir, 
               const wxString& file, 
               const wxString& args,
               int show );

   long getPid() const { return m_Pid; }

   bool setPriorityClass( tsPRIORITY_CLASS pc );

   bool isRunning() const;

   // Kills the process if it is running without sending
   // the process stopped event.
   void kill();
   
   // Detaches from the process.
   void detach();

   DECLARE_EVENT_TABLE()
};


#endif // TORSION_TSSHELLEXECUTE_H