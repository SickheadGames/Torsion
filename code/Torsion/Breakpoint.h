// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_BREAKPOINT_H
#define TORSION_BREAKPOINT_H
#pragma once

#include <wx/filename.h>


class Breakpoint
{
public:

	Breakpoint( const wxString& File, int Line, int Pass, const wxString& Condition )
		:	m_File( File ),
			m_Line( Line ),
			m_Pass( Pass ),
			m_Condition( Condition ),
			m_Enabled( true )
	{
	}

	Breakpoint( const Breakpoint& b )
		:	m_File( b.m_File ),
			m_Line( b.m_Line ),
			m_Pass( b.m_Pass ),
			m_Condition( b.m_Condition ),
			m_Enabled( b.m_Enabled )
	{
	}

   const wxString 	GetFile() const			{ return m_File.GetFullPath(); }
	int				   GetLine() const			{ return m_Line; }
	int				   GetPass() const			{ return m_Pass; }
	const wxString&	GetCondition() const	{ return m_Condition; }
	bool			      GetEnabled() const		{ return m_Enabled; }

   bool              HasCondition() const { return m_Pass > 0 || !m_Condition.IsEmpty(); }

   wxString          GetToolTip() const;

	void			SetFile( const wxString& File )  { m_File = File; }
	void			SetLine( int Line )		         { m_Line = Line; }
	void			SetPass( int Pass )		         { m_Pass = Pass; }

	void			SetCondition( const wxString& Condition ) { m_Condition = Condition; }
	void			SetEnabled( bool Enabled ) { m_Enabled = Enabled; }

	bool	IsFile( const wxString& File ) const;
	bool	IsFileAndLine( const wxString& File, long Line ) const { return m_Line == Line && IsFile( File ); }

   bool operator ==( const Breakpoint& b ) const;

   static int wxCMPFUNC_CONV LineCompare( Breakpoint** b1, Breakpoint** b2 )
   {
      wxASSERT( *b1 && *b2 );
      return (*b1)->GetLine() - (*b2)->GetLine();
   }

protected:

	wxFileName  m_File;
	int			m_Line;
	int			m_Pass;
	wxString	m_Condition;
	bool		m_Enabled;

};

WX_DEFINE_ARRAY( Breakpoint*, BreakpointArray );


DECLARE_LOCAL_EVENT_TYPE(EVT_COMMAND_BREAKPOINT, -1)


class WXDLLEXPORT BreakpointEvent : public wxEvent
{
public:

   enum TYPE {
      TYPE_NONE,
      TYPE_ADD,
      TYPE_REMOVE,
      TYPE_CHANGED,
      TYPE_MOVED,
   };

   BreakpointEvent()
       : wxEvent( wxID_ANY, EVT_COMMAND_BREAKPOINT ),
         m_Breakpoint( NULL ),
         m_Type( TYPE_NONE ),
         m_OldLine( -1 )
    {
    }

    void SetBreakpoint( Breakpoint* b ) { m_Breakpoint = b; }
    Breakpoint* GetBreakpoint() const { return m_Breakpoint; }

    void SetType( TYPE type ) { m_Type = type; }
    TYPE GetType() const { return m_Type; }

    void SetOldLine( int line ) { m_OldLine = line; }
    int GetOldLine() const { return m_OldLine; }

    virtual wxEvent* Clone() const
    {
       return new BreakpointEvent( *this );
    }

protected:

    BreakpointEvent( const BreakpointEvent& event )
       : wxEvent( event ),
         m_Type( event.m_Type ),
         m_Breakpoint( event.m_Breakpoint ),
         m_OldLine( event.m_OldLine )
    {
    }

private:

   TYPE m_Type;
   int  m_OldLine;
   Breakpoint* m_Breakpoint;

};


typedef void (wxEvtHandler::*BreakpointEventFunction)(BreakpointEvent&);

#define BreakpointEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(BreakpointEventFunction, &func)

#define EVT_BREAKPOINT_EVENTS(fn) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_COMMAND_BREAKPOINT, wxID_ANY, -1, BreakpointEventHandler(fn), NULL),


#endif // TORSION_BREAKPOINT_H
