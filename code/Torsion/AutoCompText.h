// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_AUTOCOMPTEXT_H
#define TORSION_AUTOCOMPTEXT_H
#pragma once

/// Note only one AutoCompText can be locked at any time!
class AutoCompText
{
public:

   AutoCompText();

public:

   // The functions sandwiched between these
   // require locks for proper sync'ing between threads.
   void Lock();

      bool IsSignaled() const;
      void DeleteRange( size_t pos, size_t len );
      void InsertRange( size_t pos, const wxString& text );
      wxChar* GetWriteBuf( size_t len );
      const wxChar* GetBuf() const { return m_Data.GetData(); }
      size_t GetBufLen() const { return m_Data.Len(); }
      void Empty() { m_Data.Empty(); }

   void Unlock( bool ClearSignal = false );

private:

   // This should never happen!
   AutoCompText( const AutoCompText& text );

   static wxCriticalSection s_DataLock;
   
   mutable wxCriticalSection m_SignalLock;
   bool m_Signaled;

   wxString m_Data;
};

WX_DECLARE_STRING_HASH_MAP( AutoCompText*, AutoCompTextMap );

#endif // TORSION_AUTOCOMPTEXT_H
