// UpdateBuildNumber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <wx/textfile.h>
#include <wx/datetime.h>


int UpdateHeader( const wxString& path, wxString* buildVer )
{
   wxASSERT( buildVer );

   wxTextFile file;
   if ( !file.Open( path ) )
      return -1;

   // Look for the start seconds first.
   time_t start = 0;
   int major = 0;
   int minor = 0;
   int build = 0;
   const size_t lines = file.GetLineCount();
   wxString line;
   wxString rest;
   for ( size_t i=0; i < lines; i++ )
   {
      line = file.GetLine( i );
      line.Trim( false );

      if ( line.StartsWith( "#define tsBUILDVER_START", &rest ) )
      {
         rest.Trim( false );
         double value;
         rest.ToDouble( &value );
         start = (time_t)value;
      }
      else if ( line.StartsWith( "#define tsBUILDVER_MAJOR", &rest ) )
      {
         rest.Trim( false );
         long value;
         rest.ToLong( &value );
         major = value;
      }
      else if ( line.StartsWith( "#define tsBUILDVER_MINOR", &rest ) )
      {
         rest.Trim( false );
         long value;
         rest.ToLong( &value );
         minor = value;
      }
      else if ( line.StartsWith( "#define tsBUILDVER_BUILD", &rest ) )
      {
         rest.Trim( false );
         long value;
         rest.ToLong( &value );
         build = value;
      }
   }

   // Figure out the new build number.
   int newBuild = 0;
   {
      wxDateTime today( wxDateTime::GetTimeNow() );
      wxDateTime start( start );
      wxTimeSpan span = today.Subtract( start );
      newBuild = span.GetDays();
   }

   // Return the build string.
   if ( buildVer )
      *buildVer << major << "." << minor << "." << newBuild;

   // If we don't need to update then exit.
   if ( build == newBuild )
      return 0;

   // Now replace the build and resource string with
   // the new build number.
   for ( size_t i=0; i < lines; i++ )
   {
      line = file.GetLine( i );
      line.Trim( false );

      if ( line.StartsWith( "#define tsBUILDVER_BUILD" ) )
      {
         line.Empty();
         line << "#define tsBUILDVER_BUILD\t" << newBuild;
         file[i] = line;
      }
      else if ( line.StartsWith( "#define tsBUILDVER_RESSTR" ) )
      {
         line.Empty();
         line << "#define tsBUILDVER_RESSTR\t\"" << major << "." << minor << ".0." << newBuild << "\"";
         file[i] = line;
      }
   }

   file.Write();

   return 0;
}

int UpdateInstaller( const wxString& path, const wxString& version )
{
   wxTextFile file;
   if ( !file.Open( path ) )
      return -1;

   // Look for the start seconds first.
   const size_t lines = file.GetLineCount();
   wxString line;
   wxString name;
   for ( size_t i=0; i < lines; i++ )
   {
      line = file.GetLine( i );
      line.Trim( false );

      if ( line.StartsWith( "AppName=" ) )
      {
         name = line.Remove( 0, 8 );
         name.Trim( true );
         name.Trim( false );
         name.Append( ' ' );
      }
      else if ( line.StartsWith( "AppVerName=" ) )
      {
         line.Empty();
         line << "AppVerName=" << name << version;
         file[i] = line;
         break;
      }
   }

   file.Write();

   return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
   if ( argc < 3 )
      return -1;

   wxString version;
   int result = UpdateHeader( argv[1], &version );
   if ( result < 0 )
      return result;

   return UpdateInstaller( argv[2], version );
}
