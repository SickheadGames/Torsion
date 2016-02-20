// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "DirWatcherWin9x.h"

#include <wx/dir.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


DirWatcherWin9x::DirWatcherWin9x()
   :  m_Handle( INVALID_HANDLE_VALUE ),
      m_Flags( 0 )
{
}

DirWatcherWin9x::~DirWatcherWin9x()
{
   Clear();
}

bool DirWatcherWin9x::SetWatch( const wxString& dir, int flags, const wxArrayString& exclude )
{
   Clear();

   DWORD dwFlags = 0;
   if ( flags & DIRCHANGE_FILE_NAME )  dwFlags |= FILE_NOTIFY_CHANGE_FILE_NAME;
   if ( flags & DIRCHANGE_DIR_NAME )   dwFlags |= FILE_NOTIFY_CHANGE_DIR_NAME;
   //if ( flags & DIRCHANGE_ATTRIBUTES ) dwFlags |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
   //if ( flags & DIRCHANGE_SIZE )       dwFlags |= FILE_NOTIFY_CHANGE_SIZE;
   if ( flags & DIRCHANGE_LAST_WRITE ) dwFlags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

   if ( !_SetWatch( dir, dwFlags, exclude ) )
      return false;

   m_Flags = dwFlags;
   m_Exclude = exclude;

   return true;
}

bool DirWatcherWin9x::_SetWatch( const wxString& dir, DWORD flags, const wxArrayString& exclude )
{
   wxDir fixed( dir );
   if ( !fixed.IsOpened() )
      return false;

   m_Handle = ::FindFirstChangeNotification( fixed.GetName(), FALSE, flags );
   if ( m_Handle == INVALID_HANDLE_VALUE )
      return false;

   m_Dir = fixed.GetName();

   _UpdateSubdirs( flags, exclude );

   return true;
}

int DirWatcherWin9x::GetSignaled( wxArrayString* signaled )
{
   wxASSERT( signaled );
   return _GetSignaled( signaled, m_Flags, m_Exclude );
}

int DirWatcherWin9x::_GetSignaled( wxArrayString* signaled, DWORD flags, const wxArrayString& exclude )
{
   wxASSERT( signaled );

   if ( m_Handle == INVALID_HANDLE_VALUE )
      return 0;

   size_t signals = 0;
   bool wasSignaled = false;

   DWORD status = ::WaitForSingleObject( m_Handle, 0 );
   if ( status != WAIT_TIMEOUT )
   {
      // If the dir no longer exists the parent watch 
      // will have fired... so clear us and return.
      if (  !wxFileName::DirExists( m_Dir ) ||
            !::FindNextChangeNotification( m_Handle ) )
      {
         // If the restart failed the folder was deleted or 
         // renamed, so clear our children and return.
         Clear();
         return 0;
      }

      // Add this to the signaled array.
      signaled->Add( m_Dir );
      wasSignaled = true;
      ++signals;
   }

   // Check for signals in the subdirs.
   DirWatcherWin9xMap::iterator iter = m_Subdirs.begin();
   for ( ; iter != m_Subdirs.end(); )
   {
      DirWatcherWin9x* watcher = iter->second;
      wxASSERT( watcher );

      signals += watcher->_GetSignaled( signaled, flags, exclude );

      // If this one is dead... delete it.
      if ( !watcher->_IsOk() )
      {
         delete watcher;
         DirWatcherWin9xMap::iterator next = iter;
         next++;
         m_Subdirs.erase( iter );
         iter = next;
         continue;
      }

      iter++;
   }

   // If we were signaled we need to update our children.
   if ( wasSignaled )
      _UpdateSubdirs( flags, exclude );

   return signals;
}

void DirWatcherWin9x::_UpdateSubdirs( DWORD flags, const wxArrayString& exclude )
{
   wxDir dir( m_Dir );
   if ( !dir.IsOpened() )
      return;

   wxString prefix( m_Dir );
   prefix << wxFILE_SEP_PATH;

   wxString subdir;
   bool cont = dir.GetFirst(&subdir, wxEmptyString, wxDIR_DIRS | wxDIR_HIDDEN);
   for ( ; cont; cont = dir.GetNext(&subdir) )
   {
      const wxString fulldir = prefix + subdir;

      // Is this one we exclude?
      if ( exclude.Index( subdir, false ) != wxNOT_FOUND ||
           exclude.Index( fulldir, false ) != wxNOT_FOUND )
         continue;

      // Do we have this one already?
      if ( m_Subdirs.find( subdir ) != m_Subdirs.end() )
         continue;

      // Got a new one... add it.
      DirWatcherWin9x* watcher = new DirWatcherWin9x;
      if ( !watcher->_SetWatch( fulldir, flags, exclude ) )
      {
         delete watcher;
         continue;
      }
      m_Subdirs[subdir] = watcher;
   }
}

void DirWatcherWin9x::Clear( )
{
   m_Exclude.Empty();
   m_Flags = 0;

   if ( m_Handle != INVALID_HANDLE_VALUE )
   {
      ::FindCloseChangeNotification( m_Handle );
      m_Handle = INVALID_HANDLE_VALUE;
   }

   WX_CLEAR_HASH_MAP( DirWatcherWin9xMap, m_Subdirs );
}
