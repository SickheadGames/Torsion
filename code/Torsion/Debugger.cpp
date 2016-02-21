// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"

#include "Debugger.h"
#include "TorsionApp.h"
#include "MainFrame.h"
#include "ProjectDoc.h"
#include "ScriptDoc.h"
#include "Platform.h"
#include "WatchCtrl.h"
#include "OutputPanel.h"
#include "ConnectDlg.h"
#include "tsShellExecute.h"


#ifdef __WXMSW__
   #include <process.h>
#endif

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

Debugger::Debugger()
	:	wxEvtHandler(),
		m_Server( NULL ),
		m_CurrStackLevel( 0 ),
		m_AtBreakpoint( false ),
      m_Process( NULL )
{
   m_Process = new tsShellExecute( this, tsID_STOPPED_EVENT );
}

Debugger::~Debugger()
{
   wxASSERT( !IsRunning() );
   wxDELETE( m_Process );
}

BEGIN_EVENT_TABLE( Debugger, wxEvtHandler )
	EVT_END_PROCESS( tsID_STOPPED_EVENT, Debugger::OnStoppedEvent )
	EVT_SOCKET( tsID_SOCKET_EVENT, Debugger::OnSocketEvent )
   EVT_BREAKPOINT_EVENTS( Debugger::OnBreakpointEvent )
END_EVENT_TABLE()

const bool Debugger::IsRunning() const
{ 
   return m_Process->isRunning() || ( m_Server && m_Server->IsConnected() ); 
}

bool Debugger::Start( const wxString& configName, const wxString& cmd )
{
	wxASSERT( tsGetMainFrame() );
   wxASSERT( !IsRunning() );

   ProjectDoc* Project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( Project );

   const ProjectConfig* config = Project->GetConfig( configName );
   wxASSERT( config );

   // Grab the settings from the project.
   wxString Address  = Project->GetAddress();
   wxString Password = Project->GetPassword();
   int Port          = Project->GetPort();

   // Grab the exe and arguments for launching the executable.
	wxString exe = config->GetExe();
   wxString args = config->GetArgs();

   // If we don't have a command line act like we 
   // requested a remote connection.
   if ( exe.IsEmpty() )
      return Connect( Address, Port, Password, wxEmptyString );

   // Try to inject the debugger hook if we can.
	if ( config->InjectDebugger() && !Project->SetDebugHook( Port, Password ) ) 
   {
      // If we cannot inject the debug hook we have
      // to let the user do this himself.  So fall
      // back once again to a remote connection.  Warn
      // the user first so that they can maybe correct
      // the issue.
		wxMessageDialog dlg( NULL,
         "The entry script could not be updated to inject the debug hook "
         "because it was locked, read only, or didn't exist.  Either disable "
         "OneClick Debugging in this configuration or correct the problem "
         "with your entry script file.", "Error", wxOK | wxICON_ERROR );
		dlg.ShowModal();

      return false;
   }

   // Get the working the working directory.
	wxString workingDir = Project->GetWorkingDir();

   // Launch the executable!
   wxASSERT( !m_Process->isRunning() );
	if ( !m_Process->exec( "open", workingDir, exe, args, tsSW_SHOWNORMAL ) )
   {
		wxMessageDialog MessageBox( NULL, "Executable launch failed!", "Error", wxOK | wxICON_ERROR );
		MessageBox.ShowModal();

      if ( config->InjectDebugger() )
         Project->RemoveDebugHook();

      return false;
	}

   // Connect automaticly using the requested command.
   //
   // TODO: We need to rework the connection parameters.  Lets
   // do the following...
   //
   // 1. Only store the port and password as part of the project settings.
   // 2. 127.0.0.1/localhost is always used for one-click-debugging.
   // 3. Store the address/port/password for remote connections in app prefs.
   //
   return Connect( "127.0.0.1", Port, Password, cmd );
}

bool Debugger::Connect( const wxString& Address, int Port, const wxString& Password, const wxString& cmd )
{
	wxASSERT( tsGetMainFrame() );
   wxASSERT( tsGetMainFrame()->GetProjectDoc() );

	m_AtBreakpoint = false;

	// Connect to the running server.
	wxIPV4address address;
	address.Hostname( Address );
	address.Service( Port );
   wxASSERT( m_Server == NULL );
	m_Server = new wxSocketClient( wxSOCKET_NOWAIT );
	m_Server->Notify( false );

   // Launch the connection dialog which will do this 
   // in an idle loop so that we cancel as well.
   ConnectDlg dlg( tsGetMainFrame(), m_Server, &address, Password, !cmd.IsEmpty() );
   if ( dlg.ShowModal() != wxID_OK ) 
   {
		Stop();
      return false;
   }

   // Sometimes we can get an ok, but have been disconnected!
	if ( !m_Server || !m_Server->IsConnected() ) 
   {
		Stop();

		wxMessageDialog dlg( NULL, "Connection to executable failed!", "Error", wxOK );
		dlg.ShowModal();
		return false;
	}

   // Save the connection settings back to the project.
   ProjectDoc* Project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( Project );
   Project->SetAddress( address.IPAddress() );
   Project->SetPassword( dlg.GetPassword() );
   Project->SetPort( address.Service() );

   // Start up the debugger.
   m_InputBuffer.Clear();

   m_Server->SetNotify( wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG );
   m_Server->SetEventHandler( *this, tsID_SOCKET_EVENT );
	m_Server->Notify( true );

   // Send the password, the initial breakpoints, then a continue
   // to kickstart the "enhanced" debug server... the old server
   // will ignore this and just start up in a weird state.
	_SendLine( Project->GetPassword() );
	_WriteBreakpoints( tsGetMainFrame()->GetProjectDoc()->GetBreakpoints() );
   _SendLine( cmd.IsEmpty() ? "CONTINUE" : cmd );

	return IsRunning();
}

void Debugger::_SendLine( const wxString& Line )
{
	wxASSERT( m_Server );

	wxString Buffer;
	Buffer << Line << "\r\n";
	m_Server->Write( Buffer.GetData(), Buffer.Length() );
}

void Debugger::Stop( bool disconnect )
{  
	if ( !m_Process->isRunning() && !m_Server ) 
      return;

	if ( m_Process->isRunning() && !disconnect )
      m_Process->kill();
   else
      m_Process->detach();

   OnStoppedEvent( wxProcessEvent() );
}

void Debugger::OnStoppedEvent( wxProcessEvent& event )
{
   wxASSERT( !m_Process->isRunning() );

   // The process is dead... do we need to kill the server?
   if ( !m_Server )
      return;

   m_Server->Notify( false );
   m_Server->Discard();
   m_Server->Close();
   wxDELETE( m_Server );

	m_AtBreakpoint = false;
   m_UpdateVarStack.Clear();

	// Restore the entry script to it's original state.
	ProjectDoc* Project = tsGetMainFrame()->GetProjectDoc();
   if ( Project )
      Project->RemoveDebugHook();

   // Let the fram know we're done so it can clean
   // up toolbars, menus, etc.
   if ( tsGetMainFrame() )
		tsGetMainFrame()->OnDebuggerStop();

   // Clean up the call stack.
	_ClearCallStack();
}

void Debugger::Break()
{
	wxASSERT( !m_AtBreakpoint );

	// Write the command...
	_SendLine( "BRKNEXT" );
}

void Debugger::Continue( const wxString& Command )
{
	wxASSERT( m_AtBreakpoint );
	
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( project );

   // Check for changed script files and ask about reloading them.
   ScriptDocArray scripts;
   if (  project->ExecModifiedScripts() &&
         tsGetMainFrame()->GetChangedScripts( scripts ) > 0 )
   {
      // Do we have any scripts that have the reload modified flag?
      bool reload = false;
      for ( int i=0; i < scripts.GetCount(); i++ )
      {
         if ( scripts[i]->IsReloadModified() )
         {
            reload = true;
            break;
         }
      }

      // Do we need to ask about reloading?
      if ( reload )
      {
         wxMessageDialog dlg( NULL, "There are changes to your script files.  Do you wish to reload them now?", "Torsion", wxYES_NO | wxICON_QUESTION );
         reload = dlg.ShowModal() == wxID_YES;
      }

      // Ok... did the user want to reload the changes?
      if ( reload )
      {
         // Save one at a time...
         wxArrayString files;
         for ( int i=0; i < scripts.GetCount(); i++ )
         {
            if ( !scripts[i]->Save() )
               continue;

            files.Add( scripts[i]->GetFilename() );
         }

         if ( files.GetCount() == scripts.GetCount() )
            ReloadScripts( files );
      }
      else
      {
         // Clear the reload modified flag which keeps us from
         // asking the user to reload until they make another 
         // change.
         for ( int i=0; i < scripts.GetCount(); i++ )
            scripts[i]->ClearReloadModified();
      }
   }

   // Write the command...
	_SendLine( Command );
}

void Debugger::SetStackLevel( int Level )
{
	wxASSERT( Level >= 0 );
	wxASSERT( Level < m_CallStack.GetCount() );

	if ( !IsAtBreakpoint() ) {
		return;	
	}

	m_CurrStackLevel = Level;

	// TODO: Need to request locals here!

   // Read the new locals.
	_ReadLocalsData( m_CallStack[ m_CurrStackLevel ] );

	if ( tsGetMainFrame() ) {
		tsGetMainFrame()->SetCallStack( m_CallStack, m_CurrStackLevel );
	}
}

void Debugger::OnSocketEvent( wxSocketEvent& Event )
{
   // We can sometimes get events that were cued up
   // right before the server is deleted... ignore them.
	if ( !m_Server )
      return;

   // If we get disconnected... this happens during
   // normal Torque shutdown as well when the telnet
   // debugger service is stopped.
	if ( Event.GetSocketEvent() == wxSOCKET_LOST || m_Server->Error() ) 
   {
		Stop( true );
		return;
	}

	wxASSERT( Event.GetSocketEvent() == wxSOCKET_INPUT );
	wxASSERT( !m_Server->Error() );

	// Ok the server is sending us a command... look for the first line.
	char data[MaxCommandSize];
    m_Server->Read( data, MaxCommandSize );
    m_InputBuffer.Append( data, m_Server->LastCount() );
    
	wxString line;
	int pos;
	while ( ( pos = m_InputBuffer.Find( "\r\n" ) ) != -1 ) {

        line = m_InputBuffer.Left( pos );
		m_InputBuffer = m_InputBuffer.Mid( pos + 2 );

		_OnServerMessage( line );
    }
}

void Debugger::EvalCallTip( const wxString& exp )
{
   // Here we build a special TorqueScript command to
   // evaluate the expression exposing types when we can.
   
   // TODO: So i added this neat featue that automatically detects objects
   // and gives you their class and name.  The problem is that any number in
   // any variable returns 'true' to isObject().  So it was distracting
   // until we can come up with a better way to tell the difference we'll have
   // to put this feature to rest.
   /*
   wxString eval = 
      wxString::Format( "isObject( %s ) ? \"%s = \" @ %s @ \" { object \" @ %s.getClassname() @ \"( \" @ %s.getName() @ \" ) }\" : \"%s = \\\"\" @ %s @ \"\\\"\"", 
      exp, exp, exp, exp, exp, exp, exp );

   //wxString eval = wxString::Format( "\"%s = \" @ ( %s )", exp, exp );
   */

   // TODO: Just go back to passing the expression thru unchanged
   // and store the expression here as a temporary to be passed in 
   // EVALOUT bellow.
   m_LastCallTip = exp;
   Eval( m_LastCallTip, 2 );
}

void Debugger::Eval( const wxString& expression, int tag )
{
	if ( !m_Server || !m_Server->IsConnected() )
		return;

	wxString Command;
	if ( IsAtBreakpoint() )
   {
      // TODO: We tried to enable the eval edit box when at a breakpoint but
      // the implementation of EVAL in Torque, my fault by the way, assumes
      // that all eval strings return a value.  It is implemented like so...
      //
      // dSprintf(buffer, MaxCommandSize, "return %s;", evalBuffer);
      // 
      // This causes script errors when trying to eval functions like echo()
      // which do not return a result.  If it wasn't for this wrinkle we could
      // safely allow the user to eval expressions when at a breakpoint as it
      // doesn't effect the debugger as i first thought it would.
      //
      // The following is a hack i tested, but it requires that we properly
      // parse embedded quotes...
      //
      // wxString exp;
      // exp << "eval( \"" << expression << "\" )";
      //
      // The right thing would be to fix EVAL in the TelenetDebugger to assume
      // that the expression is a complete statement and that return isn't needed.
      //
		Command << "EVAL " << tag << " " << m_CurrStackLevel << " " << expression;
   }
	else
		Command << "CEVAL " << expression;

	_SendLine( Command );
}

void Debugger::_OnServerMessage( const wxString& line )
{
	#define ISCOMMAND( CMD ) strnicmp( line.c_str(), CMD, sizeof( CMD ) - 1 ) == 0

	// What command did we get?
	if ( ISCOMMAND( "PASS Connected" ) ) 
   {
		// We're succesfully connected!
		return;
	} 
   else if ( ISCOMMAND( "PASS" ) ) 
   {
      wxMessageDialog dlg( NULL, "Connection to debug server failed: Invalid password!", "Error", wxOK );
		dlg.ShowModal();

		// Wrong password ... stop!
		Stop( true );
	} 
   else if ( ISCOMMAND( "COUT" ) ) 
   {
      wxString temp( line.c_str() + 5 );
      EscapeTorqueML( temp );

      // If the update var stack isn't empty then we must be
      // doing variable updates.  Since things are processed
      // in order we can assume any COUT at this time is owned
      // by the top variable in the update stack.  This is used
      // to redirect errors to 
      if ( m_UpdateVarStack.GetCount() > 0 )
      {
         WatchVar* var = m_UpdateVarStack[0];
         if ( !var->GetValue().IsEmpty() )
            temp = var->GetValue() + "\r\n" + temp;
         var->SetValue( temp );
      }
      else
      {
         temp.Append( "\r\n" );
		   tsGetMainFrame()->GetOutputPanel()->AppendText( temp );
      }
	} 
   else if ( ISCOMMAND( "FILELISTOUT" ) ) 
   {
		wxASSERT( false );
	} 
   else if ( ISCOMMAND( "BREAKLISTOUT" ) ) 
   {
		wxASSERT( false );
	} 
   else if ( ISCOMMAND( "BREAK" ) ) 
   {
      // The latest TelnetDebugger update now properly sleeps
      // when in a breakpoint...  but we still should protect
      // against weird behavior.  Hopefully this won't screw
      // up someones app.
      m_Process->setPriorityClass( tsPRIORITY_CLASS_IDLE );

      // Force ourselves to the forground.
      tsRaiseWindow( tsGetMainFrame() );

      _ReadBreakpointData( line.Mid( 6 ) );
	} 
   else if ( ISCOMMAND( "BRKMOV" ) ) 
   {
		wxString data = line.Mid( 7 );
      wxString file = data.BeforeFirst( ' ' );
		data = data.AfterFirst( ' ' );
      int line = atoi( data.BeforeFirst( ' ' ) );
      int newline = atoi( data.AfterFirst( ' ' ) );

   	ProjectDoc* Project = tsGetMainFrame()->GetProjectDoc();
      wxASSERT( Project );

      Breakpoint* bp = Project->FindBreakpoint( file, line );
      if ( bp )
         Project->MoveBreakpoint( bp, newline );
   } 
   else if ( ISCOMMAND( "BRKCLR" ) ) 
   {
		wxString data = line.Mid( 7 );
      wxString file = data.BeforeFirst( ' ' );
      int line = atoi( data.AfterFirst( ' ' ) );

   	ProjectDoc* Project = tsGetMainFrame()->GetProjectDoc();
      wxASSERT( Project );

      Breakpoint* bp = Project->FindBreakpoint( file, line );
      if ( bp )
         Project->DeleteBreakpoint( bp );
   } 
   else if ( ISCOMMAND( "RUNNING" ) ) 
   {
		// Disable the breakpoint.
		m_AtBreakpoint = false;
		if ( tsGetMainFrame() )
			tsGetMainFrame()->ClearBreaklines();

		// Move the process back to the top!
      m_Process->setPriorityClass( tsPRIORITY_CLASS_NORMAL );
		tsRaiseWindowFromProcessId( m_Process->getPid() );
	} 
   else if ( ISCOMMAND( "EVALOUT" ) ) 
   {
		wxString data = line.Mid( 8 );
		int tag = atoi( data.BeforeFirst( ' ' ) );
		wxString result = data.AfterFirst( ' ' );
      EscapeTorqueML( result );

      if ( !tsGetMainFrame() )
         return;

      // Zero tags just get sent back to the output.
      if ( tag == 0 ) 
      {
         wxString temp;
         temp << "EVALOUT ==> " << result << "\r\n";
         tsGetMainFrame()->GetOutputPanel()->AppendText( temp );
      } 
      else if ( tag == 1 ) 
      {
         // These are dummy tags we use when we don't want
         // the result of an eval to appear anywhere.
      } 
      else if ( tag == 2 ) 
      {
         tsGetMainFrame()->OnDebugCallTip( m_LastCallTip, result );
      } 
      else 
      {
         // Since things are evaluated in order we should find
         // this var at the top of the stack.
         WatchVar* Var = m_UpdateVarStack[0];
         wxASSERT( Var->GetTag() == tag );

         if ( Var && Var->GetValue().IsEmpty() )
            Var->SetValue( result );

         m_UpdateVarStack.RemoveAt( 0 );

         tsGetMainFrame()->GetWatchWindow()->Refresh();
		}
   } 
   else if ( ISCOMMAND( "DBGERR" ) ) 
   {
		// The debugger told us we we're bad!
		// Not a huge deal... as some functions
		// could be "unsupported".
      // 
      // TODO: Being able to color the first pane
      // red like VC++ would be nice.
      //
      tsBellEx( wxICON_ERROR );
      tsGetMainFrame()->SetStatusText( line, 0 );
	} 
   else 
   {
      // The debug server fucked up and sent 
		// us some crap.  Ignore it and assume
      // we can recover.
	}

	#undef ISCOMMAND
}

void Debugger::_WriteBreakpoints( const BreakpointArray& Breakpoints )
{
	for ( int i = 0; i < Breakpoints.GetCount(); i++ ) {

		Breakpoint* TheBreakpoint = Breakpoints[ i ];
		wxASSERT( TheBreakpoint );

		if ( !TheBreakpoint->GetEnabled() ) {
			continue;
		}

      _WriteBreakpoint( TheBreakpoint );
	}
}

void Debugger::_WriteBreakpoint( const Breakpoint* TheBreakpoint )
{
   wxASSERT( IsRunning() );
	wxASSERT( TheBreakpoint );

   wxString File = TheBreakpoint->GetFile();
	File.Replace( "\\", "/" );

	const wxString& Expression = TheBreakpoint->GetCondition();

	wxString Data;

   if ( TheBreakpoint->GetEnabled() ) {

	   Data.Printf( "BRKSET %s %d %s %d %s", 
		   File, 
		   TheBreakpoint->GetLine(),
		   "false",
		   TheBreakpoint->GetPass(),
		   !Expression.IsEmpty() ? Expression : "true" );

   } else {

      Data.Printf( "BRKCLR %s %ld",
	      File, 
	      TheBreakpoint->GetLine() );
   }

   _SendLine( Data );
}

void Debugger::OnBreakpointEvent( BreakpointEvent& event )
{
   // If we're not running we don't care about 
   // breakpoint changes.
   if ( !IsRunning() )
      return;

   wxASSERT( event.GetBreakpoint() );

   if ( event.GetType() == BreakpointEvent::TYPE_MOVED ) {

      // Remove the old line first.
      Breakpoint deleted( *event.GetBreakpoint() );
      deleted.SetEnabled( false );
      deleted.SetLine( event.GetOldLine() );
      _WriteBreakpoint( &deleted );

   } else if ( event.GetType() == BreakpointEvent::TYPE_REMOVE ) {

      Breakpoint deleted( *event.GetBreakpoint() );
      deleted.SetEnabled( false );
      _WriteBreakpoint( &deleted );

      return;
   }

   _WriteBreakpoint( event.GetBreakpoint() );
}

void Debugger::_ClearCallStack()
{
	for ( int i=0; i < m_CallStack.GetCount(); i++ ) {
		delete m_CallStack[i];
	}
	m_CallStack.Clear();
	m_CurrStackLevel = 0;
}

void Debugger::_ReadLocalsData( FunctionCall* Call )
{
	/*
	wxASSERT( Call );

	// Read the new locals...
	WatchVarArray NewLocals;
	int Count;
	m_Server->Read( &Count, sizeof( Count ) );
	for ( int i=0; i < Count; i++ ) {

		wxString Type, Name, Value;
		int Flags;
		_ReadVar( Type, Flags, Name, Value );

		NewLocals.Add( new WatchVar( NULL, Call->GetStackIndex(), Type, Flags, Name, Value ) );
	}

	// Apply the new locals...
	Call->SetLocals( NewLocals );

	// Update the call's locals that need it.
	const WatchVarArray& Locals = Call->GetLocals();
	for ( i=0; i < Locals.GetCount(); i++ ) {
        
		if ( Locals[i]->IsExpanded() ) {
            UpdateExpanded( Locals[i] );
		}
	}
	*/
}

void Debugger::_ReadBreakpointData( wxString callstackString )
{
	//wxASSERT( !m_AtBreakpoint );

	// Store the old callstack so we can
	// pluck out the local vars later.
	FunctionCallArray OldCallStack = m_CallStack;

	// Read the call stack.
	m_CallStack.Clear();
	for ( int l=0; !callstackString.IsEmpty(); l++ ) {

		// This should be formatted like so:
		//
		//	<file path> <line number> <function name>
		//

		wxString File = callstackString.BeforeFirst( ' ' );
		callstackString = callstackString.AfterFirst( ' ' );

		int Line = atoi( callstackString.BeforeFirst( ' ' ) );
		callstackString = callstackString.AfterFirst( ' ' );

		wxString Function = callstackString.BeforeFirst( ' ' );
		callstackString = callstackString.AfterFirst( ' ' );

      if ( Function == "<none>" ) 
         Function = "";

		m_CallStack.Add( new FunctionCall( l, File, Line, Function ) );
	}

   /*
	// Pluck the locals out of the old callstack.
	l = m_CallStack.GetCount() - 1;
	for ( int o = OldCallStack.GetCount()-1; o >= 0; o--, l-- ) {

		wxASSERT( OldCallStack[o]->GetStackIndex() == o );

		if ( l < 0 ) {
			break;
		}

		wxASSERT( m_CallStack[l]->GetStackIndex() == l );

		if ( !m_CallStack[l]->IsSameFunction( *OldCallStack[o] ) ) {

			// If this doesn't match then the
			// rest of the stack is potentially
			// different.
			break;
		}

		// These are the same calls... so take it's locals!
        m_CallStack[l]->SwapLocals( *OldCallStack[o] );

		// Delete and null it.
		delete OldCallStack[o];
		OldCallStack[o] = NULL;
	}
	*/

	// Delete anything remaining in the old callstack.
	for ( int l = 0; l < OldCallStack.GetCount(); ++l ) {
		delete OldCallStack[l];
	}

	//wxASSERT( m_CallStack.GetCount() == Levels );

	// Read the locals and update the expanded items.
	m_CurrStackLevel = 0;
	//_ReadLocalsData( m_CallStack[ m_CurrStackLevel ] );

	m_AtBreakpoint = true;

	// Set the call stack which will also set the break location!
	if ( tsGetMainFrame() ) {
        tsGetMainFrame()->SetCallStack( m_CallStack, m_CurrStackLevel );
	}
}

void Debugger::UpdateExpanded( WatchVar* Var )
{
	wxASSERT( Var );
	wxASSERT( Var->IsExpanded() );
	wxASSERT( IsAtBreakpoint() );

	// Buid a name.
	wxString Name = Var->GetName();
	WatchVar* Parent = Var->GetParent();
	while ( Parent ) {

		wxASSERT( Parent->IsExpanded() );
		wxASSERT( Parent->IsExpandable() );

		Name = Parent->GetName() + "." + Name;
		Parent = Parent->GetParent();
	}

   // Evaluate this var.
	wxString Eval;
	Eval.Printf( "EVAL %d %d %s", 
		Var->GetTag(),
		m_CurrStackLevel, 
		Name.c_str() );

   _SendLine( Eval );

	/*
	// Read the new expanded vars.
	WatchVarArray NewVars;
	int Count;
	m_Server->Read( &Count, sizeof( Count ) );
	for ( int i=0; i < Count; i++ ) {

		wxString Type, Name, Value;
		int Flags;
		_ReadVar( Type, Flags, Name, Value );

		NewVars.Add( new WatchVar( NULL, Var->GetStackIndex(), Type, Flags, Name, Value ) );
	}
	*/

	// Add them to the var.
	//Var->SetVars( NewVars );

	/*
   // Now recurse thru any expanded children.
	const WatchVarArray& Vars = Var->GetVars();
	for ( int i=0; i < Vars.GetCount(); i++ ) {

		if ( Vars[i]->IsExpanded() ) {
			UpdateExpanded( Vars[i] ); 
		}
	}
   */
}

void Debugger::ChangeVar( WatchVar* Var, const wxString& Value )
{
	wxASSERT( Var );
	wxASSERT( IsAtBreakpoint() );

	// Buid a name.
	wxString Name = Var->GetName();
	WatchVar* Parent = Var->GetParent();
	while ( Parent ) {

		wxASSERT( Parent->IsExpanded() );
		wxASSERT( Parent->IsExpandable() );

		Name = Parent->GetName() + "." + Name;
		Parent = Parent->GetParent();
	}

   // Fixup... if the value is non-numeric then we
   // need to assume it's a string value and not
   // a weird function call.
   wxString Fixup = Value;
   if ( !Value.IsNumber() ) {
      Fixup.Prepend( "\"" );
      Fixup.Append( "\"" );
   }

   // Send one eval to change the var.
	wxString Eval;
   Eval.Printf( "EVAL 1 %d %s=%s", 
		m_CurrStackLevel, 
		Name.c_str(),
      Fixup.c_str() );
   _SendLine( Eval );

   // Now send another to get the new value and
   // update the watch window.
   Eval.Printf( "EVAL %d %d %s", 
		Var->GetTag(),
		m_CurrStackLevel, 
		Name.c_str() );

   // Add the var to the bottom of the stack.  Since
   // the results should return in order we should get
   // them back top to bottom in EVALOUT.
   m_UpdateVarStack.Add( Var );
   Var->SetValue( wxEmptyString );

   _SendLine( Eval );
}

void Debugger::UpdateVar( WatchVar* Var )
{
	wxASSERT( Var );
	wxASSERT( IsAtBreakpoint() );

	// Buid a name.
	wxString Name = Var->GetName();
	WatchVar* Parent = Var->GetParent();
	while ( Parent ) {

		wxASSERT( Parent->IsExpanded() );
		wxASSERT( Parent->IsExpandable() );

		Name = Parent->GetName() + "." + Name;
		Parent = Parent->GetParent();
	}

   // Evaluate this var.
	wxString Eval;
	Eval.Printf( "EVAL %d %d %s", 
		Var->GetTag(),
		m_CurrStackLevel, 
		Name.c_str() );

   // Add the var to the bottom of the stack.  Since
   // the results should return in order we should get
   // them back top to bottom in EVALOUT.
   m_UpdateVarStack.Add( Var );
   Var->SetValue( wxEmptyString );

   // Send off the request... response will come
   // later and we'll use the tag to find the var.
   _SendLine( Eval );
}

bool Debugger::GetBreakpointFileAndLine( wxString& file, int& line ) const
{
   if ( !IsAtBreakpoint() )
      return false;

	wxASSERT( m_CurrStackLevel >= 0 );
	wxASSERT( m_CurrStackLevel < m_CallStack.GetCount() );
	const FunctionCall* fcall = m_CallStack[ m_CurrStackLevel ];
   wxASSERT( fcall );

   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( project );

   file = project->MakeAbsoluteTo( fcall->GetFile() );
   line = fcall->GetLine();
   return true;
}

void Debugger::MoveInstPtr( const wxString& file, int line )
{
   wxASSERT( IsRunning() );
   wxASSERT( IsAtBreakpoint() );
  
   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( project );

   wxString fixed = project->MakeReleativeTo( file );
   fixed.Replace( "\\", "/" );

   wxString cmd;
   cmd << "MOVEIP " << fixed << " " << line;
   _SendLine( cmd );
}

void Debugger::ReloadScripts( const wxArrayString& scripts )
{
   wxASSERT( IsRunning() );

   wxASSERT( tsGetMainFrame() );
   ProjectDoc* project = tsGetMainFrame()->GetProjectDoc();
   wxASSERT( project );

   // First setup the reload command for all these scripts.
   for ( int i=0; i < scripts.GetCount(); i++ )
   {
      wxString fixed = project->MakeReleativeTo( scripts[i] );
      fixed.Replace( "\\", "/" );

      wxString cmd;
      cmd << "RELOAD " << fixed;
      _SendLine( cmd );
   }

   /*
   // Now yeild until RELOAD success/failure messages returns.
   m_Errors = 0;
   m_Reloads = 0;
   m_ReloadsFailed = 0;
   while( ; scriptCount == scripts.GetCount(); )
   {
      wxSafeYield( tsGetMainFrame(), false );
      
   }
   */
}

void Debugger::EscapeTorqueML( wxString& text )
{
   // TorqueML uses the characters 0-31 for special
   // markup hints.  Other than CR, LF, and TAB replace
   // the rest with the correct /cXX string.
   const wxUChar* chr = text.c_str();
   wxString escape;
   size_t insert;
   for ( ; *chr; )
   {
      if ( *chr > 17 ||
           *chr == '\n' ||
           *chr == '\r' ||
           *chr == '\t' )
      {
         chr++;
         continue;
      }

      // We got something to escape.
      escape.Empty();

      if ( *chr == 15 )
         escape << "/cr";
      else if ( *chr == 16 )
         escape << "/cp";
      else if ( *chr == 17 )
         escape << "/co";
      else
         escape << "/c" << ((int)*chr)-1;

      insert = chr - text.c_str();
      text.replace( insert, 1, escape );
      chr = (wxUChar*)(text.wc_str() + insert + escape.Len());
   }
}

