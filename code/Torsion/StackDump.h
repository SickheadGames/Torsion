// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_STACKDUMP_H
#define TORSION_STACKDUMP_H
#pragma once

#include <wx/stackwalk.h>


class StackDump : protected wxStackWalker
{
public:

   StackDump();
   virtual ~StackDump();

   void Dump( wxString* address, wxString* data );

protected:

   virtual void OnStackFrame( const wxStackFrame& frame );

   wxString* m_Address;
   wxString* m_Data;
};

#endif // TORSION_STACKDUMP_H
