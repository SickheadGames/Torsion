// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_PROJECTCONFIG_H
#define TORSION_PROJECTCONFIG_H
#pragma once


class ProjectConfig
{
    public:
        ProjectConfig() 
           :   m_Name(),
               m_Executable(),
               m_Arguments(),
               m_HasExports( true ),
               m_Precompile( true ),
               m_InjectDebugger( true ),
               m_UseSetModPaths( false )
        {}

        ProjectConfig( const ProjectConfig& config )
            :  m_Name( config.m_Name ),
               m_Executable( config.m_Executable ),            
               m_Arguments( config.m_Arguments ),
               m_HasExports( config.m_HasExports ),
               m_Precompile( config.m_Precompile ),
               m_InjectDebugger( config.m_InjectDebugger ),
               m_UseSetModPaths( config.m_UseSetModPaths )
        {
        }

        virtual ~ProjectConfig() {}

        const wxString& GetName() const { return m_Name; }
        const wxString& GetArgs() const { return m_Arguments; }

        wxString GetRelativeExe( const wxString& path ) const;
        wxString GetExe() const;

        bool HasExports() const        { return m_HasExports; }
        bool Precompile() const        { return m_Precompile; }
        bool InjectDebugger() const    { return m_InjectDebugger; }
        bool UseSetModPaths() const    { return m_UseSetModPaths; }

        bool operator == ( const ProjectConfig& config ) const
        {
            return   m_Name == config.m_Name &&
                     m_Executable == config.m_Executable &&
                     m_Arguments == config.m_Arguments &&
                     m_HasExports == config.m_HasExports &&
                     m_Precompile == config.m_Precompile &&
                     m_InjectDebugger == config.m_InjectDebugger &&
                     m_UseSetModPaths == config.m_UseSetModPaths;
        }

        void SetName( const wxString& name ) { m_Name = name; }
        void SetArgs( const wxString& args ) { m_Arguments = args; }
        void SetExe( const wxString& exe );
        void SetExports( bool value ) { m_HasExports = value; }
        void SetPrecompile( bool value ) { m_Precompile = value; }
        void SetInjectDebugger( bool value ) { m_InjectDebugger = value; }
        void SetUseSetModPaths( bool use ) { m_UseSetModPaths = use; }

   protected:

      wxString    m_Name;
      wxString    m_Executable;
      wxString    m_Arguments;
      bool        m_HasExports;
      bool        m_Precompile;
      bool        m_InjectDebugger;
      bool        m_UseSetModPaths;
};

WX_DECLARE_OBJARRAY(ProjectConfig, ProjectConfigArray);

#endif // TORSION_PROJECTCONFIG_H
