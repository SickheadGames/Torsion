// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_TOOLCOMMAND_H
#define TORSION_TOOLCOMMAND_H
#pragma once


class ToolCommand
{
   public:
      ToolCommand() 
         :  m_Title(),
            m_Command(),
            m_Arguments(),
            m_Directory()
         {}

      ToolCommand( const ToolCommand& cmd )
         :  m_Title( cmd.m_Title ),
            m_Command( cmd.m_Command ),
            m_Arguments( cmd.m_Arguments ),
            m_Directory( cmd.m_Directory )
         {}

      virtual ~ToolCommand() {}

      const wxString& GetTitle() const { return m_Title; }
      const wxString& GetCmd() const { return m_Command; }
      const wxString& GetArgs() const { return m_Arguments; }
      const wxString& GetDir() const { return m_Directory; }

      bool operator != ( const ToolCommand& config ) const
      {
         return   m_Title != config.m_Title ||
                  m_Command != config.m_Command ||
                  m_Arguments != config.m_Arguments ||
                  m_Directory != config.m_Directory;
      }

      void SetTitle( const wxString& title ) { m_Title = title; }
      void SetCmd( const wxString& cmd ) { m_Command = cmd; }
      void SetArgs( const wxString& args ) { m_Arguments = args; }
      void SetDir( const wxString& dir ) { m_Directory = dir; }

      bool GetExec(  const wxString& filePath, 
                     const wxString& projectName, 
                     const wxString& projectPath, 
                     const wxString& projectWorkingDir,
                     const wxString& configName,
                     const wxString& configExe,
                     const wxString& configArgs, 
                     wxString* cwd, 
                     wxString* command,
                     wxString* args ) const;

      enum VAR
      {
         VAR_NULL = 0,

         VAR_FILE_PATH,
         VAR_FILE_DIR,
         VAR_FILE_NAME,
         VAR_FILE_EXT,
         VAR_PROJECT_NAME,
         VAR_PROJECT_DIR,
         VAR_PROJECT_FILE_NAME,
         VAR_PROJECT_WORKING_DIR,
         VAR_CONFIG_NAME,
         VAR_CONFIG_EXE,
         VAR_CONFIG_ARGS,
         VAR_MAX,
      };

      static const wxChar* GetVarName( VAR var );
      static const wxChar* GetVar( VAR var );


protected:

      static wxString ProcessVar(   const wxString& var,
                                    const wxString& filePath, 
                                    const wxString& projectName, 
                                    const wxString& projectPath, 
                                    const wxString& projectWorkingDir,
                                    const wxString& configName,
                                    const wxString& configExe,
                                    const wxString& configArgs );

      struct VarEntry
      {
         const wxChar* name;
         const wxChar* var;
      };
      static VarEntry smVarTable[];

      wxString    m_Title;
      wxString    m_Command;
      wxString    m_Arguments;
      wxString    m_Directory;
};

WX_DECLARE_OBJARRAY(ToolCommand, ToolCommandArray);

#endif // TORSION_TOOLCOMMAND_H
