// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPUTIL_H
#define TORSION_AUTOCOMPUTIL_H
#pragma once


template<class T> int CmpNameNoCase( T* first, T* second )
{
   wxASSERT( first );
   wxASSERT( second );
   return first->GetName().CmpNoCase( second->GetName() );
}

inline int CmpStringNoCase( const wxString& first, const wxString& second )
{
   return first.CmpNoCase( second );
}

#endif // TORSION_AUTOCOMPUTIL_H
