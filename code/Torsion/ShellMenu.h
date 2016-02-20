// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SHELLMENU_H
#define TORSION_SHELLMENU_H
#pragma once


struct IContextMenu2;


class ShellMenu : public wxMenu
{
   public:

      ShellMenu( const wxString& path );
      virtual ~ShellMenu();

      WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);

   protected:

      bool PopulateMenu();

      IContextMenu2*    m_ContextHandler;
      bool              m_IsInitialized;
      wxFileName        m_File;
};

#endif // TORSION_SHELLMENU_H