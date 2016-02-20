// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_BOOKMARK_H
#define TORSION_BOOKMARK_H
#pragma once

#include <wx/filename.h>


class Bookmark
{
public:

	Bookmark( const wxString& File, int Line )
		:	m_File( File ),
			m_Line( Line )
	{
	}

	Bookmark( const Bookmark& b )
		:	m_File( b.m_File ),
			m_Line( b.m_Line )
	{
	}

   const wxString GetFile() const   { return m_File.GetFullPath(); }
	int            GetLine() const   { return m_Line; }

	bool	IsFile( const wxString& file ) const;
	bool	IsFileAndLine( const wxString& File, int Line ) const { return m_Line == Line && IsFile( File ); }

   void  SetFile( const wxString& File )  { m_File = File; }
	void  SetLine( int Line )		         { m_Line = Line; }

   bool operator ==( const Bookmark& b ) const;

   static int wxCMPFUNC_CONV Compare( Bookmark** b1, Bookmark** b2 );

protected:

	wxFileName  m_File;
	int			m_Line;

};

WX_DEFINE_ARRAY( Bookmark*, BookmarkArray );

// This is only used with UpdateAllViews to
// notifiy windows to update their bookmarks.
class tsBookmarksUpdateHint : public wxObject 
{
   DECLARE_CLASS(tsBookmarksUpdateHint)
};

#endif // TORSION_BOOKMARK_H
