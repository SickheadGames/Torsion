// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ProjectConfig.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ProjectConfigArray);

wxString ProjectConfig::GetRelativeExe( const wxString& path ) const
{
   if ( m_Executable.IsEmpty() )
      return wxEmptyString;

   wxFileName exe( m_Executable );
   if ( exe.IsAbsolute() )
      exe.MakeRelativeTo( path );
   return exe.GetFullPath();
}

wxString ProjectConfig::GetExe() const
{
   if ( m_Executable.IsEmpty() )
      return wxEmptyString;

   wxFileName exe( m_Executable );
   //wxASSERT( exe.IsAbsolute() );
   return exe.GetFullPath();
}

void ProjectConfig::SetExe( const wxString& exe )
{
   wxFileName absolute( exe );   
   //wxASSERT( exe.IsEmpty() || absolute.IsAbsolute() );
   m_Executable = absolute.GetFullPath();      
}
