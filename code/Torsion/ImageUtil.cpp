// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ImageUtil.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


void tsColorizeImage( wxImage& image, const wxColour& black, const wxColour& white )
{
   unsigned char* data = image.GetData();
   const int w = image.GetWidth();
   const int h = image.GetHeight();
   
   float br = black.Red() / 255.0f;
   float bg = black.Green() / 255.0f;
   float bb = black.Blue() / 255.0f;

   float wr = white.Red() / 255.0f;
   float wg = white.Green() / 255.0f;
   float wb = white.Blue() / 255.0f;

   for ( int y = 0; y < h; y++ )
   {
      for ( int x = 0; x < w; x++, data += 3 )
      {
         if (  data[0] == 1 &&
               data[1] == 2 &&
               data[2] == 3 )
            continue;

         float fr = data[0] / 255.0f;
         float fg = data[1] / 255.0f;
         float fb = data[2] / 255.0f;

         // Convert to greyscale... this is Hoffman's
         // algo derived from testing Photoshop's greyscale
         // conversion tool.
         float g = ( 0.2126f * fr ) + ( 0.7152f * fg ) + ( 0.0724f * fb );

         // Change the curve a bit.
         g = powf( g, 2.0f );

         // Blend the greyscale with the target 
         // color scaled by the inverse greyscale.
         float ig = 1.0f - g;
         fr = wxMin( ( wr * g ) + ( br * ig ), 1.0f );
         fg = wxMin( ( wg * g ) + ( bg * ig ), 1.0f );
         fb = wxMin( ( wb * g ) + ( bb * ig ), 1.0f );

         data[0] = fr * 255.0f;
         data[1] = fg * 255.0f;
         data[2] = fb * 255.0f;
      }
   }
}



