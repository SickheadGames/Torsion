//-----------------------------------------------------------------------------
// Torque Shader Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleObject.h"
#include "core/fileStream.h"
#include "core/resManager.h"
#include "console/ast.h"
#include "core/tAlgorithm.h"
#include "console/consoleTypes.h"
#include "console/telnetDebugger.h"
#include "console/simBase.h"
#include "console/compiler.h"
#include <stdarg.h>
#include "platform/platformMutex.h"

ExprEvalState gEvalState;
StmtNode *statementList;
ConsoleConstructor *ConsoleConstructor::first = NULL;

static char scratchBuffer[4096];

// TO-DO: Console debugger stuff to be cleaned up later
static S32 dbgGetCurrentFrame(void)
{
   return gEvalState.stack.size() - 1;
}

static const char * prependDollar ( const char * name )
{
   if(name[0] != '$'){
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '$';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

static const char * prependPercent ( const char * name )
{
   if(name[0] != '%'){
      S32   len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '%';
      dMemcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

//--------------------------------------
void ConsoleConstructor::init(const char *cName, const char *fName, const char *usg, S32 minArgs, S32 maxArgs)
{
   mina = minArgs;
   maxa = maxArgs;
   funcName = fName;
   usage = usg;
   className = cName;
   sc = 0; fc = 0; vc = 0; bc = 0; ic = 0;
   group = false;
   next = first;
   first = this;
}

void ConsoleConstructor::setup()
{
   for(ConsoleConstructor *walk = first; walk; walk = walk->next)
   {
      if(walk->sc)
         Con::addCommand(walk->className, walk->funcName, walk->sc, walk->usage, walk->mina, walk->maxa);
      else if(walk->ic)
         Con::addCommand(walk->className, walk->funcName, walk->ic, walk->usage, walk->mina, walk->maxa);
      else if(walk->fc)
         Con::addCommand(walk->className, walk->funcName, walk->fc, walk->usage, walk->mina, walk->maxa);
      else if(walk->vc)
         Con::addCommand(walk->className, walk->funcName, walk->vc, walk->usage, walk->mina, walk->maxa);
      else if(walk->bc)
         Con::addCommand(walk->className, walk->funcName, walk->bc, walk->usage, walk->mina, walk->maxa);
      else if(walk->group)
         Con::markCommandGroup(walk->className, walk->funcName, walk->usage);
      else if(walk->overload)
         Con::addOverload(walk->className, walk->funcName, walk->usage);
      else
         AssertFatal(false, "Found a ConsoleConstructor with an indeterminate type!");
   }
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, StringCallback sfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   sc = sfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, IntCallback ifunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   ic = ifunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, FloatCallback ffunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   fc = ffunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, VoidCallback vfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   vc = vfunc;
}

ConsoleConstructor::ConsoleConstructor(const char *className, const char *funcName, BoolCallback bfunc, const char *usage, S32 minArgs, S32 maxArgs)
{
   init(className, funcName, usage, minArgs, maxArgs);
   bc = bfunc;
}

ConsoleConstructor::ConsoleConstructor(const char* className, const char* groupName, const char* aUsage)
{
   static char * lastUsage = NULL;

   init(className, groupName, usage, -1, -2);

   group = true;

   // Somewhere, the entry list is getting flipped, partially.
   // so we have to do tricks to deal with making sure usage
   // is properly populated.

   // This is probably redundant.
   if(aUsage) lastUsage = (char *)aUsage;
   usage = lastUsage;
}

bool gWarnUndefinedScriptVariables;

// We comment out the implementation of the Con namespace when doxygenizing because
// otherwise Doxygen decides to ignore our docs in console.h
#ifndef DOXYGENIZING

namespace Con
{

static Vector<ConsumerCallback> gConsumers(__FILE__, __LINE__);
static DataChunker consoleLogChunker;
static Vector<ConsoleLogEntry> consoleLog(__FILE__, __LINE__);
static bool logBufferEnabled=true;
static S32 printLevel = 10;
static FileStream consoleLogFile;
static const char *defLogFileName = "console.log";
static S32 consoleLogMode = 0;
static bool active = false;
static bool newLogFile;
static const char *logFileName;

static const int MaxCompletionBufferSize = 4096;
static char completionBuffer[MaxCompletionBufferSize];
static char tabBuffer[MaxCompletionBufferSize] = {0};
static SimObjectPtr<SimObject> tabObject;
static U32 completionBaseStart;
static U32 completionBaseLen;


/// Current script file name and root, these are registered as
/// console variables.
/// @{
const char * gCurrentFile;
const char * gCurrentRoot;

const char * getCurrentFile(){ return gCurrentFile; }
const char * getCurrentRoot(){ return gCurrentRoot; }
/// @}

ConsoleFunctionGroupBegin( Clipboard, "Miscellaneous functions to control the clipboard and clear the console.");

ConsoleFunction( cls, void, 1, 1, "Clear the screen.")
{
   consoleLogChunker.freeBlocks();
   consoleLog.setSize(0);
};

ConsoleFunction( getClipboard, const char*, 1, 1, "Get text from the clipboard.")
{
	return Platform::getClipboard();
};

ConsoleFunction( setClipboard, bool, 2, 2, "(string text)"
               "Set the system clipboard.")
{
	return Platform::setClipboard(argv[1]);
};

ConsoleFunctionGroupEnd( Clipboard );

void init()
{
   AssertFatal(active == false, "Con::init should only be called once.");

   // Set up general init values.
   active                        = true;
   logFileName                   = NULL;
   newLogFile                    = true;
   gWarnUndefinedScriptVariables = false;

   // Initialize subsystems.
   Namespace::init();
   ConsoleConstructor::setup();

   // Variables
   setVariable("Con::prompt", "% ");
   addVariable("Con::logBufferEnabled", TypeBool, &logBufferEnabled);
   addVariable("Con::printLevel", TypeS32, &printLevel);
   addVariable("Con::warnUndefinedVariables", TypeBool, &gWarnUndefinedScriptVariables);

   // Current script file name and root
   Con::addVariable( "Con::File", TypeString, &gCurrentFile );
   Con::addVariable( "Con::Root", TypeString, &gCurrentRoot );

   // And finally, ACR...
   AbstractClassRep::initialize();
}

//--------------------------------------

void shutdown()
{
   AssertFatal(active == true, "Con::shutdown should only be called once.");
   active = false;

   consoleLogFile.close();
   Namespace::shutdown();
}

bool isActive()
{
   return active;
}

//--------------------------------------

void getLog(ConsoleLogEntry *&log, U32 &size)
{
   log = &consoleLog[0];
   size = consoleLog.size();
}

U32 tabComplete(char* inputBuffer, U32 cursorPos, U32 maxResultLength, bool forwardTab)
{
   // Check for null input.
   if (!inputBuffer[0]) {
      return cursorPos;
   }

   // Cap the max result length.
   if (maxResultLength > MaxCompletionBufferSize) {
      maxResultLength = MaxCompletionBufferSize;
   }

   // See if this is the same partial text as last checked.
   if (dStrcmp(tabBuffer, inputBuffer)) {
      // If not...
      // Save it for checking next time.
      dStrcpy(tabBuffer, inputBuffer);
      // Scan backward from the cursor position to find the base to complete from.
      S32 p = cursorPos;
      while ((p > 0) && (inputBuffer[p - 1] != ' ') && (inputBuffer[p - 1] != '.') && (inputBuffer[p - 1] != '(')) {
         p--;
      }
      completionBaseStart = p;
      completionBaseLen = cursorPos - p;
      // Is this function being invoked on an object?
      if (inputBuffer[p - 1] == '.') {
         // If so...
         if (p <= 1) {
            // Bail if no object identifier.
            return cursorPos;
         }
         // Find the object identifier.
         S32 objLast = --p;
         while ((p > 0) && (inputBuffer[p - 1] != ' ') && (inputBuffer[p - 1] != '(')) {
            p--;
         }
         if (objLast == p) {
            // Bail if no object identifier.
            return cursorPos;
         }
         // Look up the object identifier.
         dStrncpy(completionBuffer, inputBuffer + p, objLast - p);
         completionBuffer[objLast - p] = 0;
         tabObject = Sim::findObject(completionBuffer);
         if (!bool(tabObject)) {
            // Bail if not found.
            return cursorPos;
         }
      }
      else {
         // Not invoked on an object; we'll use the global namespace.
         tabObject = 0;
      }
   }

   // Chop off the input text at the cursor position.
   inputBuffer[cursorPos] = 0;

   // Try to find a completion in the appropriate namespace.
   const char *newText;

   if (bool(tabObject)) {
      newText = tabObject->tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
   }
   else {
      // In the global namespace, we can complete on global vars as well as functions.
      if (inputBuffer[completionBaseStart] == '$') {
         newText = gEvalState.globalVars.tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
      }
      else {
         newText = Namespace::global()->tabComplete(inputBuffer + completionBaseStart, completionBaseLen, forwardTab);
      }
   }

   if (newText) {
      // If we got something, append it to the input text.
      S32 len = dStrlen(newText);
      if (len + completionBaseStart > maxResultLength) {
         len = maxResultLength - completionBaseStart;
      }
      dStrncpy(inputBuffer + completionBaseStart, newText, len);
      inputBuffer[completionBaseStart + len] = 0;
      // And set the cursor after it.
      cursorPos = completionBaseStart + len;
   }

   // Save the modified input buffer for checking next time.
   dStrcpy(tabBuffer, inputBuffer);

   // Return the new (maybe) cursor position.
   return cursorPos;
}

//------------------------------------------------------------------------------
static void log(const char *string)
{
   // Bail if we ain't logging.
   if (!consoleLogMode) {
      return;
   }

   // In mode 1, we open, append, close on each log write.
   if ((consoleLogMode & 0x3) == 1) {
      consoleLogFile.open(defLogFileName, FileStream::ReadWrite);
   }

   // Write to the log if its status is hunky-dory.
   if ((consoleLogFile.getStatus() == Stream::Ok) || (consoleLogFile.getStatus() == Stream::EOS)) {
      consoleLogFile.setPosition(consoleLogFile.getStreamSize());
      // If this is the first write...
      if (newLogFile) {
         // Make a header.
         Platform::LocalTime lt;
         Platform::getLocalTime(lt);
         char buffer[128];
         dSprintf(buffer, sizeof(buffer), "//-------------------------- %d/%d/%d -- %02d:%02d:%02d -----\r\n",
               lt.month + 1,
               lt.monthday,
               lt.year + 1900,
               lt.hour,
               lt.min,
               lt.sec);
         consoleLogFile.write(dStrlen(buffer), buffer);
         newLogFile = false;
         if (consoleLogMode & 0x4) {
            // Dump anything that has been printed to the console so far.
            consoleLogMode -= 0x4;
            U32 size, line;
            ConsoleLogEntry *log;
            getLog(log, size);
            for (line = 0; line < size; line++) {
               consoleLogFile.write(dStrlen(log[line].mString), log[line].mString);
               consoleLogFile.write(2, "\r\n");
            }
         }
      }
      // Now write what we came here to write.
      consoleLogFile.write(dStrlen(string), string);
      consoleLogFile.write(2, "\r\n");
   }

   if ((consoleLogMode & 0x3) == 1) {
      consoleLogFile.close();
   }
}

//------------------------------------------------------------------------------

#ifdef TORQUE_MULTITHREAD
void *gConMutex = NULL;
#endif

static void _printf(ConsoleLogEntry::Level level, ConsoleLogEntry::Type type, const char* fmt, va_list argptr)
{
#ifdef TORQUE_MULTITHREAD
   if(!gConMutex)
      gConMutex = Mutex::createMutex();

   Mutex::lockMutex(gConMutex);
#endif

   char buffer[4096];
   U32 offset = 0;
   if(gEvalState.traceOn && gEvalState.stack.size())
   {
      offset = gEvalState.stack.size() * 3;
      for(U32 i = 0; i < offset; i++)
         buffer[i] = ' ';
   }
   dVsprintf(buffer + offset, sizeof(buffer) - offset, fmt, argptr);

   for(U32 i = 0; i < gConsumers.size(); i++)
      gConsumers[i](level, buffer);

   if(logBufferEnabled || consoleLogMode)
   {
      char *pos = buffer;
      while(*pos)
      {
         if(*pos == '\t')
            *pos = '^';
         pos++;
      }
      pos = buffer;

      for(;;)
      {
         char *eofPos = dStrchr(pos, '\n');
         if(eofPos)
            *eofPos = 0;

         log(pos);
         if(logBufferEnabled)
         {
            ConsoleLogEntry entry;
            entry.mLevel  = level;
            entry.mType   = type;
            entry.mString = (const char *)consoleLogChunker.alloc(dStrlen(pos) + 1);
            dStrcpy(const_cast<char*>(entry.mString), pos);
            consoleLog.push_back(entry);
         }
         if(!eofPos)
            break;
         pos = eofPos + 1;
      }
   }

#ifdef TORQUE_MULTITHREAD
   Mutex::unlockMutex(gConMutex);
#endif
}

//------------------------------------------------------------------------------
void printf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Normal, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void warnf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, type, fmt, argptr);
   va_end(argptr);
}

void errorf(ConsoleLogEntry::Type type, const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, type, fmt, argptr);
   va_end(argptr);
}

void warnf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Warning, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

void errorf(const char* fmt,...)
{
   va_list argptr;
   va_start(argptr, fmt);
   _printf(ConsoleLogEntry::Error, ConsoleLogEntry::General, fmt, argptr);
   va_end(argptr);
}

//---------------------------------------------------------------------------

void setVariable(const char *name, const char *value)
{
   name = prependDollar(name);
   gEvalState.globalVars.setVariable(StringTable->insert(name), value);
}

void setLocalVariable(const char *name, const char *value)
{
   name = prependPercent(name);
   gEvalState.stack.last()->setVariable(StringTable->insert(name), value);
}

void setBoolVariable(const char *varName, bool value)
{
   setVariable(varName, value ? "1" : "0");
}

void setIntVariable(const char *varName, S32 value)
{
   char scratchBuffer[32];
   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%d", value);
   setVariable(varName, scratchBuffer);
}

void setFloatVariable(const char *varName, F32 value)
{
   char scratchBuffer[32];
   dSprintf(scratchBuffer, sizeof(scratchBuffer), "%f", value);
   setVariable(varName, scratchBuffer);
}

//---------------------------------------------------------------------------
void addConsumer(ConsumerCallback consumer)
{
   gConsumers.push_back(consumer);
}

// dhc - found this empty -- trying what I think is a reasonable impl.
void removeConsumer(ConsumerCallback consumer)
{
   for(U32 i = 0; i < gConsumers.size(); i++)
      if (gConsumers[i] == consumer)
      { // remove it from the list.
         gConsumers.erase(i);
         break;
      }
}

void stripColorChars(char* line)
{
   char* c = line;
   char cp = *c;
   while (cp) {
      if (cp < 18) {
         // Could be a color control character; let's take a closer look.
         if ((cp != 8) && (cp != 9) && (cp != 10) && (cp != 13)) {
            // Yep... copy following chars forward over this.
            char* cprime = c;
            char cpp;
            do {
               cpp = *++cprime;
               *(cprime - 1) = cpp;
            } while (cpp);
            // Back up 1 so we'll check this position again post-copy.
            c--;
         }
      }
      cp = *++c;
   }
}

const char *getVariable(const char *name)
{
   // get the field info from the object..
   if(name[0] != '$' && dStrchr(name, '.') && !isFunction(name))
   {
      S32 len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-1, "CONSOLE: name too long");
      dMemcpy(scratchBuffer, name, len+1);

      char * token = dStrtok(scratchBuffer, ".");
      SimObject * obj = Sim::findObject(token);
      if(!obj)
         return("");

      token = dStrtok(0, ".\0");
      if(!token)
         return("");

      while(token != NULL)
      {
         const char * val = obj->getDataField(StringTable->insert(token), 0);
         if(!val)
            return("");

         token = dStrtok(0, ".\0");
         if(token)
         {
            obj = Sim::findObject(token);
            if(!obj)
               return("");
         }
         else
            return(val);
      }
   }

   name = prependDollar(name);
   return gEvalState.globalVars.getVariable(StringTable->insert(name));
}

const char *getLocalVariable(const char *name)
{
   name = prependPercent(name);

   return gEvalState.stack.last()->getVariable(StringTable->insert(name));
}

bool getBoolVariable(const char *varName, bool def)
{
   const char *value = getVariable(varName);
   return *value ? dAtob(value) : def;
}

S32 getIntVariable(const char *varName, S32 def)
{
   const char *value = getVariable(varName);
   return *value ? dAtoi(value) : def;
}

F32 getFloatVariable(const char *varName, F32 def)
{
   const char *value = getVariable(varName);
   return *value ? dAtof(value) : def;
}

//---------------------------------------------------------------------------

bool addVariable(const char *name, S32 t, void *dp)
{
   gEvalState.globalVars.addVariable(name, t, dp);
   return true;
}

bool removeVariable(const char *name)
{
   name = StringTable->lookup(prependDollar(name));
   return name!=0 && gEvalState.globalVars.removeVariable(name);
}

//---------------------------------------------------------------------------

void addCommand(const char *nsName, const char *name,StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *nsName, const char *name,BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void markCommandGroup(const char * nsName, const char *name, const char* usage)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->markGroup(name,usage);
}

void beginCommandGroup(const char * nsName, const char *name, const char* usage)
{
   markCommandGroup(nsName, name, usage);
}

void endCommandGroup(const char * nsName, const char *name)
{
   markCommandGroup(nsName, name, NULL);
}

void addOverload(const char * nsName, const char * name, const char * altUsage)
{
   Namespace *ns = lookupNamespace(nsName);
   ns->addOverload(name,altUsage);
}

void addCommand(const char *name,StringCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,VoidCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,IntCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,FloatCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

void addCommand(const char *name,BoolCallback cb,const char *usage, S32 minArgs, S32 maxArgs)
{
   Namespace::global()->addCommand(StringTable->insert(name), cb, usage, minArgs, maxArgs);
}

bool expandScriptFilename(char *filename, U32 size, const char *src)
{
   const StringTableEntry cbName = CodeBlock::getCurrentCodeBlockName();
   if (!cbName)
   {
      dStrcpy(filename, src);
      return true;
   }

   const char *slash;
   if (dStrncmp(src, "~/", 2) == 0)
      // tilde path means load from current codeblock/mod root
      slash = dStrchr(cbName, '/');
   else if (dStrncmp(src, "./", 2) == 0)
      // dot path means load from current codeblock/mod path
      slash = dStrrchr(cbName, '/');
   else
   {
      // otherwise path must be fully specified
      if (dStrlen(src) > size)
      {
         Con::errorf("Buffer overflow attempting to expand filename: %s", src);
         *filename = 0;
         return false;
      }
      dStrcpy(filename, src);
      return true;
   }

   if (slash == NULL)
   {
      Con::errorf("Illegal CodeBlock path detected (no mod directory): %s", cbName);
      *filename = 0;
      return false;
   }

   U32 length = slash-cbName;
   if ((length+dStrlen(src)) > size)
   {
      Con::errorf("Buffer overflow attempting to expand filename: %s", src);
      *filename = 0;
      return false;
   }

   dStrncpy(filename, cbName, length);
   dStrcpy(filename+length, src+1);
   return true;
}


const char *evaluate(const char* string, bool echo, const char *fileName)
{
   if (echo)
      printf("%s%s", getVariable( "$Con::Prompt" ), string);

   if(fileName)
      fileName = StringTable->insert(fileName);

   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(fileName, string, false, fileName ? -1 : 0);
}

//------------------------------------------------------------------------------
const char *evaluatef(const char* string, ...)
{
   char buffer[4096];
   va_list args;
   va_start(args, string);
   dVsprintf(buffer, sizeof(buffer), string, args);
   CodeBlock *newCodeBlock = new CodeBlock();
   return newCodeBlock->compileExec(NULL, buffer, false, 0);
}

const char *execute(S32 argc, const char *argv[])
{
   Namespace::Entry *ent;
   StringTableEntry funcName = StringTable->insert(argv[0]);
   ent = Namespace::global()->lookup(funcName);

   if(!ent)
   {
      warnf(ConsoleLogEntry::Script, "%s: Unknown command.", argv[0]);
      return "";
   }
   return ent->execute(argc, argv, &gEvalState);
}

//------------------------------------------------------------------------------
const char *execute(SimObject *object, S32 argc, const char *argv[])
{
   static char idBuf[16];
   if(argc < 2)
      return "";
   if(object->getNamespace())
   {
      dSprintf(idBuf, sizeof(idBuf), "%d", object->getId());
      argv[1] = idBuf;

      StringTableEntry funcName = StringTable->insert(argv[0]);
      Namespace::Entry *ent = object->getNamespace()->lookup(funcName);

      if(!ent)
      {
         //warnf(ConsoleLogEntry::Script, "%s: undefined for object '%s' - id %d", funcName, object->getName(), object->getId());
         return "";
      }
      else
      {
         SimObject *save = gEvalState.thisObject;
         gEvalState.thisObject = object;
         const char *ret = ent->execute(argc, argv, &gEvalState);
         gEvalState.thisObject = save;
         return ret;
      }
   }
   warnf(ConsoleLogEntry::Script, "Con::execute - %d has no namespace: %s", object->getId(), argv[0]);
   return "";
}

const char *executef(SimObject *object, S32 argc, ...)
{
   const char *argv[128];

   va_list args;
   va_start(args, argc);
   for(S32 i = 0; i < argc; i++)
      argv[i+1] = va_arg(args, const char *);
   va_end(args);
   argv[0] = argv[1];
   argc++;

   return execute(object, argc, argv);
}

//------------------------------------------------------------------------------
const char *executef(S32 argc, ...)
{
   const char *argv[128];

   va_list args;
   va_start(args, argc);
   for(S32 i = 0; i < argc; i++)
      argv[i] = va_arg(args, const char *);
   va_end(args);
   return execute(argc, argv);
}

//------------------------------------------------------------------------------
bool isFunction(const char *fn)
{
   const char *string = StringTable->lookup(fn);
   if(!string)
      return false;
   else
      return Namespace::global()->lookup(string) != NULL;
}

//------------------------------------------------------------------------------

void setLogMode(S32 newMode)
{
   if ((newMode & 0x3) != (consoleLogMode & 0x3)) {
      if (newMode && !consoleLogMode) {
         // Enabling logging when it was previously disabled.
         newLogFile = true;
      }
      if ((consoleLogMode & 0x3) == 2) {
         // Changing away from mode 2, must close logfile.
         consoleLogFile.close();
      }
      else if ((newMode & 0x3) == 2) {
#ifdef _XBOX
         // Xbox is not going to support logging to a file. Use the OutputDebugStr
         // log consumer
         Platform::debugBreak();
#endif
         // Starting mode 2, must open logfile.
         consoleLogFile.open(defLogFileName, FileStream::Write);
      }
      consoleLogMode = newMode;
   }
}

Namespace *lookupNamespace(const char *ns)
{
   if(!ns)
      return Namespace::global();
   return Namespace::find(StringTable->insert(ns));
}

bool linkNamespaces(const char *parent, const char *child)
{
   Namespace *pns = lookupNamespace(parent);
   Namespace *cns = lookupNamespace(child);
   if(pns && cns)
      return cns->classLinkTo(pns);
   return false;
}

bool classLinkNamespaces(Namespace *parent, Namespace *child)
{
   if(parent && child)
      return child->classLinkTo(parent);
   return false;
}

static S32 typeSizes[MaxDataTypes];
static const char *typeNames[MaxDataTypes] = { NULL, };
static GetDataFunction fnGetData[MaxDataTypes];
static SetDataFunction fnSetData[MaxDataTypes];

const char *getTypeName(S32 type)
{
	return typeNames[type];
}

void registerType(const char *typeName, S32 type, S32 size, GetDataFunction gdf, SetDataFunction sdf)
{
   fnGetData[type] = gdf;
   fnSetData[type] = sdf;
   typeSizes[type] = size;
   typeNames[type] = typeName;
}

void setData(S32 type, void *dptr, S32 index, S32 argc, const char **argv, EnumTable *tbl, BitSet32 flag)
{
   SetDataFunction fn = fnSetData[type];
   AssertFatal(fn, "Error, null setData fn");
   fn((void *) (((const char *)dptr) + index * typeSizes[type]),argc, argv, tbl, flag);
}

const char *getData(S32 type, void *dptr, S32 index, EnumTable *tbl, BitSet32 flag)
{
   GetDataFunction fn = fnGetData[type];
   AssertFatal(fn, "Error, null getData fn");
   return fn((void *) (((const char *)dptr) + index * typeSizes[type]), tbl, flag);
}

} // end of Console namespace

#endif
