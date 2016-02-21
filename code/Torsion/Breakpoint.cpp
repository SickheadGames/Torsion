// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "Breakpoint.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

DEFINE_LOCAL_EVENT_TYPE(EVT_COMMAND_BREAKPOINT)


bool Breakpoint::IsFile( const wxString& File ) const
{
	wxFileName CmpFile( File );
	return m_File == CmpFile;
}

bool Breakpoint::operator ==( const Breakpoint& b ) const
{
   return   m_File == b.m_File &&
            m_Line == b.m_Line &&
            m_Condition == b.m_Condition &&
            m_Pass == b.m_Pass &&
            m_Enabled == b.m_Enabled;
}

wxString Breakpoint::GetToolTip() const
{
   wxString tooltip;
   tooltip << "At " << m_File.GetFullPath() << ", line " << m_Line;

   if ( !m_Condition.IsEmpty() || m_Pass > 0 )
      tooltip << ", ";

   if ( !m_Condition.IsEmpty() )
      tooltip << "when '" << m_Condition << "' is true";

   if ( !m_Condition.IsEmpty() && m_Pass > 0 )
      tooltip << " ";

   if ( m_Pass > 0 )
      tooltip << "and pass count is equal to " << m_Pass;

   return tooltip;
}
