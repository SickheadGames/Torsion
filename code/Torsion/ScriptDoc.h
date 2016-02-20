// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SCRIPTDOC_H
#define TORSION_SCRIPTDOC_H
#pragma once

#include <wx/docview.h>

class AutoCompText;


class ScriptDoc : public wxDocument
{
   DECLARE_DYNAMIC_CLASS(ScriptDoc)

   public:

      ScriptDoc();
      virtual ~ScriptDoc();

      virtual bool OnSaveDocument( const wxString& filename );
      virtual bool OnOpenDocument( const wxString& filename );
      //virtual bool IsModified() const;
      virtual void Modify( bool mod );
      virtual bool OnNewDocument();
      virtual bool DeleteContents();

      virtual bool Save();

      void SetViewScrollOffset( const wxPoint& scroll );
      wxPoint GetViewScrollOffset() const;

      //virtual wxWindow* GetDocumentWindow() const;

      // TODO: This autocomp text buffers are wacky... 
      // needs to be refactored for 2.x!
      void OnAutoCompHint( bool enabled );
      AutoCompText* GetTextBuffer() const { return m_TextBuffer; }
      
      bool IsReloadModified() const { return m_ReloadModified; }
      void ClearReloadModified() { m_ReloadModified = false; }

      void OnFileRename( const wxString& newPath );

   protected:

      AutoCompText*     m_TextBuffer;

      // This is an additional modified flag that is
      // used for the debugger to do modifiy reloads.
      bool              m_ReloadModified;  

      static int        s_NewDocCounter;
};

WX_DEFINE_ARRAY( ScriptDoc*, ScriptDocArray );

#endif // TORSION_SCRIPTDOC_H
