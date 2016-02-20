// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "ScriptDoc.h"

#include "ScriptView.h"
#include "ScriptCtrl.h"

#include "TorsionApp.h"
#include "AutoComp.h"
#include "AutoCompText.h"
#include "ScriptCtrl.h"


#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

IMPLEMENT_DYNAMIC_CLASS( ScriptDoc, wxDocument )

int ScriptDoc::s_NewDocCounter = 1;

ScriptDoc::ScriptDoc()
   :  wxDocument(),
      m_TextBuffer( NULL ),
      m_ReloadModified( false )
{
}

ScriptDoc::~ScriptDoc()
{
}

void ScriptDoc::OnAutoCompHint( bool enabled )
{
   wxASSERT( tsGetAutoComp() );

   if ( enabled )
   {
      wxString pageName = GetFilename();
      if ( pageName.IsEmpty() )
         pageName = GetTitle();

      wxASSERT( !m_TextBuffer );
      m_TextBuffer = tsGetAutoComp()->AddActivePage( pageName );

      // Fill the text buffer with the current 
      // contents of the doc/scriptctrl.
      wxASSERT( m_TextBuffer );
      m_TextBuffer->Lock();
      ScriptView* view = (ScriptView*)GetFirstView();
      if ( view && view->GetCtrl() )
      {
         int length = view->GetCtrl()->GetTextLength();
         wxChar* buffer = m_TextBuffer->GetWriteBuf( length );
         view->GetCtrl()->GetTextRangeRaw( buffer, 0, -1 );
      }
      m_TextBuffer->Unlock();
   }
   else
   {
      wxASSERT( m_TextBuffer );
      tsGetAutoComp()->RemoveActivePage( m_TextBuffer );
      m_TextBuffer = NULL;
   }
}

void ScriptDoc::OnFileRename( const wxString& newPath )
{
   wxFileName path( newPath );
   if ( !path.IsAbsolute() || path.IsDir() || !path.FileExists() )
      return;

   // Rename the doc... title first so that the view
   // can get that during the notification.
   SetTitle( wxFileNameFromPath( path.GetFullPath() ) );
   SetFilename( path.GetFullPath(), true );
}

bool ScriptDoc::Save()
{
    if (!IsModified() && m_savedYet)
        return true;

    if ( m_documentFile.empty() || !m_savedYet || !wxFileName::FileExists( m_documentFile ) )
        return SaveAs();

   return OnSaveDocument(m_documentFile);
}

bool ScriptDoc::OnSaveDocument( const wxString& filename )
{
   wxASSERT( tsGetAutoComp() );
   if ( m_TextBuffer )
      tsGetAutoComp()->RemoveActivePage( m_TextBuffer );
   m_TextBuffer = tsGetAutoComp()->AddActivePage( filename );

   // We always do operations on the first 
   // view... the other views will react
   // accordingly.
   ScriptView* view = (ScriptView*)GetFirstView();
   if ( !view->OnSaveFile( filename ) )
      return false;

   SetFilename( filename, true );
   SetTitle( wxFileNameFromPath( filename ) );
   Modify( false );
   SetDocumentSaved( true );

   return true;
}

bool ScriptDoc::OnOpenDocument( const wxString& filename )
{
   SetFilename( filename, true );
   SetTitle( wxFileNameFromPath( filename ) );
   Modify( false );
   m_savedYet = true;

   wxASSERT( tsGetAutoComp() );
   m_TextBuffer = tsGetAutoComp()->AddActivePage( GetFilename() );

   // We always do operations on the first 
   // view... the other views will react
   // accordingly.
   ScriptView* view = (ScriptView*)GetFirstView();
   if ( !view->OnLoadFile( filename ) )
      return false;

   UpdateAllViews();

   return true;
}

/*
bool ScriptDoc::IsModified() const
{
   ScriptView* view = (ScriptView*)GetFirstView();

   if ( view && view->GetCtrl() )
      return   wxDocument::IsModified() || 
               view->GetCtrl()->GetModify();

   return wxDocument::IsModified();
}
*/

void ScriptDoc::Modify( bool mod )
{
   if ( mod != IsModified() )
   {
      m_ReloadModified = mod;
      wxDocument::Modify( mod );
   }
}

void ScriptDoc::SetViewScrollOffset( const wxPoint& scroll )
{
   ScriptView* view = (ScriptView*)GetFirstView();
   if ( view && view->GetCtrl() ) {
      view->GetCtrl()->LineScroll( 0, scroll.y );
      view->GetCtrl()->SetXOffset( scroll.x );
   }
}

wxPoint ScriptDoc::GetViewScrollOffset() const
{
   ScriptView* view = (ScriptView*)GetFirstView();
   if ( view && view->GetCtrl() ) {

      return wxPoint(   view->GetCtrl()->GetXOffset(),
                        view->GetCtrl()->GetFirstVisibleLine() );
   }
   return wxPoint( 0, 0 );     
}

bool ScriptDoc::OnNewDocument()
{
   if (!OnSaveModified())
      return false;

   if (OnCloseDocument()==false)
      return false;

   DeleteContents();
   Modify(false);
   SetDocumentSaved(false);

   wxString name;
   name << "Script" << s_NewDocCounter++;
   SetTitle(name);
   SetFilename(wxEmptyString, true);

   wxASSERT( tsGetAutoComp() );
   wxASSERT( !m_TextBuffer );
   m_TextBuffer = tsGetAutoComp()->AddActivePage( name );

   // Activate the new view!
   //GetFirstView()->Activate( true );

   return true;
}

bool ScriptDoc::DeleteContents()
{
   if ( m_TextBuffer ) 
   {
      wxASSERT( tsGetAutoComp() );
      tsGetAutoComp()->RemoveActivePage( m_TextBuffer );
      m_TextBuffer = NULL;
   }

   return true;
}


