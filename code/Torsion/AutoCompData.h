// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPDATA_H
#define TORSION_AUTOCOMPDATA_H
#pragma once

#include "AutoCompPage.h"
#include "CallTipInfo.h"

class AutoCompExports;


class AutoCompData
{
   friend class AutoCompThread;
   friend class AutoCompManager;

public:

   AutoCompData();
   ~AutoCompData();

   void Build( const AutoCompPageArray& pages, AutoCompExports* exports );



   // These are used to define what icon to use
   // for the string list identifier.
   static const char* IDENT_VAR;             // "?0"
   static const char* IDENT_FUNCTION;        // "?1"
   static const char* IDENT_CLASS;           // "?2"
   static const char* IDENT_DATABLOCK;       // "?3"
   static const char* IDENT_OBJECT;          // "?4"
   static const char* ITYPE_DATABLOCKOBJECT; // "?5"
   static const char* IDENT_NAMESPACE;       // "?6"
   static const char* ITYPE_PACKAGE;         // "?7"
   static const char* ITYPE_KEYWORD;         // "?8"

   bool HasData() const;

   bool IsWordInCompList( const wxString& word ) const { return FindString( word, m_CompList, m_CompIndex ) != -1; }
   bool IsWordInClassList( const wxString& word ) const { return FindString( word, m_ClassList, m_ClassIndex ) != -1; }
   bool IsWordInDatablockList( const wxString& word ) const { return FindString( word, m_DatablockList, m_DatablockIndex ) != -1; }
   bool IsWordInGlobalsList( const wxString& word ) const { return FindString( word, m_GlobalsList, m_GlobalsIndex ) != -1; }

   const wxString& GetCompList() const { return m_CompList; }
   const wxString& GetClassList() const { return m_ClassList; }
   const wxString& GetGlobalsList() const { return m_GlobalsList; }
   const wxString& GetDatablockList() const { return m_DatablockList; }

   int   BuildMemberList( const wxString& Name, wxString& List ) const;
   int   BuildLocalsList( const wxString& Path, int Line, wxString& List ) const;
   int   BuildFieldList( const wxString& word, const wxString& path, int line, wxString& list ) const;
   bool  BuildPageFunctions( const wxString& path, wxArrayString& functions, wxArrayInt& startLines, wxArrayInt& endLines ) const;

   void  GetCallTip( const wxString& name, CallTipInfo& tip ) const;

   bool  GetDefinitionFileAndLine( const wxString& name, wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const;
   
   wxString GetFunctionNameAt( const wxString& path, int line ) const;
   bool     GetFunctionLineRange( const wxString& path, int line, int& start, int& end ) const;

   const AutoCompClassArray&     GetClasses() const   { return m_Classes; }
   const AutoCompClassArray&     GetObjects() const   { return m_Objects; }
   const AutoCompFunctionArray&  GetFunctions() const { return m_Functions; }
   const AutoCompVarArray&       GetVars() const      { return m_Vars; }

protected:

   void Clear();

   void MergeObjects( const AutoCompClassArray& source );
   void MergeFunctions( const AutoCompFunctionArray& source );
   void MergeVars( const AutoCompVarArray& source );

   AutoCompClass* FindClassOrObject( const wxString& name ) const;


   static void BuildString( wxArrayString& strings, wxString* output );
   static void BuildString( wxArrayString& strings, wxString* output, wxArrayInt* index );

   static int FindPartialWord( const wxString& word, const wxArrayString& array );
   static int FindString( const wxChar* word, const wxChar* words, const wxArrayInt& indicies );

   wxString       m_CompList;
   wxArrayInt     m_CompIndex;
   wxString       m_ClassList;
   wxArrayInt     m_ClassIndex;
   wxString       m_DatablockList;
   wxArrayInt     m_DatablockIndex;
   wxString       m_GlobalsList;
   wxArrayInt     m_GlobalsIndex;

   wxArrayString  m_TempArray;

   AutoCompClassArray      m_Classes;
   AutoCompClassArray      m_Objects;
   AutoCompFunctionArray   m_Functions;
   AutoCompVarArray        m_Vars;

   AutoCompClassArray      m_Namespaces;
   AutoCompExports*        m_Exports;
   AutoCompPageArray       m_Files;
};

#endif // TORSION_AUTOCOMPDATA_H
