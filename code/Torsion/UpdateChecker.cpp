// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "UpdateChecker.h"

#include "TorsionApp.h"
#include "BuildVersion.h"

#include <wx/txtstrm.h>
#include <wx/regex.h>
#include <wx/socket.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 



UpdateChecker::UpdateChecker()
{
}

UpdateChecker::~UpdateChecker()
{
}

bool UpdateChecker::CheckAvailable( wxString* version )
{
   // wxHTTP, wxURL, and wxFileSystem all suck ass.
   // Either they don't work or they won't time out
   // for me.  Implementing my own HTTP GET.

   wxIPV4address add;
   add.Hostname( "www.sickheadgames.com" );
   add.Service( 80 );
   wxSocketClient client( wxSOCKET_WAITALL | wxSOCKET_BLOCK );
   client.SetTimeout( 5 );
   client.Notify( false );

   // Wait for a connection or we're being stopped.
   if ( !client.Connect( add, true ) || !client.Ok() )
      return false;

   // Post the request.
   wxString get;
   get << "GET /tdeversion.php HTTP/1.0\r\n";
   get << "Host: www.sickheadgames.com\r\n";
   get << "\r\n";
   client.Write( get.GetData(), get.Length() );
   if ( client.Error() )
      return false;

   // Read all the data we got.
   wxString data;
   for ( ;; )
   {
      int startLen = data.Len();
      int newLen = startLen + 1024;
      wxChar* buf = data.GetWriteBuf( newLen ) + startLen;
      client.Read( buf, newLen - startLen );
      data.UngetWriteBuf( startLen + client.LastCount() );
      if ( client.LastError() != wxSOCKET_NOERROR )
         return false;

      if ( client.LastCount() == 0 )
         break;
   }

   // Look for the line after the double 
   // newline... that's our version.
   int pos = data.Find( "\r\n\r\n" );
   if ( pos == -1 )
      return false;

   wxString newVersion;
   newVersion = data.Right( data.Len()-(pos+4) );
   newVersion.Trim();
   newVersion.Trim( true );

   if ( newVersion.IsEmpty() )
      return false;

   // Extract the major, minor, and build numbers.
   wxRegEx expr;
   expr.Compile( "[v]([0-9]+)[\\.]([0-9]+)[\\.]([0-9]+)", wxRE_ADVANCED | wxRE_ICASE );
   wxASSERT( expr.IsValid() );

   if ( !expr.Matches( newVersion ) || expr.GetMatchCount() != 4 )
      return 0;

   if ( version )
      *version = newVersion;

   wxString match;
   long major, minor, build;

   match = expr.GetMatch( newVersion, 1 );
   match.ToLong( &major );
   match = expr.GetMatch( newVersion, 2 );
   match.ToLong( &minor );
   match = expr.GetMatch( newVersion, 3 );
   match.ToLong( &build );

   if (  major > tsBUILDVER_MAJOR ||
         ( major == tsBUILDVER_MAJOR && minor > tsBUILDVER_MINOR ) ||
         ( major == tsBUILDVER_MAJOR && minor == tsBUILDVER_MINOR && build > tsBUILDVER_BUILD ) )
      return true;

   return false;
}


