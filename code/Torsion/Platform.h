// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PLATFORM_H
#define TORSION_PLATFORM_H
#pragma once


/*
	Simple function for raising the top level
   window for a process to the forground.
*/
void tsRaiseWindowFromProcessId( long pid );


/*
	This call forces a window to the forground.
*/
void tsRaiseWindow( wxWindow* window );


/*
   Launches the path using the selected verb.
*/
bool tsExecuteVerb( const wxString& path, const wxString& verb, bool noUI = false );


/*
   Launches and operating system appropriate "open as" dialog.
*/
bool tsShellOpenAs( const wxString& path );


/*
   Changes the read-only flag on a file or folder.
*/
bool tsSetReadOnly( const char* name, bool readOnly );


/*
   It moves the files and/or folders into the system
   recycle bin or just deletes them if not avalible on
   the current platform.
*/
bool tsSendToRecycleBin( const wxArrayString& source, bool noConfirm );


/*
   It shows the appropriate confirmation dialog for
   overwrites, if they are to occur, and then moves
   the file or dir into the dest folder.
*/
bool tsMoveFiles( const wxArrayString& source, const wxString& destFolder );

/*
   It shows the appropriate confirmation dialog for
   overwrites, if they are to occur, and then copies
   the file or dir into the dest folder.
*/
bool tsCopyFiles( const wxArrayString& source, const wxString& destFolder );


/*
   Pops up a menu and tracks it, returing the command 
   id of the selected item or zero if the menu was canceled.
*/
int tsTrackPopupMenu( wxMenu* menu, bool leftClick, const wxPoint& point, wxWindow* parent );


/*
   This function checks to see if the process has
   a window which isn't a console window.
*/
bool tsProcessHasNonConsoleWindow( long pid );


/*
   Convert a process handle into a process id.
*/
long tsMswGetProcessId( HANDLE handle );


/*
   An extended version of bell which accepts the following
   values.

      -1
      MB_OK
      wxICON_EXCLAMATION
      wxICON_HAND 
      wxICON_ERROR 
      wxICON_QUESTION 
      wxICON_INFORMATION
*/
void tsBellEx( int style = -1 );


/*
   
*/
bool tsGetIconForFile( const wxString& file, bool largeIcon, wxIcon* result );


/*
*/
void tsShellPopupMenu( const wxString& path, const wxPoint& point, wxWindow* parent );


#endif // TORSION_PLATFORM_H