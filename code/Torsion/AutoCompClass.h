// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPCLASS_H
#define TORSION_AUTOCOMPCLASS_H
#pragma once

#include "AutoCompFunction.h"
#include "AutoCompVar.h"

class AutoCompClass;
class AutoCompExportsArray;
class CallTipInfo;

WX_DEFINE_SORTED_ARRAY( AutoCompClass*, AutoCompClassArray );
WX_DEFINE_ARRAY( AutoCompClass*, AutoCompUnsortedClassArray );


class AutoCompClass
{
   public:
      AutoCompClass( const wxString& Name, const wxString& Base );
      AutoCompClass( const AutoCompClass& Class );
      ~AutoCompClass();

      const wxString& GetName() const { return m_Name; }
      const wxString& GetBase() const { return m_Base; }
      const wxString& GetDesc() const { return m_Desc; }

      const AutoCompFunctionArray&  GetFunctions() const { return m_Functions; }
      const AutoCompVarArray&       GetVars() const { return m_Vars; }

      const wxString&   GetFile() const { return m_File; }
      int               GetLine() const { return m_Line; }
      int               GetLineSpan() const { return m_LineLast - m_Line; }

      bool InLineSpan( int line ) const { return m_Line <= line && m_LineLast >= line; }

      bool GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const;

      bool HasBase() const { return !m_Base.IsEmpty(); }
      bool HasMembers() const { return !m_Functions.IsEmpty() || !m_Vars.IsEmpty(); }

      bool IsNamespace() const;
      bool IsDatablock() const                     { return m_Datablock; }
      void SetDatablock( bool datablock = true )   { m_Datablock = datablock; }

      bool IsExport() const;

      void SetDesc( const wxString& desc ) { m_Desc = desc; }
      void SetFile( const wxString& file, int line, int lineLast ) { m_File = file; m_Line = line; m_LineLast = lineLast; }

      void              AddFunctions( AutoCompFunctionArray* functions );
      void              AddFunction( AutoCompFunction* func );
      AutoCompFunction* AddFunction( const wxString& Name );
      bool              DeleteFunction( const wxString& name );

      void           AddVars( AutoCompVarArray* vars );
      AutoCompVar*   AddVar( const wxString& Name );
      bool           DeleteVar( const wxString& name );

      //int BuildMemberList( wxString& list ) const;
      bool GetCallTip( const wxString& name, CallTipInfo& tip ) const;

      int BuildFieldList( wxArrayString& list ) const;

      void MergeMembers( const AutoCompUnsortedClassArray& objects, const AutoCompExportsArray& exports );

      void AddDupe( AutoCompClass* dupe );

      static AutoCompClass* Find( const wxString& name, const AutoCompClassArray& array );
      static void SetDatablocks( AutoCompClassArray* classes );

   protected:

      wxString                m_Name;
      wxString                m_Base;
      wxString                m_Desc;
      bool                    m_Datablock;

      AutoCompFunctionArray   m_Functions;
      AutoCompVarArray        m_Vars;

      wxString                m_File;
      int                     m_Line;
      int                     m_LineLast;

      AutoCompClassArray      m_Dupes;
};


#endif // TORSION_AUTOCOMPCLASS_H
