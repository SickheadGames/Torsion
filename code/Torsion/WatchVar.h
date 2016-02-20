// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_WATCHVAR_H
#define TORSION_WATCHVAR_H
#pragma once

class WatchVar;
WX_DEFINE_ARRAY( WatchVar*, WatchVarArray );


class WatchVar
{
	friend class FunctionCall;

public:

	WatchVar( WatchVar* Parent, const wxString& Type, int Flags, const wxString& Name,  const wxString& Value );
   WatchVar( const WatchVar& wv );
	~WatchVar();

public:

   static WatchVar* FindVarByTag( int tag );

	WatchVar*	GetParent() const		{ return m_Parent; }

   int GetTag() const;
   
	const wxString&	GetType() const		{ return m_Type; }
	const wxString&	GetName() const		{ return m_Name; }
	const wxString&	GetValue() const	{ return m_Value; }

	void SetName( const wxString& Name )	{ m_Name = Name; }
	void SetValue( const wxString& Value )	{ m_Value = Value; }
	void SetType( const wxString& Type )	{ m_Type = Type; }
	void SetFlags( int Flags )				{ m_Flags = Flags; SetExpanded( false );  }

	enum {
		FLAG_EXPANDABLE = 1<<0
	};

	bool	IsAncestor( WatchVar* Var ) const;
	int		GetAncestorDepth() const;

	void SetExpanded( bool expand = true );
	bool IsExpandable() const	{ return m_Flags & FLAG_EXPANDABLE; }
	bool IsExpanded() const		{ return m_Expanded; }

	void SetVars( WatchVarArray& NewVars );

	const WatchVarArray& GetVars() const { return m_Vars; }

	static int FindVar( const WatchVarArray& Vars, const wxString& Name, const wxString& Type );

protected:

   static unsigned int  s_Salt;
   static WatchVarArray s_AllVars;

   unsigned int   m_Id;
   
	WatchVar*		m_Parent;

	wxString		m_Type;
	int         m_Flags;
	wxString		m_Name;
	wxString		m_Value;

	bool			m_Expanded;
	WatchVarArray	m_Vars;	
};


#endif // TORSION_WATCHVAR_H
