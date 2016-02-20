// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ColourHLS.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


ColourHLS::ColourHLS( float h, float l, float s )
   :  m_Hue( h ),
      m_Saturation( s ),
      m_Luminance( l )
{
}

ColourHLS::ColourHLS( const ColourHLS& c )
   :  m_Hue( c.m_Hue ),
      m_Saturation( c.m_Saturation ),
      m_Luminance( c.m_Luminance )
{
}

ColourHLS::ColourHLS( const wxColour& c )
   :  m_Hue( 0 ),
      m_Saturation( 0 ),
      m_Luminance( 0 )
{
   ToHLS( c );
}

#ifdef __WXMSW__

ColourHLS::ColourHLS( DWORD rgb )
   :  m_Hue( 0 ),
      m_Saturation( 0 ),
      m_Luminance( 0 )
{
   ToHLS( wxColour( GetRValue( rgb ), GetGValue( rgb ), GetBValue( rgb ) ) );
}

DWORD ColourHLS::ToDWORD() const
{
   wxColour rgb( ToRGB() );
   return RGB( rgb.Red(), rgb.Green(), rgb.Blue() );
}

#endif

void ColourHLS::SetLuminance( float l )
{
  wxASSERT(l >= 0.0 && l <= 1.0);
  m_Luminance = l;
}

void ColourHLS::SetSaturation( float s )
{
  wxASSERT(s >= 0.0 && s <= 1.0);
  m_Saturation = s;
}

wxColour ColourHLS::ToRGB() const
{
   float R, G, B;
   if ( m_Saturation <= 0.0001f )   //HSL values = 0 ÷ 1
   {
      R = m_Luminance * 255.0f;                      //RGB results = 0 ÷ 255
      G = m_Luminance * 255.0f;
      B = m_Luminance * 255.0f;
   }
   else
   {
      float var_2;
      if ( m_Luminance < 0.5f ) 
         var_2 = m_Luminance * ( 1.0f + m_Saturation );
      else
         var_2 = ( m_Luminance + m_Saturation ) - ( m_Saturation * m_Luminance );

      float var_1 = 2.0f * m_Luminance - var_2;

      float H = m_Hue / 360.0f;
      R = 255.0f * HueToRGB( var_1, var_2, H + ( 1.0f / 3.0f ) );
      G = 255.0f * HueToRGB( var_1, var_2, H );
      B = 255.0f * HueToRGB( var_1, var_2, H - ( 1.0f / 3.0f ) );
   }

   return wxColour( R, G, B );
}

float ColourHLS::HueToRGB( float v1, float v2, float vH ) const
{
   if ( vH < 0.0f ) 
      vH += 1.0f;
   if ( vH > 1.0f ) 
      vH -= 1.0f;

   if ( ( 6.0f * vH ) < 1.0f ) 
      return ( v1 + ( v2 - v1 ) * 6.0f * vH );
   if ( ( 2.0f * vH ) < 1.0f )
      return v2;
   if ( ( 3.0f * vH ) < 2.0f ) 
      return ( v1 + ( v2 - v1 ) * ( ( 2.0f / 3.0f ) - vH ) * 6.0f );

   return v1;
}

ColourHLS& ColourHLS::ToHLS( const wxColour& c )
{
   float var_R = ( (float)c.Red() / 255.0f );                     //Where RGB values = 0 ÷ 255
   float var_G = ( (float)c.Green() / 255.0f );
   float var_B = ( (float)c.Blue() / 255.0f );

   float var_Min = wxMin( var_R, wxMin( var_G, var_B ) );    //Min. value of RGB
   float var_Max = wxMax( var_R, wxMax( var_G, var_B ) );    //Max. value of RGB
   float del_Max = var_Max - var_Min;             //Delta RGB value

   float L = ( var_Max + var_Min ) / 2.0f;
   float H, S;

   if ( del_Max <= 0.0001f )                     //This is a gray, no chroma...
   {
      H = 0.0f;                                //HSL results = 0 ÷ 1
      S = 0.0f;
   }
   else                                    //Chromatic data...
   {
      if ( L < 0.5f )   S = del_Max / ( var_Max + var_Min );
      else              S = del_Max / ( 2.0f - var_Max - var_Min );

      float del_R = ( ( ( var_Max - var_R ) / 6.0f ) + ( del_Max / 2.0f ) ) / del_Max;
      float del_G = ( ( ( var_Max - var_G ) / 6.0f ) + ( del_Max / 2.0f ) ) / del_Max;
      float del_B = ( ( ( var_Max - var_B ) / 6.0f ) + ( del_Max / 2.0f ) ) / del_Max;

      if ( var_R == var_Max ) 
         H = del_B - del_G;
      else if ( var_G == var_Max ) 
         H = ( 1.0f / 3.0f ) + del_R - del_B;
      else if ( var_B == var_Max ) 
         H = ( 2.0f / 3.0f ) + del_G - del_R;

      if ( H < 0.0f )
         H += 1.0f;
      if ( H > 1.0f ) 
         H -= 1.0f;
   }

   m_Hue          = H * 360.0f;
   m_Saturation   = S;
   m_Luminance    = L;

   return *this;
}

ColourHLS& ColourHLS::operator =( const ColourHLS& c )
{
   m_Hue = c.m_Hue;
   m_Saturation = c.m_Saturation;
   m_Luminance = c.m_Luminance;
   return *this;
}

ColourHLS ColourHLS::operator -( const ColourHLS& c ) const
{
   return ColourHLS( m_Hue - c.m_Hue, m_Saturation - c.m_Saturation, m_Luminance - c.m_Luminance );
}

ColourHLS ColourHLS::operator +( const ColourHLS& c ) const
{
   return ColourHLS( m_Hue + c.m_Hue, m_Saturation + c.m_Saturation, m_Luminance + c.m_Luminance );
}

ColourHLS& ColourHLS::operator *=( const ColourHLS& c )
{
   m_Saturation *= c.m_Saturation;      
   m_Luminance *= c.m_Luminance;      
   return *this;
}

ColourHLS& ColourHLS::operator /=( const ColourHLS& c )
{
   m_Saturation /= c.m_Saturation;      
   m_Luminance /= c.m_Luminance;      
   return *this;
}

ColourHLS& ColourHLS::operator *=( const float s )
{
   m_Saturation *= s;      
   m_Luminance *= s;      
   return *this;
}

ColourHLS& ColourHLS::operator /=( const float s )
{
   m_Saturation /= s;      
   m_Luminance /= s;      
   return *this;
}