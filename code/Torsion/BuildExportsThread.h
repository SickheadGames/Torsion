// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_BUILDEXPORTSTHREAD_H
#define TORSION_BUILDEXPORTSTHREAD_H
#pragma once

#include <wx/thread.h>

#include "AutoCompPage.h"
#include "AutoCompExports.h"
#include "ProjectDoc.h"


class BuildExportsThread : public wxThread
{
public:

   BuildExportsThread(  const wxString& exportScriptPath,
                        const wxString& workingDir,
                        const ProjectConfigArray& Configs );

   virtual ~BuildExportsThread();

   float GetProgressPct();

   AutoCompExports* TakeExports();

protected:

   void _AddProgress( float inc );

   wxFileName           m_SourceExportScript;
   wxString             m_WorkingDir;
   ProjectConfigArray   m_Configs;

   wxCriticalSection    m_DataLock;
   float                m_Progress;

   // Stuff to cleanup after canceled export.
   wxFileName              m_ExportScript;
   AutoCompExportsArray    m_ConfigExports;
   AutoCompExports*        m_Exports;

   virtual ExitCode Entry();
   virtual void OnExit();

};

#endif // TORSION_BUILDEXPORTSTHREAD_H
