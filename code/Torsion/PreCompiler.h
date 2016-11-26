// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PRECOMPILER_H
#define TORSION_PRECOMPILER_H
#pragma once

#include <wx/dir.h>


DECLARE_LOCAL_EVENT_TYPE(tsEVT_PRECOMPILER_OUTPUT, -1)
DECLARE_LOCAL_EVENT_TYPE(tsEVT_PRECOMPILER_DONE, -1)

class PreCompilerThread;

class PreCompiler
{
public:

   PreCompiler();
   virtual ~PreCompiler();

   // TODO: Add setters and remove all these args!
   bool Run(   const wxString& workingDir, 
               const wxString& exec, 
               const wxArrayString& mods, 
               const wxString& activeScript, 
               bool clearDSOs, 
               bool setModPaths, 
               wxEvtHandler* outputWnd );

   void Stop();

   void WaitForFinish( bool* success, int* scripts, int* errors );
   
protected:

   PreCompilerThread* m_Thread;
};

#endif // TORSION_PRECOMPILER_H
