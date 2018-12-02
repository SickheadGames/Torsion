// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ToolCommand.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ToolCommandArray);

ToolCommand::VarEntry ToolCommand::smVarTable[] =
{
   { L"", L"" },
   {L"File Path",                   L"$(FilePath)" },
   {L"File Directory",              L"$(FileDir)" },
   {L"File Name",                   L"$(FileName)" },
   {L"File Extension",              L"$(FileExt)" },
   {L"Project Name",                L"$(ProjectName)" },
   {L"Project Directory",           L"$(ProjectDir)" },
   {L"Project File Name",           L"$(ProjectFileName)" },
   {L"Project Working Directory",   L"$(ProjectWorkingDir)" },
   {L"Config Name",                 L"$(ConfigName)" },
   {L"Config Executable",           L"$(ConfigExe)" },
   {L"Config Arguments",            L"$(ConfigArgs)" },
};

const wxChar* ToolCommand::GetVarName( VAR var )
{
   return smVarTable[ var ].name;
}

const wxChar* ToolCommand::GetVar( VAR var )
{
   return smVarTable[ var ].var;
}

bool ToolCommand::GetExec( const wxString& filePath, 
                           const wxString& projectName, 
                           const wxString& projectPath, 
                           const wxString& projectWorkingDir,
                           const wxString& configName,
                           const wxString& configExe,
                           const wxString& configArgs,  
                           wxString* cwd, 
                           wxString* command,
                           wxString* args ) const
{
   wxASSERT( cwd );
   wxASSERT( command );
   wxASSERT( args );

   // Process the command first!
   if ( !m_Command.IsEmpty() )
   {
      *command = ProcessVar(   m_Command, 
                               filePath, 
                               projectName, 
                               projectPath, 
                               projectWorkingDir,
                               configName,
                               configExe,
                               configArgs ); 
   }

   // Start by adding the command.
   wxFileName fixup( *command );
   fixup.Normalize();
   if ( fixup.FileExists() )
      *command = fixup.GetFullPath();
   if ( command->IsEmpty() )
      return false;
   
   // Process the arguments for variables.
   if ( !m_Arguments.IsEmpty() )
   {
      *args = ProcessVar(  m_Arguments, 
                           filePath, 
                           projectName, 
                           projectPath, 
                           projectWorkingDir,
                           configName,
                           configExe,
                           configArgs );

      if ( args->IsEmpty() )
         return false;
   }

   // Get the working dir.
   if ( !m_Directory.IsEmpty() )
   {
      *cwd = ProcessVar(   m_Directory, 
                           filePath, 
                           projectName, 
                           projectPath, 
                           projectWorkingDir,
                           configName,
                           configExe,
                           configArgs  );

      if ( cwd->IsEmpty() )
         return false;
   }

   return true;
}

wxString ToolCommand::ProcessVar(   const wxString& text,
                                    const wxString& filePath, 
                                    const wxString& projectName, 
                                    const wxString& projectPath, 
                                    const wxString& projectWorkingDir,
                                    const wxString& configName,
                                    const wxString& configExe,
                                    const wxString& configArgs )
{
   wxString out( text );
   
   // First process the file vars.
   wxFileName file( filePath );
   if ( out.Replace( GetVar( VAR_FILE_PATH ), file.GetFullPath() ) > 0 && filePath.IsEmpty() )
      return wxEmptyString;
   if ( out.Replace( GetVar( VAR_FILE_DIR ), file.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) ) > 0 && filePath.IsEmpty() )
      return wxEmptyString;
   if ( out.Replace( GetVar( VAR_FILE_NAME ), file.GetName() ) > 0 && filePath.IsEmpty() )
      return wxEmptyString;
   if ( out.Replace( GetVar( VAR_FILE_EXT ), file.GetExt() ) > 0 && filePath.IsEmpty() )
      return wxEmptyString;

   // Got a project name?
   if ( out.Replace( GetVar( VAR_PROJECT_NAME ), projectName ) > 0 && projectName.IsEmpty() )
      return wxEmptyString;

   // Now process the project vars.
   wxFileName dir( projectPath );
   if ( out.Replace( GetVar( VAR_PROJECT_DIR ), dir.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR ) ) > 0 && projectPath.IsEmpty() )
      return wxEmptyString;
   if ( out.Replace( GetVar( VAR_PROJECT_FILE_NAME ), dir.GetName() ) > 0 && projectPath.IsEmpty() )
      return wxEmptyString;

   // Process the working dir.
   dir.AssignDir( projectWorkingDir );
   if ( out.Replace( GetVar( VAR_PROJECT_WORKING_DIR ), dir.GetFullPath() ) > 0 && projectWorkingDir.IsEmpty() )
      return wxEmptyString;

   // Process the config vars.
   if ( out.Replace( GetVar( VAR_CONFIG_NAME ), configName ) > 0 && configName.IsEmpty() )
      return wxEmptyString;
   if ( out.Replace( GetVar( VAR_CONFIG_EXE ), configExe ) > 0 && configExe.IsEmpty() )
      return wxEmptyString;
   out.Replace( GetVar( VAR_CONFIG_ARGS ), configArgs );

   out.Trim();
   out.Trim( false );

   return out;
}

