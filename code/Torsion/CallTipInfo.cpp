// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "CallTipInfo.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


CallTipInfo::CallTipInfo( int tabSize )
   :  m_TabSize( tabSize )
{
}

CallTipInfo::~CallTipInfo()
{
    WX_CLEAR_ARRAY( m_Args );
}

wxString CallTipInfo::FormatTip( const wxString& tip, int tabSize )
{
   wxString cleaned( tip );

   // Fix up newlines, convert tabs to spaces, 
   // and trim any whitespace.
   cleaned.Replace( "\r\n", "\n" );
   cleaned.Replace( "\r", "\n" );
   cleaned.Replace( "\t", wxString( ' ', tabSize ) );
   cleaned.Trim(); cleaned.Trim( false ); 

   // Clean up long lines by breaking them if they are
   // over 80 chars... don't bother reformatting them
   // any further as we cannot guess at what format they
   // intended anyway.
   wxString result;
   while ( !cleaned.IsEmpty() )
   {
      wxString line = cleaned.BeforeFirst( '\n' );
      cleaned = cleaned.Mid( line.Len() + 1 );

      while ( line.Len() > 80 )
      {
         wxString newLine = line.Left( 80 );
         newLine = newLine.BeforeLast( ' ' );
         if ( newLine.IsEmpty() )
            break;
         line = line.Mid( newLine.Len() );
         line.Trim( false );
         result += newLine + '\n';
      }
                 
      result += line + '\n';
   }
   result.Trim(); result.Trim( false ); 

   return result;
}

void CallTipInfo::SetTip( const wxString& decl, const wxString& desc )
{
   // Store the originals first.
   m_Tip = decl;
   m_Description = desc;

   // Pull out the special identifiers.

   // Create the full tip first.
   m_FormattedTip = decl;
   if ( !desc.IsEmpty() )
      m_FormattedTip << '\n' << FormatTip( desc, m_TabSize );

   // Parse out the arguments... store the indicies.
   WX_CLEAR_ARRAY( m_Args );
   int start = decl.Find( '(' ) + 1;
   const int end = decl.Find( ')', true );
   int len = 0;
   for ( ; start < end; start++ )
   {
      wxChar ch = decl.GetChar( start );
      if ( wxIsspace( ch ) )
         continue;

      // Start collecting the var length.
      int i = start;
      for ( ; i < end; i++ )
      {
         wxChar ch = decl.GetChar( i );
         if ( ch == ',' )
            break;
      }

      Arg* arg = new Arg;
      arg->start = start;
      arg->end = i;
      m_Args.Add( arg );
      start = i;
   }
}

bool CallTipInfo::GetArgumentRange( int arg, int* start, int* end ) const
{
   if ( arg < 0 || arg >= m_Args.Count() )
      return false;

   *start   = m_Args[arg]->start;
   *end     = m_Args[arg]->end;

   return true;
}

void CallTipInfo::RemoveThis()
{
   // We're checking the first parameter if it is
   // %this.  If it is we remove it.
   int start = m_Tip.Find( '(' ) + 1;
   int end = m_Tip.Find( ')', true );
   for ( ; start < end; start++ )
   {
      wxChar ch = m_Tip.GetChar( start );
      if ( wxIsspace( ch ) )
         continue;

      // Start collecting the var length.
      int i = start;
      for ( ; i < end; i++ )
      {
         wxChar ch = m_Tip.GetChar( i );
         if ( ch == ',' || wxIsspace( ch )  )
            break;
      }

      end = i;
      break;
   }

   // Read it...
   wxString param = m_Tip.Mid( start, end - start );
   if ( param.CmpNoCase( "%this" ) != 0 )
      return;

   // Remove it.
   m_Tip.erase( start-1, end - start + 2 );

   // Now reprocess.
   SetTip( m_Tip, m_Description );
}


