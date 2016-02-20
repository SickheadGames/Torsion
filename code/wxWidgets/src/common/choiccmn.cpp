/////////////////////////////////////////////////////////////////////////////
// Name:        common/choiccmn.cpp
// Purpose:     common (to all ports) wxChoice functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     26.07.99
// RCS-ID:      $Id: choiccmn.cpp,v 1.19 2005/02/13 16:29:23 VZ Exp $
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "choicebase.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CHOICE

#ifndef WX_PRECOMP
    #include "wx/choice.h"
#endif

// ============================================================================
// implementation
// ============================================================================

wxChoiceBase::~wxChoiceBase()
{
    // this destructor is required for Darwin
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

void wxChoiceBase::Command(wxCommandEvent& event)
{
    SetSelection(event.GetInt());
    (void)ProcessEvent(event);
}

#endif // wxUSE_CHOICE

