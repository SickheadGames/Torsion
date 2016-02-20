// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPPAGE_H
#define TORSION_AUTOCOMPPAGE_H
#pragma once

#include "AutoCompClass.h"

class AutoCompExports;
class AutoCompPageArray;


class AutoCompPage
{
   public:

      AutoCompPage( const wxString& name );
      AutoCompPage( const AutoCompPage& page );
      ~AutoCompPage();

      void Clear();

      void AddFunction( AutoCompFunction* func );

      AutoCompClass*    AddObject( const wxString& name, const wxString& base = wxEmptyString );
      AutoCompVar*      AddVar( const wxString& name );

      const wxString& GetName() const { return m_Name; }

      const AutoCompClassArray&     GetObjects() const { return m_Objects; }
      const AutoCompFunctionArray&  GetFunctions() const { return m_Functions; }
      const AutoCompVarArray&       GetVars() const { return m_Vars; }

      static AutoCompPage* Find( const wxString& Name, const AutoCompPageArray& Array );

      const wxDateTime& GetLastUpdate() const { return m_LastUpdate; }
      void SetLastUpdate( const wxDateTime& updated ) { m_LastUpdate = updated; }

   protected:

      wxString m_Name;

      wxDateTime              m_LastUpdate;

      AutoCompClassArray      m_Objects;
      AutoCompFunctionArray   m_Functions;
      AutoCompVarArray        m_Vars;
};

WX_DEFINE_SORTED_ARRAY( AutoCompPage*, AutoCompPageArray );

/*
typedef AutoCompClassArray& (*AutoCompPageGetArrayFunc)(AutoCompPage*);

inline AutoCompClassArray& GetAutoCompClassArray( AutoCompPage* page ) 
{ 
   wxASSERT( page );
   return page->m_Classes; 
}

inline AutoCompClassArray& GetAutoCompDatablockArray( AutoCompPage* page ) 
{ 
   wxASSERT( page );
   return page->m_Datablocks; 
}
*/

#endif // TORSION_AUTOCOMPPAGE_H
