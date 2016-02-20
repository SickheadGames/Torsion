// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef SCRIPTSCANNER_H
#define SCRIPTSCANNER_H
#pragma once

#include <wx/stream.h>

enum SSTOKEN
{
   SSTOKEN_WHITESPACE, // Never returned from ScriptScanner
   
   SSTOKEN_WORD,
   SSTOKEN_GLOBAL,
   SSTOKEN_LOCAL,
   SSTOKEN_RESERVED,
   SSTOKEN_OPERATOR,
   SSTOKEN_SYMBOL,
   SSTOKEN_STRING,
   SSTOKEN_TAGSTRING,
   SSTOKEN_NUMBER,
   SSTOKEN_COMMENT,
   SSTOKEN_EOF,
   SSTOKEN_UKNOWN,
};


/// Returns a token stream from a script file while
/// tracking line numbers.
class ScriptScanner
{
   public:

      ScriptScanner();
      ~ScriptScanner();

      bool Open( const wxString& path );
      void Open( const wxChar* data, size_t length );

      /// Increments the scanner fetching the next
      /// token from the stream.
      const SSTOKEN& Step( bool skipComments = true );

      /// Gets the current token from the last call
      /// to Step().
      const SSTOKEN& GetToken() const { return m_Token; }

      /// Returns the value, if any, that is assocaited
      /// with the current token.
      const wxString& GetValue() const { return m_Value; }

      /// The current line the scanner is on.
      int GetLine() const { return m_Line; }

      /// This is true if the last call to step
      /// went across one or more lines.
      bool LineChanged() const { return m_LineChanged; }

   protected: 

      bool IsReservedWord( const wxString& word );
      bool IsOperator( const wxString& op );

      static inline bool IsWordChar( const wxChar ch ) 
      {
         return (ch < 0x80) && ( isalnum( (wxUChar)ch ) || ch == '_' );
      }

      wxInputStream* m_Stream;
      wxString m_Memory;

      SSTOKEN  m_Token;
      wxString m_Value;
      int      m_Line;
      bool     m_LineChanged;

      wxSortedArrayString  m_ReservedWords;
      size_t               m_ReservedMaxLen;
      size_t               m_ReservedMinLen;

      wxSortedArrayString  m_Operators;
      size_t               m_OperatorMaxLen;
      size_t               m_OperatorMinLen;
};

#endif // SCRIPTSCANNER_H