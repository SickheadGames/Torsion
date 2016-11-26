//
// SICKHEAD GAMES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement 
// or nondisclosure agreement with Sickhead Games.  It may not 
// be copied or disclosed except in accordance with the terms of that
// agreement.
//
//      Copyright (c) 2005 Sickhead Games, LLC
//      All Rights Reserved.
//
// Sickhead Games - Richardson, Texas 75080
// http://www.sickheadgames.com
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <../src/stc/scintilla/include/Scintilla.h>
#include <../src/stc/scintilla/include/ILexer.h>
#include <../src/stc/scintilla/lexlib/PropSetSimple.h>
#include <../src/stc/scintilla/lexlib/LexAccessor.h>
#include <../src/stc/scintilla/lexlib/WordList.h>
#include <../src/stc/scintilla/lexlib/LexerModule.h>
#include <../src/stc/scintilla/lexlib/LexerBase.h>
#include <../src/stc/scintilla/lexlib/Accessor.h>
#include <../src/stc/scintilla/lexlib/StyleContext.h>

#include <../src/stc/scintilla/include/Platform.h>
#include <../src/stc/scintilla/include/Scintilla.h>
#include <../src/stc/scintilla/include/SciLexer.h>


#include "LexTCS.h"

#define KEYWORD_BOXHEADER 1
#define KEYWORD_FOLDCONTRACTED 2

static inline bool IsAWordChar(const int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '.' || ch == '_');
}

static inline bool IsAWordStart(const int ch) {
	return (ch < 0x80) && (isalnum(ch) || ch == '_');
}

static inline bool IsStateComment(const int state) {
	return state == SCE_TCS_COMMENTLINE;
}

static inline bool IsStateString(const int state) {
	return ((state == SCE_TCS_STRING)); // || (state == SCE_TCS_VERBATIM));
}

static const char* TCSOperators[] = 
{
   "!",
   "!$=",
   "!=",
   "-",
   "--",
   "$=",
   "%",
   "%=",
   "&",
   "&&",
   "&=",
   "*",
   "*=",
   "/",
   "/=",
   "@",
   "^",
   "^=",
   "|",
   "||",
   "|=",
   "+",
   "++",
   "+=",
   "<",
   "<<",
   "<<=",
   "<=",
   "=",
   "-=",
   "==",
   ">",
   ">=",
   ">>",
   ">>=",
   "->",
   "-->",
   0
};

static inline bool IsOperatorChar( char ch, int pos ) 
{
	if (isascii(ch) && isalnum(ch))
      return false;

   for ( int i=0; TCSOperators[i] != 0; i++ ) {
      
      if ( pos == 0 && TCSOperators[i][0] == ch )
         return true;
      else if ( pos == 1 && TCSOperators[i][1] == ch )
         return true;                  
      else if ( pos == 2 && TCSOperators[i][1] != 0 && TCSOperators[i][2] == ch )
         return true;
   }

   return false;
}

static inline bool IsOperator( const char* s, size_t len ) 
{
   for ( int i=0;; i++ ) 
   {
      if ( !TCSOperators[i] )
         break;

      if ( strncmp( TCSOperators[i], s, len ) == 0 )
         return true;
   }

   return false;
}

static void ColouriseTCSDoc(unsigned int startPos, int length, 
   int initStyle, WordList *keywordlists[], Accessor &styler) 
{
   WordList &reserved_keywords = *keywordlists[0];
	WordList &library_keywords = *keywordlists[1];
	WordList &globalvar_keywords = *keywordlists[2];

   // Do not leak onto next line
	if (initStyle == SCE_TCS_STRINGEOL)
		initStyle = SCE_TCS_DEFAULT;

	int chPrevNonWhite = ' ';
	int visibleChars = 0;
   char cur[256];
   char low[256];

	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward()) {

      if (sc.atLineStart && (sc.state == SCE_TCS_STRING)) {
			// Prevent SCE_TCS_STRINGEOL from leaking back to previous line
			sc.SetState(SCE_TCS_STRING);
		}

		// Handle line continuation generically.
		if (sc.ch == '\\') {
			if (sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}

		// Determine if the current state should terminate.
		if (sc.state == SCE_TCS_OPERATOR) {

         sc.GetCurrent(cur, sizeof(cur));
         size_t len = strlen( cur );
         cur[ len ] = sc.ch;
         cur[ len + 1 ] = 0;

         // Fix for the !$var case...
         if ( strcmp( cur, "!$" ) == 0 && sc.chNext != '=' )
            sc.SetState(SCE_TCS_DEFAULT);
         
         else if ( !IsOperator( cur, len+1 ) )
         {
            // Make sure this is really a valid operator.
            cur[ len ] = 0;
            if ( IsOperator( cur, len ) )
               sc.SetState(SCE_TCS_DEFAULT);
            else
               sc.ChangeState(SCE_TCS_DEFAULT);
         }

      } else if (sc.state == SCE_TCS_NUMBER) {
			if (!IsAWordChar(sc.ch)) {
				sc.SetState(SCE_TCS_DEFAULT);
			}
		} else if (sc.state == SCE_TCS_IDENTIFIER) {

         // Silly hack for switch$.
         sc.GetCurrent(cur, sizeof(cur));
         if (  !(stricmp( "switch", cur ) == 0 && sc.ch == '$' ) &&
               !(cur[0] == '$' && sc.ch == ':') &&
               !(cur[0] == '%' && sc.ch == ':') &&
               (!IsAWordChar(sc.ch) || (sc.ch == '.'))) {

				sc.GetCurrentLowered(low, sizeof(low));

            if (  strcmp( cur, "SPC" ) == 0 ||
                  strcmp( cur, "TAB" ) == 0 ||
                  strcmp( cur, "NL" ) == 0 )
					sc.ChangeState(SCE_TCS_OPERATOR);
            else if (reserved_keywords.InList(cur))
					sc.ChangeState(SCE_TCS_WORD);
				else if (library_keywords.InList(low))
					sc.ChangeState(SCE_TCS_WORD2);
				else if (globalvar_keywords.InList(low))
					sc.ChangeState(SCE_TCS_GLOBALCLASS);
            else if ( cur[0] == '%' )
					sc.ChangeState(SCE_TCS_VAR);
            else if ( cur[0] == '$' )
					sc.ChangeState(SCE_TCS_GLOBALVAR);
            else
               sc.ChangeState(SCE_TCS_IDENTIFIER);

            sc.SetState(SCE_TCS_DEFAULT);
         }

      } else if (sc.state == SCE_TCS_COMMENT) {

			if ( sc.Match('*', '/') ) {
				sc.Forward();
				sc.ForwardSetState(SCE_TCS_DEFAULT);
			}

		} else if (sc.state == SCE_TCS_COMMENTLINE ) {

         if ( sc.atLineEnd ) {
            sc.SetState(SCE_TCS_DEFAULT);
				visibleChars = 0;
			}

      } else if (sc.state == SCE_TCS_STRING) {
			if (sc.ch == '\\') {
				if (sc.chNext == '\"' || sc.chNext == '\'' || sc.chNext == '\\') {
					sc.Forward();
				}
			} else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_TCS_DEFAULT);
			} else if (sc.atLineEnd) {
				sc.ChangeState(SCE_TCS_STRINGEOL);
				sc.ForwardSetState(SCE_TCS_DEFAULT);
				visibleChars = 0;
			}
		} else if (sc.state == SCE_TCS_CHARACTER) {
			if (sc.atLineEnd) {
				sc.ChangeState(SCE_TCS_STRINGEOL);
				sc.ForwardSetState(SCE_TCS_DEFAULT);
				visibleChars = 0;
			} else if (sc.ch == '\\') {
				if (sc.chNext == '\"' || sc.chNext == '\'' || sc.chNext == '\\') {
					sc.Forward();
				}
			} else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_TCS_DEFAULT);
			}
		}

      // Determine if a new state should be entered.
		if (sc.state == SCE_TCS_DEFAULT) {
         if (isdigit(sc.ch) || (sc.ch == '.' && isdigit(sc.chNext))) {
				sc.SetState(SCE_TCS_NUMBER);
			} else if ( IsAWordStart(sc.ch) || 
                     ( sc.ch == '%' && IsAWordStart(sc.chNext) ) ||
                     ( sc.ch == '$' && IsAWordStart(sc.chNext) ) ) {
            sc.SetState(SCE_TCS_IDENTIFIER);
			} else if (sc.Match('/', '*')) {
            sc.SetState(SCE_TCS_COMMENT);
				sc.Forward();	// Eat the * so it isn't used for the end of the comment
			} else if (sc.Match('/', '/')) {
            sc.SetState(SCE_TCS_COMMENTLINE);
			} else if (sc.ch == '\"') {
				sc.SetState(SCE_TCS_STRING);
			} else if (sc.ch == '\'') {
				sc.SetState(SCE_TCS_CHARACTER);
			} else if ( /*sc.lastState != SCE_TCS_OPERATOR &&*/ IsOperatorChar(sc.ch,0) ) {
				sc.SetState(SCE_TCS_OPERATOR);
			}
		}

		if (sc.atLineEnd) {
			// Reset states to begining of colourise so no surprises
			// if different sets of lines lexed.
			chPrevNonWhite = ' ';
			visibleChars = 0;
		}
		if (!isspace(sc.ch)) {
			chPrevNonWhite = sc.ch;
			visibleChars++;
		}
	}
	sc.Complete();
}

static int GetStyleFirstWord( unsigned int szLine, Accessor& styler )
{
	int nsPos = styler.LineStart( szLine );
	int nePos = styler.LineStart( szLine+1 ) - 1;

   while ( isspace( styler.SafeGetCharAt( nsPos ) ) && nsPos < nePos )
		nsPos++;
         
   // Get the style... sans indicator bits!
   return styler.StyleAt( nsPos ) & 0x1F;
}

static bool GetNextWord( int nsPos, Accessor& styler, char* word )
{
   int szLine = styler.GetLine( nsPos );
	int nePos = styler.LineStart( szLine+1 ) - 1;

   while ( isspace( styler.SafeGetCharAt( nsPos ) ) && nsPos < nePos )
		nsPos++;

   int i = 0;
   while ( (styler.StyleAt( nsPos ) & 0x1F) == SCE_TCS_WORD && nsPos < nePos )
      word[i++] = styler.SafeGetCharAt( nsPos++ );

   word[i] = 0;

   return i > 0;
}

static int FindStartBracket( int nsPos, Accessor& styler )
{
   int parenthisis = 0;

   for ( ;; ) {

      char ch = styler.SafeGetCharAt( nsPos++, 0 );
      if ( ch == 0 )
         break;

      // This is usually function parameters, for arguments,
      // if/while conditions, etc...
      if ( ch == '(' )
         ++parenthisis;
      else if ( ch == ')' )
         --parenthisis;

      if ( parenthisis != 0 )
         continue;

      // If we reached the end of a statement then
      // there is not a start bracket.
      if ( ch == ';' )
         break;

      if ( ch == '{' )
         return nsPos;
   }

   return -1;
}

static bool FindFoldWordReverse( int nsPos, Accessor& styler )
{
   int parenthisis = 0;

   for ( ;; ) {

      char ch = styler.SafeGetCharAt( nsPos--, 0 );
      if ( ch == 0 )
         break;

      // This is usually function parameters, for arguments,
      // if/while conditions, etc...
      if ( ch == '(' )
         --parenthisis;
      else if ( ch == ')' )
         ++parenthisis;

      if ( parenthisis != 0 )
         continue;

      // If we reached the end of a statement then
      // there is not fold word.
      if ( ch == ';' )
         break;

      char word[100];
      if ( GetNextWord( nsPos, styler, word ) ) {
         
         if (  strcmp( word, "function" ) == 0 ||
               strcmp( word, "datablock" ) == 0 ||
               strcmp( word, "package" ) == 0 ||
               strcmp( word, "switch" ) == 0 || 
               strcmp( word, "switch$" ) == 0 || 
               strcmp( word, "if" ) == 0 ||
               strcmp( word, "while" ) == 0 ||
               strcmp( word, "for" ) == 0 ||
               strcmp( word, "else" ) == 0 ||
               strcmp( word, "new" ) == 0 ||
               strcmp( word, "singleton" ) == 0 )
         {
            return true;
         }
      }
   }

   return false;
}

static void FoldNoBoxTCSDoc( unsigned int startPos, int length,
   int initStyle, Accessor &styler )
{
   const bool foldComments = styler.GetPropertyInt("fold.comment") != 0;
	//const bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	const bool foldAtElse = styler.GetPropertyInt("fold.at.else", 0) != 0;
	const bool clearFolds = styler.GetPropertyInt("fold.clearfold", 0) != 0;

   int lineCurrent = styler.GetLine( startPos );
   int i = styler.LineStart( lineCurrent );
   const int endPos = styler.LineStart( styler.GetLine( startPos + length ) + 2 );

   int levelCurrent = SC_FOLDLEVELBASE;
   if ( lineCurrent > 0 )
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;

   int style = styler.StyleAt( i - 1 ) & 0x1F;
   int stylePrev, styleNext;

   for ( ; i < endPos; ) 
   {
      int levelNext = levelCurrent;
      int levelMinCurrent = levelCurrent;
      char word[100];

      const int lineEnd = styler.LineStart( lineCurrent + 1 );


      // Loop for the next line!
      while ( i < lineEnd ) {

         const char ch = styler.SafeGetCharAt( i, 0 );
         if ( ch == 0 )
            break;
         
         //const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

         // Get the style... sans indicator bits!
   		stylePrev = style;
         style = styler.StyleAt( i ) & 0x1F;
   		styleNext = styler.StyleAt(i + 1) & 0x1F;

         if ( style == SCE_TCS_COMMENTLINE ) 
         {
            // Is this the start or end of a region?
            if ( styler.Match( i, "//#region" ) )
               levelNext++;
            else if ( styler.Match( i, "//#endregion" ) )
               levelNext--;

            // We don't deal with line comments...
            // this line is over!
            i = lineEnd;
            break;
         }
	      else if ( foldComments && style == SCE_TCS_COMMENT ) 
         {
		      if ( stylePrev != SCE_TCS_COMMENT )
			      levelNext++;
		      else if (styleNext != SCE_TCS_COMMENT)
			      levelNext--;
	      }
         else if ( style == SCE_TCS_WORD && GetNextWord( i, styler, word ) ) 
         {
            const int wordLen = strlen( word );

            if (  strcmp( word, "function" ) == 0 ||
                  strcmp( word, "datablock" ) == 0 ||
                  strcmp( word, "package" ) == 0 ||
                  strcmp( word, "switch" ) == 0 || 
                  strcmp( word, "switch$" ) == 0 || 
                  strcmp( word, "if" ) == 0 ||
                  strcmp( word, "while" ) == 0 ||
                  strcmp( word, "for" ) == 0 ||
                  strcmp( word, "else" ) == 0 ||
                  strcmp( word, "new" ) == 0 ||
                  strcmp( word, "singleton" ) == 0 )
            {
               int j = FindStartBracket( i + wordLen, styler );
               if ( j != -1 ) 
               {
				      if ( levelMinCurrent > levelNext )
					      levelMinCurrent = levelNext;

                  levelNext++;
                  i = j;
                  continue;
               }
            }

            i += wordLen;
            continue;
         }
         else if ( style == SCE_TCS_DEFAULT ) 
         {
            // We must look up to see if one of those folding
            // keywords appears above us...
            if ( ch == '{' && !FindFoldWordReverse( i, styler ) )
               levelNext++;
            else if ( ch == '}' )
               levelNext--;
		   }

         i++;
      }

      // We've finished the previous line... figure out the next level.
		int levelUsed = levelCurrent;
		if (foldAtElse)
			levelUsed = levelMinCurrent;

      // Never allow negative levels!
      if ( levelNext < SC_FOLDLEVELBASE )
         levelNext = SC_FOLDLEVELBASE;

      int lev = levelUsed | levelNext << 16;

      // Are we starting a fold?
      if ( levelUsed < levelNext )
         lev |= SC_FOLDLEVELHEADERFLAG;
      
      styler.SetLevel( lineCurrent, lev );

      // Did we skip multiple lines?
      int lineNext = styler.GetLine( i );
      for ( int j=lineCurrent+1; j < lineNext; j++ )
         styler.SetLevel( j, levelNext | levelNext << 16 );

      lineCurrent = lineNext;
      levelCurrent = levelNext;
	}
}

static void FoldTCSDoc( unsigned int startPos, int length, int initStyle, 
                       WordList *[], Accessor &styler ) {

   FoldNoBoxTCSDoc(startPos, length, initStyle, styler);
}

static const char * const tcsWordLists[] = {
            "Case sensitive reserved words.",
            "Case insensitive imported engine functions.",
            "Case insensitive imported engine variables.",
            0,
        };

LexerModule lmTCS(SCLEX_TCS, ColouriseTCSDoc, "tcs", FoldTCSDoc, tcsWordLists);
