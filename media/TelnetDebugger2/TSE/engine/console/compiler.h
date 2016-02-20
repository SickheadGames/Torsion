//-----------------------------------------------------------------------------
// Torque Shader Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

enum CompiledInstructions
{
   OP_FUNC_DECL,
   OP_CREATE_OBJECT,
   OP_CREATE_DATABLOCK,

   OP_NAME_OBJECT,
   OP_ADD_OBJECT,
   OP_END_OBJECT,
   OP_JMPIFFNOT,
   OP_JMPIFNOT,
   OP_JMPIFF,
   OP_JMPIF,
   OP_JMPIFNOT_NP,
   OP_JMPIF_NP,
   OP_JMP,
   OP_RETURN,
   OP_CMPEQ,
   OP_CMPGR,
   OP_CMPGE,
   OP_CMPLT,
   OP_CMPLE,
   OP_CMPNE,
   OP_XOR,
   OP_MOD,
   OP_BITAND,
   OP_BITOR,
   OP_NOT,
   OP_NOTF,
   OP_ONESCOMPLEMENT,

   OP_SHR,
   OP_SHL,
   OP_AND,
   OP_OR,

   OP_ADD,
   OP_SUB,
   OP_MUL,
   OP_DIV,
   OP_NEG,

   OP_SETCURVAR,
   OP_SETCURVAR_CREATE,
   OP_SETCURVAR_ARRAY,
   OP_SETCURVAR_ARRAY_CREATE,

   OP_LOADVAR_UINT,
   OP_LOADVAR_FLT,
   OP_LOADVAR_STR,

   OP_SAVEVAR_UINT,
   OP_SAVEVAR_FLT,
   OP_SAVEVAR_STR,

   OP_SETCUROBJECT,
   OP_SETCUROBJECT_NEW,

   OP_SETCURFIELD,
   OP_SETCURFIELD_ARRAY,

   OP_LOADFIELD_UINT,
   OP_LOADFIELD_FLT,
   OP_LOADFIELD_STR,

   OP_SAVEFIELD_UINT,
   OP_SAVEFIELD_FLT,
   OP_SAVEFIELD_STR,

   OP_STR_TO_UINT,
   OP_STR_TO_FLT,
   OP_STR_TO_NONE,
   OP_FLT_TO_UINT,
   OP_FLT_TO_STR,
   OP_FLT_TO_NONE,
   OP_UINT_TO_FLT,
   OP_UINT_TO_STR,
   OP_UINT_TO_NONE,

   OP_LOADIMMED_UINT,
   OP_LOADIMMED_FLT,
   OP_TAG_TO_STR,
   OP_LOADIMMED_STR,
   OP_LOADIMMED_IDENT,

   OP_CALLFUNC_RESOLVE,
   OP_CALLFUNC,
   OP_PROCESS_ARGS,

   OP_ADVANCE_STR,
   OP_ADVANCE_STR_APPENDCHAR,
   OP_ADVANCE_STR_COMMA,
   OP_ADVANCE_STR_NUL,
   OP_REWIND_STR,
   OP_TERMINATE_REWIND_STR,
   OP_COMPARE_STR,

   OP_PUSH,
   OP_PUSH_FRAME,

   OP_BREAK,

   OP_INVALID
};

class Stream;

class CodeBlock
{
private:
   static CodeBlock* sCurrentCodeBlock;

public:
   static const StringTableEntry getCurrentCodeBlockName();
   StringTableEntry name;

   char *globalStrings;
   char *functionStrings;

   F64 *globalFloats;
   F64 *functionFloats;

   U32 codeSize;
   U32 *code;

   U32 refCount;
   U32 lineBreakPairCount;
   U32 *lineBreakPairs;
   U32 breakListSize;
   U32 *breakList;
   CodeBlock *nextFile;
   StringTableEntry mRoot;

   static CodeBlock *find(StringTableEntry);
   CodeBlock();
   ~CodeBlock();

   void addToCodeList();
   void removeFromCodeList();
   void calcBreakList();
   void clearAllBreaks();
   void setAllBreaks();
   void clearBreakpoint(U32 lineNumber);

   /// Set a OP_BREAK instruction on a line.  If a break 
   /// is not possible on that line it returns false.
   /// @param   lineNumber    The one based line number.
   bool setBreakpoint(U32 lineNumber);

   /// Returns the first breakable line or 0 if none was found.
   /// @param   lineNumber    The one based line number.
   U32 findFirstBreakLine(U32 lineNumber);

   void findBreakLine(U32 ip, U32 &line, U32 &instruction);
   void getFunctionArgs(char buffer[1024], U32 offset);
   const char *getFileLine(U32 ip);

   bool read(StringTableEntry fileName, Stream &st);
   bool compile(const char *dsoName, StringTableEntry fileName, const char *script);

   void incRefCount();
   void decRefCount();
   
   /// Compiles and executes a block of script storing the compiled code in this
   /// CodeBlock.  If there is no filename breakpoints will not be generated and 
   /// the CodeBlock will not be added to the linked list of loaded CodeBlocks.  
   /// Note that if the script contains no executable statements the CodeBlock
   /// will delete itself on return an empty string.  The return string is any 
   /// result of the code executed, if any, or an empty string.
   ///
   /// @param fileName The file name, including path and extension, for the 
   /// block of code or an empty string.
   /// @param script The script code to compile and execute.
   /// @param noCalls Skips calling functions from the script.
   /// @param setFrame A zero based index of the stack frame to execute the code 
   /// with, zero being the top of the stack. If the the index is
   /// -1 a new frame is created. If the index is out of range the
   /// top stack frame is used.
   const char *compileExec(StringTableEntry fileName, const char *script, 
      bool noCalls, int setFrame = -1 );

   /// Executes the existing code in the CodeBlock.  The return string is any 
   /// result of the code executed, if any, or an empty string.
   ///
   /// @param offset The instruction offset to start executing from.
   /// @param fnName The name of the function to execute or null.
   /// @param ns The namespace of the function to execute or null.
   /// @param argc The number of parameters passed to the function or
   /// zero to execute code outside of a function.
   /// @param argv The function parameter list.
   /// @param noCalls Skips calling functions from the script.
   /// @param setFrame A zero based index of the stack frame to execute the code 
   /// with, zero being the top of the stack. If the the index is
   /// -1 a new frame is created. If the index is out of range the
   /// top stack frame is used.
   /// @param packageName The code package name or null.
   const char *exec(U32 offset, const char *fnName, Namespace *ns, U32 argc, 
      const char **argv, bool noCalls, StringTableEntry packageName, 
      S32 setFrame = -1);

};
extern CodeBlock *codeBlockList;

extern F64 consoleStringToNumber(const char *str, StringTableEntry file = 0, U32 line = 0);
extern U32 precompileBlock(StmtNode *block, U32 loopCount);
extern U32 compileBlock(StmtNode *block, U32 *codeStream, U32 ip, U32 continuePoint, U32 breakPoint);

struct CompilerIdentTable
{
   struct Entry
   {
      U32 offset;
      U32 ip;
      Entry *next;
      Entry *nextIdent;
   };
   Entry *list;
   void add(StringTableEntry ste, U32 ip);
   void reset();
   void write(Stream &st);
};

struct CompilerStringTable
{
   U32 totalLen;
   struct Entry
   {
      char *string;
      U32 start;
      U32 len;
      bool tag;
      Entry *next;
   };
   Entry *list;

   char buf[256];

   U32 add(const char *str, bool caseSens = true, bool tag = false);
   U32 addIntString(U32 value);
   U32 addFloatString(F64 value);
   void reset();
   char *build();
   void write(Stream &st);
};

extern CompilerStringTable *currentStringTable;
extern CompilerStringTable gGlobalStringTable;
extern CompilerStringTable gFunctionStringTable;

//------------------------------------------------------------

struct CompilerFloatTable
{
   struct Entry
   {
      F64 val;
      Entry *next;
   };
   U32 count;
   Entry *list;

   U32 add(F64 value);
   void reset();
   F64 *build();
   void write(Stream &st);
};

extern CompilerFloatTable *currentFloatTable, gGlobalFloatTable, gFunctionFloatTable;

extern U32 (*STEtoU32)(StringTableEntry ste, U32 ip);

inline StringTableEntry U32toSTE(U32 u)
{
   return *((StringTableEntry *) &u);
}

