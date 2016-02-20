// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_FUNCTIONCALL_H
#define TORSION_FUNCTIONCALL_H
#pragma once

#include "WatchVar.h"


class FunctionCall
{
public:

	FunctionCall( int StackIndex, const wxString& File, int Line, const wxString& Function )
		:	m_StackIndex( StackIndex ),
			m_File( File ),
			m_Line( Line ),
			m_Function( Function )
	{
	}

	FunctionCall( const FunctionCall& fc )
		:	m_StackIndex( fc.m_StackIndex ),
			m_File( fc.m_File ),
			m_Line( fc.m_Line ),
			m_Function( fc.m_Function )
	{
	}

	bool IsSameFunction( const FunctionCall& call ) const;

	const wxString&	GetFile() const			{ return m_File; }
	long			GetLine() const			{ return m_Line; }
	const wxString&	GetFunction() const		{ return m_Function; }
	int				GetStackIndex() const	{ return m_StackIndex; }

	void SwapLocals( FunctionCall& call );

	const WatchVarArray& GetLocals() const { return m_Locals; }

	void SetLocals( WatchVarArray& NewLocals );

	void AddLocal( const wxString& Type, int Flags, const wxString& Name,  const wxString& Value );

protected:

	int				m_StackIndex;
	wxString		m_File;
	int				m_Line;
	wxString		m_Function;

	WatchVarArray	m_Locals;

};

WX_DEFINE_ARRAY( FunctionCall*, FunctionCallArray );

#endif // TORSION_FUNCTIONCALL_H
