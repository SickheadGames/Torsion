// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _CONFIGDLG_H_
#define _CONFIGDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "ConfigDlg.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/propgrid/manager.h"
////@end includes

/*!
 * Forward declarations
 */
class ProjectConfig;
class ProjectConfigArray;

////@begin forward declarations
class wxPropertyGridManager;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CONFIGDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CONFIGDLG_TITLE _("Configuration Settings")
#define SYMBOL_CONFIGDLG_IDNAME wxID_ANY
#define SYMBOL_CONFIGDLG_SIZE wxSize(425, 340)
#define SYMBOL_CONFIGDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * ConfigDlg class declaration
 */

class ConfigDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( ConfigDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ConfigDlg( );
    ConfigDlg( wxWindow* parent, wxWindowID id = SYMBOL_CONFIGDLG_IDNAME, const wxString& caption = SYMBOL_CONFIGDLG_TITLE, const wxPoint& pos = SYMBOL_CONFIGDLG_POSITION, const wxSize& size = SYMBOL_CONFIGDLG_SIZE, long style = SYMBOL_CONFIGDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CONFIGDLG_IDNAME, const wxString& caption = SYMBOL_CONFIGDLG_TITLE, const wxPoint& pos = SYMBOL_CONFIGDLG_POSITION, const wxSize& size = SYMBOL_CONFIGDLG_SIZE, long style = SYMBOL_CONFIGDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    int ShowModal( ProjectConfig* config, ProjectConfigArray* configs, const wxString& baseDir, bool forceModified = false );

////@begin ConfigDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end ConfigDlg event handler declarations

////@begin ConfigDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ConfigDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ConfigDlg member variables
    wxPropertyGridManager* m_PropGrid;
////@end ConfigDlg member variables

   ProjectConfig*       m_Config;
   ProjectConfigArray*  m_Configs;
   wxString             m_BaseDir;
};

#endif
    // _CONFIGDLG_H_
