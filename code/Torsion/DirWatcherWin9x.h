// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_DIRWATCHERWIN9X_H
#define TORSION_DIRWATCHERWIN9X_H
#pragma once

#include "DirWatcher.h"


class DirWatcherWin9x;
WX_DECLARE_STRING_HASH_MAP( DirWatcherWin9x*, DirWatcherWin9xMap );

class DirWatcherWin9x : public DirWatcherImp
{
   protected:

      friend class DirWatcher;

      DirWatcherWin9x();

   public:

      virtual ~DirWatcherWin9x();

      virtual bool SetWatch( const wxString& dir, int flags, const wxArrayString& exclude );
      virtual int GetSignaled( wxArrayString* signaled );
      virtual void Clear();

   protected:

      bool _IsOk() const { return m_Handle != INVALID_HANDLE_VALUE; }
      bool _SetWatch( const wxString& dir, DWORD flags, const wxArrayString& exclude );
      int _GetSignaled( wxArrayString* signaled, DWORD flags, const wxArrayString& exclude );
      void _UpdateSubdirs( DWORD flags, const wxArrayString& exclude );

      DWORD          m_Flags;
      wxArrayString  m_Exclude;

      wxString                m_Dir;
      HANDLE                  m_Handle;
      DirWatcherWin9xMap      m_Subdirs;
};

#endif // TORSION_DIRWATCHERWIN9X_H