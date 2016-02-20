/////////////////////////////////////////////////////////////////////////////
// Name:        helpbase.cpp
// Purpose:     Help system base classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: helpbase.cpp,v 1.15 2004/09/15 13:20:18 ABX Exp $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "helpbase.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/defs.h"
#endif

#if wxUSE_HELP

#include "wx/helpbase.h"

IMPLEMENT_CLASS(wxHelpControllerBase, wxObject)

#endif // wxUSE_HELP
