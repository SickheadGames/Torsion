// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_DIRWATCHER_H
#define TORSION_DIRWATCHER_H
#pragma once


#define DIRCHANGE_FILE_NAME     (1<<0)
#define DIRCHANGE_DIR_NAME      (1<<1)
#define DIRCHANGE_LAST_WRITE    (1<<2)

class DirWatcherImp;


class DirWatcher
{
   public:

      DirWatcher();
      virtual ~DirWatcher();

   public:

      bool SetWatch( const wxString& dir, int flags, const wxArrayString& exclude );
      void Clear();

      int GetSignaled( wxArrayString* signaled );

   protected:

      DirWatcherImp* m_Impl;
};


class DirWatcherImp
{
   public:
      virtual ~DirWatcherImp() {};
      virtual bool SetWatch( const wxString& dir, int flags, const wxArrayString& exclude ) = 0;
      virtual int GetSignaled( wxArrayString* signaled ) = 0;
      virtual void Clear() = 0;
};

#endif // TORSION_DIRWATCHER_H