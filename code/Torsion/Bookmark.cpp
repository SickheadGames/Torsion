// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "Bookmark.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_CLASS(tsBookmarksUpdateHint, wxObject)


int wxCMPFUNC_CONV Bookmark::Compare( Bookmark** b1, Bookmark** b2 )
{
   wxASSERT( *b1 && *b2 );

   wxString f1 = (*b1)->m_File.GetFullPath();
   wxString f2 = (*b2)->m_File.GetFullPath();

   int result = f1.CompareTo( f2, wxFileName::IsCaseSensitive() ? wxString::exact : wxString::ignoreCase );
   if ( result != 0 )
      return result;

   result = (*b1)->GetLine() < (*b2)->GetLine();

   return result < 0 ? -1 : ( result > 0 ? 1 : 0 );
}

bool Bookmark::IsFile( const wxString& File ) const
{
	wxFileName CmpFile( File );
	return m_File == CmpFile;
}

bool Bookmark::operator ==( const Bookmark& b ) const
{
   return   m_File == b.m_File &&
            m_Line == b.m_Line;
}
