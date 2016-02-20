// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_COLOURHLS_H
#define TORSION_COLOURHLS_H
#pragma once


class ColourHLS
{
public:

	ColourHLS( float h, float l, float s );
   ColourHLS( const wxColour& c );
   ColourHLS( const ColourHLS& c );
   
   // Some special color operations only used 
   // on windows systems.
   #ifdef __WXMSW__
      ColourHLS( DWORD rgb );
      DWORD ToDWORD() const;
   #endif

public:

   void SetLuminance( float l );  // 0 to 1
   void SetSaturation( float s ); // 0 to 1

   wxColour    ToRGB() const;
   ColourHLS&  ToHLS( const wxColour& c );

   ColourHLS& operator =( const ColourHLS& c );

   ColourHLS operator -( const ColourHLS& c ) const;
   ColourHLS operator +( const ColourHLS& c ) const;
   ColourHLS& operator *=( const ColourHLS& c );
   ColourHLS& operator /=( const ColourHLS& c );
   ColourHLS& operator *=( const float s );
   ColourHLS& operator /=( const float s );

public:

   //unsigned short HueToRGB( unsigned short n1, unsigned short n2, unsigned short hue ) const;
   float HueToRGB( float v1, float v2, float vH ) const;

   float m_Hue;         // 0 to 360
   float m_Saturation;  // 0 to 1
   float m_Luminance;   // 0 to 1

};


#endif // TORSION_COLOURHLS_H
