// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompVar.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompVar::AutoCompVar( const wxString& name )
   : m_Name( name )
{
}

AutoCompVar::AutoCompVar( const AutoCompVar& Var )
   :  m_Name( Var.m_Name ),
      m_Desc( Var.m_Desc ),
      m_Lines( Var.m_Lines )
{
}

void AutoCompVar::AddLine( const wxString& File, int Line )
{
   wxString file;
   int line;
   for ( int i=0; i < m_Lines.GetCount(); i++ ) {

      GetFileAndLine( m_Lines[i], file, line );
      if ( line == Line && wxFileName( file ).SameAs( File ) ) {
         return;
      }
   }

   m_Lines.Add( wxString( File ) << ';' << Line );
}

wxString AutoCompVar::GetFile( int i ) const
{
   wxString file;
   int line;
   GetFileAndLine( m_Lines[i], file, line );
   return file;
}

int AutoCompVar::GetLine( int i ) const
{
   wxString file;
   int line;
   GetFileAndLine( m_Lines[i], file, line );
   return line;
}

AutoCompVar* AutoCompVar::Find( const wxString& Name, const AutoCompVarArray& Array )
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

int AutoCompVar::FindScoped( const wxString& name, const AutoCompVarArray& source, AutoCompVarArray& found )
{
   // Do a binary search to find one of the scoped vars.
   size_t i,
      lo = 0,
      hi = source.GetCount();
   int res;

   wxString scoped = name + "::";
   const size_t len = scoped.Len();

   while ( lo < hi ) {

      i = (lo + hi) / 2;
      wxASSERT( source[i] );

      res = wxStrnicmp( scoped.c_str(), source[i]->m_Name.c_str(), len );
      if ( res < 0 )
         hi = i;
      else if ( res > 0 )
         lo = i + 1;
      else
      {
         // We found a match.  Since it's sorted, just
         // add matching strings above and below and
         // we've captured everything.
         found.Add( source[i] );

         for ( int j=i-1; j >= 0; j-- )
         {
            if ( wxStrnicmp( scoped.c_str(), source[j]->m_Name.c_str(), len ) != 0 )
               break;

            found.Add( source[j] );
         }

         for ( int j=i+1; j < source.GetCount(); j++ )
         {
            if ( wxStrnicmp( scoped.c_str(), source[j]->m_Name.c_str(), len ) != 0 )
               break;

            found.Add( source[j] );
         }

         break;
      }
   }

   return found.GetCount();
}

bool AutoCompVar::GetFileAndLine( wxArrayString* files, wxArrayInt* lines, wxArrayString* symbols ) const
{
   wxASSERT( files );
   wxASSERT( lines );

   for ( int i=0; i < m_Lines.Count(); i++ )
   {
      wxString file;
      int line;
      GetFileAndLine( m_Lines[i], file, line );

      files->Add( file );
      lines->Add( line );
      if ( symbols )
         symbols->Add( m_Name );
   }

   wxASSERT( files->GetCount() == lines->GetCount() );
   wxASSERT( !symbols || files->GetCount() == symbols->GetCount() );
   return !files->IsEmpty();
}

void AutoCompVar::GetFileAndLine( const wxString& str, wxString& file, int& line )
{
   file = str.BeforeFirst( ';' );
   line = 0;
   long value;
   if ( str.AfterFirst( ';' ).ToLong( &value ) ) {
      line = value;
   }
}
