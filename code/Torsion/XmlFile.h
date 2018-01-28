// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_XMLFILE_H
#define TORSION_XMLFILE_H
#pragma once


#include "tinyxml2.h"

class XmlFile : public tinyxml2::XMLDocument
{
public:
   XmlFile();
   XmlFile( const wxChar* buffer );

   wxString GetStringElem( const wxString& name, const wxString& value );

   wxPoint GetPointElem( const wxString& name, const wxPoint& value );
   void AddPointElem( const wxString& name, const wxPoint& value );

   bool GetBoolElem( const wxString& name, bool value );
   void AddBoolElem( const wxString& name, bool value );

   int GetIntElem( const wxString& name, int value );
   void AddIntElem( const wxString& name, int value );
   int GetIntAttrib( const wxString& element, const wxString& attrib, int value );

   wxColour GetColorElem( const wxString& name, const wxColour& value );
   void AddColorElem( const wxString& name, const wxColour& value );
   wxColour GetColorAttrib( const wxString& element, const wxString& attrib, const wxColour& color );

   static wxColour StringToColor( const wxString& color );
   static bool StringToBool( const wxChar* boolean );
   static int StringToInt( const wxChar* integer );
   static wxPoint StringToPoint( const wxString& point );

   static wxString ColorToString( const wxColour& color );
   static wxString BoolToString( bool value );
   static wxString IntToString( int value );
   static wxString PointToString( const wxPoint& value );

   int GetArrayStringElems( wxArrayString& output, const wxString& name, const wxString& elemName );
   void AddArrayStringElems( const wxString& name, const wxString& elemName, const wxArrayString& strings );

protected:

   static wxChar s_Temp[MAX_PATH];

};

inline bool XmlFile::StringToBool( const wxChar* boolean )
{
   return stricmp( boolean, "true" ) == 0 || atoi( boolean );
}

inline int XmlFile::StringToInt( const wxChar* integer )
{
   return atoi( integer );
}

inline wxString XmlFile::BoolToString( bool value )
{
   return value ? "true" : "false";
}

inline wxString XmlFile::IntToString( int value )
{
   return itoa( value, s_Temp, 10 ); 
}

#endif // TORSION_XMLFILE_H
