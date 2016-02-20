//-----------------------------------------------------------------------------
// Torque Game Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformVideo.h"
#include "platform/platformInput.h"
#include "platform/platformAudio.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "core/tVector.h"
#include "math/mMath.h"
#include "dgl/dgl.h"
#include "dgl/gBitmap.h"
#include "core/resManager.h"
#include "core/fileStream.h"
#include "dgl/gTexManager.h"
#include "dgl/gFont.h"
#include "console/console.h"
#include "console/simBase.h"
#include "gui/guiCanvas.h"
#include "sim/actionMap.h"
#include "core/dnet.h"
#include "core/bitStream.h"
#include "console/telnetConsole.h"
#include "console/telnetDebugger.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "dgl/gTexManager.h"
#include "core/resManager.h"
#include "platform/platformVideo.h"
#include "dgl/materialPropertyMap.h"
#include "sim/netStringTable.h"
#include "platform/platformRedBook.h"
#include "game/demoGame.h"
#include "core/frameAllocator.h"
#include "game/version.h"
#include "platform/profiler.h"
#include "game/net/serverQuery.h"

#ifndef BUILD_TOOLS
DemoGame GameObject;
DemoNetInterface GameNetInterface;
#endif

ConsoleFunctionGroupBegin( Platform , "General platform functions.");

ConsoleFunction( lockMouse, void, 2, 2, "(bool isLocked)"
                "Lock the mouse (or not, depending on the argument's value) to the window.")
{
   Platform::setWindowLocked(dAtob(argv[1]));
}

ConsoleFunction( setNetPort, bool, 2, 2, "(int port)"
                "Set the network port for the game to use.")
{
   return Net::openPort(dAtoi(argv[1]));
}

ConsoleFunction( createCanvas, bool, 2, 2, "(string windowTitle)"
                "Create the game window/canvas, with the specified window title.")
{
   AssertISV(!Canvas, "cCreateCanvas: canvas has already been instantiated");

#if !defined(TORQUE_OS_MAC) // macs can only run one instance in general.
#if !defined(TORQUE_DEBUG) && !defined(INTERNAL_RELEASE)
   if(!Platform::excludeOtherInstances("TorqueTest"))
      return false;
#endif
#endif
   Platform::initWindow(Point2I(800, 600), argv[1]);

   // create the canvas, and add it to the manager
   Canvas = new GuiCanvas();
   Canvas->registerObject("Canvas"); // automatically adds to GuiGroup


   return true;
}

ConsoleFunction( saveJournal, void, 2, 2, "(string filename)"
                "Save the journal to the specified file.")
{
   Game->saveJournal(argv[1]);
}

ConsoleFunction( playJournal, void, 2, 3, "(string filename, bool break=false)"
                "Begin playback of a journal from a specified field, optionally breaking at the start.")
{
   bool jBreak = (argc > 2)? dAtob(argv[2]): false;
   Game->playJournal(argv[1],jBreak);
}

extern void netInit();
extern void processConnectedReceiveEvent( ConnectedReceiveEvent * event );
extern void processConnectedNotifyEvent( ConnectedNotifyEvent * event );
extern void processConnectedAcceptEvent( ConnectedAcceptEvent * event );
extern void ShowInit();

/// Initalizes the components of the game like the TextureManager, ResourceManager
/// console...etc.
static bool initLibraries()
{
   if(!Net::init())
   {
      Platform::AlertOK("Network Error", "Unable to initialize the network... aborting.");
      return false;
   }

   // asserts should be created FIRST
   PlatformAssert::create();

   FrameAllocator::init(3 << 20);      // 3 meg frame allocator buffer

//   // Cryptographic pool next
//   CryptRandomPool::init();

   _StringTable::create();
   TextureManager::create();
   ResManager::create();

   // Register known file types here
#ifndef _NO_JPG_SUPPORT
   ResourceManager->registerExtension(".jpg", constructBitmapJPEG);
#endif // _NO_JPG_SUPPORT
#ifndef _NO_GIF_SUPPORT
   ResourceManager->registerExtension(".gif", constructBitmapGIF);
#endif // _NO_GIF_SUPPORT
#ifndef _NO_BMP_SUPPORT
   ResourceManager->registerExtension(".bmp", constructBitmapBMP);
#endif // _NO_BMP_SUPPORT

   ResourceManager->registerExtension(".bm8", constructBitmapBM8);
   ResourceManager->registerExtension(".dbm", constructBitmapDBM);
   ResourceManager->registerExtension(".png", constructBitmapPNG);
   ResourceManager->registerExtension(".gft", constructFont);

   RegisterCoreTypes();
   RegisterMathTypes();
   RegisterGuiTypes();
   
   Con::init();
   NetStringTable::create();
   
   TelnetConsole::create();
   TelnetDebugger::create();

   Processor::init();
   Math::init();
   Platform::init();    // platform specific initialization
   RedBook::init();
   
   return true;
}

/// Destroys all the things initalized in initLibraries
static void shutdownLibraries()
{
   // Purge any resources on the timeout list...
   if (ResourceManager)
      ResourceManager->purge();

   RedBook::destroy();

   TextureManager::preDestroy();
   
   Platform::shutdown();
   TelnetDebugger::destroy();
   TelnetConsole::destroy();

   NetStringTable::destroy();
   Con::shutdown();

   ResManager::destroy();
   TextureManager::destroy();

   _StringTable::destroy();

   // asserts should be destroyed LAST
   FrameAllocator::destroy();

   PlatformAssert::destroy();
   Net::shutdown();
}

ConsoleFunction( setModPaths, void, 2, 2, "(string paths)"
                "Set the mod paths the resource manager is using. These are semicolon delimited.")
{
   char buf[512];
   dStrncpy(buf, argv[1], sizeof(buf) - 1);
   buf[511] = '\0';

   Vector<char*> paths;
   char* temp = dStrtok( buf, ";" );
   while ( temp )
   {
      if ( temp[0] )
         paths.push_back( temp );
      temp = dStrtok( NULL, ";" );
   }

   ResourceManager->setModPaths( paths.size(), (const char**) paths.address() );
}

ConsoleFunction( getModPaths, const char*, 1, 1, "Return the mod paths the resource manager is using.")
{
   return( ResourceManager->getModPaths() );
}

ConsoleFunction( getVersionNumber, S32, 1, 1, "Get the version of the build, as a string.")
{
   return getVersionNumber();
}

ConsoleFunction( getVersionString, const char*, 1, 1, "Get the version of the build, as a string.")
{
   return getVersionString();
}

ConsoleFunction( getCompileTimeString, const char*, 1, 1, "Get the time of compilation.")
{
   return getCompileTimeString();
}

ConsoleFunction( getBuildString, const char*, 1, 1, "Get the type of build, \"Debug\" or \"Release\".")
{
#ifdef TORQUE_DEBUG
   return "Debug";
#else
   return "Release";
#endif
}

ConsoleFunction( getSimTime, S32, 1, 1, "Return the current sim time in milliseconds.\n\n"
                "Sim time is time since the game started.")
{
   return Sim::getCurrentTime();
}

ConsoleFunction( getRealTime, S32, 1, 1, "Return the current real time in milliseconds.\n\n"
                "Real time is platform defined; typically time since the computer booted.")
{
   return Platform::getRealMilliseconds();
}

ConsoleFunctionGroupEnd(Platform);

static F32 gTimeScale = 1.0;
static U32 gTimeAdvance = 0;
static U32 gFrameSkip = 0;
static U32 gFrameCount = 0;

// Executes an entry script; can be controlled by command-line options.
bool runEntryScript (int argc, const char **argv) 
{
   // Executes an entry script file. This is "main.cs" 
   // by default, but any file name (with no whitespace 
   // in it) may be run if it is specified as the first 
   // command-line parameter. The script used, default 
   // or otherwise, is not compiled and is loaded here 
   // directly because the resource system restricts 
   // access to the "root" directory.
   
   FileStream str;                              // The working filestream.
   const char*  defaultScriptName = "main.cs";
   bool useDefaultScript = true;                
   
   // Check if any command-line parameters were passed (the first is just the app name).
   if (argc > 1)
   {
      // If so, check if the first parameter is a file to open.  
      if ( (str.open(argv[1], FileStream::Read)) && (argv[1] != "") )
      {
         // If it opens, we assume it is the script to run.
         useDefaultScript = false;
      }
   }
      
   if (useDefaultScript)
   {
      if (!str.open(defaultScriptName, FileStream::Read))
      {
         char msg[1024];
         dSprintf(msg, sizeof(msg), "Failed to open \"%s\".", defaultScriptName);
         Platform::AlertOK("Error", msg);
         return false;
      }
   }
   
   U32 size = str.getStreamSize();
   char *script = new char[size + 1];
   str.read(size, script);
   str.close();
   script[size] = 0;

   Con::evaluate(script, false, useDefaultScript ? defaultScriptName : argv[1]); 
   delete[] script;

   return true;
}

/// Initalize game, run the specified startup script
bool initGame(int argc, const char **argv)
{
   Con::setFloatVariable("Video::texResidentPercentage", -1.0f);
   Con::setIntVariable("Video::textureCacheMisses", -1);
   Con::addVariable("timeScale", TypeF32, &gTimeScale);
   Con::addVariable("timeAdvance", TypeS32, &gTimeAdvance);
   Con::addVariable("frameSkip", TypeS32, &gFrameSkip);
   //
#ifdef GATHER_METRICS
   Con::addVariable("Video::numTexelsLoaded", TypeS32, &TextureManager::smTextureSpaceLoaded);
#else
   static U32 sBogusNTL = 0;
   Con::addVariable("Video::numTexelsLoaded", TypeS32, &sBogusNTL);
#endif
   netInit();
   //GameInit();
   //ShowInit();
   //MoveManager::init();

   Sim::init();

   ActionMap* globalMap = new ActionMap;
   globalMap->registerObject("GlobalActionMap");
   Sim::getActiveActionMapSet()->pushObject(globalMap);

   // run the entry script and return.
   return runEntryScript(argc, argv);
}

/// Shutdown the game and delete core objects
void shutdownGame()
{
   //exec the script onExit() function
   Con::executef(1, "onExit");

   // Note: tho the SceneGraphs are created after the Manager, delete them after, rather
   //  than before to make sure that all the objects are removed from the graph.
   Sim::shutdown();
}

extern bool gDGLRender;
bool gShuttingDown   = false;

/// Main loop of the game
int DemoGame::main(int argc, const char **argv)
{
//   if (argc == 1) {
//      static const char* argvFake[] = { "dtest.exe", "-jload", "test.jrn" };
//      argc = 3;                                                               
//      argv = argvFake;
//   }

//   Memory::enableLogging("testMem.log");
//   Memory::setBreakAlloc(104717);

   if(!initLibraries())
      return 0;

#ifdef IHVBUILD
   char* pVer = new char[sgVerStringLen + 1];
   U32 hi;
   for (hi = 0; hi < sgVerStringLen; hi++)
      pVer[hi] = sgVerString[hi] ^ 0xFF;
   pVer[hi] = '\0';

   SHA1Context hashCTX;
   hashCTX.init();
   hashCTX.hashBytes(pVer, sgVerStringLen);
   hashCTX.finalize();

   U8 hash[20];
   hashCTX.getHash(hash);

   for (hi = 0; hi < 20; hi++)
      if (U8(hash[hi]) != U8(sgHashVer[hi]))
         return 0;
#endif

   // Set up the command line args for the console scripts...
   Con::setIntVariable("Game::argc", argc);
   U32 i;
   for (i = 0; i < argc; i++)
      Con::setVariable(avar("Game::argv%d", i), argv[i]);
   if (initGame(argc, argv) == false)
   {
      Platform::AlertOK("Error", "Failed to initialize game, shutting down.");
      shutdownGame();
      shutdownLibraries();
      gShuttingDown = true;
      return 0;
   }

#ifdef IHVBUILD
   char* pPrint = new char[dStrlen(sgVerPrintString) + 1];
   for (U32 pi = 0; pi < dStrlen(sgVerPrintString); pi++)
      pPrint[pi] = sgVerPrintString[pi] ^ 0xff;
   pPrint[dStrlen(sgVerPrintString)] = '\0';

   Con::printf("");
   Con::errorf(ConsoleLogEntry::General, pPrint, pVer);
   delete [] pVer;
#endif

   while(Game->isRunning())
   {
      PROFILE_START(MainLoop);
      Game->journalProcess();
      Net::process();      // read in all events
      Platform::process(); // keys, etc.
      TelConsole->process();
      TelDebugger->process();
      TimeManager::process(); // guaranteed to produce an event
      PROFILE_END();
   }
   shutdownGame();
   shutdownLibraries();

   gShuttingDown = true;

#if 0
// tg: Argh! This OS version check should be part of Platform, not here...
//
   // check os
   OSVERSIONINFO osInfo;
   dMemset(&osInfo, 0, sizeof(OSVERSIONINFO));
   osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   // see if osversioninfoex fails
   if(!GetVersionEx((OSVERSIONINFO*)&osInfo))
   {
      osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if(!GetVersionEx((OSVERSIONINFO*)&osInfo))
         return 0;
   }

   // terminate the process if win95 only!
   if((osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&      // 95, 98, ME
      (osInfo.dwMajorVersion == 4) &&                             // 95, 98, ME, NT
      (osInfo.dwMinorVersion == 0))                               // 95
   {
      AssertWarn(0, "Forcing termination of app (Win95)!  Upgrade your OS now!");
      TerminateProcess(GetCurrentProcess(), 0xffffffff);
   }
#endif

   return 0;
}


static bool serverTick = false;


static F32 fpsRealStart;
static F32 fpsRealLast;
//static F32 fpsRealTotal;
static F32 fpsReal;
static F32 fpsVirtualStart;
static F32 fpsVirtualLast;
//static F32 fpsVirtualTotal;
static F32 fpsVirtual;
static F32 fpsFrames;
static F32 fpsNext;
static bool fpsInit = false;
const F32 UPDATE_INTERVAL = 0.25f;

//--------------------------------------

/// Resets the FPS variables
void fpsReset()
{
   fpsRealStart    = (F32)Platform::getRealMilliseconds()/1000.0f;      // Real-World Tick Count
   fpsVirtualStart = (F32)Platform::getVirtualMilliseconds()/1000.0f;   // Engine Tick Count (does not vary between frames)
   fpsNext         = fpsRealStart + UPDATE_INTERVAL;

//   fpsRealTotal= 0.0f;
   fpsRealLast = 0.0f;
   fpsReal     = 0.0f;
//   fpsVirtualTotal = 0.0f;
   fpsVirtualLast  = 0.0f;
   fpsVirtual      = 0.0f;
   fpsFrames = 0;
   fpsInit   = true;
}   

//--------------------------------------

/// Updates the FPS variables
void fpsUpdate()
{
   if (!fpsInit)
      fpsReset();

   const float alpha  = 0.07f;
   F32 realSeconds    = (F32)Platform::getRealMilliseconds()/1000.0f;
   F32 virtualSeconds = (F32)Platform::getVirtualMilliseconds()/1000.0f;

   fpsFrames++;
   if (fpsFrames > 1)
   {
      fpsReal    = fpsReal*(1.0-alpha) + (realSeconds-fpsRealLast)*alpha;     
      fpsVirtual = fpsVirtual*(1.0-alpha) + (virtualSeconds-fpsVirtualLast)*alpha;     
   }
//   fpsRealTotal    = fpsFrames/(realSeconds - fpsRealStart);
//   fpsVirtualTotal = fpsFrames/(virtualSeconds - fpsVirtualStart);

   fpsRealLast    = realSeconds;
   fpsVirtualLast = virtualSeconds;

   // update variables every few frames
   F32 update = fpsRealLast - fpsNext;
   if (update > 0.5f)
   {
//      Con::setVariable("fps::realTotal",    avar("%4.1f", fpsRealTotal));
//      Con::setVariable("fps::virtualTotal", avar("%4.1f", fpsVirtualTotal));
      Con::setVariable("fps::real",    avar("%4.1f", 1.0f/fpsReal));
      Con::setVariable("fps::virtual", avar("%4.1f", 1.0f/fpsVirtual));
      if (update > UPDATE_INTERVAL)
         fpsNext  = fpsRealLast + UPDATE_INTERVAL;
      else
         fpsNext += UPDATE_INTERVAL;
   }
}   


/// Process a mouse movement event, essentially pass to the canvas for handling
void DemoGame::processMouseMoveEvent(MouseMoveEvent * mEvent)
{
   if (Canvas) 
      Canvas->processMouseMoveEvent(mEvent);
}

/// Process an input event, pass to canvas for handling
void DemoGame::processInputEvent(InputEvent *event)
{
   PROFILE_START(ProcessInputEvent);
   if (!ActionMap::handleEventGlobal(event))
   {
      // Other input consumers here...
      if (!(Canvas && Canvas->processInputEvent(event)))
         ActionMap::handleEvent(event);
   }
   PROFILE_END();
}

/// Process a quit event
void DemoGame::processQuitEvent()
{
   setRunning(false);
}

/// Refresh the game window, ask the canvas to set all regions to dirty (need to be updated)
void DemoGame::refreshWindow()
{
   if(Canvas)
      Canvas->resetUpdateRegions();
}

/// Process a console event
void DemoGame::processConsoleEvent(ConsoleEvent *event)
{
   char *argv[2];
   argv[0] = "eval";
   argv[1] = event->data;
   Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(2, const_cast<const char**>(argv), false));
}

/// Process a time event and update all sub-processes
void DemoGame::processTimeEvent(TimeEvent *event)
{
   PROFILE_START(ProcessTimeEvent);
   U32 elapsedTime = event->elapsedTime;
   // cap the elapsed time to one second
   // if it's more than that we're probably in a bad catch-up situation

   if(elapsedTime > 1024)
      elapsedTime = 1024;
   
   U32 timeDelta;
   
   if(gTimeAdvance)
      timeDelta = gTimeAdvance;
   else
      timeDelta = (U32) (elapsedTime * gTimeScale);

   Platform::advanceTime(elapsedTime);
   bool tickPass = 0;
   PROFILE_START(ServerProcess);
   //tickPass = serverProcess(timeDelta);
   PROFILE_END();
   PROFILE_START(ServerNetProcess);
   // only send packets if a tick happened
   if(tickPass)
      GNet->processServer();
   PROFILE_END();

   PROFILE_START(SimAdvanceTime);
   Sim::advanceTime(timeDelta);
   PROFILE_END();

   PROFILE_START(ClientProcess);
   tickPass = clientProcess(timeDelta);
   PROFILE_END();
   PROFILE_START(ClientNetProcess);
   if(tickPass)
      GNet->processClient();
   PROFILE_END();

   if(Canvas && gDGLRender)
   {
      bool preRenderOnly = false;
      if(gFrameSkip && gFrameCount % gFrameSkip)
         preRenderOnly = true;

      PROFILE_START(RenderFrame);
      //ShapeBase::incRenderFrame();
      Canvas->renderFrame(preRenderOnly);
      PROFILE_END();
      gFrameCount++;
   }
   GNet->checkTimeouts();
   fpsUpdate();
   PROFILE_END();

   // Update the console time
   Con::setFloatVariable("Sim::Time",F32(Platform::getVirtualMilliseconds()) / 1000);
}

/// Re-activate the game from, say, a minimized state
void GameReactivate()
{
   if ( !Input::isEnabled() )
      Input::enable();

   if ( !Input::isActive() )
      Input::reactivate();

   gDGLRender = true;
   if ( Canvas )
      Canvas->resetUpdateRegions();
}

/// De-activate the game in responce to, say, a minimize event
void GameDeactivate( bool noRender )
{
   if ( Input::isActive() )
      Input::deactivate();

   if ( Input::isEnabled() )
      Input::disable();

   if ( noRender )
      gDGLRender = false;
}

/// Invalidate all the textures
void DemoGame::textureKill()
{
   TextureManager::makeZombie();
}

/// Reaquire all textures
void DemoGame::textureResurrect()
{
   TextureManager::resurrect();
}

/// Process recieved net-packets
void DemoGame::processPacketReceiveEvent(PacketReceiveEvent * prEvent)
{
   GNet->processPacketReceiveEvent(prEvent);
}
