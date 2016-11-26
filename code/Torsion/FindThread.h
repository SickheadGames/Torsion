// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_FINDTHREAD_H
#define TORSION_FINDTHREAD_H
#pragma once


#include <wx/dir.h>

class FindResultsCtrl;
class FindThreadInternal;


DECLARE_LOCAL_EVENT_TYPE(tsEVT_FIND_STATUS, -1)
DECLARE_LOCAL_EVENT_TYPE(tsEVT_FIND_OUTPUT, -1)

class FindThread 
{
public:
   FindThread();

   bool Find(  wxEvtHandler* handler,
               const wxString& what,
               const wxArrayString& paths,
               const wxString& types,
               bool MatchCase,
               bool MatchWord );

   bool IsSearching();

   void Cancel();

protected:

   FindThreadInternal* m_Thread;
};

#endif // TORSION_FINDTHREAD_H