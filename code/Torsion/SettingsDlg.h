// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _SETTINGSDLG_H_
#define _SETTINGSDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "SettingsDlg.h"
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

////@begin forward declarations
class wxPropertyGridManager;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_SETTINGSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_SETTINGSDLG_TITLE _("Preferences")
#define SYMBOL_SETTINGSDLG_IDNAME wxID_ANY
#define SYMBOL_SETTINGSDLG_SIZE wxSize(425, 475)
#define SYMBOL_SETTINGSDLG_POSITION wxDefaultPosition
#define ID_CUSTOM 10018
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * SettingsDlg class declaration
 */

class SettingsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( SettingsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SettingsDlg( );
    SettingsDlg( wxWindow* parent, wxWindowID id = SYMBOL_SETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_SETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_SETTINGSDLG_POSITION, const wxSize& size = SYMBOL_SETTINGSDLG_SIZE, long style = SYMBOL_SETTINGSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_SETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_SETTINGSDLG_POSITION, const wxSize& size = SYMBOL_SETTINGSDLG_SIZE, long style = SYMBOL_SETTINGSDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin SettingsDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end SettingsDlg event handler declarations

////@begin SettingsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end SettingsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin SettingsDlg member variables
    wxPropertyGridManager* m_PropGrid;
////@end SettingsDlg member variables
};

#endif
    // _SETTINGSDLG_H_
