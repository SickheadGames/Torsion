// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _BREAKPOINTPROPERTYDLG_H_
#define _BREAKPOINTPROPERTYDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BreakpointPropertyDlg.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/spinctrl.h"
////@end includes

/*!
 * Forward declarations
 */
class Breakpoint;


////@begin forward declarations
class wxSpinCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_BREAKPOINTPROPERTYDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BREAKPOINTPROPERTYDLG_TITLE _("Breakpoint Properties")
#define SYMBOL_BREAKPOINTPROPERTYDLG_IDNAME wxID_ANY
#define SYMBOL_BREAKPOINTPROPERTYDLG_SIZE wxSize(400, 300)
#define SYMBOL_BREAKPOINTPROPERTYDLG_POSITION wxDefaultPosition
#define bpID_BROWSE 10015
#define bpID_CONDITIONCHECK 10010
#define bpID_PASSCHECK 10009
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * BreakpointPropertyDlg class declaration
 */

class BreakpointPropertyDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( BreakpointPropertyDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    BreakpointPropertyDlg( );

    /// Creation
    bool Create( wxWindow* parent, Breakpoint* bp );

    /// Creates the controls and sizers
    void CreateControls();

////@begin BreakpointPropertyDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for bpID_BROWSE
    void OnBrowseClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for bpID_CONDITIONCHECK
    void OnConditioncheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for bpID_PASSCHECK
    void OnPasscheckClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end BreakpointPropertyDlg event handler declarations

////@begin BreakpointPropertyDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end BreakpointPropertyDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    Breakpoint* m_Breakpoint;

////@begin BreakpointPropertyDlg member variables
    wxTextCtrl* m_File;
    wxSpinCtrl* m_Line;
    wxCheckBox* m_ConditionCheck;
    wxTextCtrl* m_Condition;
    wxCheckBox* m_PassCheck;
    wxSpinCtrl* m_Pass;
////@end BreakpointPropertyDlg member variables
};

#endif
    // _BREAKPOINTPROPERTYDLG_H_
