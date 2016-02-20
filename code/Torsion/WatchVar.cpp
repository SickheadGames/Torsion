// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "WatchVar.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

unsigned int WatchVar::s_Salt = 0;
WatchVarArray WatchVar::s_AllVars;

WatchVar::WatchVar( WatchVar* Parent, const wxString& Type,
   int Flags, const wxString& Name,  const wxString& Value )
      :	m_Id( s_Salt++ ),
         m_Type( Type ),
         m_Flags( Flags ),
         m_Name( Name ),
         m_Value( Value ),
         m_Parent( Parent ),
         m_Expanded( false )
{
   s_AllVars.Add( this ); 
}

WatchVar::WatchVar( const WatchVar& wv )
   :  m_Id( wv.m_Id ),
      m_Type( wv.m_Type ),
      m_Flags( wv.m_Flags ),
      m_Name( wv.m_Name ),
      m_Value( wv.m_Value ),
      m_Parent( wv.m_Parent ),
      m_Expanded( wv.m_Expanded )
{
   s_AllVars.Add( this ); 
}

WatchVar::~WatchVar()
{
   SetExpanded( false );
   s_AllVars.Remove( this ); 
}

int WatchVar::GetTag() const 
{ 
   // TODO: Fix the tag system so we don't get 
   // warnings on this cast.
   return reinterpret_cast<int>( this ); 
}

WatchVar* WatchVar::FindVarByTag( int tag )
{
   WatchVar* FromTag = reinterpret_cast<WatchVar*>( tag );
   int i = s_AllVars.Index( FromTag );
   if ( i != wxNOT_FOUND ) {
      return FromTag;
   }

   return NULL;
}

bool WatchVar::IsAncestor( WatchVar* Var ) const
{
	WatchVar* Parent = m_Parent;
	while ( Parent ) {
		if ( Parent == Var ) {
			return true;
		}
		Parent = Parent->GetParent();
	}

	return false;
}

int	WatchVar::GetAncestorDepth() const
{
   int Depth = 0;

	WatchVar* Parent = m_Parent;
	while ( Parent ) {
		++Depth;
		Parent = Parent->GetParent();
	}

	return Depth;
}

void WatchVar::SetExpanded( bool expand )
{
	if ( m_Expanded && !expand ) {

		for ( int i=0; i < m_Vars.GetCount(); i++ ) {
			delete m_Vars[ i ];
		}
		m_Vars.Empty();

		m_Expanded = false;

	} else if ( expand && IsExpandable() ) {

		m_Expanded = true;
	}
}

void WatchVar::SetVars( WatchVarArray& NewVars )
{
	WatchVarArray OldVars = m_Vars;
	m_Vars = NewVars;
	NewVars.Clear();

	// Loop thru the new var replacing with matching old vars.
	for ( int i=0; i < m_Vars.GetCount(); i++ ) {
		     
		wxASSERT( m_Vars[i] );
		int l = FindVar( OldVars, m_Vars[i]->GetName(), m_Vars[i]->GetType() );
		if ( l != -1 ) {

			WatchVar* Temp = m_Vars[i];
			m_Vars[i] = OldVars[l];
			m_Vars[i]->m_Value = Temp->m_Value;
			m_Vars[i]->m_Parent = this;
			delete Temp;
            OldVars[l] = NULL;

		} else {

			m_Vars[i]->m_Parent = this;
		}
	}

	// Delete any remaining old vars.
	for ( int i=0; i < OldVars.GetCount(); i++ ) {
		delete OldVars[i];
	}
}

int WatchVar::FindVar( const WatchVarArray& Vars, const wxString& Name, const wxString& Type )
{
	for ( int i=0; i < Vars.GetCount(); i++ ) {

		if (	Vars[i] &&
				Vars[i]->GetName() == Name &&
				Vars[i]->GetType() == Type ) {

			return i;
		}
	}	

	return -1;
}

