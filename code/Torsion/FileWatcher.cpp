// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "FileWatcher.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

// TODO: This may need to be fixed for Linux and Mac as 
// it uses wxStatus which i'm not sure is crossplatform.

FileWatcher::FileWatcher()
   : m_Flags( 0 )
{
}

FileWatcher::FileWatcher( const FileWatcher& watcher )
   :  m_File( watcher.m_File ),
      m_Flags( watcher.m_Flags ),
      m_LastStatus( watcher.m_LastStatus )
{
}

FileWatcher::~FileWatcher()
{
}

FileWatcher& FileWatcher::operator =( const FileWatcher& watcher )
{
   m_File         = watcher.m_File;
   m_Flags        = watcher.m_Flags;
   m_LastStatus   = watcher.m_LastStatus;
   return *this;
}

bool FileWatcher::SetWatch( const wxString& file, int flags )
{
   m_File = file;
   wxASSERT( m_File.IsAbsolute() );
   if ( !m_File.IsOk() )
      return false;

   m_Flags = flags;

   wxZeroMemory( m_LastStatus );
   return wxStat( m_File.GetFullPath(), &m_LastStatus ) == 0;
}

bool FileWatcher::IsSignaled( unsigned long wait )
{
   if ( !m_File.IsOk() )
      return false;

   // Look for false signals.
   wxStructStat Status;
   wxZeroMemory( Status );
   wxStat( m_File.GetFullPath(), &Status );

   if (  ( !(m_Flags & FILECHANGE_LAST_WRITE) || m_LastStatus.st_mtime == Status.st_mtime ) &&
         ( !(m_Flags & FILECHANGE_ATTRIBUTES) || m_LastStatus.st_mode == Status.st_mode ) &&
         ( !(m_Flags & FILECHANGE_SIZE) || m_LastStatus.st_size == Status.st_size ) )
      return false;

   m_LastStatus = Status;
   return true;
}

void FileWatcher::Clear()
{
   m_File.Clear();
}


