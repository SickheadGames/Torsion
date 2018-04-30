// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ShellMenu.h"
#include "Identifiers.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <winternl.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


ShellMenu::ShellMenu( const wxString& path )
   :  m_ContextHandler( NULL ),
      m_IsInitialized( false )
{
   if ( wxFileName::FileExists( path ) )
      m_File.Assign( path );
   else
      m_File.AssignDir( path );
}

ShellMenu::~ShellMenu()
{
   if ( m_ContextHandler )
      m_ContextHandler->Release();
}

bool ShellMenu::PopulateMenu()
{
   m_IsInitialized = true;
   
   IShellFolder* desktop = NULL;
   HRESULT hr = SHGetDesktopFolder(&desktop);
   if ( FAILED( hr ) || !desktop )
      return false;

   // Needed to free the pidl.
   IMalloc* malloc = NULL;
   hr = SHGetMalloc( &malloc );
   if ( FAILED( hr ) || !malloc )
      return false;

   // Get the item path in unicode.
   OLECHAR itemName[MAX_PATH];
   OLECHAR itemDir[MAX_PATH];
   {
      wxString name, dir;
      if ( m_File.IsDir() )
      {
         wxFileName dummy( m_File );
         name = dummy.GetDirs()[ dummy.GetDirCount() - 1 ];
         dummy.RemoveLastDir();
         dir = dummy.GetPath();
      }
      else
      {
         name = m_File.GetFullName();
         dir = m_File.GetPath();
      }

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name.c_str(), -1, itemName, MAX_PATH);
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, dir.c_str(), -1, itemDir, MAX_PATH);
   }

   // Get the pidl of the parent and of the item.
   ITEMIDLIST* pidl = NULL;
   hr = desktop->ParseDisplayName(NULL, NULL, itemDir, NULL, &pidl, NULL);
   if ( FAILED( hr ) || !pidl )
   {
      desktop->Release();
      return false;
   }
   
   // Get the parent folder.
   IShellFolder* parent = NULL;
   hr = desktop->BindToObject( pidl, NULL, IID_IShellFolder, (void**)&parent );
   desktop->Release();
   malloc->Free(pidl);
   pidl = NULL; desktop = NULL;
   if ( FAILED( hr ) || !parent )
      return false;

   // Get the pidl of the item.
   hr = parent->ParseDisplayName(NULL, NULL, itemName, NULL, &pidl, NULL);
   if ( FAILED( hr ) || !pidl )
   {
      parent->Release();
      return false;
   }

   hr = parent->GetUIObjectOf( NULL, 1, (LPCITEMIDLIST*)&pidl, IID_IContextMenu, NULL, (void**)&m_ContextHandler );
   malloc->Free(pidl);
   malloc->Release();
   parent->Release();
   if ( FAILED( hr ) || !m_ContextHandler )
      return false;

   hr = m_ContextHandler->QueryInterface(IID_IContextMenu2, (void**)&m_ContextHandler);
   if ( FAILED( hr ) || !m_ContextHandler )
      return false;

   hr = m_ContextHandler->QueryContextMenu((HMENU)GetHMenu(), 0, tsID_SHELL_MIN, tsID_SHELL_MAX, CMF_EXPLORE | CMF_NORMAL | CMF_NODEFAULT);

   return SUCCEEDED( hr );
}

WXLRESULT ShellMenu::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
   UINT uItem;
   //TCHAR szBuf[MAX_PATH];
   CHAR szBuf[MAX_PATH];

   if (  !m_ContextHandler &&
         ( nMsg == WM_INITMENUPOPUP ) &&
         (HMENU)wParam == (HMENU)GetHMenu() &&
         !m_IsInitialized )
   {
      if ( !PopulateMenu() )
         return 1;
   }

   if ( !m_ContextHandler )
      return 1;

   switch (nMsg) 
   {
      case WM_DRAWITEM:
      {
         // If this is for a menu it would be zero.
         if ( wParam != 0 ) 
            break;

         DRAWITEMSTRUCT* di = (DRAWITEMSTRUCT*)lParam;
         if (di->itemID < tsID_SHELL_MIN || di->itemID > tsID_SHELL_MAX)
            break;

         m_ContextHandler->HandleMenuMsg(nMsg, wParam, lParam);
         return 0;
      }

      case WM_MEASUREITEM:
      {
         // If this is for a menu it would be zero.
         if ( wParam != 0 ) 
            break;

         MEASUREITEMSTRUCT * mi = (MEASUREITEMSTRUCT*)lParam;
         if (mi->itemID < tsID_SHELL_MIN || mi->itemID > tsID_SHELL_MAX)
            break;

         m_ContextHandler->HandleMenuMsg(nMsg, wParam, lParam);
         return 0;
      }

      case WM_INITMENUPOPUP:
      {
         m_ContextHandler->HandleMenuMsg(nMsg, wParam, lParam);
         return 0; // handled
      }

      case WM_MENUSELECT:
      {
         // if this is a shell item, get it's descriptive text
         uItem = (UINT) LOWORD(wParam);   
         if( uItem >= tsID_SHELL_MIN && uItem <= tsID_SHELL_MAX ) 
         {
            m_ContextHandler->GetCommandString(uItem-tsID_SHELL_MIN, GCS_HELPTEXT,
               NULL, szBuf, sizeof(szBuf)/sizeof(szBuf[0]) );

            // set the status bar text
            //((CFrameWnd*)(AfxGetApp()->m_pMainWnd))->SetMessageText(szBuf);
            return 0;
         }

         break;
      }

      case WM_COMMAND:
      {
         uItem = (UINT)LOWORD(wParam);   
         if (uItem < tsID_SHELL_MIN || uItem > tsID_SHELL_MAX)
            break;

         wxString dir( m_File.GetPath() );

         CMINVOKECOMMANDINFOEX ici;
         ZeroMemory(&ici, sizeof(ici));
         ici.hwnd = NULL;
         ici.cbSize = sizeof(CMINVOKECOMMANDINFO);
         ici.lpVerbW = MAKEINTRESOURCE(uItem-tsID_SHELL_MIN);
         ici.lpDirectory = dir.c_str();
         ici.nShow = SW_SHOWNORMAL;
         m_ContextHandler->InvokeCommand((CMINVOKECOMMANDINFO*)&ici);

         return 0;
      }

      default:
         break;
   }

   return 1;
}