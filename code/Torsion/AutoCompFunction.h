// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPFUNCTION_H
#define TORSION_AUTOCOMPFUNCTION_H
#pragma once

#include "AutoCompVar.h"

class AutoCompFunction;
WX_DEFINE_SORTED_ARRAY( AutoCompFunction*, AutoCompFunctionArray );


/*
template<class T> int CmpFullNameNoCase( T* first, T* second )
{
   wxASSERT( first );
   wxASSERT( second );
   return first->GetFullName().CmpNoCase( second->GetFullName() );
}

template<class T> int CmpMethodNameNoCase( T* first, T* second )
{
   wxASSERT( first );
   wxASSERT( second );
   return first->GetMethodName().CmpNoCase( second->GetMethodName() );
}
*/


class AutoCompFunction
{
   public:

      AutoCompFunction( const wxString& Name );
      AutoCompFunction( const AutoCompFunction& Func );
      ~AutoCompFunction();

      int BuildLocalsList( wxString& list ) const;

      const wxString& GetName() const { return m_Name; }
      wxString GetNamespace() const;
      wxString GetMethodName() const;

      const wxString& GetArgs() const { return m_Args; }
      //const wxString& GetReturn() const { return m_Return; }
      const wxString& GetDesc() const { return m_Desc; }

      const wxString&   GetFile() const      { return m_File; }
      int               GetLine() const      { return m_Line; }
      int               GetLineLast() const  { return m_LineLast; }

      bool InLineSpan( int line ) const { return m_Line <= line && m_LineLast >= line; }

      bool IsExport() const;

      bool GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const;

      const AutoCompVarArray& GetVars() const   { return m_Vars; }

      void AddDupe( AutoCompFunction* dupe );
      int GetDupeCount() const { return m_Dupes.GetCount(); }
      bool HasDupes() const { return !m_Dupes.IsEmpty(); }
      
      void SetName( const wxString& name )  { m_Name = name; }

      void SetArgs( const wxString& args )      { m_Args = args; }
      //void SetReturn( const wxString& return_ ) { m_Return = return_; }
      void SetDesc( const wxString& desc )      { m_Desc = desc; }

      void SetFile( const wxString& file, int line, int lineLast );
      

      AutoCompVar* AddVar( const wxString& Name );

      bool IsAt( const wxString& file, int line );

      static AutoCompFunction* Find( const wxString& Name, const AutoCompFunctionArray& Array );

      AutoCompFunction* GetDupeAt( const wxString& file, int line );

   protected:

      wxString    m_Name;
      wxString    m_Args;
      //wxString    m_Return;

      wxString          m_Desc;
      AutoCompVarArray  m_Vars;

      wxString       m_File;
      int            m_Line;
      int            m_LineLast;

      AutoCompFunctionArray   m_Dupes;
};


#endif // TORSION_AUTOCOMPFUNCTION_H
