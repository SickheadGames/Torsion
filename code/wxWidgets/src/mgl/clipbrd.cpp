/////////////////////////////////////////////////////////////////////////////
// Name:        gtk/clipbrd.cpp
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: clipbrd.cpp,v 1.7 2004/05/23 20:52:45 JS Exp $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "clipbrd.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/clipbrd.h"

#if wxUSE_CLIPBOARD

// FIXME_MGL

IMPLEMENT_DYNAMIC_CLASS(wxClipboard,wxObject)

#endif
  // wxUSE_CLIPBOARD

