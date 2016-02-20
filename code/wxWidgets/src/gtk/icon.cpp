/////////////////////////////////////////////////////////////////////////////
// Name:        icon.cpp
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: icon.cpp,v 1.11 2004/05/23 20:52:20 JS Exp $
// Copyright:   (c) 1998 Robert Roebling
// Licence:   	wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "icon.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/icon.h"

//-----------------------------------------------------------------------------
// wxIcon
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxIcon,wxBitmap)

wxIcon::wxIcon( const char **bits, int WXUNUSED(width), int WXUNUSED(height) ) :
    wxBitmap( bits )
{
}

wxIcon::wxIcon( char **bits, int WXUNUSED(width), int WXUNUSED(height) ) :
    wxBitmap( bits )
{
}

wxIcon::wxIcon() :  wxBitmap()
{
}

wxIcon::wxIcon( const wxIcon& icon ) : wxBitmap()
{
    Ref(icon);
}

wxIcon& wxIcon::operator = ( const wxIcon& icon )
{
    if (*this == icon) return (*this);
    Ref(icon);
    return *this;
}

void wxIcon::CopyFromBitmap(const wxBitmap& bmp)
{
    wxIcon *icon = (wxIcon*)(&bmp);
    *this = *icon;
}
