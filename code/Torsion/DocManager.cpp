// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "DocManager.h"

#include "TorsionApp.h"
#include "ScriptDoc.h"
#include "ScriptView.h"
#include "ProjectDoc.h"
#include "ProjectView.h"

#include "Identifiers.h"

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 


// TODO: Rewrite DocManager to support project files
// natively and remove the functionality from MainFrame.


DocManager::DocManager()
   :  wxDocManager(),
      m_ProjectHistory( NULL ),
      m_ScriptTemplate( NULL ),
      m_ProjectTemplate( NULL )
{
   // Create the project template.
   m_ProjectTemplate = new wxDocTemplate( 
      this, 
      _T( "Project" ),
      _T( "*.torsion" ), 
      _T( "" ), 
      _T( "torsion" ), 
      _T( "Project Doc" ), 
      _T( "Project View" ),
      CLASSINFO(ProjectDoc),
      CLASSINFO(ProjectView),
      wxTEMPLATE_INVISIBLE );
   
   // TODO: It would be better if you could have multiple
   // wxFileHistorys which can be associated to different
   // wxDocTemplates.
   m_ProjectHistory = new wxFileHistory( 5, tsID_PROJECT1 );

   UpdateScriptFileTemplates();
}

DocManager::~DocManager()
{
   // These are already cleaned up in the base.
   m_ProjectTemplate = NULL;
   m_ScriptTemplate = NULL;

   wxDELETE( m_ProjectHistory );
}

void DocManager::AddFileToHistory(const wxString& file)
{
   // Check if it's a project file.
   if ( m_ProjectTemplate->FileMatchesTemplate( file ) )
   {
      m_ProjectHistory->AddFileToHistory( file );
      return;
   }

   wxDocManager::AddFileToHistory( file );
}


void DocManager::UpdateScriptFileTemplates()
{
   // We can never remove a template when running without closing
   // and reopening all the current script files.  So for now just
   // add missing invisible templates and update the filter on the
   // visible template. :\

   // Go thru the script extensions.
   const wxArrayString& exts = tsGetPrefs().GetScriptExtensions();
   wxString defaultExt;
   wxString allExtsFilter;
   for ( int i=0; i < exts.GetCount(); i++ )
   {
      wxString ext = exts[i];
      ext.MakeLower();

      wxString filter;
      filter << "*." << ext;
      allExtsFilter << filter << ";";

      if ( i == 0 )
      {
         defaultExt = ext;
         continue;
      }

      // Check to see if we already have this template or not?
      if ( FindTemplateForPath( filter ) )
         continue;

      new wxDocTemplate(
         this, 
         _T( "TorqueScript" ), 
         filter, 
         _T( "" ), 
         ext, 
         _T( "TorqueScript Doc" ),
         _T( "TorqueScript View" ),
         CLASSINFO(ScriptDoc), 
         CLASSINFO(ScriptView),
         wxTEMPLATE_INVISIBLE );
   }

   // Remove the trailing ;
   allExtsFilter.RemoveLast();

   // Create the visible script template or update it.
   if ( !m_ScriptTemplate )
   {
      m_ScriptTemplate = new wxDocTemplate(
         this, 
         _T( "TorqueScript" ), 
         allExtsFilter, 
         _T( "" ),
         defaultExt, 
         _T( "TorqueScript Doc" ), 
         _T( "TorqueScript View" ),
         CLASSINFO(ScriptDoc), 
         CLASSINFO(ScriptView) );
   }
   else
   {
      m_ScriptTemplate->SetFileFilter( allExtsFilter );
      m_ScriptTemplate->SetDefaultExtension( defaultExt );
   }
}