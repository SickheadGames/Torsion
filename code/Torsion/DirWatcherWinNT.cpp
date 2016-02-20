// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "DirWatcherWinNT.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


DirWatcherWinNT::DirWatcherWinNT( size_t bufferSize )
   :  m_Handle( INVALID_HANDLE_VALUE ),
      m_Flags( 0 ),
      m_BufferSize( bufferSize ),
      m_Buffer( new wxChar[ bufferSize ] )
{
   ::ZeroMemory( m_Buffer, m_BufferSize );
   ::ZeroMemory( &m_Overlapped, sizeof( m_Overlapped ) );
}

DirWatcherWinNT::~DirWatcherWinNT()
{
   Clear();
   delete [] m_Buffer;
}

bool DirWatcherWinNT::SetWatch( const wxString& dir, int flags, const wxArrayString& exclude )
{
   Clear();

   // Get a handle to the directory.
   m_Handle = ::CreateFile(   dir.c_str(),
                              FILE_LIST_DIRECTORY,
                              FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                              NULL );
   if ( m_Handle == INVALID_HANDLE_VALUE )
      return false;

   m_Dir = dir;

   // Convert the flags.
   m_Flags = 0;
   if ( flags & DIRCHANGE_FILE_NAME )  m_Flags |= FILE_NOTIFY_CHANGE_FILE_NAME;
   if ( flags & DIRCHANGE_DIR_NAME )   m_Flags |= FILE_NOTIFY_CHANGE_DIR_NAME;
   //if ( flags & DIRCHANGE_ATTRIBUTES ) m_Flags |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
   //if ( flags & DIRCHANGE_SIZE )       m_Flags |= FILE_NOTIFY_CHANGE_SIZE;
   if ( flags & DIRCHANGE_LAST_WRITE ) m_Flags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

   // Need an event to work with ReadDirectoryChangesW asyncronously.
   m_Overlapped.hEvent=::CreateEvent( NULL, true, false, NULL ); 
   if ( !m_Overlapped.hEvent )
   {
      Clear();
      return false;
   }

   // Start the asyncronous watch.
   DWORD dummy = 0;
   if ( !ReadDirectoryChangesW(  m_Handle,   
                                 m_Buffer,
                                 m_BufferSize,
                                 true,
                                 m_Flags,
                                 &dummy,
                                 &m_Overlapped,
                                 NULL ) )
   {
      Clear();
      return false;
   }

   m_Exclude = exclude;

   return true;
}

int DirWatcherWinNT::GetSignaled( wxArrayString* signaled )
{
   wxASSERT( signaled );

   if ( m_Handle == INVALID_HANDLE_VALUE )
      return 0;

   DWORD bytes;
   if ( !::GetOverlappedResult(  m_Handle, 
                                 &m_Overlapped,
                                 &bytes,
                                 false ) )
   {
      // Make sure it's just incomplete io... else
      // the watch is dead and we need to recreate it.
      if ( GetLastError() != ERROR_IO_INCOMPLETE )
      {
         Clear();
      }

      return 0;
   }

   // We got some results... process them.
   int signals = 0;
   wxChar* curr = m_Buffer;
   wxChar file[MAX_PATH];
   wxFileName fixup;
   wxString dir;
   for ( ; bytes != 0; )
   {
      FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)curr;

      // Get the changed file.
      const size_t chars = fni->FileNameLength / sizeof( WCHAR );
      WideCharToMultiByte( CP_ACP, WC_NO_BEST_FIT_CHARS, 
         fni->FileName, chars, 
         file, MAX_PATH, NULL, NULL );
      file[ chars ] = 0;

      // Look to see if this is an excluded file/folder.
      fixup.Assign( file );
      if ( !IsExcluded( fixup ) )
      {
         
         // The caller only expects to get changed
         // directories... so suck out the parent
         // folder of the changed item.
         fixup.MakeAbsolute( m_Dir );
         dir = fixup.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
         
         // We'll get repeated folders, so lets filter those out.
         if ( signaled->Index( dir, false ) == wxNOT_FOUND )
         {
            signaled->Add( dir );
            ++signals;
         }
      }

      if ( fni->NextEntryOffset == 0 )
         break;

      curr += fni->NextEntryOffset;
   } 
   
   // Restart the watch.
   DWORD dummy;
   if ( !ReadDirectoryChangesW(  m_Handle,   
                                 m_Buffer,
                                 m_BufferSize,
                                 true,
                                 m_Flags,
                                 &dummy,
                                 &m_Overlapped,
                                 NULL ) )
   {
      // The watch couldn't be restarted... we lost
      // the folder we were watching!
      Clear();
   }

   // If we got some results try again to see if we
   // get more... by buffering like this we ensure
   // we get as few updates as possible.
   if ( signals > 0 )
      signals += GetSignaled( signaled );

   return signals;
}

bool DirWatcherWinNT::IsExcluded( const wxFileName& path ) const
{
   // First check to see if the whole path is excluded
   if ( m_Exclude.Index( path.GetFullPath(), false ) != wxNOT_FOUND )
      return true;

   // Check the name.
   if ( m_Exclude.Index( path.GetFullName(), false ) != wxNOT_FOUND )
      return true;

   // Now check to see if any of the dir components are excluded.
   const wxArrayString& dirs = path.GetDirs();
   for ( int i=0; i < dirs.GetCount(); i++ )
   {
      if ( m_Exclude.Index( dirs[i], false ) != wxNOT_FOUND )
         return true;
   }

   return false;
}

void DirWatcherWinNT::Clear( )
{
   m_Flags = 0;
   m_Dir.Empty();
   m_Exclude.Empty();

   if ( m_Handle != INVALID_HANDLE_VALUE )
   {
      ::CloseHandle( m_Handle );
      m_Handle = INVALID_HANDLE_VALUE;
   }

   if ( m_Overlapped.hEvent )
      ::CloseHandle( m_Overlapped.hEvent );

   ::ZeroMemory( &m_Overlapped, sizeof( m_Overlapped ) );
   ::ZeroMemory( m_Buffer, m_BufferSize );
}
