/////////////////////////////////////////////////////////////////////////////
// Name:        xh_toolb.h
// Purpose:     XML resource handler for wxBoxSizer
// Author:      Vaclav Slavik
// Created:     2000/08/11
// RCS-ID:      $Id: xh_toolb.h,v 1.5 2005/01/07 21:27:20 VS Exp $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_TOOLB_H_
#define _WX_XH_TOOLB_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "xh_toolb.h"
#endif

#include "wx/xrc/xmlres.h"

#if wxUSE_TOOLBAR

class WXDLLEXPORT wxToolBar;

class WXDLLIMPEXP_XRC wxToolBarXmlHandler : public wxXmlResourceHandler
{
DECLARE_DYNAMIC_CLASS(wxToolBarXmlHandler)
public:
    wxToolBarXmlHandler();
    virtual wxObject *DoCreateResource();
    virtual bool CanHandle(wxXmlNode *node);

private:
    bool m_isInside;
    wxToolBar *m_toolbar;
};

#endif

#endif // _WX_XH_TOOLBAR_H_
