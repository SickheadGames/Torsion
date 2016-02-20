/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/timer.h
// Purpose:
// Author:      Robert Roebling
// Id:          $Id: timer.h,v 1.12 2005/08/02 22:57:58 MW Exp $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __GTKTIMERH__
#define __GTKTIMERH__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "timer.h"
#endif

//-----------------------------------------------------------------------------
// wxTimer
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxTimer : public wxTimerBase
{
public:
    wxTimer() { Init(); }
    wxTimer(wxEvtHandler *owner, int id = -1) : wxTimerBase(owner, id)
        { Init(); }
    ~wxTimer();

    virtual bool Start( int millisecs = -1, bool oneShot = FALSE );
    virtual void Stop();

    virtual bool IsRunning() const { return m_tag != -1; }

protected:
    void Init();

    int  m_tag;

private:
    DECLARE_ABSTRACT_CLASS(wxTimer)
};

#endif // __GTKTIMERH__
