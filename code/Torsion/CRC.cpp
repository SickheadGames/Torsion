// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "CRC.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


static unsigned int tsCRCTable[256];
static bool tsCRCTableValid = false;

static void tsCalculateCRCTable()
{
   for(int i = 0; i < 256; i++)
   {
      unsigned int val = i;
      for(int j = 0; j < 8; j++)
      {
         if(val & 0x01)
            val = 0xedb88320 ^ (val >> 1);
         else
            val = val >> 1;
      }
      tsCRCTable[i] = val;
   }
   
   tsCRCTableValid = true;
}

unsigned int tsGetCRC( const void* buffer, int len, unsigned int crcVal )
{
   // check if need to generate the crc table
   if ( !tsCRCTableValid )
      tsCalculateCRCTable();
   
   // now calculate the crc
   char * buf = (char*)buffer;
   for(int i = 0; i < len; i++)
      crcVal = tsCRCTable[(crcVal ^ buf[i]) & 0xff] ^ (crcVal >> 8);

   return crcVal;
}

