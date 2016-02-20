// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompFunction.h"

#include "AutoComp.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompFunction::AutoCompFunction( const wxString& Name )
   :  m_Name( Name ),
      m_Vars( CmpNameNoCase ),
      m_Line( 0 ),
      m_LineLast( 0 ),
      m_Dupes( CmpNameNoCase )
{
}

AutoCompFunction::AutoCompFunction( const AutoCompFunction& Func )
   :  m_Name( Func.m_Name ),
      m_Args( Func.m_Args ),
      //m_Return( Func.m_Return ),
      m_Desc( Func.m_Desc ),
      m_File( Func.m_File ),
      m_Line( Func.m_Line ),
      m_LineLast( Func.m_LineLast ),
      m_Vars( CmpNameNoCase ),
      m_Dupes( CmpNameNoCase )
{
   for ( int i=0; i < Func.m_Vars.GetCount(); i++ )
      m_Vars.Add( new AutoCompVar( *Func.m_Vars[i] ) );

   for ( int i=0; i < Func.m_Dupes.GetCount(); i++ )
      m_Dupes.Add( new AutoCompFunction( *Func.m_Dupes[i] ) );
}

AutoCompFunction::~AutoCompFunction()
{
   WX_CLEAR_ARRAY( m_Vars );
   WX_CLEAR_ARRAY( m_Dupes );
}

wxString AutoCompFunction::GetNamespace() const
{
   int sep = m_Name.Find( "::" );
   if ( sep == -1 )
      return wxEmptyString;

   return m_Name.Mid( 0, sep );
}

wxString AutoCompFunction::GetMethodName() const
{
   int sep = m_Name.Find( "::" );
   if ( sep == -1 )
      return m_Name;

   return m_Name.Mid( sep + 2 );
}

AutoCompVar* AutoCompFunction::AddVar( const wxString& Name )
{
   wxASSERT( !Name.IsEmpty() );

   AutoCompVar* var = AutoCompVar::Find( Name, m_Vars );
   if ( !var ) {

      var = new AutoCompVar( Name );
      m_Vars.Add( var );
   }

   return var;
}

int AutoCompFunction::BuildLocalsList( wxString& list ) const
{
   list.Alloc( m_Vars.GetCount() * 10 ); // Take a good guess at the size.
   wxString Name;

   // The vars array is already in alphabetical order, 
   // so there is no need to sort things.
   int count = 0;
   for ( int i=0; i < m_Vars.GetCount(); i++ ) {

      Name = m_Vars[i]->m_Name + AutoCompData::IDENT_VAR;
      if ( Name[0] == '%' ) {

         list.Append( Name );
         list.Append( ' ' );
         ++count;
      }
   }

   list.RemoveLast();
   list.Shrink();

   return count;
}

AutoCompFunction* AutoCompFunction::Find( const wxString& Name, const AutoCompFunctionArray& Array )
{
   // Do a simple binary search as we know the list
   // is sorted... this should be super quick.
   size_t i,
      lo = 0,
      hi = Array.GetCount();
   int res;

   while ( lo < hi ) {

      i = (lo + hi) / 2;
      wxASSERT( Array[i] );

      res = Name.CmpNoCase( Array[i]->m_Name );
      if ( res < 0 )
         hi = i;
      else if ( res > 0 )
         lo = i + 1;
      else
         return Array[i];
   }

   return NULL;
}

void AutoCompFunction::SetFile( const wxString& file, int line, int lineLast )
{
   wxASSERT( line <= lineLast );

   // I have a suspicion that there is something wrong with
   // the "COW" strings here, so we force a copy.
   m_File = file.c_str();
   m_Line = line;
   m_LineLast = lineLast;
}

AutoCompFunction* AutoCompFunction::GetDupeAt( const wxString& file, int line )
{
   const bool caseSensitive = wxFileName::IsCaseSensitive();

   if (  m_File.IsSameAs( file, caseSensitive ) &&
         m_Line <= line && m_LineLast >= line )
         return this;

   for ( int i=0; i < m_Dupes.Count(); i++ )
   {
      AutoCompFunction* dupe = m_Dupes[i]->GetDupeAt( file, line );
      if ( dupe )
         return dupe;
   }

   return NULL;
}

void AutoCompFunction::AddDupe( AutoCompFunction* dupe )
{
   wxASSERT( dupe );
   //wxASSERT( m_Dupes.Index( dupe ) == wxNOT_FOUND ); // This compares using or predicate!
   m_Dupes.Add( dupe );
}

bool AutoCompFunction::IsExport() const
{
   if ( !m_File.IsEmpty() )
      return false;

   for ( int i=0; i < m_Dupes.Count(); i++ )
   {
      const AutoCompFunction* dupe = m_Dupes[i];
      wxASSERT( dupe );

      if ( !dupe->m_File.IsEmpty() )
         return false;
   }

   return true;
}

bool AutoCompFunction::GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const
{
   wxASSERT( files );
   wxASSERT( lines );

   if ( !m_File.IsEmpty() && m_Line > 0 )
   {
      files->Add( m_File );
      lines->Add( m_Line );
      if ( symbols )
         symbols->Add( m_Name );
   }

   for ( int i=0; i < m_Dupes.Count(); i++ )
   {
      const AutoCompFunction* dupe = m_Dupes[i];
      wxASSERT( dupe );

      if ( !dupe->m_File.IsEmpty() && dupe->m_Line > 0 )
      {
         files->Add( dupe->m_File );
         lines->Add( dupe->m_Line );
         if ( symbols )
            symbols->Add( dupe->m_Name );
      }
   }

   wxASSERT( files->GetCount() == lines->GetCount() );
   wxASSERT( files->GetCount() == symbols->GetCount() );
   return !files->IsEmpty();
}
