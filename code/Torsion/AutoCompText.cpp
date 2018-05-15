// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompText.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

wxCriticalSection AutoCompText::s_DataLock;


AutoCompText::AutoCompText()
   : m_Signaled( false )
{
}

void AutoCompText::Lock()
{ 
   s_DataLock.Enter(); 
}

void AutoCompText::Unlock( bool ClearSignal ) 
{ 
   if ( ClearSignal )
      m_Signaled = false;

   s_DataLock.Leave(); 
}

bool AutoCompText::IsSignaled() const
{
   wxCriticalSectionLocker lock( m_SignalLock );
   return m_Signaled;
}

void AutoCompText::DeleteRange( size_t pos, size_t len )
{
   wxASSERT( pos < m_Data.Len() );
   wxASSERT( pos+len <= m_Data.Len() );

   if ( len == 0 )
      return;

   m_Data.Remove( pos, len );

   wxCriticalSectionLocker lock( m_SignalLock );
   m_Signaled = true;
}

void AutoCompText::InsertRange( size_t pos, const wxString& text )
{
   wxASSERT( pos <= m_Data.Len() );

   size_t len = text.Len();
   if ( len == 0 )
      return;

   m_Data.insert( pos, text );

   wxCriticalSectionLocker lock( m_SignalLock );
   m_Signaled = true;
}

wxChar* AutoCompText::GetWriteBuf( size_t len ) 
{
	wxStringBuffer buf(m_Data, len + 1);
   wxChar* result = buf; // never allow zero!
   m_Data.UngetWriteBuf( len );

   wxCriticalSectionLocker lock( m_SignalLock );
   m_Signaled = true;

   return result;
}
