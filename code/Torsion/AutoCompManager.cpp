// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompManager.h"

#include "TorsionApp.h"
#include "AutoCompExports.h"
#include "AutoCompThread.h"
#include "MainFrame.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(EvtHandlerList);

IMPLEMENT_CLASS(tsAutoCompStateHint, wxObject)

DEFINE_EVENT_TYPE(tsEVT_AUTOCOMPDATA_UPDATED)

AutoCompManager::AutoCompManager()
   :  m_Data( new AutoCompData() ),
      m_Thread( NULL ),
      m_Exports( NULL )
{
   SetEnable( tsGetPrefs().GetCodeCompletion() );
}

void AutoCompManager::SetEnable( bool enable )
{
   if ( enable && !m_Thread )
   {
      m_Thread = new AutoCompThread;
      m_Thread->Create();
      m_Thread->SetPriority( 40 ); // THREAD_PRIORITY_BELOW_NORMAL
      m_Thread->Run();

      SetProjectPath( m_ProjectPath, m_Mods, m_ScriptExts );
      SetExports( m_Exports );

      // Holla to a brotha.
      tsAutoCompStateHint hint;
      hint.isEnabled = true;
      if ( tsGetMainFrame() )
         tsGetMainFrame()->SendHintToAllViews( &hint, true );
   }
   else if ( !enable && m_Thread )
   {
      // Holla to a brotha.
      tsAutoCompStateHint hint;
      hint.isEnabled = false;
      if ( tsGetMainFrame() )
         tsGetMainFrame()->SendHintToAllViews( &hint, true );

      Clear();

      _DeleteThread();
   }
}

bool AutoCompManager::HasData()
{
   // Are we even running?
   if ( m_Thread == NULL )
      return false;

   // Ok check the data itself.
   const AutoCompData* data = Lock();
   if ( !data )
      return false;

   bool hasData = data->HasData();
 
   Unlock();

   return hasData;
}

AutoCompManager::~AutoCompManager()
{
   _DeleteThread();
   wxDELETE( m_Data );
}

void AutoCompManager::_DeleteThread()
{
   // Shutdown the thread...
   if ( !m_Thread )
      return;

   // Give it top priority and a little time before we
   // try to kill it in the hopes that it will settle
   // down and finish whatever it is chewing on.
   m_Thread->SetPriority( WXTHREAD_MAX_PRIORITY );
   wxThread::Sleep( 250 );

   // Let the thread delete itself.
   if ( m_Thread->Delete() == wxTHREAD_KILLED )
   {
      // If the thread was killed then it is unsafe to 
      // free the memory associated with it or the data.
      // We just drop the pointers and leave the OS to 
      // cleanup our mess.
      m_Thread = NULL;
      m_Data = NULL;
      return;
   }

   // Should be safe to delete now.
   wxDELETE( m_Thread );
}

void AutoCompManager::Clear()
{
   if ( m_Thread )
   {
      m_Thread->SetExports( NULL );
      m_Thread->SetPath( wxEmptyString, wxArrayString(), wxArrayString() );
   }

   wxDELETE( m_Exports );

   m_ExportedFunctions.Clear();
   m_ExportedVars.Clear();

   wxMutexLocker locker( m_DataLock );
   wxDELETE( m_Data );
   m_Data = new AutoCompData();

   SendNotify();     
}

void AutoCompManager::SetProjectPath( const wxString& path, const wxArrayString& mods, const wxArrayString& scriptExts )
{
   m_ProjectPath = path;

   // NOTE: The first thing the assignment of arrays does
   // is clear the current array... so never allow the same
   // array to assign to itself!
   if ( &m_Mods != &mods )
      m_Mods = mods;
   if ( &m_ScriptExts != &scriptExts )
      m_ScriptExts = scriptExts;

   if ( m_Thread )
      m_Thread->SetPath( m_ProjectPath, m_Mods, m_ScriptExts );
}

bool AutoCompManager::LoadExports( const wxString& path )
{
   AutoCompExports* exports = new AutoCompExports();
   if ( !exports->LoadXml( path ) ) 
   {
      delete exports;
      return false;
   }

   SetExports( exports );

   return true;
}

void AutoCompManager::SetExports( AutoCompExports* exports )
{
   if ( m_Exports != exports )
   {
      wxDELETE( m_Exports );
      m_Exports = exports;

      m_ExportedFunctions.Clear();
      m_ExportedVars.Clear();
   }

   if ( m_Exports )
   {
      m_Exports->GetFunctionsString( &m_ExportedFunctions, ' ' );

      // HACK: We should fix the syntax highligher to support
      // object name highlighting differently.
      m_ExportedFunctions.Append( ' ' );
      m_Exports->GetClassString( &m_ExportedFunctions, ' ' );
      m_ExportedFunctions.MakeLower();

      m_Exports->GetVarsString( &m_ExportedVars, ' ' );
      m_ExportedVars.MakeLower();
   }

   if ( m_Thread )
   {
      m_Thread->SetExports( m_Exports );
      m_Exports = NULL;
   }
}

void AutoCompManager::ClearExports()
{
   if ( m_Thread )
      m_Thread->SetExports( NULL );

   wxDELETE( m_Exports );

   m_ExportedFunctions.Clear();
   m_ExportedVars.Clear();
}

AutoCompText* AutoCompManager::AddActivePage( const wxString& path )
{
   if ( m_Thread )
      return m_Thread->AddActivePage( path );

   return NULL;
}

void AutoCompManager::RemoveActivePage( AutoCompText* text )
{
   if ( m_Thread )
      m_Thread->RemoveActivePage( text );
}

void AutoCompManager::AddNotify( wxEvtHandler* target )
{
   if ( m_NotifyList.Find( target ) )
      return;

   m_NotifyList.Append( target );
}

void AutoCompManager::RemoveNotify( wxEvtHandler* target )
{
   EvtHandlerList::Node* node = m_NotifyList.Find( target );
   if ( !node )
      return;

   m_NotifyList.Erase( node );
}

bool AutoCompManager::UpdateData( const AutoCompData* data )
{
   wxASSERT( data );

   // Make sure we can lock the current data.
   if ( m_DataLock.TryLock() != wxMUTEX_NO_ERROR )
      return false;

   wxASSERT( m_Data );
   delete m_Data;
   m_Data = data;

   m_DataLock.Unlock();

   SendNotify();

   return true;
}

void AutoCompManager::SendNotify()
{
   // Let whoever is listening know we've updated!
   wxCommandEvent event( tsEVT_AUTOCOMPDATA_UPDATED, wxID_ANY );
   EvtHandlerList::Node* node = m_NotifyList.GetFirst();
   while ( node )
   {
      ::wxPostEvent( node->GetData(), event );
      node = node->GetNext();
   }
}

const AutoCompData* AutoCompManager::Lock()
{
   m_DataLock.Lock();
   wxASSERT( m_Data );
   return m_Data;
}

void AutoCompManager::Unlock()
{
   wxASSERT( m_Data );
   m_DataLock.Unlock();
}
