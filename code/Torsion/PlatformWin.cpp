// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "PlatformWin.h"

#include <wx/dynlib.h>
#include <wx/msw/private.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

//TODO ? extern wxMenu *wxCurrentPopupMenu;
wxMenu *wxCurrentPopupMenu;


void tsRaiseHWND( HWND hWnd )
{
   if ( GetForegroundWindow() == hWnd )
      return;

   // Since since 98 and 2000 was released, SetForegroundWindow would
   // only flash the taskbar icon if the calling thread was not the
   // current forground thread.  This works by a timeout value from the
   // last input event the thread has gotten.
   //
   // The trick here is to simulate a dummy key first to circumvent 
   // this "feature".
   //
   keybd_event( 0, 0, 0, 0 );
   SetForegroundWindow( hWnd ); 
}


BOOL CALLBACK RaiseWindowFromPid( HWND hwnd, LPARAM lParam )
{
	DWORD Pid = lParam;
	DWORD WindowPid;
	::GetWindowThreadProcessId( hwnd, &WindowPid );
	if ( Pid == WindowPid ) 
   {
      tsRaiseHWND( hwnd );
		return FALSE;
	}

	return TRUE;
}


void tsRaiseWindowFromProcessId( long pid )
{
	EnumWindows( RaiseWindowFromPid, pid ); 
}


void tsRaiseWindow( wxWindow* window )
{
   wxASSERT( window );

   HWND hWnd = window->GetHandle();
   tsRaiseHWND( hWnd );
}


bool tsExecuteVerb( const wxString& path, const wxString& verb, bool noUI )
{
   // TODO: Test not using DDE wait and passing the strings
   // in static char strings.
   SHELLEXECUTEINFO sei;
   ZeroMemory( &sei, sizeof( sei ) );
   sei.cbSize = sizeof(sei);
   sei.lpFile = path.c_str();
   sei.lpVerb = verb.c_str();
   sei.fMask  = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_DDEWAIT | ( noUI ? SEE_MASK_FLAG_NO_UI : 0 );
   sei.nShow  = SW_SHOWNORMAL;
   return ShellExecuteEx( &sei ) ? true : false; 
}

/*
  HWND hwnd = ::FindWindow(_T("wxWindowClass­"),APP_NAME); 
  if (hwnd) { 
    SetForegroundWindow(hwnd);  // <-This works, but is non-portable. 
  } else { 
    wxMessageBox(wxT("Failed to find that window!"),APP_NAME,wxOK); 
  } 
}
*/

bool tsShellOpenAs( const wxString& path )
{
   //
   //    %WINDIR%\rundll32.exe shell32.dll,OpenAs_RunDLL
   //

   wxString command;
   command << "rundll32.exe shell32.dll,OpenAs_RunDLL " << path;
   return wxExecute( command, wxEXEC_ASYNC, NULL ) != 0;
}

bool tsSetReadOnly( const char* name, bool readOnly )
{
   return _chmod( name, readOnly ? _S_IREAD : _S_IREAD | _S_IWRITE ) == 0;
}

void NullDelimitedBuffer( const wxArrayString& strings, wxMemoryBuffer* result )
{
   wxASSERT( result );

   if ( strings.IsEmpty() ) 
   {
      // Just return the double null terminator.
      result->AppendByte( 0 );
      result->AppendByte( 0 );
      return;
   }
   
   wxFileName file;
   wxString fixed;;

   for ( int i=0; i < strings.GetCount(); i++ ) {
      
      file = strings[i];
      if ( file.IsDir() )
         fixed = file.GetPath( wxPATH_GET_VOLUME );
      else 
         fixed = file.GetFullPath();

      result->AppendData( (void*)fixed.wc_str(), fixed.Len() + 1 );
   }

   // We should have a double null terminator now.
   result->AppendByte( 0 );
}

bool tsSendToRecycleBin( const wxArrayString& source, bool noConfirm )
{
   wxMemoryBuffer del;
   NullDelimitedBuffer( source, &del );

   SHFILEOPSTRUCT fileop; 
   ZeroMemory( &fileop, sizeof(fileop) ); 
   fileop.wFunc    = FO_DELETE; 
   fileop.pFrom = (LPWSTR)del.GetData();
   fileop.fFlags   = FOF_SILENT | FOF_ALLOWUNDO | ( noConfirm ? FOF_NOCONFIRMATION : 0 ) | 0x2000 /* FOF_NO_CONNECTED_ELEMENTS */;
   return SHFileOperation( &fileop ) == 0 && fileop.fAnyOperationsAborted == false;
}

bool tsMoveFiles( const wxArrayString& source, const wxString& destFolder )
{
   wxASSERT( wxFileName( destFolder ).IsDir() );
   wxArrayString dest;
   dest.Add( destFolder );

   wxMemoryBuffer from, to;
   NullDelimitedBuffer( source, &from );
   NullDelimitedBuffer( dest, &to );

   SHFILEOPSTRUCT fileop; 
   ZeroMemory( &fileop, sizeof(fileop) ); 
   fileop.wFunc    = FO_MOVE; 
   fileop.pFrom    = (LPWSTR)from.GetData(); ; 
   fileop.pTo      = (LPWSTR)to.GetData(); ; 
   fileop.fFlags   = FOF_SILENT | FOF_ALLOWUNDO | 0x2000 /* FOF_NO_CONNECTED_ELEMENTS */;

   return SHFileOperation( &fileop ) == 0;
}

bool tsCopyFiles( const wxArrayString& source, const wxString& destFolder )
{
   wxASSERT( wxFileName( destFolder ).IsDir() );
   wxArrayString dest;
   dest.Add( destFolder );

   wxMemoryBuffer from, to;
   NullDelimitedBuffer( source, &from );
   NullDelimitedBuffer( dest, &to );

   SHFILEOPSTRUCT fileop; 
   ZeroMemory( &fileop, sizeof(fileop) ); 
   fileop.wFunc    = FO_COPY; 
   fileop.pFrom    = (LPWSTR)from.GetData(); ; 
   fileop.pTo      = (LPWSTR)to.GetData(); ; 
   fileop.fFlags   = FOF_SILENT | FOF_ALLOWUNDO | 0x2000 /* FOF_NO_CONNECTED_ELEMENTS */;

   return SHFileOperation( &fileop ) == 0;
}

int tsTrackPopupMenu( wxMenu* menu, bool leftClick, const wxPoint& point, wxWindow* parent )
{
   wxASSERT( menu );
   wxASSERT( parent );

   menu->SetInvokingWindow( parent );
   menu->UpdateUI();

   HWND hWnd = parent->GetHandle();
   HMENU hMenu = GetHmenuOf( menu );
   wxPoint screen;

   if ( point == wxDefaultPosition )
      screen = wxGetMousePosition();
   else
      screen = parent->ClientToScreen( point );

   wxCurrentPopupMenu = menu;
   UINT mflags = ( leftClick ? TPM_LEFTBUTTON : TPM_RIGHTBUTTON ) | TPM_NONOTIFY | TPM_RETURNCMD;

   int result = ::TrackPopupMenu( hMenu, mflags, screen.x, screen.y, 0, hWnd, NULL );

   // we need to do it righ now as otherwise the events are never going to be
   // sent to wxCurrentPopupMenu from HandleCommand()
   //
   // note that even eliminating (ugly) wxCurrentPopupMenu global wouldn't
   // help and we'd still need wxYieldForCommandsOnly() as the menu may be
   // destroyed as soon as we return (it can be a local variable in the caller
   // for example) and so we do need to process the event immediately
   //wxYieldForCommandsOnly();

   wxCurrentPopupMenu = NULL;

   menu->SetInvokingWindow(NULL);

   return result;
}

BOOL CALLBACK PidHasNonConsoleWindow( HWND hwnd, LPARAM lParam )
{
	DWORD pid = lParam;
	DWORD windowPid;
	::GetWindowThreadProcessId( hwnd, &windowPid );

   wchar_t name[MAX_PATH];
   GetClassName( hwnd, name, MAX_PATH );
	if ( pid == windowPid && IsWindowVisible( hwnd ) ) {

      // These are the console class names for NT and 9x.
      if (  wxStrcmp( name, "ConsoleWindowClass" ) != 0 &&
		  wxStrcmp( name, "tty" ) != 0 )
      {
         return FALSE;
      }
	}

	return TRUE;
}

bool tsProcessHasNonConsoleWindow( long pid )
{
   return EnumWindows( PidHasNonConsoleWindow, pid ) == FALSE;
}

long tsMswGetProcessId( HANDLE handle )
{
   // First try GetProcessId.       
   wxDynamicLibrary kernel32( "kernel32.dll" );
   if (kernel32.IsLoaded())
   {
      typedef DWORD (WINAPI GETSYM)(HANDLE);

      GETSYM* getSymbol = (GETSYM*)kernel32.GetSymbol( "GetProcessId" );
      if ( getSymbol )
         return getSymbol( handle );
   }

   // Fall back to NtQueryInformationProcess.
   wxDynamicLibrary ntdll( "ntdll.dll" );
   if (ntdll.IsLoaded())
   {
      typedef NTSTATUS (WINAPI NTQIP)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

      NTQIP* ntQueryInformationProcess = (NTQIP*)ntdll.GetSymbol( "NtQueryInformationProcess" );
      if ( ntQueryInformationProcess )
      {
         PROCESS_BASIC_INFORMATION pbi;
         DWORD dwSize;
         ntQueryInformationProcess(handle, ProcessBasicInformation, 
            &pbi, sizeof(pbi), &dwSize);

         return pbi.UniqueProcessId;
      }
   }

   return 0;
}

void tsBellEx( int style )
{
   if (style == -1)
   {
      ::MessageBeep((UINT)-1);        // default sound
      return;
   }

   int msStyle;
   if (style & wxOK)
      msStyle = MB_OK;
   else if (style & wxICON_EXCLAMATION)
      msStyle = MB_ICONEXCLAMATION;
   else if (style & wxICON_HAND)
      msStyle = MB_ICONHAND;
   else if (style & wxICON_INFORMATION)
      msStyle = MB_ICONINFORMATION;
   else if (style & wxICON_QUESTION)
      msStyle = MB_ICONQUESTION;

   ::MessageBeep( msStyle );
}

bool tsGetIconForFile( const wxString& file, bool largeIcon, wxIcon* result )
{
   // TODO: Add function to get icon overlays! SHGetIconOverlayIndex 

   SHFILEINFO info;
   if ( SHGetFileInfo( file.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof( info ), 
      SHGFI_ICON | ( largeIcon ? SHGFI_LARGEICON : SHGFI_SMALLICON ) ) == 0 )
      return false;

   result->SetHICON( (WXHICON)info.hIcon );

   if ( largeIcon )
      result->SetSize( ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON) );
   else
      result->SetSize( ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON) );

   return true;
}
