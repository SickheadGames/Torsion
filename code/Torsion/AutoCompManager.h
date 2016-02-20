// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPMANAGER_H
#define TORSION_AUTOCOMPMANAGER_H
#pragma once

#include "AutoCompData.h"


class AutoCompText;
class AutoCompThread;
class AutoCompExports;

DECLARE_EVENT_TYPE(tsEVT_AUTOCOMPDATA_UPDATED, -1)

WX_DECLARE_LIST(wxEvtHandler, EvtHandlerList);

class AutoCompManager
{
   friend class AutoCompThread;

public:
   AutoCompManager();
   ~AutoCompManager();

public:

   void SetEnable( bool enable );
   
   bool HasData();

   bool LoadExports( const wxString& file );
   void SetExports( AutoCompExports* exports );
   void ClearExports();

   const wxString& GetExportedFunctionsString() const		{ return m_ExportedFunctions; }
	const wxString& GetExportedVarsString() const		   { return m_ExportedVars; }

   void SetProjectPath( const wxString& path, const wxArrayString& mods, const wxArrayString& scriptExts );

   void Clear();

   AutoCompText* AddActivePage( const wxString& path );
   void RemoveActivePage( AutoCompText* text );

   const AutoCompData* Lock();
   void Unlock();

   void AddNotify( wxEvtHandler* target );
   void RemoveNotify( wxEvtHandler* target );

protected:

   void _DeleteThread();

   void SendNotify();

   bool UpdateData( const AutoCompData* data );

   EvtHandlerList m_NotifyList;
   
   wxString          m_ProjectPath;
   wxArrayString     m_Mods;
   wxArrayString     m_ScriptExts;

   AutoCompExports*  m_Exports;
   wxString          m_ExportedFunctions;
   wxString          m_ExportedVars;

   AutoCompThread*   m_Thread;

   wxMutex              m_DataLock;
   const AutoCompData*  m_Data;
};


// A hint sent to the view/doc when the autocomp
// state is toggled from enabled to disabled.
class tsAutoCompStateHint : public wxObject 
{
   DECLARE_CLASS(tsAutoCompStateHint)

public:
   bool  isEnabled;
};

#endif // TORSION_AUTOCOMPMANAGER_H
