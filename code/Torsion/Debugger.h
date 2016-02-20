// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_DEBUGGER_H
#define TORSION_DEBUGGER_H
#pragma once

#include "Breakpoint.h"
#include "FunctionCall.h"
#include "WatchVar.h"

#include <vector>

class wxProcessEvent;
class wxSocketEvent;
class wxSocketClient;
class tsShellExecute;

class Debugger : public wxEvtHandler
{
public:
    
	Debugger();
	virtual ~Debugger();

	bool		Start( const wxString& config, const wxString& cmd );
	bool		Connect( const wxString& Address, int Port, const wxString& Password, const wxString& cmd );

	void		Break();
	void		Continue( const wxString& Command = "CONTINUE" );
	void		Stop( bool disconnect = false );
	void		Step()		{ Continue( "STEPIN" ); }
	void		StepOver()	{ Continue( "STEPOVER" ); }
	void		StepOut()	{ Continue( "STEPOUT" ); }

	void		EvalCallTip( const wxString& expression );
	void		Eval( const wxString& expression, int tag = 0 );

   void     MoveInstPtr( const wxString& file, int line );

   void     ReloadScripts( const wxArrayString& scripts );

	void		UpdateVar( WatchVar* Var );
   void     ChangeVar( WatchVar* Var, const wxString& Value );
	void		UpdateExpanded( WatchVar* Var );

	void		SetStackLevel( int Level );
	int		GetStackLevel() const	{ return m_CurrStackLevel; }

   const bool	IsRunning() const;
	const bool	IsAtBreakpoint() const { return IsRunning() && m_AtBreakpoint; }
   bool        GetBreakpointFileAndLine( wxString& file, int& line ) const;

   void OnBreakpointEvent( BreakpointEvent& event );

   void OnStoppedEvent( wxProcessEvent& event );
	void OnSocketEvent( wxSocketEvent& event );

   static void EscapeTorqueML( wxString& text );

protected:

	void _OnServerMessage( const wxString& line );

   enum {
      PasswordMaxLength = 32,
      MaxCommandSize = 2048,
      CallTipEvalTag = 44032,
   };

	void				_SendLine( const wxString& Line );

	void				_ClearCallStack();

	//Breakpoint*			_FindBreakpoint( const wxString& File, int Line );

	void				_WriteBreakpoints( const BreakpointArray& Breakpoints );
	void				_WriteBreakpoint( const Breakpoint* TheBreakpoint );
	void				_ReadBreakpointData( wxString callstackString );
	void				_ReadLocalsData( FunctionCall* Call );

	bool		         m_AtBreakpoint;

   wxString          m_LastCallTip;

	int					m_CurrStackLevel;
	FunctionCallArray	m_CallStack;
   wxArrayString     m_LoadedScripts;

	wxString	         m_InputBuffer;

	wxSocketClient*	m_Server;
   tsShellExecute*   m_Process;

   WatchVarArray     m_UpdateVarStack;

   DECLARE_EVENT_TABLE()
};

#endif // TORSION_DEBUGGER_H
