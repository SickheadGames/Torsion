/////////////////////////////////////////////////////////////////////////////
// Name:        icon.cpp
// Author:      Vaclav Slavik
// Id:          $Id: icon.cpp,v 1.7 2004/05/23 20:52:46 JS Exp $
// Copyright:   (c) 2001-2002 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:   	wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "icon.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/icon.h"

//-----------------------------------------------------------------------------
// wxIcon
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxIcon, wxBitmap)

wxIcon::wxIcon(const char **bits, int WXUNUSED(width), int WXUNUSED(height)) :
    wxBitmap(bits)
{
}

wxIcon::wxIcon(char **bits, int WXUNUSED(width), int WXUNUSED(height)) :
    wxBitmap(bits)
{
}

wxIcon::wxIcon(const wxIcon& icon) : wxBitmap()
{
    Ref(icon);
}

wxIcon& wxIcon::operator = (const wxIcon& icon)
{
    if (*this == icon) 
        return (*this);
    Ref(icon);
    return *this;
}

void wxIcon::CopyFromBitmap(const wxBitmap& bmp)
{
    wxIcon *icon = (wxIcon*)(&bmp);
    *this = *icon;
}

