//
// SICKHEAD GAMES PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement 
// or nondisclosure agreement with Sickhead Games.  It may not 
// be copied or disclosed except in accordance with the terms of that
// agreement.
//
//      Copyright (c) 2007 Sickhead Games, LLC
//      All Rights Reserved.
//
// Sickhead Games - Dallas, Texas 75238
// http://www.sickheadgames.com
//

// Start the console logger in a locked 
// mode and clear the junk at the top.
setLogMode(2);
cls();

// Dump all the functions, classes, and globals to the log.
dumpConsoleFunctions();
echo( "" );
dumpConsoleClasses();
//echo( "" );
//export( "*" ); // This crashes 1.4... why?
//echo( "" );
//RootGroup.save( "./rootgroup.log" );

function onExit()
{
   // This is just here to keep the thing
   // from bitching... curious that it does
   // not appear in the function dump.
}

// We're done!
quit();
