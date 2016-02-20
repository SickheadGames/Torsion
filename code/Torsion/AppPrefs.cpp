// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "AppPrefs.h"

#include <wx/file.h>
#include <wx/tokenzr.h>
#include <wx/docview.h>

#include "XmlFile.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

const wxChar* AppPrefs::s_ReservedWords = 
   "break case continue datablock package default else false function if for new or package return " \
   "switch switch$ true %this while singleton local";

IMPLEMENT_CLASS(tsPrefsUpdateHint, wxObject)


AppPrefs::AppPrefs() :
	m_bDirty( true ),
	m_DefaultFont( 10, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Courier New" ),
   m_TabsAndSpaces( false ),
   m_LineBreaks( false ),
   m_LineNumbers( true ),
   m_Position( 0,0,0,0 ),
   m_Maximized( false ),
   m_CodeFolding( true ),
   m_TabWidth( 3 ),
   m_UseTabs( false ),
   m_LineWrap( true ),
   m_UseEdgeMarker( true ),
   m_CheckForUpdates( true ),
   m_ShowAllMods( false ),
   m_ShowAllFiles( true ),
   m_SyncProjectTree( true ),
   m_ProjectSashWidth( 0 ),
   m_BottomSashHeight( 0 ),
   m_LoadLastProject( true ),
   m_EdgeColumn( 80 )
{
}

bool AppPrefs::Load( const wxString& Path )
{
	wxFile File;
	if ( !File.Open( Path, wxFile::read ) ) {

      // This loads defaults.
      LoadFromString( "" );
		return false;
	}

	size_t Length = File.Length();
    char* Buffer = new char[ Length+1 ];
	File.Read( Buffer, Length );
	Buffer[ Length ] = 0;
   LoadFromString( Buffer );
	delete [] Buffer;

   return true;
}

void AppPrefs::LoadFromString( const wxChar* Buffer )
{
   XmlFile Xml( Buffer );

	Xml.FindElem( "TorsionPrefs" );
	Xml.IntoElem();

   wxString fontInfo = Xml.GetStringElem( "DefaultFont", m_DefaultFont.GetNativeFontInfoDesc() );
   m_DefaultFont.SetNativeFontInfo( fontInfo );

   m_Reserved = Xml.GetStringElem( "ReservedWords", s_ReservedWords );
	ReformatWords( m_Reserved );
   m_ReservedColor = Xml.GetColorAttrib( "ReservedWords", "color", wxColour( 0, 0, 255 ) );

   m_ExportsColor = Xml.GetColorElem( "ExportsColor", wxColour( 0, 0, 128 ) );

   m_BgColor = Xml.GetColorElem( "BgColor", wxColour( 255,255,255 ) );

   m_DefaultColor = Xml.GetColorElem( "DefaultColor", wxColour( 0,0,0 ) );
   m_CommentColor = Xml.GetColorElem( "CommentColor", wxColour( 0,128,0 ) );
   m_NumberColor = Xml.GetColorElem( "NumberColor", wxColour( 0,0,0 ) );
   m_StringColor = Xml.GetColorElem( "StringColor", wxColour( 128,0,128 ) );
   m_OperatorsColor = Xml.GetColorElem( "OperatorsColor", wxColour( 128,128,0 ) );
   m_LocalsColor = Xml.GetColorElem( "LocalsColor", wxColour( 0,128,128 ) );
   m_GlobalsColor = Xml.GetColorElem( "GlobalsColor", wxColour( 196,92,0 ) );

   m_SelColor = Xml.GetColorElem( "SelColor", wxColour( 255,255,255 ) );
   m_SelBgColor = Xml.GetColorElem( "SelBgColor", wxColour( 49,106,197 ) );

   m_BraceMatchColor = Xml.GetColorElem( "BraceMatchColor", wxColour( 0,0,0 ) );
   m_BraceMatchBgColor = Xml.GetColorElem( "BraceMatchBgColor", wxColour( 240,240,240 ) );
   m_BraceMatchErrColor = Xml.GetColorElem( "BraceMatchErrColor", wxColour( 197,0,0 ) );

   m_CalltipColor = Xml.GetColorElem( "CalltipColor", wxColour( 0,0,0 ) );
   m_CalltipBgColor = Xml.GetColorElem( "CalltipBgColor", wxColour( 255,255,225 ) );
   m_CalltipHiColor = Xml.GetColorElem( "CalltipHiColor", wxColour( 0,128,128 ) );

   m_MarginColor = Xml.GetColorElem( "MarginColor", wxColour( 240,240,240 ) );
   m_MarginTextColor = Xml.GetColorElem( "MarginTextColor", wxColour( 0,0,0 ) );

   m_CodeCompletion = Xml.GetBoolElem( "CodeCompletion", true );
   
   m_EnhancedCompletion = Xml.GetBoolElem( "EnhancedCompletion", true );

   m_CodeFolding = Xml.GetBoolElem( "CodeFolding", true );
   m_CodeFoldingColor = Xml.GetColorElem( "CodeFoldingColor", wxColour( 128, 128, 128 ) );

   wxRect desktop( wxScreenDC().GetSize() );

   Xml.FindElem( "Position" );
	m_Position = StringToRect( Xml.GetData().c_str() );
   {
      if ( !desktop.Inside( m_Position.GetTopLeft() ) )
         m_Position.SetTopLeft( wxPoint( 0, 0 ) );
      if ( m_Position.GetWidth() > desktop.GetWidth() )
         m_Position.SetWidth( desktop.GetWidth() );
      if ( m_Position.GetHeight() > desktop.GetHeight() )
         m_Position.SetHeight( desktop.GetHeight() );
   }
	Xml.ResetMainPos();
   m_Maximized = Xml.GetBoolElem( "Maximized", false );

   m_ProjectSashWidth = Xml.GetIntElem( "ProjectSashWidth", -1 );
   m_BottomSashHeight = Xml.GetIntElem( "BottomSashHeight", -1 );

   m_FileHistory.Empty();
   Xml.GetArrayStringElems( m_FileHistory, "FileHistory", "File" );

   m_ProjectHistory.Empty();
   Xml.GetArrayStringElems( m_ProjectHistory, "ProjectHistory", "Project" );

   m_FindStrings.Empty();
   Xml.GetArrayStringElems( m_FindStrings, "FindHistory", "String" );

   m_FindTypes.Empty();
   if ( Xml.GetArrayStringElems( m_FindTypes, "FindTypes", "Type" ) < 1 )
      m_FindTypes.Add( "*.cs;*.gui;*.mis;*.t2d" );

   m_FindPaths.Empty();
   Xml.GetArrayStringElems( m_FindPaths, "FindPaths", "Path" );
   FixupFindPaths( m_FindPaths );

   m_FindMatchCase = Xml.GetBoolElem( "FindMatchCase", false );
   m_FindMatchWord = Xml.GetBoolElem( "FindMatchWord", false );
   m_FindSearchUp = Xml.GetBoolElem( "FindSearchUp", false );
   
   m_FindPos = Xml.GetPointElem( "FindPos", wxDefaultPosition );
   if ( !desktop.Inside( m_FindPos ) )
      m_FindPos = wxDefaultPosition;

   m_FindSymbols.Empty();
   Xml.GetArrayStringElems( m_FindSymbols, "FindSymbols", "String" );

   m_LoadLastProject = Xml.GetBoolElem( "LoadLastProject", true );
   m_LastProject = Xml.GetStringElem( "LastProject", wxEmptyString );

   m_ScriptExts.Empty();
   wxString exts = Xml.GetStringElem( "ScriptExtensions", "cs,gui,mis,t2d" );
   SetScriptExtsString( exts );

   m_DSOExts.Empty();
   exts = Xml.GetStringElem( "DSOExts", "dso,edso" );
   SetDSOExtsString( exts );

   m_ExcludedFiles.Empty();
   if ( Xml.GetArrayStringElems( m_ExcludedFiles, "ExcludedFileNames", "Name" ) == -1 ) 
   {
      m_ExcludedFiles.Add( ".dll" );
      m_ExcludedFiles.Add( ".exe" );
      m_ExcludedFiles.Add( ".lnk" );
      m_ExcludedFiles.Add( ".torsion" );
      m_ExcludedFiles.Add( ".bak" );
      m_ExcludedFiles.Add( ".opt" );  // TODO: Change to options once we fix that
      m_ExcludedFiles.Add( ".exports" );
      m_ExcludedFiles.Add( ".tmp" );
      m_ExcludedFiles.Add( ".ilk" );
      m_ExcludedFiles.Add( ".pdb" );
   }

   m_ExcludedFolders.Empty();
   if ( Xml.GetArrayStringElems( m_ExcludedFolders, "ExcludedFolderNames", "Name" ) == -1 ) {

      m_ExcludedFolders.Add( "_svn" );
      m_ExcludedFolders.Add( ".svn" );
      m_ExcludedFolders.Add( "cvs" );
   }

   m_TextExts.Empty();
   if ( Xml.GetArrayStringElems( m_TextExts, "TextExts", "Ext" ) == -1 ) {

      /*
      TODO: Support opening text files!
      m_TextExts.Add( "txt" );
      m_TextExts.Add( "ini" );
      m_TextExts.Add( "log" );
      m_TextExts.Add( "readme" );
      m_TextExts.Add( "html" );
      m_TextExts.Add( "htm" );
      m_TextExts.Add( "hlsl" );
      m_TextExts.Add( "bat" );
      m_TextExts.Add( "xml" );
      */
   }

   m_UseTabs = Xml.GetBoolElem( "UseTabs", false );
   m_TabWidth = Xml.GetIntElem( "TabWidth", 3 );

   m_LineWrap = Xml.GetBoolElem( "LineWrap", false );
   
   m_UseEdgeMarker = Xml.GetBoolElem( "EdgeMarker", true );
   m_EdgeColumn = Xml.GetIntElem( "EdgeMarkerColumn", 80 );
   m_EdgeMarkerColor = Xml.GetColorElem( "EdgeMarkerColor", wxColour( 240,240,240 ) );

   m_TabsAndSpaces = Xml.GetBoolElem( "ShowTabsAndSpaces", false );
   m_LineBreaks = Xml.GetBoolElem( "ShowLineBreaks", false );
   m_LineNumbers = Xml.GetBoolElem( "ShowLineNumbers", true );

   m_ShowAllMods        = Xml.GetBoolElem( "ShowAllMods", false );
   m_ShowAllFiles       = Xml.GetBoolElem( "ShowAllFiles", true );
   m_SyncProjectTree    = Xml.GetBoolElem( "SyncProjectTree", true );

   m_CheckForUpdates = Xml.GetBoolElem( "CheckForUpdates", true );

   m_ToolCommands.Clear();
   if ( Xml.FindElem( "ExternalTools" ) )
   {
      Xml.IntoElem();

      while ( Xml.FindElem( "Tool" ) && Xml.IntoElem() )
      {
         ToolCommand cmd;
         cmd.SetTitle( Xml.GetStringElem( "Title", "(Empty)" ) );
         cmd.SetCmd( Xml.GetStringElem( "Command", wxEmptyString ) );
         cmd.SetArgs( Xml.GetStringElem( "Arguments", wxEmptyString ) );
         cmd.SetDir( Xml.GetStringElem( "Directory", wxEmptyString ) );
         m_ToolCommands.Add( cmd );
         Xml.OutOfElem();
      }

      Xml.OutOfElem();
   }
   else
   {
      ToolCommand cmd;
      cmd.SetTitle( "(Empty)" );
      m_ToolCommands.Add( cmd );
      m_ToolCommands.Add( cmd );
      m_ToolCommands.Add( cmd );
      m_ToolCommands.Add( cmd );
   }

   m_bDirty = false;
}

bool AppPrefs::Save( const wxString& Path )
{
	wxFile File;
	if ( !File.Open( Path, wxFile::write ) ) {
		return false;
	}

	XmlFile Xml;
	Xml.AddElem( "TorsionPrefs" );
	Xml.IntoElem();

   Xml.AddElem( "DefaultFont", m_DefaultFont.GetNativeFontInfoDesc() );

   Xml.AddElem( "ReservedWords", m_Reserved );
	Xml.AddAttrib( "color", Xml.ColorToString( m_ReservedColor ) );

	Xml.AddElem( "ExportsColor", Xml.ColorToString( m_ExportsColor ) );

   Xml.AddColorElem( "BgColor", m_BgColor );

   Xml.AddColorElem( "DefaultColor", m_DefaultColor );
   Xml.AddColorElem( "CommentColor", m_CommentColor );
   Xml.AddColorElem( "StringColor", m_StringColor );
   Xml.AddColorElem( "NumberColor", m_NumberColor );
   Xml.AddColorElem( "OperatorsColor", m_OperatorsColor );
   Xml.AddColorElem( "LocalsColor", m_LocalsColor );
   Xml.AddColorElem( "GlobalsColor", m_GlobalsColor );

   Xml.AddColorElem( "SelColor", m_SelColor );
   Xml.AddColorElem( "SelBgColor", m_SelBgColor );

   Xml.AddColorElem( "BraceMatchColor", m_BraceMatchColor );
   Xml.AddColorElem( "BraceMatchBgColor", m_BraceMatchBgColor );
   Xml.AddColorElem( "BraceMatchErrColor", m_BraceMatchErrColor );

   Xml.AddColorElem( "CalltipColor", m_CalltipColor );
   Xml.AddColorElem( "CalltipBgColor", m_CalltipBgColor );
   Xml.AddColorElem( "CalltipHiColor", m_CalltipHiColor );

   Xml.AddColorElem( "MarginColor", m_MarginColor );
   Xml.AddColorElem( "MarginTextColor", m_MarginTextColor );

   Xml.AddBoolElem( "CodeFolding", m_CodeFolding );
   Xml.AddColorElem( "CodeFoldingColor", m_CodeFoldingColor );

   Xml.AddBoolElem( "CodeCompletion", m_CodeCompletion );

   Xml.AddBoolElem( "EnhancedCompletion", m_EnhancedCompletion );
   
   Xml.AddElem( "Position", RectToString( m_Position ) );
   Xml.AddBoolElem( "Maximized", m_Maximized );

   Xml.AddIntElem( "ProjectSashWidth", m_ProjectSashWidth );
   Xml.AddIntElem( "BottomSashHeight", m_BottomSashHeight );

   Xml.AddBoolElem( "UseTabs", GetUseTabs() );
   Xml.AddIntElem( "TabWidth", GetTabWidth() );
   Xml.AddBoolElem( "LineWrap", GetLineWrap() );

   Xml.AddBoolElem( "EdgeMarker", GetEdgeMarker() );
   Xml.AddIntElem( "EdgeMarkerColumn", GetEdgeMarkerCol() );
   Xml.AddColorElem( "EdgeMarkerColor", GetEdgeMarkerColor() );

   Xml.AddBoolElem( "ShowTabsAndSpaces", GetTabsAndSpaces() );
   Xml.AddBoolElem( "ShowLineBreaks", GetLineBreaks() );
   Xml.AddBoolElem( "ShowLineNumbers", GetLineNumbers() );

   Xml.AddArrayStringElems( "FileHistory", "File", m_FileHistory );
   Xml.AddArrayStringElems( "ProjectHistory", "Project", m_ProjectHistory );

   Xml.AddArrayStringElems( "FindHistory", "String", m_FindStrings );
   Xml.AddArrayStringElems( "FindTypes", "Type", m_FindTypes );
   Xml.AddArrayStringElems( "FindPaths", "Path", m_FindPaths );

   Xml.AddBoolElem( "FindMatchCase", m_FindMatchCase );
   Xml.AddBoolElem( "FindMatchWord", m_FindMatchWord );
   Xml.AddBoolElem( "FindSearchUp", m_FindSearchUp );

   Xml.AddPointElem( "FindPos", m_FindPos );

   Xml.AddArrayStringElems( "FindSymbols", "String", m_FindSymbols );

   Xml.AddBoolElem( "LoadLastProject", m_LoadLastProject );
   Xml.AddElem( "LastProject", m_LastProject );

   Xml.AddElem( "ScriptExtensions", GetScriptExtsString() );
   Xml.AddElem( "DSOExts", GetDSOExtsString() );
   Xml.AddArrayStringElems( "ExcludedFileNames", "Name", m_ExcludedFiles );
   Xml.AddArrayStringElems( "ExcludedFolderNames", "Name", m_ExcludedFolders );
   Xml.AddArrayStringElems( "TextExts", "Ext", m_TextExts );

   Xml.AddBoolElem( "ShowAllMods", m_ShowAllMods );
   Xml.AddBoolElem( "ShowAllFiles", m_ShowAllFiles );
   Xml.AddBoolElem( "SyncProjectTree", m_SyncProjectTree );

   Xml.AddBoolElem( "CheckForUpdates", m_CheckForUpdates );

   Xml.AddElem( "ExternalTools" );
	Xml.IntoElem();
   for ( size_t i=0; i < m_ToolCommands.GetCount(); i++ ) 
   {
      const ToolCommand& cmd = m_ToolCommands[i];

      Xml.AddElem( "Tool" );
	   Xml.IntoElem();

         Xml.AddElem( "Title", cmd.GetTitle() );
         Xml.AddElem( "Command", cmd.GetCmd() );
         Xml.AddElem( "Arguments", cmd.GetArgs() );
         Xml.AddElem( "Directory", cmd.GetDir() );

      Xml.OutOfElem();
   }
   Xml.OutOfElem();

   std::string Buffer( Xml.GetDoc() );
	File.Write( Buffer.c_str(), Buffer.length() );

   return true;
}

bool AppPrefs::SaveIfDirty( const wxString& Path )
{
	if ( IsDirty() ) {
		return Save( Path );
	}
	return true;
}

static int CmpStringOrderNoCase( const wxString& first, const wxString& second )
{
    return first.CmpNoCase( second );
}

void AppPrefs::ReformatWords( wxString& Words )
{
	wxStringTokenizer Tokenizer( Words, "\t\r\n ,;", wxTOKEN_STRTOK );
   wxArrayString Sorted;
	wxString Token;
	while ( Tokenizer.HasMoreTokens() ) {

		Token = Tokenizer.GetNextToken();
		Token.Trim( true );
		Token.Trim( false );
      Sorted.Add( Token );
	}

   Sorted.Sort( CmpStringOrderNoCase );

   Words.Empty();
   for ( int i=0; i < Sorted.GetCount(); i++ ) {
      Words << ( i != 0 ? " " : "" ) << Sorted[i];
   }
}

wxString AppPrefs::ColorToString( const wxColour& Color )
{
	wxString Result;
   Result << Color.Red() << "," << Color.Green() << "," << Color.Blue();
	return Result;
}

wxColour AppPrefs::StringToColor( const wxString& Color )
{
	wxStringTokenizer Tokenizer( Color, "\t\r\n ,;", wxTOKEN_STRTOK );

	wxString Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Red = 0;
	Token.ToLong( &Red );
	Red = wxMin( 255, wxMax( 0, Red ) );
	
	Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Green = 0;
	Token.ToLong( &Green );
	Green = wxMin( 255, wxMax( 0, Green ) );

	Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Blue = 0;
	Token.ToLong( &Blue );
	Blue = wxMin( 255, wxMax( 0, Blue ) );

	return wxColour( Red, Green, Blue );
}

wxString AppPrefs::RectToString( const wxRect& Rect )
{
	wxString Result;
   Result << Rect.GetLeft() << "," << Rect.GetTop() << "," << Rect.GetWidth() << "," << Rect.GetHeight();
	return Result;
}

wxRect AppPrefs::StringToRect( const wxString& Rect )
{
	wxStringTokenizer Tokenizer( Rect, "\t\r\n ,;", wxTOKEN_STRTOK );

	wxString Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Left = 0;
	Token.ToLong( &Left );

	Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Top = 0;
	Token.ToLong( &Top );

	Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Width = 0;
	Token.ToLong( &Width );

   Token = Tokenizer.GetNextToken();
	Token.Trim( true );
	Token.Trim( false );
	long Height = 0;
	Token.ToLong( &Height );

   return wxRect( Left, Top, Width, Height );
}

void AppPrefs::GetFileHistory( wxFileHistory* history ) const
{
   wxASSERT( history );

   // Note we add here in reverse because AddFileToHistory 
   // inserts items into the top.
   for ( size_t i=m_FileHistory.GetCount(); i > 0; i-- ) 
      history->AddFileToHistory( m_FileHistory[ i - 1 ] );
}

void AppPrefs::SetFileHistory( wxFileHistory* history )
{
   wxASSERT( history );

   m_FileHistory.Empty();
   for ( size_t i=0; i < history->GetCount(); i++ )
      m_FileHistory.Add( history->GetHistoryFile( i ) );

   m_bDirty = true;
}

void AppPrefs::GetProjectHistory( wxFileHistory* history ) const
{
   wxASSERT( history );

   // Note we add here in reverse because AddFileToHistory 
   // inserts items into the top.
   for ( size_t i=m_ProjectHistory.GetCount(); i > 0; i-- ) 
      history->AddFileToHistory( m_ProjectHistory[ i - 1 ] );
}

void AppPrefs::SetProjectHistory( wxFileHistory* history )
{
   wxASSERT( history );

   m_ProjectHistory.Empty();
   for ( size_t i=0; i < history->GetCount(); i++ )
      m_ProjectHistory.Add( history->GetHistoryFile( i ) );

   m_bDirty = true;
}

bool AppPrefs::IsScriptFile( const wxString& file ) const
{
   wxFileName ext( file );
   return m_ScriptExts.Index( ext.GetExt(), wxFileName::IsCaseSensitive() ) != wxNOT_FOUND;
}

bool AppPrefs::IsTextFile( const wxString& file ) const
{
   wxFileName ext( file );
   return m_TextExts.Index( ext.GetExt(), wxFileName::IsCaseSensitive() ) != wxNOT_FOUND;
}

bool AppPrefs::IsExcludedFile( const wxString& file ) const
{
   wxFileName fixed( file );

   wxString ext;
   ext << "." << fixed.GetExt();

   return 
      m_ExcludedFiles.Index( fixed.GetFullName(), wxFileName::IsCaseSensitive() ) != wxNOT_FOUND ||
      m_ExcludedFiles.Index( ext, wxFileName::IsCaseSensitive() ) != wxNOT_FOUND;
}

bool AppPrefs::IsExcludedFolder( const wxString& name ) const
{
   return m_ExcludedFolders.Index( name, wxFileName::IsCaseSensitive() ) != wxNOT_FOUND;
}

void AppPrefs::SetFindStrings( const wxArrayString& strings ) 
{ 
   m_FindStrings = strings; 
   m_bDirty = true;
}

void AppPrefs::SetFindTypes( const wxArrayString& types ) 
{ 
   m_FindTypes = types;
   m_bDirty = true;
}

void AppPrefs::SetFindPaths( const wxArrayString& paths ) 
{ 
   m_FindPaths = paths; 
   m_bDirty = true;
}

void AppPrefs::SetFindPos( const wxPoint& point )
{
   if ( point != m_FindPos )
   {
      m_FindPos = point;
      m_bDirty = true;
   }
}

void AppPrefs::FixupFindPaths( wxArrayString& paths )
{
   if ( paths.GetCount() > 20 )
      paths.SetCount( 20 );

   if ( paths.Index( "Project", false ) != wxNOT_FOUND ) 
      paths.RemoveAt( paths.GetCount() - 1 );
   if ( paths.Index( "Base Directory", false ) != wxNOT_FOUND ) 
      paths.RemoveAt( paths.GetCount() - 1 );

   if ( paths.Index( "Project", false ) == wxNOT_FOUND )
      paths.Add( "Project" );
   if ( paths.Index( "Base Directory", false ) == wxNOT_FOUND )
      paths.Add( "Base Directory" );
}

void AppPrefs::SetFindSymbols( const wxArrayString& strings ) 
{ 
   m_FindSymbols = strings; 
   m_bDirty = true;
}

int AppPrefs::GetStringsFromCombo( wxComboBox* combo, wxArrayString& output )
{
   wxASSERT( combo );

   int count = 0;
   for ( int i=0; i < combo->GetCount(); i++ ) {
      output.Add( combo->GetString( i ) );
      ++count;
   }

   return count;
}

void AppPrefs::AddStringsToCombo( wxComboBox* combo, const wxArrayString& strings )
{
   wxASSERT( combo );
   for ( int i=0; i < strings.GetCount(); i++ ) {
      
      if ( strings[i].IsEmpty() )
         continue;

      combo->Append( strings[i] ); 
   }
   if ( strings.GetCount() > 0 )
      combo->SetValue( combo->GetString( 0 ) );
}

bool AppPrefs::SetScriptExtsString( const wxString& value )
{
	wxStringTokenizer toker( value, ",;", wxTOKEN_STRTOK );
   wxArrayString exts;
   wxString ext;
	while ( toker.HasMoreTokens() ) {

      ext = toker.GetNextToken();
		ext.Trim( true );
		ext.Trim( false );
      
      if ( ext.IsEmpty() )
         continue;

      if ( ext[0] == '.' )
         ext.Remove( 0, 1 );

      if ( !ext.IsEmpty() )
         exts.Add( ext );
	}

   // Sort the extensions alphabetically.
   //exts.Sort();

   if ( exts != m_ScriptExts )
   {
      m_ScriptExts = exts;
      m_bDirty = true;
      return true;
   }

   return false; 
}

wxString AppPrefs::GetDefaultScriptExtension() const
{
   wxString ext( "cs" );
   if ( m_ScriptExts.GetCount() > 0 )
      ext = m_ScriptExts[0];
   return ext;
}

wxString AppPrefs::GetScriptExtsString() const
{
   wxString exts;

   for ( int i=0; i < m_ScriptExts.GetCount(); i++ )
      exts << m_ScriptExts[i] << "; ";

   exts.RemoveLast(2);

   return exts;
}

bool AppPrefs::SetExcludedFiles( const wxString& exclude )
{
	wxStringTokenizer toker( exclude, ",;", wxTOKEN_STRTOK );
   wxArrayString files;
   wxString file;
	while ( toker.HasMoreTokens() ) {

      file = toker.GetNextToken();
		file.Trim( true );
		file.Trim( false );
		files.Add( file );
	}

   if ( files != m_ExcludedFiles )
   {
      m_ExcludedFiles = files;
      m_bDirty = true;
      return true;
   }

   return false; 
}

wxString AppPrefs::GetExcludedFilesString() const
{
   wxString exclude;

   for ( int i=0; i < m_ExcludedFiles.GetCount(); i++ )
      exclude << m_ExcludedFiles[i] << "; ";

   exclude.RemoveLast(2);

   return exclude;
}

bool AppPrefs::SetExcludedFolders( const wxString& exclude )
{
	wxStringTokenizer toker( exclude, ",;", wxTOKEN_STRTOK );
   wxArrayString folders;
   wxString folder;
	while ( toker.HasMoreTokens() ) {

      folder = toker.GetNextToken();
		folder.Trim( true );
		folder.Trim( false );
		folders.Add( folder );
	}

   if ( folders != m_ExcludedFolders )
   {
      m_ExcludedFolders = folders;
      m_bDirty = true;
      return true;
   }

   return false; 
}

wxString AppPrefs::GetExcludedFoldersString() const
{
   wxString exclude;

   for ( int i=0; i < m_ExcludedFolders.GetCount(); i++ )
      exclude << m_ExcludedFolders[i] << "; ";

   exclude.RemoveLast(2);

   return exclude;
}

bool AppPrefs::SetDSOExtsString( const wxString& value )
{
	wxStringTokenizer toker( value, ",;", wxTOKEN_STRTOK );
   wxArrayString exts;
   wxString ext;
	while ( toker.HasMoreTokens() ) {

      ext = toker.GetNextToken();
		ext.Trim( true );
		ext.Trim( false );
      
      if ( ext.IsEmpty() )
         continue;

      if ( ext[0] == '.' )
         ext.Remove( 0, 1 );

      if ( !ext.IsEmpty() )
         exts.Add( ext );
	}

   // Sort the extensions alphabetically.
   //exts.Sort();

   if ( exts != m_ScriptExts )
   {
      m_DSOExts = exts;
      m_bDirty = true;
      return true;
   }

   return false; 
}

wxString AppPrefs::GetDSOExtsString() const
{
   wxString exts;

   for ( int i=0; i < m_DSOExts.GetCount(); i++ )
      exts << m_DSOExts[i] << "; ";

   exts.RemoveLast(2);

   return exts;
}

bool AppPrefs::IsDSOExt( const wxString& ext ) const
{
   for ( int i=0; i < m_DSOExts.GetCount(); i++ )
   {
      if ( m_DSOExts[i].IsSameAs( ext, wxFileName::IsCaseSensitive() ) )
         return true;
   }

   return false;
}

bool AppPrefs::IsDSO( const wxString& file ) const
{
   wxFileName fname( file );
   return IsDSOExt( fname.GetExt() );
}

wxString AppPrefs::GetDSOForScript( const wxString& script ) const
{
   for ( int i=0; i < m_DSOExts.GetCount(); i++ )
   {
      wxString dsofile;
      dsofile << script << '.' << m_DSOExts[i];
      if ( wxFileName::FileExists( dsofile ) )
         return dsofile;
   }

   return wxEmptyString;
}

bool AppPrefs::SetToolCommands( const ToolCommandArray& cmds )
{
   // Check to see if we're dirty...
   bool dirty = true;
   if ( cmds.GetCount() == m_ToolCommands.GetCount() )
   {
      dirty = false;

      for ( int i=0; i < cmds.GetCount(); i++ ) 
      {
         if ( cmds[i] != m_ToolCommands[i] )
         {
            dirty = true;
            break;
         }
      }
   }

   if ( dirty ) 
   {
      m_ToolCommands = cmds;
      m_bDirty = true;
      return true;
   }

   return false;
}



