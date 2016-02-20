// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "StackDump.h"

#include "BuildVersion.h"

#include <wx/file.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


//#include <psapi.h>

StackDump::StackDump()
{
    /*
    HMODULE hMods[1024];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;
    unsigned int i;
    if ( EnumProcessModules( hProcess, hMods, sizeof( hMods ), &cbNeeded ) )
    {
        for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
            char szModName[MAX_PATH];

            // Get the full path to the module's file.
            if ( GetModuleFileNameEx( hProcess, hMods[i], szModName,
                                      sizeof(szModName)))
            {
                // Print the module name and handle value.
                //printf("\t%s (0x%08X)\n", szModName, hMods[i] );
            }
        }
   }
   */
}

StackDump::~StackDump()
{
}

void StackDump::Dump( wxString* address, wxString* data )
{
   wxASSERT( address );
   wxASSERT( data );

   m_Address = address;
   m_Data = data;

   m_Data->Clear();
   m_Data->Alloc( 1024 );

   // Add some basic info first!
   (*m_Data) << tsGetFullBuildString() << "\r\n";
   (*m_Data) << wxGetOsDescription() << "\r\n\r\n";

   // Do the walk.
   WalkFromException();
}

void StackDump::OnStackFrame( const wxStackFrame& frame )
{
   // To lookup the file and line number in the map file
   // we need two values.... the address of the function
   // and the offset within that function.

   DWORD funcAddress = (DWORD)frame.GetAddress() - frame.GetOffset();

   wxString address;
   address.Printf( "0x%.8x:%.6d", funcAddress, frame.GetOffset() );
   if ( m_Address->IsEmpty() )
      *m_Address = address;

   (*m_Data) << address << '\t' << frame.GetModule() << "\t" << frame.GetName() << "\r\n";
}



