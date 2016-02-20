// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_DOCMANAGER_H
#define TORSION_DOCMANAGER_H
#pragma once

#include <wx/docview.h>

class wxFileHistory;


class DocManager : public wxDocManager
{
   public:
      DocManager();
      virtual ~DocManager();

      virtual void AddFileToHistory(const wxString& file);

      wxFileHistory* GetProjectHistory() const { return m_ProjectHistory; }

      wxDocTemplate* GetProjectTemplate() const { return m_ProjectTemplate; }

      void UpdateScriptFileTemplates();

   protected:

      wxDocTemplate* m_ScriptTemplate;
      wxDocTemplate* m_ProjectTemplate;
      wxFileHistory* m_ProjectHistory;
};


#endif // TORSION_DOCMANAGER_H
