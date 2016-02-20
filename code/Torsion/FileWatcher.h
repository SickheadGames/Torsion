// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_FILEWATCHER_H
#define TORSION_FILEWATCHER_H
#pragma once


#define FILECHANGE_ATTRIBUTES    0x01
#define FILECHANGE_SIZE          0x02
#define FILECHANGE_LAST_WRITE    0x04
#define FILECHANGE_ANY           ( FILECHANGE_ATTRIBUTES | FILECHANGE_SIZE | FILECHANGE_LAST_WRITE )

class FileWatcher;
WX_DEFINE_ARRAY( FileWatcher*, FileWatcherArray );


class FileWatcher
{
   public:

      FileWatcher();
      FileWatcher( const FileWatcher& watcher );
      virtual ~FileWatcher();

   public:

      bool SetWatch( const wxString& file, int flags = FILECHANGE_ANY );
      bool IsSignaled( unsigned long wait );
      void Clear();

      FileWatcher& operator =( const FileWatcher& watcher );

   protected:

      wxFileName     m_File;
      int            m_Flags;
      wxStructStat   m_LastStatus;
};

#endif // TORSION_FILEWATCHER_H