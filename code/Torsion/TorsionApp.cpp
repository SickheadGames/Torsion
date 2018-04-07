// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "TorsionApp.h"

#include "MainFrame.h"
#include "ScriptFrame.h"
#include "DocManager.h"
#include "Debugger.h"
#include "AutoComp.h"
#include "BuildVersion.h"
#include "Platform.h"
#include "UpdateChecker.h"
#include "StackDump.h"

#include <wx/image.h>
#include <wx/mimetype.h>
#include <wx/cmdline.h>
#include <wx/snglinst.h>
#include <wx/dde.h>
#include <wx/uri.h>
#include <wx/msw/registry.h>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

static MainFrame* ts_MainFrame = NULL;


class TorsionDDEConn : public wxDDEConnection
{
   public:
      virtual bool OnExecute(const wxString& topic, char* data, int size, wxIPCFormat format)
      {
         wxASSERT( ts_MainFrame );
         wxString file( data, size );

         // Look for a line number at the end of the file.
         long line = 0;
         if ( file.Find( ':', true ) > 1 )
         {
            file.AfterLast( ':' ).ToLong( &line );
            file = file.BeforeLast( ':' );
         }
         
         ts_MainFrame->OpenFile( file, line - 1 );
         return true;
      }
};

class TorsionDDEServer : public wxDDEServer
{
   public:
      virtual wxConnectionBase * OnAcceptConnection(const wxString& topic)
      {
         if ( topic.CmpNoCase( "open" ) == 0 )
            return new TorsionDDEConn;
         return NULL;
      }
};

IMPLEMENT_APP(TorsionApp)


BEGIN_EVENT_TABLE( TorsionApp, wxApp )
   EVT_KEY_DOWN( OnKeyDown )
END_EVENT_TABLE()


TorsionApp::TorsionApp()
   :  wxApp(),
      m_DocManager( NULL ),
      m_Prefs(),
      m_Debugger( NULL ),
      m_AutoCompManager( NULL ),
      m_InstChecker( NULL ),
      m_UserInstChecker( NULL ),
      m_DDEServer( NULL )
{
   // We create two... one for the installer to check
   // for running instances.  Another for each user.
   wxString instName( "Torsion.SickheadGames" );
   m_InstChecker = new wxSingleInstanceChecker( instName );
   m_UserInstChecker = new wxSingleInstanceChecker( wxString::Format( "%s|%s", instName.c_str(), wxGetUserId().c_str() ) );
}

TorsionApp::~TorsionApp()
{
   // Don't delete the installer instance checker till the very end!
   wxDELETE( m_InstChecker );
   wxDELETE( m_UserInstChecker );
}

bool TorsionApp::OnInit()
{
   // Use the crash handler!
   #ifndef __WXDEBUG__
      wxHandleFatalExceptions( true );
   #endif

   // We don't want error dialogs from the wxWindows runtime!
	delete wxLog::SetActiveTarget( new wxLogStderr() );

   SetAppName( "Torsion" );
   SetVendorName( "Sickhead Games, LLC" );

   // Initialize the application path which is used
   // in various places for access to data like prefs,
   // export scripts, help file, etc.
   wxFileName appPath( argv[0] );
   if ( !appPath.IsAbsolute() )
   {
      wxPathList pathList;
      pathList.AddEnvList( wxT( "PATH" ) );
      appPath.Assign( pathList.FindAbsoluteValidPath( argv[0] ) );
   }
   m_AppPath = appPath.GetFullPath();

   // Parse the command line.
   wxCmdLineParser cmdLine( argc, argv );
   cmdLine.AddSwitch( "exts", wxEmptyString, "Register the application file extensions.", wxCMD_LINE_PARAM_OPTIONAL );
   cmdLine.AddSwitch( "unexts", wxEmptyString, "Unregister the application file extensions.", wxCMD_LINE_PARAM_OPTIONAL );
   cmdLine.AddOption( "debug", wxEmptyString, "When a project file is specified it starts a debug session.", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR );
   cmdLine.AddParam( "Input file(s)", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE );
   cmdLine.Parse( false );

   // Load the preferences!
   wxFileName prefs( GetAppPath() );
   prefs.SetFullName( "preferences.xml" );
	m_Prefs.Load( prefs.GetFullPath() );

   // Are we registering or unregistering extensions?
   if ( cmdLine.Found( "exts" ) )
   {
      RegisterScriptExts();      
      return false;
   }
   else if ( cmdLine.Found( "unexts" ) )
   {
      UnregisterScriptExts( tsGetPrefs().GetScriptExtensions() );
      return false;
   }

   // Pick any files from the command line.
   wxArrayString Files;
   wxArrayString Projects;
   for ( int i=0; i < cmdLine.GetParamCount(); i++ ) 
   {
      wxFileName name( cmdLine.GetParam( i ) );
      name = name.GetLongPath();
      if ( name.GetExt().CmpNoCase( "torsion" ) == 0 )
         Projects.Add( name.GetFullPath() );
      else
         Files.Add( name.GetFullPath() );
   }

   // Get the debug config...
   wxString debugConfig;
   cmdLine.Found( "debug", &debugConfig );

   // If we have another running and we're opening a script file
   // then just send the file list to it.
   wxASSERT( m_UserInstChecker );
   if ( Files.GetCount() > 0 && m_UserInstChecker->IsAnotherRunning() ) 
   {
      wxDDEClient client;
      wxDDEConnection* conn = wxDynamicCast( client.MakeConnection( "", "Torsion", "open" ), wxDDEConnection );
      if ( conn ) 
      {
         for ( int i=0; i < Files.GetCount(); i++ )
            conn->Execute( Files[i] );

         conn->Disconnect();
         return false;

      } else {
         
         //wxMessageDialog dlg( NULL, "BOO!" );
         //dlg.ShowModal();         
      }
   }

   // Add the image handlers.
   wxImage::AddHandler( new wxXPMHandler );
   wxImage::AddHandler( new wxJPEGHandler );

   // Gotta do this first or we cannot
   // launch socket classes from threads.
   wxSocketBase::Initialize();

   // Init the debugger.
   m_Debugger = new Debugger();

   // TODO: This is broken in wxMSW
   // wxThread::SetConcurrency( 2 ); 

   // Startup the autocomplete system.
   m_AutoCompManager = new AutoCompManager();

   // Create the document manager.
   m_DocManager = new DocManager;
   m_Prefs.GetFileHistory( m_DocManager->GetFileHistory() );
   m_Prefs.GetProjectHistory( m_DocManager->GetProjectHistory() );
   
   // Get a good default size for the window.
   wxRect Position( tsGetPrefs().GetPosition() );
   if ( Position.IsEmpty() ) 
   {
      Position.SetLeft( wxScreenDC().GetSize().x * 0.10f );
      Position.SetTop( wxScreenDC().GetSize().y * 0.10f );
      Position.SetWidth( ( wxScreenDC().GetSize().x * 0.90f ) - Position.GetLeft() );
      Position.SetHeight( ( wxScreenDC().GetSize().y * 0.90f ) - Position.GetTop() );
   }

   // Create the main frame window.
   ts_MainFrame = new MainFrame;
   ts_MainFrame->Create( m_DocManager, NULL, GetAppName(),
      Position.GetPosition(), Position.GetSize() );

   #ifndef __WXMAC__
      ts_MainFrame->Show(true);
   #endif
   SetTopWindow( ts_MainFrame );

   // Restore the maximized state after show
   // else it won't restore to the right size.
   if ( tsGetPrefs().GetMaximized() )
      ts_MainFrame->Maximize( true );

   // Load the DDE server.
   m_DDEServer = new TorsionDDEServer;
   m_DDEServer->Create( "Torsion" );

   // Load the first project we got or the last
   // project if that is enabled and set.
   if ( !Projects.IsEmpty() )
      ts_MainFrame->OpenProject( Projects[0] );
   else if (   tsGetPrefs().GetLoadLastProject() && 
               !tsGetPrefs().GetLastProject().IsEmpty() )
      ts_MainFrame->OpenProject( tsGetPrefs().GetLastProject() );

   // Now load all the other files.
   for ( int i=0; i < Files.GetCount(); i++ )
   {
      wxString file( Files[i] );

      // Look for a line number at the end of the file.
      long line = 0;
      if ( file.Find( ':', true ) > 1 )
      {
         file.AfterLast( ':' ).ToLong( &line );
         file = file.BeforeLast( ':' );
      }

      ts_MainFrame->OpenFile( file, line - 1 );
   }

   // Do the update check now that the window is up.
   if ( tsGetPrefs().GetCheckForUpdates() )
      CheckForUpdate( false );

   // Are we starting a debug session?
   if (  !Projects.IsEmpty() && 
         !debugConfig.IsEmpty() && 
         ts_MainFrame->SetActiveConfig( debugConfig ) )
   {
      ts_MainFrame->OnDebugStart( wxCommandEvent() );
   }

   return true;
}

void TorsionApp::CleanUp()
{
   // We delete the DDE stuff at first exit
   // to protect against false opens.
   wxDELETE( m_UserInstChecker );
   wxDELETE( m_DDEServer );

   // Cleanup the doc manager.
   if ( m_DocManager )
   {
      m_Prefs.SetFileHistory( m_DocManager->GetFileHistory() );
      m_Prefs.SetProjectHistory( m_DocManager->GetProjectHistory() );
      wxDELETE( m_DocManager );
   }

   wxDELETE( m_Debugger );
   wxDELETE( m_AutoCompManager );

   // Save the app prefs.
   wxFileName prefs( GetAppPath() );
   prefs.SetFullName( "preferences.xml" );
   m_Prefs.SaveIfDirty( prefs.GetFullPath() );

   wxApp::CleanUp();
}

int TorsionApp::OnExit()
{
   // By this point the frame should be dead!
   ts_MainFrame = NULL;
   return 0;
}

bool TorsionApp::ProcessEvent( wxEvent& event )
{
   wxKeyEvent* keyEvent = wxDynamicCast( &event, wxKeyEvent );
   if (  keyEvent && 
         keyEvent->GetKeyCode() == WXK_CONTROL &&
         keyEvent->GetEventType() == wxEVT_KEY_UP &&
         ScriptFrame::GetFrame() ) {

      ScriptFrame::GetFrame()->m_Tabbing = false;
   }

   return wxApp::ProcessEvent( event );
}

bool TorsionApp::CheckForUpdate( bool noUpdateMsg )
{
   UpdateChecker checker;

   wxString version;
   if ( !checker.CheckAvailable( &version ) )
   {
      if ( noUpdateMsg )
      {
         wxString msg;
         if ( version.IsEmpty() )
            msg = "The update website failed to respond.";
         else
            msg = "You have the most current version.";

         wxMessageDialog dlg( ts_MainFrame, msg, "Torsion", wxOK | wxICON_INFORMATION );
         dlg.ShowModal();
      }

      return false;
   }

   wxMessageDialog dlg( ts_MainFrame, "There is a new version of Torsion available.  Do you want to download it now?", "Torsion", wxYES_NO | wxICON_INFORMATION );
   if ( dlg.ShowModal() == wxID_YES )
   {
      wxLaunchDefaultBrowser( "http://www.garagegames.com/myAccount/" );
      return false;
   }

   return true;
}

void TorsionApp::OnFatalException()
{
   wxString address;
   wxString data;
   StackDump dump;
   dump.Dump( &address, &data );

   if ( wxMessageBox( "Torsion has crashed!  Do you want to submit "
         "the crash report?", "Torsion", wxYES_NO | wxICON_ERROR ) == wxYES )
   {
      wxString cmd;
      cmd << "http://mantis.sickheadgames.com/bug_report_page.php?project_id=2&severity=6&";
      cmd << "summary=Crash Report - " << address << "&";
      cmd <<   "description=Place information on what triggered the crash here.  Do not "
               "remove any information in the additional information box below.  Steps "
               "to reproduce the crash are extremely helpful.&";
      cmd << "additional_info=" << data;

      wxURI uri;
      uri.Create( cmd );
      cmd = uri.BuildURI();
      wxLaunchDefaultBrowser( cmd );
   }
}

void TorsionApp::RegisterScriptExts()
{
   const wxArrayString& exts = m_Prefs.GetScriptExtensions();

   for ( int i=0; i < exts.GetCount(); i++ )
   {
      wxString extKey;
      extKey << "HKCR\\." << exts[i];
      wxRegKey key( extKey );

      if ( !key.Exists() )
      {
         // The extension does not exist... so we create 
         // it and own it.
         key.Create();
         key.SetValue( "", "TorsionTorqueScript" );
         key.SetValue( "Content Type", "text/plain" );
         key.SetValue( "PerceivedType", "text" );
         continue;
      }

      // Ok the key exists... check the default value.  If
      // it's empty we can take over this ext.  If it's already
      // set to TorsionTorqueScript refresh it.
      wxString value;
      if (  key.QueryValue( "", value ) && 
            ( value.IsEmpty() || value == "TorsionTorqueScript" ) )
      {
         key.SetValue( "", "TorsionTorqueScript" );
         key.SetValue( "Content Type", "text/plain" );
         key.SetValue( "PerceivedType", "text" );
         continue;
      }

      // Ok... someone is using this key, so all we can do
      // is add ourselves to the 'open with' list.
      extKey << "\\OpenWithList\\torsion.exe";
      wxRegKey openWithList( extKey );
      openWithList.Create();
   }
}

void TorsionApp::UnregisterScriptExts( const wxArrayString& exts )
{
   for ( int i=0; i < exts.GetCount(); i++ )
   {
      wxString extKey;
      extKey << "HKCR\\." << exts[i];
      wxRegKey key( extKey );
      if ( !key.Exists() )
         continue;

      // Check the default key for our value 'TorsionTorqueScript'.
      wxString value;
      key.QueryValue( "", value );
      if ( value == "TorsionTorqueScript" )
      {
         // We own this... delete stuff we put in it.
         key.DeleteValue( "" );
         key.DeleteValue( "Content Type" );
         key.DeleteValue( "PerceivedType" );
      }

      // Check the open with list for our key... and remove it.
      extKey << "\\OpenWithList\\torsion.exe";
      wxRegKey openWithList( extKey );
      if ( openWithList.Exists() )
         openWithList.DeleteSelf();

      // Delete it if its empty.
      if ( key.IsEmpty() )
         key.DeleteSelf();
   }
}

void TorsionApp::OnKeyDown( wxKeyEvent& event )
{
   // Here we're trying to catch a few accelerators 
   // manually so we don't need to add bogus menu items.
   int id = wxID_ANY;

   if ( event.GetKeyCode() == WXK_F3 )
   {
      if ( !event.ShiftDown() )
         id = tsID_FINDNEXT;
      else if ( event.ShiftDown() )
         id = tsID_FINDPREV;
   }

   // A special second method to force the member list
   // to be visible... just like Ctrl+J
   else if ( !event.ShiftDown() && event.ControlDown() && event.GetKeyCode() == WXK_SPACE )
      id = tsID_LIST_MEMBERS;

   if ( id != wxID_ANY )
      ts_MainFrame->ProcessCommand( id );
   else
      event.Skip();
}

/*
bool TorsionApp::OnExceptionInMainLoop()
{
   return true;
}
*/

MainFrame* tsGetMainFrame()
{
   return ts_MainFrame;
}