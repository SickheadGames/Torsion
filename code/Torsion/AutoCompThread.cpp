// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AutoCompThread.h"

#include "TorsionApp.h"
#include "AutoComp.h"
#include "AutoCompExports.h"
#include "AutoCompManager.h"
#include "ScriptScanner.h"

#include <wx/dir.h>
#include <stack>

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


AutoCompThread::AutoCompThread() 
   :  wxThread( wxTHREAD_JOINABLE ),
      m_Pages( CmpNameNoCase ),
      m_Exports( NULL ),
      m_NewExports( NULL ),
      m_UpdateExports( false ),
      m_Data( NULL )
{
   m_Excluded.Add( ".svn" );
   m_Excluded.Add( "cvs" );
}

AutoCompThread::~AutoCompThread()
{
   Clear();
   wxDELETE( m_Exports );
   wxDELETE( m_NewExports ); 
   WX_CLEAR_HASH_MAP( AutoCompTextMap, m_ActiveMap );
}

void AutoCompThread::SetPath( const wxString& path, const wxArrayString& mods, const wxArrayString& exts )
{
   wxCriticalSectionLocker lock( m_DataLock );
   m_ProjectPath = path;
   m_Mods = mods;
   m_ScriptExts = exts;
}

void AutoCompThread::SetExports( AutoCompExports* exports )
{
   wxCriticalSectionLocker lock( m_DataLock );

   if ( m_Exports != exports ) 
   {
      wxDELETE( m_NewExports );
      m_NewExports = exports;
      m_UpdateExports = true;
   }
}

AutoCompText* AutoCompThread::AddActivePage( const wxString& path )
{
   // TODO: There is a potential issue here... with
   // data changed on the disk when the non-saved data
   // has not been changed (the user didn't allow the
   // updated).  We need some sort of signal from the
   // client side that tells us it's ok to accept/reject
   // the change of the file on disk.

   wxCriticalSectionLocker lock( m_DataLock );

   AutoCompTextMap::iterator iter = m_ActiveMap.find( path );
   if ( iter == m_ActiveMap.end() || !iter->second ) 
   {
      AutoCompText* text = new AutoCompText;
      m_ActiveMap[ path ] = text;
      return text;
   }

   return iter->second;
}

void AutoCompThread::RemoveActivePage( AutoCompText* text )
{
   wxCriticalSectionLocker lock( m_DataLock );

   AutoCompTextMap::iterator iter = m_ActiveMap.begin();
   for ( ; iter != m_ActiveMap.end(); iter++ )
   {
      if ( iter->second == text )
      {
         delete iter->second;
         iter->second = NULL;
         break;
      }
   }
}

void AutoCompThread::Clear()
{
   for ( int i=0; i < m_Pages.GetCount(); i++ )
      delete m_Pages[i];
   m_Pages.Clear();

   wxDELETE( m_Data );
}

bool AutoCompThread::ScanPath( const wxString& path, 
   const wxString& basePath, 
   const wxArrayString& mods,
   const wxArrayString& scriptExts )
{
   // If we have specific mods to process.
   if ( mods.GetCount() > 0 )
   {
      wxFileName modDir;
      modDir.AssignDir( path );
      modDir.MakeRelativeTo( basePath );

      // If we're under a mod folder...
      if ( modDir.GetDirCount() > 0 )
      {
         wxString mod = modDir.GetDirs()[0];
         if ( mods.Index( mod, wxFileName::IsCaseSensitive() ) == wxNOT_FOUND )
            return false;
      }
   }

   wxDir dir( path );
   if ( !dir.IsOpened() )
      return false;

   bool Rebuild = false;

   // TODO: We need to add a method here to delete pages
   // that no longer exist on disk after this scan.  At 
   // the moment you can delete a script from a project
   // and it's code completion data will remain loaded.

   // The name of this dir with path delimiter at the end
   wxString fullpath;
   wxString prefix = dir.GetName();
   prefix += wxFILE_SEP_PATH;

   // First process the files in this folder... do one 
   // pass per filespec.
   for ( int i=0; i < scriptExts.GetCount(); i++ ) 
   {
      wxString spec;
      spec << "*." << scriptExts[i];

      wxString filename;
      wxDateTime lastUpdate;
      bool cont = dir.GetFirst( &filename, spec, wxDIR_FILES );
      for ( ; cont; cont = dir.GetNext( &filename ) ) 
      {
         if ( TestDestroy() )
            return Rebuild;

         // What was our last update time?
         fullpath = prefix + filename;
         lastUpdate = wxFileModificationTime( fullpath );

         // Skip if the file hasn't changed.
         AutoCompPage* found = AutoCompPage::Find( fullpath, m_Pages );
         if ( found && found->GetLastUpdate() == lastUpdate )
            continue;

         // Scan the changed or new file.
         if ( m_Scanner.Open( fullpath ) )
         {
            AutoCompPage* page = ScanFile( fullpath, m_Scanner );
            if ( page )
            {
               page->SetLastUpdate( lastUpdate );
               AddPage( page );
               Rebuild = true;
            }
         }

         if ( TestDestroy() )
            return Rebuild;

      } // for ( ; cont; cont = dir.GetNext( &filename ) ) 

   } // for ( int i=0; i < scriptExts.GetCount(); i++ )

   // Now scan the subdirs.
   wxString dirname, fulldir;
   bool cont = dir.GetFirst(&dirname, wxEmptyString, wxDIR_DIRS);
   for ( ; cont; cont = dir.GetNext(&dirname) )
   {
      if ( TestDestroy() )
         return Rebuild;

      if ( m_Excluded.Index( dirname, wxFileName::IsCaseSensitive() ) != wxNOT_FOUND )
         continue;

      fulldir = prefix + dirname;
      Rebuild |= ScanPath( fulldir, basePath, mods, scriptExts );
   }

   return Rebuild;
}

AutoCompPage* AutoCompThread::ScanFile( const wxString& path, ScriptScanner& sc )
{
   // We're storing the data to a new page.
   AutoCompPage* page = new AutoCompPage( path );
   AutoCompFunction* function = NULL;
   std::stack<AutoCompClass*> objects;

   int bracket = 0;
   int flevel;
   int startLine;

   wxString Comment;

   while ( sc.Step( false ) != SSTOKEN_EOF ) 
   {
      // We hate gotos normally, but this is needed to 
      // skip doing the step in some cases.
      SKIP_STEP:

      /*
      if ( TestDestroy() ) {
         delete page;
         return NULL;
      }
      */

      // Grab the value for processing below.
      if ( sc.GetToken() == SSTOKEN_COMMENT )
      {
         if (  sc.GetValue().StartsWith( "///" ) ||
               sc.GetValue().StartsWith( "//!" ) ||
               sc.GetValue().StartsWith( "/*!" ) ||
               sc.GetValue().StartsWith( "/**" ) ) 
         {
            // Add it, but strip the comment marks.
            Comment << sc.GetValue().Mid( 3 );
            int end = Comment.Find( "*/" );
            if ( end != -1 )
            {
               Comment = Comment.Mid( 0, end );
               Comment.Trim();
            }
         } 
         else
            Comment.Empty();
      }

      else if (   sc.GetToken() == SSTOKEN_LOCAL ||
                  sc.GetToken() == SSTOKEN_GLOBAL )
      {
         AutoCompVar* var = NULL;

         // If its a global and it ends in a colon then skip 
         // it... we don't want partial vars in the database.
         if (  sc.GetToken() == SSTOKEN_GLOBAL &&
               sc.GetValue().Last() == ':' )
         {
            Comment.Empty();
            continue;
         }

         // If it's a global or a local outside of a 
         // function add it to the page.
         if ( sc.GetToken() == SSTOKEN_GLOBAL || !function )
         {
            var = page->AddVar( sc.GetValue() );
            var->AddLine( path, sc.GetLine() );
            if ( !Comment.IsEmpty() )
               var->SetDesc( Comment );
         }

         // If we're in a function add it there as well
         // so we can provide it in the autowatch window.
         if ( function )
         {
            var = function->AddVar( sc.GetValue() );
            var->AddLine( path, sc.GetLine() );
            if ( !Comment.IsEmpty() )
               var->SetDesc( Comment );
         }

         // Clear the comment if we got a var.
         if ( var )
            Comment.Empty();
      }

      else if (   objects.size() > 0 && 
                  (  sc.GetToken() == SSTOKEN_RESERVED || 
                     sc.GetToken() == SSTOKEN_WORD ) &&
                  ( sc.GetValue() != "new" && sc.GetValue() != "singleton" ) ) 
      {
            wxASSERT( objects.top() );
            objects.top()->AddVar( sc.GetValue() )->AddLine( path, sc.GetLine() );
            
            // Go to the next line.
            while (  sc.Step() != SSTOKEN_EOF && 
                  !( sc.GetToken() == SSTOKEN_SYMBOL && sc.GetValue() == '}' ) &&
                  !sc.LineChanged() );

            goto SKIP_STEP;
      }

      else if ( sc.GetToken() == SSTOKEN_RESERVED && 
               ( sc.GetValue() == "new" || sc.GetValue() == "singleton" ) )
      {
         // Grab the object type.
         if ( sc.Step() != SSTOKEN_WORD )
            goto ERROR_RECOVERY;

         wxString type = sc.GetValue();

         // Make sure we have a parenthisis next.
         if ( sc.Step() != SSTOKEN_SYMBOL || sc.GetValue() != '(' )
            goto SKIP_STEP;

         // Check for the local keyword.
         bool isLocal = sc.Step() == SSTOKEN_RESERVED && sc.GetValue() == "local";
         if ( isLocal )
            sc.Step();

         // Grab the name next... if it has one.  Note that this
         // allows unnamed objects to pass into the system.
         wxString name;
         if ( sc.GetToken() == SSTOKEN_WORD )
            name = sc.GetValue();

         // Create the object... don't worry about
         // multiply defined object as the merge
         // will fix it later.
         AutoCompClass* object = page->AddObject( name, type );

         // Set the page and line.
         object->SetFile( path, sc.GetLine(), sc.GetLine() );

         // Store the special comment aka description.
         object->SetDesc( Comment );
         Comment.Empty();

         // Skip over everything till we get the close bracket.
         while (  sc.GetValue() != ')' &&
                  sc.Step() != SSTOKEN_EOF &&
                  ( sc.GetValue() == ':' || sc.GetToken() != SSTOKEN_SYMBOL ) );

         if ( sc.GetValue() != ')' )
            goto ERROR_RECOVERY;

         sc.Step();
         if ( sc.GetValue() == ';' )
            goto SKIP_STEP;

         if ( sc.GetValue() != '{' )
            goto ERROR_RECOVERY;

         // Push the object on the the object stack.
         objects.push( object );
         ++bracket;
      }

      else if ( sc.GetToken() == SSTOKEN_RESERVED && sc.GetValue() == "function" ) 
      {
         // Grab the function name.
         if ( sc.Step() != SSTOKEN_WORD )
            goto ERROR_RECOVERY;

         // Add it to the page... don't worry about 
         // multiply defined functions... the merge
         // will take care of it.
         function = new AutoCompFunction( sc.GetValue() );
         page->AddFunction( function );

         // Store the special comment aka description.
         function->SetDesc( Comment );
         Comment.Empty();

         // Store the start line for later.
         startLine = sc.GetLine();

         // Look for the start of the parameters.
         if ( sc.Step() != SSTOKEN_SYMBOL || sc.GetValue() != '(' )
            goto ERROR_RECOVERY;

         // Grab all the parameters.
         wxString Args;
         for ( ; sc.GetToken() != SSTOKEN_EOF; ) {

            if ( sc.Step() == SSTOKEN_LOCAL ) {

               Args << sc.GetValue();

               function->AddVar( sc.GetValue() )->AddLine( path, sc.GetLine() );

               if ( sc.Step() == SSTOKEN_SYMBOL && sc.GetValue() == ',' ) {
                  Args << ", ";
                  continue;
               }
            }

            break;
         }
         function->SetArgs( Args );

         // If we didn't get the close ) last
         // then we've gotten a bad parameter 
         // list... error out.
         if ( sc.GetToken() != SSTOKEN_SYMBOL && sc.GetValue() != ')' )
            goto ERROR_RECOVERY;

         // Look for the start of function body.
         if ( sc.Step() != SSTOKEN_SYMBOL || sc.GetValue() != '{' )
            goto ERROR_RECOVERY;

         flevel = bracket;
         ++bracket;
      }

      else if ( sc.GetToken() == SSTOKEN_RESERVED && sc.GetValue() == "datablock" && bracket == 0 ) 
      {
         // Grab the base name.
         if ( sc.Step() != SSTOKEN_WORD )
            goto ERROR_RECOVERY;

         wxString base = sc.GetValue(); // TODO: This isn't a base... it's the type!

         //
         // TODO: Here we fail for objects and datablocks that
         // do not have a name, because... duh... we don't add
         // nameless objects or datablocks to the database.  We 
         // need to devise a method to add nameless blocks... 
         // possibly in a seperate structure in the page.
         //

         // Look for the name.
         if ( sc.Step() != SSTOKEN_SYMBOL || sc.GetValue() != '(' ||
              sc.Step() != SSTOKEN_WORD )
            goto ERROR_RECOVERY;

         // Create the datablock... don't worry about multiply defined
         // datablocks as the merge will fix it later.
         AutoCompClass* datablock = page->AddObject( sc.GetValue(), base );
         datablock->SetDatablock();

         // Set the page and line.
         datablock->SetFile( path, sc.GetLine(), sc.GetLine() );

         // Store the special comment aka description.
         datablock->SetDesc( Comment );
         Comment.Empty();

         // If there has one, skip over the : and copy name.
         sc.Step();
         if ( sc.GetValue() == ':' ) {
            sc.Step();
            sc.Step();
         }

         if ( sc.GetValue() != ')' )
            goto ERROR_RECOVERY;

         sc.Step();
         if ( sc.GetValue() != '{' )
            goto ERROR_RECOVERY;

         // Loop thru the datablock grabing the values
         // until we hit the close bracket.
         for ( ; sc.Step(); ) {

            if ( sc.GetValue() == '}' )
               break;
            
            // A comment above the var?
            if ( sc.GetToken() == SSTOKEN_COMMENT ) {

               if (  sc.GetValue().StartsWith( "///" ) ||
                     sc.GetValue().StartsWith( "//!" ) ||
                     sc.GetValue().StartsWith( "/*!" ) ) {

                  // Add it, but strip the comment marks.
                  Comment << sc.GetValue().Mid( 3 );
               
               } else
                  Comment.Empty();
            }

            // Ok... this should be an identifier.
            else if ( sc.GetToken() != SSTOKEN_WORD )
               goto ERROR_RECOVERY;

            AutoCompVar* var = datablock->AddVar( sc.GetValue() );
            var->AddLine( path, sc.GetLine() );

            // Store the special comment aka description.
            var->m_Desc = Comment;
            Comment.Empty();

            // Wait for the end of the assignment.
            while ( sc.Step() != SSTOKEN_EOF && sc.GetValue() != ";" ) {}

            // TODO: Look for trailing comments that 
            // need to apply to the current var!
         }

         datablock->SetFile( path, datablock->GetLine(), sc.GetLine() );

         // If we didn't get a close bracket then
         // we've encountered an error... exit.
         if ( sc.GetValue() != '}' )
            goto ERROR_RECOVERY;
      }

      else if ( sc.GetToken() == SSTOKEN_SYMBOL ) 
      {
         if ( sc.GetValue() == '{' )
            ++bracket;

         else if ( sc.GetValue() == '}' ) 
         {
            --bracket;

            // Are we out of the function?
            if ( function && flevel == bracket ) 
            {
               function->SetFile( path, startLine, sc.GetLine() );
               function = NULL;
            }
            else if ( objects.size() > 0 ) 
            {
               AutoCompClass* object = objects.top();
               object->SetFile( path, object->GetLine(), sc.GetLine() );
               objects.pop();
            }
         }
      }
      else
         Comment.Empty();

      // Restart the loop.
      continue;

      // Try to recover from errors by resetting the
      // state and looking for a good recovery point
      // this is either the start of a datablock or
      // function.
      ERROR_RECOVERY:
      {
         bracket = 0;
         flevel = 0;
         Comment.Empty();
         objects.empty();

         // Set the file and line range for the function.
         if ( function ) 
         {
            function->SetFile( path, startLine, sc.GetLine() );
            function = NULL;
         }

         while ( sc.Step() != SSTOKEN_EOF ) 
         {
            if (  sc.GetToken() == SSTOKEN_RESERVED && 
                  (  sc.GetValue() == "datablock" ||
                     sc.GetValue() == "function" ) ) 
               goto SKIP_STEP;
         }
 
         // We've found the eof, so break out.
         break;
      }
   }

   // If we ended up with an open function close it.
   if ( function )
      function->SetFile( path, startLine, sc.GetLine() );

   return page;
}

void AutoCompThread::AddPage( AutoCompPage* page )
{
   wxASSERT( page );

   // First remove any existing page 
   // with the same path.
   AutoCompPage* found = AutoCompPage::Find( page->GetName(), m_Pages );
   if ( found ) 
   {
      m_Pages.Remove( found );
      delete found;
   }

   // Add the new page.
   m_Pages.Add( page );
}


void* AutoCompThread::Entry()
{
   wxArrayString Signaled;
   wxString ScriptData;
   wxString File;
   enum {
      RESCAN_NONE,
      RESCAN_FILE,
      RESCAN_BUFFER,

   } RescanCmd;

   wxString ProjectPath;
   wxArrayString Mods;
   wxArrayString ScriptExts;
   wxDateTime IdleTimer = wxDateTime::Now();
   bool Rebuild = false;

   while ( !TestDestroy() )
   {
      // If we've looped a bunch of times without doing any
      // work then throttle down using sleeps... it isn't 
      // optimal, but it should work across all platforms.
      if ( wxDateTime::Now().Subtract( IdleTimer ).GetMilliseconds() > 500 )
      {
         wxThread::Sleep( 250 );
         if ( TestDestroy() )
            break;
      }
      

      // 1st Task
      //
      // Our first priority is to make sure that dynamic
      // autocomp text buffers are re-built.  This keeps
      // autocompletion while typing fresh.  Our first 
      // step with this finding whatever buffer currently
      // needs updating.
      //
      // TODO: We should consider having only one active
      // buffer at a time... this might make the design
      // simpler and remove some work from this thread.
      //
      m_DataLock.Enter();

         RescanCmd = RESCAN_NONE;
         AutoCompTextMap::iterator iter = m_ActiveMap.begin();
         for ( ; iter != m_ActiveMap.end(); iter++ ) 
         {
            if ( !iter->second ) 
            {
               RescanCmd = RESCAN_FILE;

               // wxString uses "copy on write" ref counting.  We assign via
               // c_str() here in order to avoid this... i think this is 
               // causing some crashes on shutdown!
               File = iter->first.c_str();

               m_ActiveMap.erase( iter );
               break;
            }

            else if ( iter->second->IsSignaled() )
            {
               RescanCmd = RESCAN_BUFFER;

               // wxString uses "copy on write" ref counting.  We assign via
               // c_str() here in order to avoid this... i think this is 
               // causing some crashes on shutdown!
               File = iter->first.c_str(); 

               iter->second->Lock();
               ScriptData.assign( iter->second->GetBuf(), iter->second->GetBufLen() );
               iter->second->Unlock( true );
               break;
            }
         }

      m_DataLock.Leave();

      if ( RescanCmd == RESCAN_FILE ) {

         AutoCompPage* page = AutoCompPage::Find( File, m_Pages );
         if ( page ) 
         {
            m_Pages.Remove( page );
            delete page;
            Rebuild |= true;
         }

         // If it was part of the root path then
         // we'll need an update from disk too!
         wxFileName path( File );
         if (  !ProjectPath.IsEmpty() &&
               path.IsAbsolute() &&
               path.MakeRelativeTo( ProjectPath ) ) 
         {
            if ( m_Scanner.Open( File ) )
            {
               AutoCompPage* page = ScanFile( File, m_Scanner );
               if ( page )
               {
                  page->SetLastUpdate( wxDateTime::Now() );
                  AddPage( page );
                  Rebuild = true;
               }
            }
         }

         IdleTimer.SetToCurrent();
         continue;

      } 
      else if ( RescanCmd == RESCAN_BUFFER ) 
      {
         m_Scanner.Open( ScriptData.GetData(), ScriptData.Len() );
         AutoCompPage* page = ScanFile( File, m_Scanner );
         if ( page )
         {
            AddPage( page );
            Rebuild = true;
         }

         IdleTimer.SetToCurrent();
         continue;
      }


      // 2nd Task
      //
      // Watch for files changing on disk that need to be rescanned.
      //
      if ( m_DirWatcher.GetSignaled( &Signaled ) > 0 ) 
      {
         for ( int i=0; i < Signaled.GetCount(); i++ )
         {
            const wxString& path = Signaled[i];

            // If the folder exists we need to rescan it.
            if ( wxFileName::DirExists( path ) ) 
               Rebuild |= ScanPath( path, ProjectPath, Mods, ScriptExts );
            else
            {
               wxFileName file;

               // Delete all pages within this folder.
               for ( int i=0; i < m_Pages.GetCount(); ) 
               {
                  wxASSERT( m_Pages[i] );
                  
                  file = m_Pages[i]->GetName();
                  file.SetFullName( wxEmptyString );
                  if ( file.SameAs( path ) )
                  {
                     delete m_Pages[i];
                     m_Pages.RemoveAt( i );
                     continue;
                  }

                  i++;
               }

               Rebuild |= true;
            }           
         }

         Signaled.Empty();
         IdleTimer.SetToCurrent();
         continue;
      }


      // 3rd Task
      //
      // Look for requests to change the root project 
      // path or the exports.
      //
      m_DataLock.Enter();

         if ( m_UpdateExports )
         {
            wxASSERT( m_NewExports == NULL || m_Exports != m_NewExports );
            wxDELETE( m_Exports );
            m_Exports = m_NewExports;
            m_NewExports = NULL;
            m_UpdateExports = false;

            Rebuild = true;

            m_DataLock.Leave();

            IdleTimer.SetToCurrent();
            continue;
         }

         if (  ProjectPath != m_ProjectPath ||
               Mods != m_Mods ||
               ScriptExts != m_ScriptExts ) 
         {
            // Avoid "COW" strings by doing assignments which force copies!
            ProjectPath = m_ProjectPath.c_str();
            Mods.Clear();
            for ( int i=0; i < m_Mods.GetCount(); i++ )
               Mods.Add( m_Mods[i].c_str() );
            ScriptExts.Clear();
            for ( int i=0; i < m_ScriptExts.GetCount(); i++ )
               ScriptExts.Add( m_ScriptExts[i].c_str() );

            m_DataLock.Leave();

            // Delete the existing pages and watch.
            m_DirWatcher.Clear();
            Clear();

            if ( !ProjectPath.IsEmpty() ) 
            {
               // Add a watch for the root folder.
               m_DirWatcher.SetWatch( ProjectPath, DIRCHANGE_FILE_NAME | DIRCHANGE_DIR_NAME | DIRCHANGE_LAST_WRITE, m_Excluded );

               // Now we need to process every file in the
               // project path to get the initial processed
               // state going.
               Rebuild |= ScanPath( ProjectPath, ProjectPath, Mods, ScriptExts );
            }

            IdleTimer.SetToCurrent();
            continue;
         }

      m_DataLock.Leave();


      // 5th Task
      //
      // Rebuild the autocomplete data when everything
      // else isn't going on.
      //
      if ( Rebuild ) 
      {
         Rebuild = false;
         RebuildCompData( m_Exports );
         IdleTimer.SetToCurrent();
         continue;
      }
   }

   return NULL;
}

void AutoCompThread::OnExit()
{
}

void AutoCompThread::RebuildCompData( AutoCompExports* exports )
{
   // How things are structured we have triple the data
   // in existance at once.  We have the individual pages,
   // the new AutoCompData, and the old AutoCompData.  Bad
   // on memory usage, but it is good for performance.

   // We'll build everyhing into a new data object.
   if ( !m_Data )
      m_Data = new AutoCompData();
   m_Data->Build( m_Pages, exports );

   // TODO: Maybe this lock should be forced?  Neither
   // us nor the forground thread hold the lock on the
   // data for very long.  Maybe we should stall if we
   // cannot replace the data immediately?

   // Now copy all the new data over... be sure it
   // is as fast as possible and that no one is 
   // touching them while we do this.
   wxASSERT( tsGetAutoComp() );
   if ( tsGetAutoComp()->UpdateData( m_Data ) )
      m_Data = NULL;
}