// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ProjectDoc.h"

#include "TorsionApp.h"
#include "MainFrame.h"
#include "ProjectView.h"
#include "XmlFile.h"
#include "Debugger.h"
#include "ScriptDoc.h"
#include "ScriptView.h"
#include "AutoComp.h"
#include "AutoCompExports.h"
#include "BreakpointsPanel.h"
#include "Platform.h"
#include "BuildExportsDlg.h"

#include <wx/file.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileInfoArray);

IMPLEMENT_DYNAMIC_CLASS( ProjectDoc, wxDocument )

const wxString ProjectDoc::sm_SearchURL = "http://tdn.garagegames.com/elixir/search.php";

const wxString ProjectDoc::sm_DebugHookBegin = "// BEGIN TORSION";
const wxString ProjectDoc::sm_DebugHookEnd   = "// END TORSION";
const wxString ProjectDoc::sm_DefaultHook = "dbgSetParameters( #port#, \"#password#\", true );";


ProjectDoc::ProjectDoc()
   :  wxDocument(),
      m_Name( "Untitled" ),
      m_Address( "127.0.0.1" ),
      m_Port( 6060 ),
      m_Password( "password" ),
      m_EntryScript( "main.cs" ),
      m_DebugHook( sm_DefaultHook ),
      m_SearchURL( wxEmptyString ), //sm_SearchURL ),
      m_SearchProduct( "TGE" ),
      m_SearchVersion( "HEAD" ),
      m_ExecModifiedScripts( true )
{
   SetScannerExtsString( "cs;gui" );
}

ProjectDoc::~ProjectDoc()
{
   DeleteAllBreakpoints();
   DeleteAllBookmarks();
}

void ProjectDoc::CopyDocument( const ProjectDoc* doc )
{
   m_Name         = doc->m_Name;
   m_WorkingDir   = doc->m_WorkingDir;
   m_Configs      = doc->m_Configs;
   m_Address         = doc->m_Address;
   m_Port            = doc->m_Port;
   m_Password        = doc->m_Password;
   m_EntryScript     = doc->m_EntryScript;
   m_DebugHook       = doc->m_DebugHook;
   m_SearchURL       = doc->m_SearchURL;
   m_SearchProduct   = doc->m_SearchProduct;
   m_SearchVersion   = doc->m_SearchVersion;
   m_Mods            = doc->m_Mods;
   m_ScannerExts     = doc->m_ScannerExts;
   m_LastConfig      = doc->m_LastConfig;
   m_ExecModifiedScripts   = doc->m_ExecModifiedScripts;

   Modify( true );   
   UpdateAllViews();
}


void ProjectDoc::UpdateOpenFilesArray()
{
   m_OpenFiles.Clear();
   m_ActiveOpenFile = -1;

   ScriptFrame* frame = ScriptFrame::GetFrame( false );
   wxDocManager* docMan = GetDocumentManager();
   wxASSERT( docMan );
   wxList& docs = docMan->GetDocuments();
   wxList::compatibility_iterator node = docs.GetFirst();
   while (node)
   {
      ScriptDoc *doc = wxDynamicCast( node->GetData(), ScriptDoc );
      if ( doc ) 
      {
         FileInfo info;
         info.Name = doc->GetFilename();
         info.Offset = doc->GetViewScrollOffset();
         
         wxASSERT( frame );
         ScriptView* view = (ScriptView*)doc->GetFirstView();
         info.Index = frame->GetViewIndex( view );

         // Store the active view index.
         if ( view == frame->GetView() )
            m_ActiveOpenFile = m_OpenFiles.GetCount();

         if ( wxFileName::FileExists( info.Name ) )
            m_OpenFiles.Add( info );
      }

      node = node->GetNext();
   }

   // Sort it by the index order, so that it is stored
   // in the order they should be created later.
   m_OpenFiles.Sort( FileInfo::CmpIndex );
}

bool ProjectDoc::Close()
{
   // Grab a snapshot of the currently open files
   // SaveOptions looks for it first.
   m_OpenFiles.Clear();
   
   // Close all open files!
   {
      wxDocManager* docMan = GetDocumentManager();
      wxASSERT( docMan );
      wxList& docs = docMan->GetDocuments();
      wxList::compatibility_iterator node = docs.GetFirst();
      while ( node )
      {
         ScriptDoc *doc = wxDynamicCast( node->GetData(), ScriptDoc );

         if ( doc && !doc->Close() )
         {
            m_OpenFiles.Clear();
            return false;
         }

         node = node->GetNext();
      }

      // Now update the array that we have all
      // the saved file names.
      UpdateOpenFilesArray();

      node = docs.GetFirst();
      while ( node )
      {
         ScriptDoc *doc = wxDynamicCast( node->GetData(), ScriptDoc );
         wxList::compatibility_iterator next = node->GetNext();

         if ( doc ) 
         {
            // Implicitly deletes the document when
            // the last view is deleted
            doc->DeleteAllViews();

            // Check we're really deleted
            if (docs.Member(doc))
                  delete doc;
         }

         node = next;
      }
   }

   // If we're modified then OnSaveDocument() 
   // will save the options... so detect that 
   // so we don't do it twice here.
   bool modified = IsModified();

   if ( !wxDocument::Close() )
      return false;

   // If the save would have occured above we don't 
   // need to save the options, as save already has.
   // We have to wait till here as this depends on
   // the file name assigned during the save.
   if ( !modified )
      SaveOptions();

   // Clear the autocomp system.
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->Clear();

   return true;
}

bool ProjectDoc::OnSaveDocument( const wxString& filename )
{
	wxFile File;
	if ( !File.Open( filename, wxFile::write ) )
   {
      wxMessageBox(_("The project could not be saved.  The file is read-only or locked!"), wxTheApp->GetAppName(), wxOK|wxICON_EXCLAMATION,
                        GetDocumentWindow());
		return false;
   }

   const wxString filePath = wxFileName( filename ).GetPath();

   // Convert the working dir to be relative 
   // to the save dir.
   wxFileName working = wxFileName::DirName( m_WorkingDir );
   working.MakeRelativeTo( filePath );

	XmlFile Xml;
	Xml.AddElem(L"TorsionProject" );
	Xml.IntoElem();
	Xml.AddElem(L"Name", m_Name.wc_str() );
	Xml.AddElem(L"WorkingDir", working.GetFullPath().wc_str() );
   Xml.AddElem(L"EntryScript", m_EntryScript.wc_str() );
   Xml.AddElem(L"DebugHook", m_DebugHook.wc_str() );
   Xml.AddArrayStringElems(L"Mods", L"Folder", m_Mods );
   Xml.AddElem(L"ScannerExts", GetScannerExtsString().wc_str() );

   Xml.AddElem(L"Configs" );
	Xml.IntoElem();
   for ( size_t i=0; i < m_Configs.GetCount(); i++ ) {

      Xml.AddElem(L"Config" );
	   Xml.IntoElem();

         Xml.AddElem(L"Name", m_Configs[i].GetName().wc_str() );
         wxString exe( m_Configs[i].GetRelativeExe( filePath ) );
         Xml.AddElem(L"Executable", exe.wc_str() );
         Xml.AddElem(L"Arguments", m_Configs[i].GetArgs().wc_str() );
         Xml.AddBoolElem(L"HasExports", m_Configs[i].HasExports() );
         Xml.AddBoolElem(L"Precompile", m_Configs[i].Precompile() );
         Xml.AddBoolElem(L"InjectDebugger", m_Configs[i].InjectDebugger() );
         Xml.AddBoolElem(L"UseSetModPaths", m_Configs[i].UseSetModPaths() );

      Xml.OutOfElem();
   }
   Xml.OutOfElem();

   //Xml.AddBoolElem( "OneClickDebugging", m_AllowDebugHook );
   //Xml.AddBoolElem( "Precompile", m_Precompile );

   Xml.AddElem(L"SearchURL", m_SearchURL.wc_str() );
   Xml.AddElem(L"SearchProduct", m_SearchProduct.wc_str() );
   Xml.AddElem(L"SearchVersion", m_SearchVersion.wc_str() );

   Xml.AddBoolElem( "ExecModifiedScripts", m_ExecModifiedScripts );

   // Store it.
   std::wstring Buffer( Xml.GetDoc() );
	File.Write( Buffer.c_str(), Buffer.length() );

   Modify( false );
   SetFilename( filename );
   SetTitle( wxFileNameFromPath( filename ) );
   SetDocumentSaved( true );

   SaveOptions();

   return true;
}

bool ProjectDoc::OnOpenDocument( const wxString& filename )
{
	wxFile File;
	if ( !File.Open( filename, wxFile::read ) ) 
   {
      wxMessageBox(_("Sorry, could not open this project."), wxTheApp->GetAppName(), wxOK|wxICON_EXCLAMATION,
                        GetDocumentWindow());
		return false;
	}

   const wxString filePath = wxFileName( filename ).GetPath();

	XmlFile Xml;
   {
	   size_t Length = File.Length();
      wchar_t* Buffer = new wchar_t[ Length+1 ];
	   File.Read( Buffer, Length );
	   Buffer[ Length ] = 0;
      Xml.SetDoc( Buffer );
	   delete [] Buffer;
   }

   if ( !Xml.FindElem(L"TorsionProject" ) )
   {
      wxMessageBox(_("Sorry, this project file is corrupt."), wxTheApp->GetAppName(), wxOK|wxICON_EXCLAMATION,
                        GetDocumentWindow());
		return false;
	}
	
	Xml.IntoElem();
	Xml.FindElem(L"Name" );
	m_Name = Xml.GetData().c_str();
	Xml.ResetMainPos();
	Xml.FindElem(L"WorkingDir" );
   wxFileName working( Xml.GetData().c_str() );
	working.MakeAbsolute( filePath );
   m_WorkingDir = working.GetFullPath();
	Xml.ResetMainPos();
   m_EntryScript = Xml.GetStringElem( "EntryScript", "main.cs" );
   m_DebugHook = Xml.GetStringElem( "DebugHook", sm_DefaultHook );

   m_Mods.Clear();
   Xml.GetArrayStringElems( m_Mods, "Mods", "Folder" );

   m_ScannerExts.Clear();
   wxString exts = Xml.GetStringElem( "ScannerExts", "cs;gui" );
   SetScannerExtsString( exts );

   if ( Xml.FindElem(L"Configs" ) )
   {
      Xml.IntoElem();

      m_Configs.Clear();

      while ( Xml.FindElem(L"Config" ) && Xml.IntoElem() )
      {
         ProjectConfig config;
	      config.SetName( Xml.GetStringElem( "Name", wxEmptyString ) );

         // The executable is either absolute or needs to be made 
         // absolute, relative to the directory of this project file.
         wxString exe( Xml.GetStringElem( "Executable", wxEmptyString ) );
         {
            wxFileName absolute( exe );
            if ( !absolute.IsAbsolute() )
            {
               absolute.MakeAbsolute( filePath );
               if ( absolute.FileExists() )
                  exe = absolute.GetFullPath();
            }
         }
         config.SetExe( exe );
         config.SetArgs( Xml.GetStringElem( "Arguments", wxEmptyString ) );
         config.SetExports( Xml.GetBoolElem( "HasExports", true ) );
         config.SetPrecompile( Xml.GetBoolElem( "Precompile", true ) );
         config.SetInjectDebugger( Xml.GetBoolElem( "InjectDebugger", true ) );
         config.SetUseSetModPaths( Xml.GetBoolElem( "UseSetModPaths", false ) );

         m_Configs.Add( config );

         Xml.OutOfElem();
      }

      Xml.OutOfElem();
   }

   //m_AllowDebugHook = Xml.GetBoolElem( "OneClickDebugging", true );
   //m_Precompile = Xml.GetBoolElem( "Precompile", true );

   m_SearchURL = Xml.GetStringElem( "SearchURL", wxEmptyString ); //sm_SearchURL );
   m_SearchProduct = Xml.GetStringElem( "SearchProduct", "TGE" );
   m_SearchVersion = Xml.GetStringElem( "SearchVersion", "HEAD" );

   m_ExecModifiedScripts = Xml.GetBoolElem( "ExecModifiedScripts", true );

   SetFilename( filename, true );
   SetTitle( wxFileNameFromPath( filename ) );
   Modify( false );
   m_savedYet = true;

   LoadOptions();

   // Look for debug hooks that were not restored!
   RemoveDebugHook();

   // Load exports from xml if it already exists!
   wxASSERT( tsGetAutoComp() );
   if ( tsGetAutoComp()->LoadExports( GetExportsFilePath() ) )
   {
      // Let all the views know that the exports
      // have changed and that they should update
      // the syntax highlighting.
      wxASSERT( tsGetMainFrame() );
      tsPrefsUpdateHint hint;
      tsGetMainFrame()->SendHintToAllViews( &hint, true );
   }
   else
      BuildExportsDB();

   tsGetAutoComp()->SetProjectPath( m_WorkingDir, m_Mods, m_ScannerExts );

   UpdateAllViews();

   return true;
}

bool ProjectDoc::OnNewDocument()
{
   Modify(true);
   SetDocumentSaved(false);

   // Set the title and default filename based on the working path and name.
   SetTitle( GetName() );
   wxFileName path( wxFileName::DirName( GetWorkingDir() ) );
   path.SetName( GetName() );
   wxDocTemplate* temp = GetDocumentTemplate();
   wxASSERT( temp );
   path.SetExt( temp->GetDefaultExtension() );
   SetFilename( path.GetFullPath(), true );

   // Setup the autocomplete stuff.
   BuildExportsDB();
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->LoadExports( GetExportsFilePath() );

   // Let all the views know that the exports
   // have changed and that they should update
   // the syntax highlighting.
   wxASSERT( tsGetMainFrame() );
   tsPrefsUpdateHint hint;
   tsGetMainFrame()->SendHintToAllViews( &hint, true );

   // TODO: Deal with failure here!
   tsGetAutoComp()->SetProjectPath( m_WorkingDir, m_Mods, m_ScannerExts );

   return true;
}

void ProjectDoc::LoadOptions()
{
   wxASSERT( tsGetDebugger() );
   wxASSERT( tsGetMainFrame() );

   // Now load the project state file.
	wxFile File;
	if ( !File.Open( GetFilename() + ".opt", wxFile::read ) )
		return;

   XmlFile Xml;
   {
	   size_t Length = File.Length();
      wchar_t* Buffer = new wchar_t[ Length+1 ];
	   File.Read( Buffer, Length );
	   Buffer[ Length ] = 0;
      Xml.SetDoc( Buffer );
	   delete [] Buffer;
   }

   if ( !Xml.FindElem(L"TorsionProjectOptions" ) )
		return;

   Xml.IntoElem();

   m_Address = Xml.GetStringElem( "Address", "127.0.0.1" );
   m_Password = Xml.GetStringElem( "Password", "password" );
   m_Port = Xml.GetIntElem( "Port", 6060 );
   
   m_LastConfig = Xml.GetStringElem( "LastConfig", wxEmptyString );

   if ( Xml.FindElem(L"Breakpoints" ) )
   {
      Xml.IntoElem();

      while ( Xml.FindElem(L"Breakpoint" ) && Xml.IntoElem() )
      {
	      Xml.FindElem(L"File" );
		  wxString File = Xml.GetData();
	      Xml.ResetMainPos();
	      Xml.FindElem(L"Line" );
	      int Line = wxAtoi( Xml.GetData() );
	      Xml.ResetMainPos();
	      Xml.FindElem(L"Pass" );
	      int Pass = wxAtoi( Xml.GetData() );
	      Xml.ResetMainPos();
	      Xml.FindElem(L"Condition" );
	      wxString Condition = Xml.GetData().c_str();
	      Xml.ResetMainPos();
         bool Enabled = Xml.FindElem(L"Enabled" );

         if ( !File.IsEmpty() )
            AddBreakpoint( File, Line, Pass, Condition, Enabled );

         Xml.OutOfElem();
      }

      Xml.OutOfElem();
   }

	Xml.ResetMainPos();

   if ( Xml.FindElem(L"Bookmarks" ) )
   {
      Xml.IntoElem();

      while ( Xml.FindElem(L"Bookmark" ) && Xml.IntoElem() )
      {
	      Xml.FindElem(L"File" );
	      wxString File = Xml.GetData().c_str();
	      Xml.ResetMainPos();
	      Xml.FindElem(L"Line" );
	      int Line = wxAtoi( Xml.GetData() );
	      Xml.ResetMainPos();

         wxString absolutePath = MakeAbsoluteTo( File );
         if ( !File.IsEmpty() && wxFileName::FileExists( absolutePath ) )
            AddBookmark( File, Line );

         Xml.OutOfElem();
      }

      Xml.OutOfElem();
   }

	Xml.ResetMainPos();

   if ( Xml.FindElem(L"OpenFiles" ) )
   {
      Xml.IntoElem();

      wxASSERT( tsGetMainFrame() );
      ScriptView* activeView = NULL;
      while ( Xml.FindElem(L"File" ) )
      {
         wxString File = Xml.GetData().c_str();

         ScriptView* view = tsGetMainFrame()->OpenFile( MakeAbsoluteTo( File ) );
         if ( !view )
            continue;

         ScriptDoc* doc = wxDynamicCast( view->GetDocument(), ScriptDoc );
         if ( doc )
         {
            wxPoint scroll(   wxAtoi( Xml.GetAttrib(L"ScrollX" ) ),
                              wxAtoi( Xml.GetAttrib(L"ScrollY" )) );

            // Store the active view!
            if ( XmlFile::StringToBool( Xml.GetAttrib(L"Active" ).c_str() ) )
               activeView = view;

            doc->SetViewScrollOffset( scroll );
         }
      }

      // If we got an active view then make it active!
      if ( activeView )
      {
         ScriptFrame* frame = ScriptFrame::GetFrame( false );
         wxASSERT( frame );
         frame->SetActive( activeView );
      }

      Xml.OutOfElem();
   }

	Xml.ResetMainPos();

   return;
}

void ProjectDoc::SaveOptions()
{
   if ( GetFilename().IsEmpty() || !tsGetDebugger() || !tsGetMainFrame() )
      return;

   // We use the project name with 'state' appended.
	wxFile File;
	if ( !File.Open( GetFilename() + ".opt", wxFile::write ) )
		return;

   // If our open file list is empty then try to fill it!
   if ( m_OpenFiles.IsEmpty() ) 
      UpdateOpenFilesArray();

   char temp[MAX_PATH];

	CMarkup Xml;
	Xml.AddElem(L"TorsionProjectOptions" );
	Xml.IntoElem();

	Xml.AddElem(L"Address", m_Address.wc_str() );
	Xml.AddElem(L"Password", m_Password.wc_str() );
	Xml.AddElem(L"Port", wxString(itoa( m_Port, temp, 10 )).wc_str() );

   // Capture what the last selected config is and store it.
   m_LastConfig = tsGetMainFrame()->GetActiveConfigName();
   Xml.AddElem(L"LastConfig", m_LastConfig.wc_str() );

   Xml.AddElem(L"Breakpoints" );
	Xml.IntoElem();
   const BreakpointArray& breakpoints = GetBreakpoints();
   for ( int i = 0; i < breakpoints.GetCount(); i++ ) 
   {
      Xml.AddElem(L"Breakpoint" );
	   Xml.IntoElem();

         wxASSERT( !wxFileName( breakpoints[i]->GetFile() ).IsAbsolute() );
         Xml.AddElem(L"File", breakpoints[i]->GetFile().wx_str() );
         Xml.AddElem(L"Line", wxString(itoa( breakpoints[i]->GetLine(), temp, 10 )).wc_str() );
         Xml.AddElem(L"Pass", wxString(itoa( breakpoints[i]->GetPass(), temp, 10 )).wc_str() );
         Xml.AddElem(L"Condition", breakpoints[i]->GetCondition().wc_str() );

         if ( breakpoints[i]->GetEnabled() )
            Xml.AddElem(L"Enabled" );

      Xml.OutOfElem();
   }
   Xml.OutOfElem();

   Xml.AddElem(L"Bookmarks" );
	Xml.IntoElem();
   const BookmarkArray& bookmarks = GetBookmarks();
   for ( int i = 0; i < bookmarks.GetCount(); i++ ) 
   {
      Xml.AddElem(L"Bookmark" );
	   Xml.IntoElem();

         wxASSERT( !wxFileName( bookmarks[i]->GetFile() ).IsAbsolute() );
         Xml.AddElem(L"File", bookmarks[i]->GetFile().wc_str() );
         Xml.AddElem(L"Line", wxString(itoa( bookmarks[i]->GetLine(), temp, 10 )).wc_str() );

      Xml.OutOfElem();
   }
   Xml.OutOfElem();

   Xml.AddElem(L"OpenFiles" );
	Xml.IntoElem();

   // The open files array is sorted by the creation
   // order... order is importaint here.
   for ( int i = 0; i < m_OpenFiles.GetCount(); i++ ) 
   {
      wxString File = m_OpenFiles[i].Name;
      wxASSERT( !File.IsEmpty() && wxFileName( File ).IsAbsolute() );
      Xml.AddElem(L"File", MakeReleativeTo( File ).wc_str() );
      Xml.AddAttrib(L"ScrollX", wxString(itoa( m_OpenFiles[i].Offset.x, temp, 10 )).wc_str() );
      Xml.AddAttrib(L"ScrollY", wxString(itoa( m_OpenFiles[i].Offset.y, temp, 10 )).wc_str() );
      if ( m_ActiveOpenFile == i )
         Xml.AddAttrib(L"Active", L"true" );
   }

   m_OpenFiles.Clear();
   Xml.OutOfElem();

   std::wstring Buffer( Xml.GetDoc() );
	File.Write( Buffer.c_str(), Buffer.length() );
}

void ProjectDoc::BuildExportsDB() const
{
   wxASSERT( tsGetAutoComp() );
   tsGetAutoComp()->ClearExports(); 

   // If we have exports to capture..
   if ( HasExports() )
   {
      wxFileName exportScript( wxGetApp().GetAppPath() );
      exportScript.SetFullName( "torsion_exports.cs" );

      wxASSERT( tsGetMainFrame() );

      // Start the export process.
      BuildExportsDlg dlg;
      dlg.Create( tsGetMainFrame() );
      if ( dlg.ShowModal(  exportScript.GetFullPath(), 
                           GetWorkingDir(),
                           m_Configs ) == wxID_OK )
      {
         // Save the exports to disk!
         AutoCompExports* exports = dlg.TakeExports();
         wxASSERT( exports );
         exports->SaveXml( GetExportsFilePath() );

         // Give the results to the autocomp system
         // and it will take ownership.
         tsGetAutoComp()->SetExports( exports );
      }
   }

   // Let all the views know that the exports
   // have changed and that they should update
   // the syntax highlighting.
   tsPrefsUpdateHint hint;
   tsGetMainFrame()->SendHintToAllViews( &hint, true );

   //tsGetAutoComp()->Resume();

   /*
   // TODO: Replace with progress dialog with cancel!
   wxMessageDialog dlg( tsGetMainFrame(), 
      "The Torque executable exports file was not found.  Do you wish to generate them now?", 
      wxTheApp->GetAppName(), wxYES_NO | wxICON_QUESTION );
   if ( dlg.ShowModal() != wxID_YES )
      return;
   */
}

wxString ProjectDoc::GetExportsFilePath() const
{
   wxFileName exports;
   exports.AssignDir( GetWorkingDir() );
   wxASSERT( !GetFilename().IsEmpty() );
   wxFileName docName( GetFilename() );
   exports.SetFullName( docName.GetFullName() + ".exports" );

   return exports.GetFullPath();
}

void ProjectDoc::SetName( const wxString& name )
{
	if ( name != m_Name ) {
		m_Name = name;
      Modify( true ); 
	}
}

void ProjectDoc::SetWorkingDir( const wxString& dir )
{
	if ( dir != m_WorkingDir ) {
		m_WorkingDir = dir;
      tsGetAutoComp()->SetProjectPath( m_WorkingDir, m_Mods, m_ScannerExts );
      Modify( true ); 
	}
}

void ProjectDoc::SetEntryScript( const wxString& script ) 
{ 
   if ( m_EntryScript != script ) 
   {
      m_EntryScript = script; 
      Modify( true ); 
   }
}

void ProjectDoc::SetDebugHook( const wxString& hook ) 
{ 
   if ( m_DebugHook != hook ) 
   {
      m_DebugHook = hook; 
      Modify( true ); 
   }
}


void ProjectDoc::SetMods( const wxArrayString& mods )
{
   if ( m_Mods != mods ) 
   {
      m_Mods = mods; 
      tsGetAutoComp()->SetProjectPath( m_WorkingDir, m_Mods, m_ScannerExts );
      Modify( true ); 
   }
}

void ProjectDoc::SetModsString( const wxString& modsString )
{
   // Scan thru the string... use ; and , as seperators.
   wxArrayString mods;
   wxStringTokenizer toker( modsString, ";," );
   while ( toker.HasMoreTokens() )
   {
      wxString mod = toker.GetNextToken();
      mod.Trim();
      mod.Trim( false );
      mods.Add( mod );
   }

   SetMods( mods ); 
}

wxString ProjectDoc::GetModsString() const
{
   wxString mods;

   for ( int i=0; i < m_Mods.GetCount(); i++ )
      mods << m_Mods[i] << "; ";

   mods.RemoveLast();
   return mods;
}

bool ProjectDoc::IsMod( const wxString& mod ) const
{
   // If we have no mods defined then all mods are valid!
   if ( m_Mods.GetCount() == 0 )
      return true;

   wxFileName dir;
   dir.AssignDir( mod );
   dir.MakeRelativeTo( m_WorkingDir );
   
   // If we didn't get at least one dir this is 
   // the working dir... so just accept it.
   if ( dir.GetDirCount() == 0 )
      return true;

   do
   {
      if ( m_Mods.Index( dir.GetPath(), wxFileName::IsCaseSensitive() ) != wxNOT_FOUND )
         return true;

      if ( dir.GetDirCount() > 0 )
         dir.RemoveLastDir();

   } while ( dir.GetDirCount() != 0 );

   return false;
}

wxString ProjectDoc::GetModPath( const wxString& FullPath ) const
{
   wxFileName Path( FullPath );
   Path.SetFullName( "" );

   const wxFileName projectDir = wxFileName::DirName( GetWorkingDir() );

   wxString ModFolder;
   do {

      if ( Path.GetDirCount() < 2 )
         return "";

      ModFolder = Path.GetDirs().Last();
      Path.RemoveLastDir();

   } while ( !Path.SameAs( projectDir ) );

   // We now know the mod folder.
   Path.Assign( projectDir );
   Path.AppendDir( ModFolder );
   return Path.GetFullPath();
}

void ProjectDoc::SetScannerExtsString( const wxString& extensions )
{
   // Scan thru the string... use ; and , as seperators.
   wxArrayString exts;
   wxStringTokenizer toker( extensions, ";," );
   while ( toker.HasMoreTokens() )
   {
      wxString ext = toker.GetNextToken();
      ext.Trim();
      ext.Trim( false );

      if ( ext.IsEmpty() )
         continue;

      if ( ext[0] == '.' )
         ext.Remove( 0, 1 );

      if ( !ext.IsEmpty() )
         exts.Add( ext );
   }

   SetScannerExts( exts ); 
}

void ProjectDoc::SetScannerExts( const wxArrayString& exts )
{
   if ( m_ScannerExts != exts ) 
   {
      m_ScannerExts = exts; 
      tsGetAutoComp()->SetProjectPath( m_WorkingDir, m_Mods, m_ScannerExts );
      Modify( true ); 
   }
}

wxString ProjectDoc::GetScannerExtsString() const
{
   wxString exts;

   for ( int i=0; i < m_ScannerExts.GetCount(); i++ )
      exts << m_ScannerExts[i] << "; ";
   
   exts.RemoveLast(2);
   return exts;
}

void ProjectDoc::SetExecModifiedScripts( bool exec )
{
   if ( m_ExecModifiedScripts != exec ) {
      m_ExecModifiedScripts = exec; 
      Modify( true ); 
   }   
}

void ProjectDoc::SetSearchUrl( const wxString& url )
{
   if ( m_SearchURL != url ) {
      m_SearchURL = url; 
      Modify( true ); 
   }
}

void ProjectDoc::SetSearchProduct( const wxString& product )
{
   if ( m_SearchProduct != product ) {
      m_SearchProduct = product; 
      Modify( true ); 
   }
}

void ProjectDoc::SetSearchVersion( const wxString& version )
{
   if ( m_SearchVersion != version ) {
      m_SearchVersion = version; 
      Modify( true ); 
   }
}

const wxString ProjectDoc::MakeReleativeTo( const wxString& FilePath ) const
{
	wxFileName RelativePath( FilePath );

   if ( RelativePath.IsAbsolute() ) {
      RelativePath.MakeRelativeTo( GetWorkingDir() );
   }
   wxASSERT( !RelativePath.IsAbsolute() );

   return RelativePath.GetFullPath();
}

const wxString ProjectDoc::MakeAbsoluteTo( const wxString& FilePath ) const
{
	wxFileName AbsolutePath( FilePath );

   if ( !AbsolutePath.IsAbsolute() ) {
      AbsolutePath.MakeAbsolute( GetWorkingDir() );
   }
   wxASSERT( AbsolutePath.IsAbsolute() );

   return AbsolutePath.GetFullPath();
}

const wxString ProjectDoc::GetExecuteCommand( const wxString& config, bool withArgs ) const
{
   const ProjectConfig* configPtr = GetConfig( config );
   if ( !configPtr )
      return wxEmptyString;

   const wxString& args = configPtr->GetArgs();
   return configPtr->GetExe() + ( withArgs && !args.IsEmpty() ? " " + configPtr->GetArgs() : "" );
}

const ProjectConfig* ProjectDoc::GetConfig( const wxString& config ) const
{
   for ( int i=0; i < m_Configs.GetCount(); i++ ) {
      if ( m_Configs[i].GetName() == config ) {
         return &m_Configs[i]; 
      }
   }

   return NULL;
}

bool ProjectDoc::SetConfigs( const ProjectConfigArray& configs )
{
   // Check to see if we're dirty...
   int matches = 0;
   for ( int i=0; i < configs.GetCount(); i++ ) {
      for ( int j=0; j < m_Configs.GetCount(); j++ ) {
         if ( configs[i] == m_Configs[j] ) {
            ++matches;
         }
      }
   }

   if (  m_Configs.GetCount() != configs.GetCount() ||
         matches != m_Configs.GetCount() ) {

      m_Configs = configs;
      Modify( true ); 
      return true;
   }

   return false;
}

bool ProjectDoc::HasExports() const
{
   bool exports = false;

   for ( int i=0; i < m_Configs.GetCount(); i++ )
      exports |= m_Configs[i].HasExports();

   return exports;
}

void ProjectDoc::BreakpointNotify( BreakpointEvent& event )
{
   wxASSERT( event.GetBreakpoint() );
   wxASSERT( tsGetMainFrame() );

   // First let the appropriate view know.
   ScriptView* view = tsGetMainFrame()->GetOpenView( event.GetBreakpoint()->GetFile() );
   if ( view )
      view->ProcessEvent( event );

   // Now let the breakpoint manager know.
   wxASSERT( tsGetMainFrame()->GetBreakpointsPanel() );
   tsGetMainFrame()->GetBreakpointsPanel()->ProcessEvent( event );

   // Finally let the debugger know.
   wxASSERT( tsGetDebugger() );
   tsGetDebugger()->ProcessEvent( event );
}

void ProjectDoc::AddBreakpoint( const wxString& File, int Line, int Pass, const wxString& Condition, bool Enabled )
{
	wxString RelativePath = MakeReleativeTo( File );
   BreakpointEvent event;
   event.SetEventObject( this );

   Breakpoint* TheBreakpoint = FindBreakpoint( RelativePath, Line );
	if ( TheBreakpoint ) {

		// Update the condition and make sure it's on!
		TheBreakpoint->SetCondition( Condition );
      if ( Enabled ) {
         TheBreakpoint->SetEnabled( true );
      }

      event.SetBreakpoint( TheBreakpoint );
      event.SetType( BreakpointEvent::TYPE_CHANGED );
      BreakpointNotify( event );
      return;
	}

	// Add a new one.
	TheBreakpoint = new Breakpoint( RelativePath, Line, Pass, Condition );
   TheBreakpoint->SetEnabled( Enabled );
	m_Breakpoints.Add( TheBreakpoint );

   event.SetBreakpoint( TheBreakpoint );
   event.SetType( BreakpointEvent::TYPE_ADD );
   BreakpointNotify( event );
}

void ProjectDoc::ChangeBreakpoint( Breakpoint* bp, Breakpoint* changes )
{
   wxASSERT( bp );
   wxASSERT( changes );

   wxString RelativePath = MakeReleativeTo( bp->GetFile() );
   BreakpointEvent event;
   event.SetEventObject( this );

   wxASSERT( FindBreakpoint( RelativePath, bp->GetLine() ) == bp );

   // If the file name changed then just delete the old breakpoint.
   if ( !bp->IsFile( changes->GetFile() ) ) 
   {
      DeleteBreakpoint( bp );
      AddBreakpoint( changes->GetFile(), changes->GetLine(), changes->GetPass(), changes->GetCondition(), changes->GetEnabled() );
      return;
   }

   // If just the line changed then move it first.
   if ( bp->GetLine() != changes->GetLine() ) 
      MoveBreakpoint( bp, changes->GetLine() );

	// Update the other values.
	bp->SetCondition( changes->GetCondition() );
	bp->SetPass( changes->GetPass() );
	bp->SetEnabled( changes->GetEnabled() );

   // Send out the notification.
   event.SetBreakpoint( bp );
   event.SetType( BreakpointEvent::TYPE_CHANGED );
   BreakpointNotify( event );
}


void ProjectDoc::DeleteBreakpoint( Breakpoint* Bp )
{
   wxASSERT( Bp );

   BreakpointEvent event;
   event.SetEventObject( this );
   event.SetBreakpoint( Bp );
   event.SetType( BreakpointEvent::TYPE_REMOVE );
   BreakpointNotify( event );

   // Remove and delete it.
   m_Breakpoints.Remove( Bp );
   delete Bp;
}

void ProjectDoc::MoveBreakpoint( Breakpoint* Bp, int Line )
{
   wxASSERT( Bp );

   if ( Bp->GetLine() == Line )
      return;

   BreakpointEvent event;
   event.SetEventObject( this );
   event.SetBreakpoint( Bp );
   event.SetType( BreakpointEvent::TYPE_MOVED );
   event.SetOldLine( Bp->GetLine() );

   Bp->SetLine( Line );

   BreakpointNotify( event );
}

Breakpoint* ProjectDoc::FindBreakpoint( const wxString& File, int Line )
{
	wxString RelativePath = MakeReleativeTo( File );

   for ( size_t i=0; i < m_Breakpoints.GetCount(); i++ ) {

      wxASSERT( m_Breakpoints[ i ] );
		if ( m_Breakpoints[ i ]->IsFileAndLine( RelativePath, Line ) ) {

			return m_Breakpoints[ i ];
		}
	}

   return NULL;
}

bool ProjectDoc::ToggleBreakpoint( const wxString& File, int Line, bool Disable )
{
	wxString RelativePath = MakeReleativeTo( File );
   
   Breakpoint* TheBreakpoint = FindBreakpoint( RelativePath, Line );
	if ( TheBreakpoint ) {

      if ( !TheBreakpoint->GetEnabled() ) {

			// Just call add which will enable it and notify
			// the debug server if it's connected!
			AddBreakpoint( RelativePath, Line, 0, TheBreakpoint->GetCondition() );
			wxASSERT( TheBreakpoint->GetEnabled() );
			return true;
		}

      // If we're not disabling it then we're 
      // removing it from the project.
      if ( !Disable ) {

         DeleteBreakpoint( TheBreakpoint );

      } else {

         TheBreakpoint->SetEnabled( false );

         BreakpointEvent event;
         event.SetEventObject( this );
         event.SetBreakpoint( TheBreakpoint );
         event.SetType( BreakpointEvent::TYPE_CHANGED );
         BreakpointNotify( event );
      }

      return false;
	}

	// Add one if it doesn't exist!
	AddBreakpoint( RelativePath, Line, 0, "" );
	return true;
}

void ProjectDoc::EnableBreakpoint( Breakpoint* Bp, bool Enable )
{
   wxASSERT( Bp );
   wxASSERT( m_Breakpoints.Index( Bp ) != wxNOT_FOUND  );

   if ( Bp->GetEnabled() == Enable )
      return;

   Bp->SetEnabled( Enable );

   BreakpointEvent event;
   event.SetEventObject( this );
   event.SetBreakpoint( Bp );
   event.SetType( BreakpointEvent::TYPE_CHANGED );
   BreakpointNotify( event );
}

bool ProjectDoc::GetBreakpoints( const wxString& File, BreakpointArray& Breakpoints )
{
	wxString RelativePath = MakeReleativeTo( File );

   Breakpoints.Clear();

   for ( size_t i=0; i < m_Breakpoints.GetCount(); i++ ) {
		
		wxASSERT( m_Breakpoints[ i ] );
		if ( m_Breakpoints[ i ]->IsFile( RelativePath ) ) {
			Breakpoints.Add( m_Breakpoints[ i ] );
		}
	}

   return !m_Breakpoints.IsEmpty();
}

void ProjectDoc::DeleteAllBreakpoints()
{
   while( !m_Breakpoints.IsEmpty() ) {
      DeleteBreakpoint( m_Breakpoints[0] );
	}
}

void ProjectDoc::EnableAllBreakpoints( bool enable )
{
   BreakpointEvent event;
   event.SetEventObject( this );

   for ( size_t i=0; i < m_Breakpoints.GetCount(); i++ ) {

      Breakpoint* bp = m_Breakpoints[ i ];
      wxASSERT( bp );
      if ( bp->GetEnabled() == enable )
         continue;

      bp->SetEnabled( enable );

      event.SetBreakpoint( bp );
      event.SetType( BreakpointEvent::TYPE_CHANGED );
      BreakpointNotify( event );
	}
}

bool ProjectDoc::GetBreakpointsEnabled() const
{
   bool enabled = false;

   for ( size_t i=0; i < m_Breakpoints.GetCount(); i++ ) {

      Breakpoint* bp = m_Breakpoints[ i ];
      wxASSERT( bp );
      enabled |= bp->GetEnabled();
	}

   return enabled;
}

Bookmark* ProjectDoc::FindBookmark( const wxString& File, int Line )
{
	wxString RelativePath = MakeReleativeTo( File );

   for ( size_t i=0; i < m_Bookmarks.GetCount(); i++ ) 
   {
      wxASSERT( m_Bookmarks[ i ] );
		if ( m_Bookmarks[ i ]->IsFileAndLine( RelativePath, Line ) )
			return m_Bookmarks[ i ];
	}

   return NULL;
}

void ProjectDoc::AddBookmark( const wxString& File, int Line )
{
	wxString RelativePath = MakeReleativeTo( File );

   // If we already have one, don't add it twice.
   Bookmark* TheBookmark = FindBookmark( RelativePath, Line );
	if ( TheBookmark ) 
      return;

   // Add a new one.
	TheBookmark = new Bookmark( RelativePath, Line );
	m_Bookmarks.Add( TheBookmark );

   wxASSERT( tsGetMainFrame() );
   tsBookmarksUpdateHint hint;
   tsGetMainFrame()->SendHintToAllViews( &hint, true );
}

bool ProjectDoc::GetBookmarks( const wxString& File, BookmarkArray& Bookmarks )
{
	wxString RelativePath = MakeReleativeTo( File );

   const size_t OrigCount = Bookmarks.GetCount();

   for ( size_t i=0; i < m_Bookmarks.GetCount(); i++ ) 
   {
		wxASSERT( m_Bookmarks[ i ] );
		if ( m_Bookmarks[ i ]->IsFile( RelativePath ) ) 
			Bookmarks.Add( m_Bookmarks[ i ] );
	}

   return OrigCount != Bookmarks.GetCount();
}

void ProjectDoc::DeleteBookmark( Bookmark* bm )
{
   wxASSERT( bm );

   m_Bookmarks.Remove( bm );
   delete bm;

   wxASSERT( tsGetMainFrame() );
   tsBookmarksUpdateHint hint;
   tsGetMainFrame()->SendHintToAllViews( &hint, true );
}

void ProjectDoc::DeleteAllBookmarks()
{
   WX_CLEAR_ARRAY( m_Bookmarks );

   wxASSERT( tsGetMainFrame() );
   tsBookmarksUpdateHint hint;
   tsGetMainFrame()->SendHintToAllViews( &hint, true );
}

const wxString ProjectDoc::GetEntryScriptPath() const
{
   return MakeAbsoluteTo( m_EntryScript );
}

bool ProjectDoc::SetDebugHook( int port, wxString& password )
{
   // To be safe... always try to remove any
   // existing hook that may have been left
   // behind from a previous run.
   RemoveDebugHook();

   wxFileName script = GetEntryScriptPath();
   if (  !script.IsOk() || 
         !script.FileExists() || 
         !wxFile::Access( script.GetFullPath(), wxFile::write ) )
      return false;

   // Store the current modified date.
   wxDateTime modified;
   if ( !script.GetTimes( NULL, &modified, NULL ) )
      return false;

   // Open the entry script.. gonna add the hook.
   wxTextFile file;
   if ( !file.Open( script.GetFullPath() ) )
      return false;

   // Add our debug hook to the top of the file.
   file.InsertLine( sm_DebugHookBegin, 0 );
   wxString portString;
   portString << port;
   wxString line( m_DebugHook );
   line.Replace( "#port#", portString );
   line.Replace( "#password#", password );
   file.InsertLine( line, 1 );
   file.InsertLine( sm_DebugHookEnd, 2);
   const int bump = 3;

   // Bump up any breakpoints in the file.
   BreakpointArray breaks;
   GetBreakpoints( script.GetFullPath(), breaks );
   for ( int i=0; i < breaks.GetCount(); i++ ) 
   {
      Breakpoint* bp = breaks[i];
      wxASSERT( bp );
      int line = bp->GetLine() + bump;
      bp->SetLine( line );
   }

   // wxTextFile is unreliable when writing the file to disk as
   // it uses a temp file and rename, but sometimes fails in an
   // uncontrolled manner deleting the main.cs!
   //
   // So copy the content of the wxTextFile to a string then use
   // a normal file to save it.
   {
      wxString content;
      for ( size_t i=0; i < file.GetLineCount(); i++ ) 
         content << file.GetLine( i ) << wxTextFile::GetEOL( file.GetLineType( i ) );
      file.Close();

      wxFile realFile;
      if ( !realFile.Open( script.GetFullPath(), wxFile::write ) )
         return false;
      realFile.Write( content );
      realFile.Close();
   }

   // Restore the original modification time.
   script.SetTimes( NULL, &modified, NULL );

   // Look for the open file and force a reload without
   // notification to the user.
   wxASSERT( tsGetMainFrame() );
   ScriptView* view = tsGetMainFrame()->GetOpenView( script.GetFullPath() );
   if ( view )
      view->ReloadFile( wxID_YESTOALL, true );

   return true;
}

void ProjectDoc::RemoveDebugHook()
{
   wxFileName script = GetEntryScriptPath();
   if (  !script.IsOk() || 
         !script.FileExists() || 
         !wxFile::Access( script.GetFullPath(), wxFile::write ) )
      return;

   // Store the current modified date.
   wxDateTime modified;
   if ( !script.GetTimes( NULL, &modified, NULL ) )
      return;

   // Open the entry script and remove all '// BEGIN/END TORSION' blocks.
   wxTextFile file;
   if ( !file.Open( script.GetFullPath() ) )
      return;

   bool save = false;
   for ( int i=0; i < file.GetLineCount(); i++ )
   {
      wxString line = file.GetLine( i );
      
      // Ignore any whitespace that may have been added.
      line.Trim( false ); line.Trim( true );
      if ( wxStrnicmp( line, sm_DebugHookBegin, sm_DebugHookBegin.Len() ) != 0 )
         continue;

      // Look for the end line.
	  int j=i+1;
      for ( ; j < file.GetLineCount(); j++ )
      {
         line = file.GetLine( j );
         line.Trim( false ); line.Trim( true );
         if ( wxStrnicmp( line, sm_DebugHookEnd, sm_DebugHookEnd.Len() ) == 0 )
            break;
      }

      // If we didn't find the end of the block then do
      // not edit the file... the user screwed something
      // up and we don't want to fuck it further.
      if ( j >= file.GetLineCount() )
         return;

      // How many lines in the block?
      int count = ( j - i ) + 1;
      wxASSERT( count >= 2 );
      save = true;

      // Bump any breakpoints below our block up.
      BreakpointArray breaks;
      GetBreakpoints( script.GetFullPath(), breaks );
      for ( int k=0; k < breaks.GetCount(); k++ ) 
      {
         Breakpoint* bp = breaks[k];
         wxASSERT( bp );
         if ( bp->GetLine() >= j )
            bp->SetLine( bp->GetLine() - count );
      }

      // Remove all lines in our hook block.
      while ( count-- )
         file.RemoveLine( i );

      // Continue the search on the same line that we
      // started this loop with.
      --i;
   }

   // Save the modified entry script file.
   if ( save )
   {
      // wxTextFile is unreliable when writing the file to disk as
      // it uses a temp file and rename, but sometimes fails in an
      // uncontrolled manner deleting the main.cs!
      //
      // So copy the content of the wxTextFile to a string then use
      // a normal file to save it.
      {
         wxString content;
         for ( size_t i=0; i < file.GetLineCount(); i++ ) 
            content << file.GetLine( i ) << wxTextFile::GetEOL( file.GetLineType( i ) );
         file.Close();

         wxFile realFile;
         if ( !realFile.Open( script.GetFullPath(), wxFile::write ) )
            return;
         realFile.Write( content );
         realFile.Close();
      }

      // Restore the original modification time.
      script.SetTimes( NULL, &modified, NULL );

      // Look for the open file and force a reload without
      // notification to the user.
      wxASSERT( tsGetMainFrame() );
      ScriptView* view = tsGetMainFrame()->GetOpenView( script.GetFullPath() );
      if ( view )
      {
         if ( view->GetDocument()->IsModified() )
            view->ReloadFile( 0, true );
         else
            view->ReloadFile( wxID_YESTOALL, true );
      }
   }

   file.Close();
}

wxString ProjectDoc::GetSearchUrl( const wxString& name ) const
{
   if ( m_SearchURL.IsEmpty() )
      return wxEmptyString;

   wxString url = m_SearchURL + "?docType=script";
   wxString product = m_SearchProduct.IsEmpty() ? "TGE" : m_SearchProduct;
   wxString version = m_SearchVersion.IsEmpty() ? "HEAD" : m_SearchVersion;
   url << "&product=" << product;
   url << "&version=" << version;
   url << "&name=" << name;
   return url;
}

wxString ProjectDoc::GetDefaultSearchUrl( const wxString& name )
{
   return wxEmptyString;

   // TODO: Enable once Elixir is online.
   /*
   wxString url = sm_SearchURL + "?docType=script";
   url << "&product=TGE";
   url << "&version=HEAD";
   url << "&name=" << name;
   return url;
   */
}

