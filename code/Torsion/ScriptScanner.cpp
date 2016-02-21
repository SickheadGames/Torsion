// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScriptScanner.h"

#include <wx/mstream.h>
#include <wx/wfstream.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


ScriptScanner::ScriptScanner()
   :  m_Stream( NULL ),
      m_Line( 0 ),
      m_LineChanged( false ),
      m_Token( SSTOKEN_EOF )
{
   // Setup reserved words.
   m_ReservedWords.Add( "break" );
   m_ReservedWords.Add( "case" );
   m_ReservedWords.Add( "continue" );
   m_ReservedWords.Add( "datablock" );
   m_ReservedWords.Add( "default" );
   m_ReservedWords.Add( "else" );
   m_ReservedWords.Add( "false" );
   m_ReservedWords.Add( "for" );
   m_ReservedWords.Add( "function" );
   m_ReservedWords.Add( "if" );
   m_ReservedWords.Add( "local" );
   m_ReservedWords.Add( "new" );
   m_ReservedWords.Add( "or" );
   m_ReservedWords.Add( "package" );
   m_ReservedWords.Add( "return" );
   m_ReservedWords.Add( "singleton" );
   m_ReservedWords.Add( "switch" );
   m_ReservedWords.Add( "switch$" );
   m_ReservedWords.Add( "true" );
   m_ReservedWords.Add( "while" );

   m_ReservedMaxLen = 0;
   m_ReservedMinLen = 99999;

   for ( int i=0; i < m_ReservedWords.GetCount(); i++ )
   {
      size_t len = m_ReservedWords[i].Len();
      if ( len > m_ReservedMaxLen )
         m_ReservedMaxLen = len;
      if ( len < m_ReservedMinLen )
         m_ReservedMinLen = len;
   }

   // Setup operators.
   m_Operators.Add( "!" );
   m_Operators.Add( "!$=" );
   m_Operators.Add( "!=" );
   m_Operators.Add( "-" );
   m_Operators.Add( "--" );
   m_Operators.Add( "$=" );
   m_Operators.Add( "%" );
   m_Operators.Add( "%=" );
   m_Operators.Add( "&" );
   m_Operators.Add( "&&" );
   m_Operators.Add( "&=" );
   m_Operators.Add( "*" );
   m_Operators.Add( "*=" );
   m_Operators.Add( "/" );
   m_Operators.Add( "/=" );
   m_Operators.Add( "@" );
   m_Operators.Add( "^" );
   m_Operators.Add( "^=" );
   m_Operators.Add( "|" );
   m_Operators.Add( "||" );
   m_Operators.Add( "|=" );
   m_Operators.Add( "+" );
   m_Operators.Add( "++" );
   m_Operators.Add( "+=" );
   m_Operators.Add( "<" );
   m_Operators.Add( "<<" );
   m_Operators.Add( "<<=" );
   m_Operators.Add( "<=" );
   m_Operators.Add( "=" );
   m_Operators.Add( "-=" );
   m_Operators.Add( "==" );
   m_Operators.Add( ">" );
   m_Operators.Add( ">=" );
   m_Operators.Add( ">>" );
   m_Operators.Add( ">>=" );
   m_OperatorMaxLen = 0;
   m_OperatorMinLen = 99999;
   for ( int i=0; i < m_Operators.GetCount(); i++ )
   {
      size_t len = m_Operators[i].Len();
      if ( len > m_OperatorMaxLen )
         m_OperatorMaxLen = len;
      if ( len < m_OperatorMinLen )
         m_OperatorMinLen = len;
   }

   // This might save some time with small
   // allocations when the scanner warms up.
   m_Value.Alloc( 1024 );
}

ScriptScanner::~ScriptScanner()
{
   wxDELETE( m_Stream );
}

bool ScriptScanner::Open( const wxString& path )
{
   // Copy the file into a string and create
   // the memory buffer from that.  This should
   // speed up scanning and keep us from issues
   // with files being deleted out from under us.
   wxFile file( path );
   m_Memory.Empty();
   if ( !file.IsOpened() )
      return false;
   size_t len = (size_t)file.Length();
   if ( len > 0 )
   {
	   wxChar* buff = new wxChar(m_Memory.GetChar(len));
      if ( file.Read(buff, len) != len )
         return false;
      //m_Memory.UngetWriteBuf(len);
   }

   m_Line = 0;
   m_LineChanged = false;
   m_Token = SSTOKEN_EOF;

   wxDELETE( m_Stream );
   m_Stream = new wxMemoryInputStream( m_Memory.GetData(), m_Memory.Len() );

   // This might save some time with small
   // allocations when the scanner warms up.
   m_Value.Alloc( 1024 );

   return true;
}

void ScriptScanner::Open( const wxChar* data, size_t length )
{
   m_Memory.Empty();
   m_Line = 0;
   m_LineChanged = false;
   m_Token = SSTOKEN_EOF;

   wxDELETE( m_Stream );
   m_Stream = new wxMemoryInputStream( data, length );

   // This might save some time with small
   // allocations when the scanner warms up.
   m_Value.Alloc( 1024 );
}

bool ScriptScanner::IsReservedWord( const wxString& word )
{
   int len = word.Len();
   if (  len < m_ReservedMinLen ||
         len > m_ReservedMaxLen )
      return false;

   return m_ReservedWords.Index( word ) != wxNOT_FOUND;
}

bool ScriptScanner::IsOperator( const wxString& op )
{
   int len = op.Len();
   if (  len < m_OperatorMinLen ||
         len > m_OperatorMaxLen )
      return false;

   return m_Operators.Index( op ) != wxNOT_FOUND;
}

const SSTOKEN& ScriptScanner::Step( bool skipComments )
{
   m_Value.Clear();
   m_LineChanged = false;
   m_Token = SSTOKEN_WHITESPACE;

   wxChar ch;
   bool blockComment;

   if ( !m_Stream ) {

      m_Token = SSTOKEN_EOF;
      return m_Token;
   }

   for ( ;; )
   {
      if ( !m_Stream->IsOk() || m_Stream->Eof() ) {

         if ( m_Token == SSTOKEN_WHITESPACE )
            m_Token = SSTOKEN_EOF;

         break;
      }

      ch = m_Stream->GetC();

      // Count lines and detect line changes
      // regardless of the token.
      if ( ch == '\n' ) {
         ++m_Line;
         m_LineChanged = true;
      }

      // If we're in an existing token mode then
      // test to see if we need to break out of it.
      if ( m_Token == SSTOKEN_COMMENT ) {

         if ( ch == '\n' && !blockComment ) {

            if ( skipComments ) {

               m_Token = SSTOKEN_WHITESPACE;
               continue;
            }            

            break;
         }

         m_Value += ch;

         if (  blockComment &&
               m_Value.Len() > 3 &&
               m_Value.Last() == '/' &&
               m_Value.GetChar( m_Value.Len() - 2 ) == '*' )
         {

            if ( skipComments ) {

               m_Token = SSTOKEN_WHITESPACE;
               continue;
            }            

            break;
         }
      }

      else if (   m_Token == SSTOKEN_STRING ||
                  m_Token == SSTOKEN_TAGSTRING ) {

         // TODO: Should we report this as an error
         // or just handle it as we do here?
         if ( ch == '\n' ) {

            // We have an open string... close it
            // and stop the token.
            m_Value += m_Value[0];
            break;
         }

         if ( ch != '\r' )
            m_Value += ch;

         if ( ( ch == '\"' || ch == '\'' ) && m_Value[ m_Value.Len() - 2 ] != '\\' )
            break;
      }

      else if (   m_Token == SSTOKEN_WORD || 
                  m_Token == SSTOKEN_GLOBAL || 
                  m_Token == SSTOKEN_LOCAL ) {

         if (  !IsWordChar( ch ) &&
               !( ch == ':' && m_Stream->Peek() == ':' ) &&
               !( m_Value.Last() == ':' && ch == ':' && m_Stream->Peek() != ':' ) &&
               !( m_Value == "switch" && ch == '$' ) ) {

            m_Stream->SeekI( -1, wxFromCurrent );

            ch = m_Value[0];
            if ( ch == '%' || ch == '$' )
            {
               // Do nothing... this is not a reserved
               // word or special operator.  This block
               // skips the next two if statements.
            }

            // Did we get a reserved word?
            else if ( IsReservedWord( m_Value ) )
               m_Token = SSTOKEN_RESERVED;

            // Did we maybe get the special operators; TAB,
            // NL, SPC?  Note the case matters!
            else if ( m_Value == "TAB" ||
                      m_Value == "NL" ||
                      m_Value == "SPC" )
               m_Token = SSTOKEN_OPERATOR;

            break;
         }

         m_Value += ch;
      }

      else if ( m_Token == SSTOKEN_NUMBER ) {

         // TODO: We should support scientific notation
         // and hex a little better no?
         if ( !isalnum( (wxUChar)ch ) && ch != '.' ) {

            m_Stream->SeekI( -1, wxFromCurrent );
            break;
         }
         
         m_Value += ch;
      }

      else if ( m_Token == SSTOKEN_SYMBOL ) {

         if ( isspace( (wxUChar)ch ) || isalnum( (wxUChar)ch ) ) {

            m_Stream->SeekI( -1, wxFromCurrent );
            break;         
         }

         m_Value += ch;
      }

       // If we're in a non-whitespace token then don't look
      // for another till we're done with it.
      if ( m_Token != SSTOKEN_WHITESPACE )
         continue;

      // Look for comments.
      if ( ch == '/' && m_Stream->Peek() == '/' ) {

         m_Token = SSTOKEN_COMMENT;
         blockComment = false;
         m_Value = ch;
      }
      else if ( ch == '/' && m_Stream->Peek() == '*' ) {

         m_Token = SSTOKEN_COMMENT;
         blockComment = true;
         m_Value = ch;
         m_Value << m_Stream->GetC();
      }

      // Look for strings.
      else if ( ch == '\"' ) {

         m_Token = SSTOKEN_STRING;
         m_Value = ch;
      }
      else if ( ch == '\'' ) {

         m_Token = SSTOKEN_TAGSTRING;
         m_Value = ch;
      }

      // Is it a number?
      else if (   wxIsdigit( ch ) ||
                  ( ( ch == '-' || ch == '+' ) && 
                  wxIsdigit( m_Stream->Peek() ) ) ) 
      {
         m_Token = SSTOKEN_NUMBER;
         m_Value = ch;
      }

      // Look for words... these can start with
      // a $ or % in the case of gloabal and 
      // local vars.
      else if ( IsWordChar( ch ) ) 
      {
         m_Token = SSTOKEN_WORD;
         m_Value = ch;
      }

      else if ( ch == '$' && IsWordChar( m_Stream->Peek() ) ) 
      {
         m_Token = SSTOKEN_GLOBAL;
         m_Value = ch;
      }

      else if ( ch == '%' && IsWordChar( m_Stream->Peek() ) ) 
      {
         m_Token = SSTOKEN_LOCAL;
         m_Value = ch;
      }

      else if ( !isspace( (wxUChar)ch ) ) 
      {
         // Shortcut some common symbols.
         if (  ch == '(' || ch == ')' ||
               ch == '{' || ch == '}' ||
               ch == '[' || ch == ']' ||
               ch == ',' || ch == ';' ||
               ch == ':' || ch == '.' ||
               ch == '?' )
         {
            m_Token = SSTOKEN_SYMBOL;
            m_Value = ch;
            break;
         }

         // This should be an operator of some sort.
         m_Value = ch;
         if ( !m_Stream->Eof() )
            m_Value += m_Stream->GetC();
         else
            m_Value = m_Value;
         if ( !m_Stream->Eof() )
            m_Value += m_Stream->GetC();
         else
            m_Value = m_Value;

         if ( !IsOperator( m_Value ) )
         {
            if ( m_Value.Len() == 3 )
            {
               m_Value.RemoveLast();
               m_Stream->SeekI( -1, wxFromCurrent );
            }

            if ( !IsOperator( m_Value ) )
            {
               if ( m_Value.Len() == 2 )
               {
                  m_Value.Truncate( 1 );
                  m_Stream->SeekI( -1, wxFromCurrent );
               }

               if ( !IsOperator( m_Value ) )
               {
                  m_Token = SSTOKEN_UKNOWN;
                  break;
               }
            }
         }

         m_Token = SSTOKEN_OPERATOR;
         break;
      }

   } // for ( ;; )
 
   return m_Token;
}


