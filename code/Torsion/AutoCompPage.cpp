// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompPage.h"

#include "AutoCompUtil.h"
#include "AutoCompData.h"
#include "AutoCompExports.h"

#include <wx/regex.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompPage::AutoCompPage( const wxString& name )
   :  m_Name( name ),
      m_Objects( CmpNameNoCase ),
      m_Functions( CmpNameNoCase ),
      m_Vars( CmpNameNoCase ),
      m_LastUpdate( (time_t)0 ) 
{
}

AutoCompPage::AutoCompPage( const AutoCompPage& Page )
   :  m_Name( Page.m_Name ),
      m_Objects( CmpNameNoCase ),
      m_Functions( CmpNameNoCase ),
      m_Vars( CmpNameNoCase ),
      m_LastUpdate( Page.m_LastUpdate )
{
   for ( int i=0; i < Page.m_Objects.GetCount(); i++ )
      m_Objects.Add( new AutoCompClass( *Page.m_Objects[i] ) );
   for ( int i=0; i < Page.m_Functions.GetCount(); i++ )
      m_Functions.Add( new AutoCompFunction( *Page.m_Functions[i] ) );
   for ( int i=0; i < Page.m_Vars.GetCount(); i++ )
      m_Vars.Add( new AutoCompVar( *Page.m_Vars[i] ) );
}

AutoCompPage::~AutoCompPage()
{
   Clear();
}

void AutoCompPage::Clear()
{
   WX_CLEAR_ARRAY( m_Objects );
   WX_CLEAR_ARRAY( m_Functions );
   WX_CLEAR_ARRAY( m_Vars );
}

AutoCompVar* AutoCompPage::AddVar( const wxString& Name )
{
   wxASSERT( !Name.IsEmpty() );
   wxASSERT( Name[0] == '$' || Name[0] == '%' );

   AutoCompVar* var = AutoCompVar::Find( Name, m_Vars );
   if ( !var ) {

      var = new AutoCompVar( Name );
      m_Vars.Add( var );
   }

   return var;
}

void AutoCompPage::AddFunction( AutoCompFunction* func )
{
   wxASSERT( func );
   m_Functions.Add( func );
}

AutoCompClass* AutoCompPage::AddObject( const wxString& name, const wxString& base )
{
   AutoCompClass* class_ = new AutoCompClass( name, base );
   m_Objects.Add( class_ );

   return class_;
}

AutoCompPage* AutoCompPage::Find( const wxString& Name, const AutoCompPageArray& Array )
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
