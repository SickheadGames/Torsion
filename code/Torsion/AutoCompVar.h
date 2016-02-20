// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPVAR_H
#define TORSION_AUTOCOMPVAR_H
#pragma once

class AutoCompVarArray;


class AutoCompVar
{
   public:
      AutoCompVar( const wxString& name );
      AutoCompVar( const AutoCompVar& Var );
      ~AutoCompVar() {}

      void AddLine( const wxString& File, int Line );

      bool IsGlobal() const   { return m_Name[0] == '$'; }
      bool IsLocal() const    { return m_Name[0] == '%'; }

      bool IsExport() const   { return m_Lines.IsEmpty(); }

      // TODO: Remove these if unused... or fix it... these are wasteful!
      wxString GetFile( int i ) const;
      int GetLine( int i ) const;
      
      bool GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const;

      static void GetFileAndLine( const wxString& str, wxString& file, int& line );

      static AutoCompVar* Find( const wxString& Name, const AutoCompVarArray& Array );
      static int FindScoped( const wxString& name, const AutoCompVarArray& source, AutoCompVarArray& found );
      static void Merge( AutoCompVarArray& Dest, const AutoCompVarArray& Source, bool NoLocals );

      const wxString& GetName() const        { return m_Name; }
      const wxString& GetDesc() const        { return m_Desc; }
      const wxArrayString& GetLines() const  { return m_Lines; }

      void SetDesc( const wxString& desc )   { m_Desc = desc; }

   public:

      wxString       m_Name;
      wxString       m_Desc;

      wxArrayString  m_Lines;
};

WX_DEFINE_SORTED_ARRAY( AutoCompVar*, AutoCompVarArray );

#endif // TORSION_AUTOCOMPVAR_H
