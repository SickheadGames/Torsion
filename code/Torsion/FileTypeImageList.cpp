// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "FileTypeImageList.h"

#include "Platform.h"

#include <wx/mimetype.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


FileTypeImageList::FileTypeImageList()
   :  wxImageList( 16, 16, true )
{
}

FileTypeImageList::~FileTypeImageList()
{
}

int FileTypeImageList::AddFileIcon( const wxString& file )
{
   // First get the file type from the extension.
   wxFileName fixed( file );
   wxString ext = fixed.GetExt().Lower();

   // Check to see if we've already loaded it.
   StringToIntMap::iterator it = m_TypeToIndex.find( ext );
   if ( it != m_TypeToIndex.end() )
   {
      wxASSERT( it->second >= -1 );
      wxASSERT( it->second < GetImageCount() );
      return it->second;
   }

   int index = -1;

   wxIcon icon;
   if ( tsGetIconForFile( file, false, &icon ) )
      index = wxImageList::Add( icon );

   // Cache it here so that we don't request it again!
   m_TypeToIndex[ext] = index;

   return index;
}


