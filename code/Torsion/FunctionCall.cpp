// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "FunctionCall.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

bool FunctionCall::IsSameFunction( const FunctionCall& call ) const
{
	return	m_File == call.m_File &&
			m_Function == call.m_Function;
}

void FunctionCall::SetLocals( WatchVarArray& NewLocals )
{
	WatchVarArray OldLocals = m_Locals;
	m_Locals = NewLocals;
	NewLocals.Clear();

    // Loop thru the new locals replacing with matching old locals.
	for ( int i=0; i < m_Locals.GetCount(); i++ ) {
		     
		wxASSERT( m_Locals[i] );
		int l = WatchVar::FindVar( OldLocals, m_Locals[i]->GetName(), m_Locals[i]->GetType() );
		if ( l != -1 ) {

			WatchVar* Temp = m_Locals[i];
			m_Locals[i] = OldLocals[l];
			m_Locals[i]->SetValue( Temp->GetValue() );
			delete Temp;
            OldLocals[l] = NULL;
		}
	}
    
	// Delete any remaining old locals.
	for ( int i=0; i < OldLocals.GetCount(); i++ ) {
		delete OldLocals[i];
	}
}

void FunctionCall::SwapLocals( FunctionCall& call )
{
	WatchVarArray Temp = call.m_Locals;
	call.m_Locals = m_Locals;
	m_Locals = Temp;
}

void FunctionCall::AddLocal( const wxString& Type, int Flags, const wxString& Name,  const wxString& Value )
{
	wxASSERT( WatchVar::FindVar( m_Locals, Name, Type ) == -1 );
	m_Locals.Add( new WatchVar( NULL, Type, Flags, Name, Value ) );
}
