// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _BUILDEXPORTSDLG_H_
#define _BUILDEXPORTSDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "BuildExportsDlg.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class BuildExportsThread;
class ProjectConfigArray;
class AutoCompExports;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10008
#define SYMBOL_BUILDEXPORTSDLG_STYLE wxCAPTION
#define SYMBOL_BUILDEXPORTSDLG_TITLE _("Build Exports")
#define SYMBOL_BUILDEXPORTSDLG_IDNAME ID_DIALOG
#define SYMBOL_BUILDEXPORTSDLG_SIZE wxSize(400, 300)
#define SYMBOL_BUILDEXPORTSDLG_POSITION wxDefaultPosition
#define ID_GAUGE 10016
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * BuildExportsDlg class declaration
 */

class BuildExportsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( BuildExportsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    BuildExportsDlg( );
    BuildExportsDlg( wxWindow* parent, wxWindowID id = SYMBOL_BUILDEXPORTSDLG_IDNAME, const wxString& caption = SYMBOL_BUILDEXPORTSDLG_TITLE, const wxPoint& pos = SYMBOL_BUILDEXPORTSDLG_POSITION, const wxSize& size = SYMBOL_BUILDEXPORTSDLG_SIZE, long style = SYMBOL_BUILDEXPORTSDLG_STYLE );

    virtual ~BuildExportsDlg();

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BUILDEXPORTSDLG_IDNAME, const wxString& caption = SYMBOL_BUILDEXPORTSDLG_TITLE, const wxPoint& pos = SYMBOL_BUILDEXPORTSDLG_POSITION, const wxSize& size = SYMBOL_BUILDEXPORTSDLG_SIZE, long style = SYMBOL_BUILDEXPORTSDLG_STYLE );

    int ShowModal(   const wxString& exportScriptPath,
                     const wxString& workingDir,
                     const ProjectConfigArray& configs );

    AutoCompExports* TakeExports();

protected:

    /// Creates the controls and sizers
    void CreateControls();

////@begin BuildExportsDlg event handler declarations

    /// wxEVT_IDLE event handler for ID_DIALOG
    void OnIdle( wxIdleEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancel( wxCommandEvent& event );

////@end BuildExportsDlg event handler declarations

    void OnTimer( wxTimerEvent& event );

////@begin BuildExportsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end BuildExportsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin BuildExportsDlg member variables
    wxGauge* m_Progress;
////@end BuildExportsDlg member variables

    wxTimer    m_Timer;

    AutoCompExports* m_Exports;

    BuildExportsThread* m_Thread;
};

#endif
    // _BUILDEXPORTSDLG_H_
