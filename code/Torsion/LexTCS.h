#pragma once
// This is a Scintilla lexer for Torque C Script, aka TorqueScript.
// It is based on LexCPP by Neil Hodgson.

// Manually added for TorqueScript lexer.
#define SCLEX_TCS 78
#define SCE_TCS_DEFAULT 0
#define SCE_TCS_COMMENT 1
#define SCE_TCS_COMMENTLINE 2
#define SCE_TCS_NUMBER 4
#define SCE_TCS_WORD 5
#define SCE_TCS_STRING 6
#define SCE_TCS_CHARACTER 7
#define SCE_TCS_OPERATOR 10
#define SCE_TCS_IDENTIFIER 11
#define SCE_TCS_STRINGEOL 12
//#define SCE_TCS_VERBATIM 13
#define SCE_TCS_WORD2 16
#define SCE_TCS_GLOBALCLASS 19
#define SCE_TCS_VAR 20
#define SCE_TCS_GLOBALVAR 21
#define SCE_TCS_FOLDED 22

// Lexical states for SCLEX_TCS
#define wxSTC_TCS_DEFAULT 0
#define wxSTC_TCS_COMMENT 1
#define wxSTC_TCS_COMMENTLINE 2
#define wxSTC_TCS_COMMENTDOC 3
#define wxSTC_TCS_NUMBER 4
#define wxSTC_TCS_WORD 5
#define wxSTC_TCS_STRING 6
#define wxSTC_TCS_CHARACTER 7
#define wxSTC_TCS_UUID 8
#define wxSTC_TCS_PREPROCESSOR 9
#define wxSTC_TCS_OPERATOR 10
#define wxSTC_TCS_IDENTIFIER 11
#define wxSTC_TCS_STRINGEOL 12
//#define wxSTC_TCS_VERBATIM 13
//#define wxSTC_TCS_REGEX 14
#define wxSTC_TCS_COMMENTLINEDOC 15
#define wxSTC_TCS_WORD2 16
#define wxSTC_TCS_COMMENTDOCKEYWORD 17
#define wxSTC_TCS_COMMENTDOCKEYWORDERROR 18
#define wxSTC_TCS_GLOBALCLASS 19
#define wxSTC_TCS_VAR 20
#define wxSTC_TCS_GLOBALVAR 21
#define wxSTC_TCS_FOLDED 22

#define wxSTC_LEX_TCS 78