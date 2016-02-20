// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _FINDINFILESDLG_H_
#define _FINDINFILESDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "FindInFilesDlg.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

class FindThread;

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FINDINFILESDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FINDINFILESDLG_TITLE _("Find in Files")
#define SYMBOL_FINDINFILESDLG_IDNAME wxID_ANY
#define SYMBOL_FINDINFILESDLG_SIZE wxSize(266, 184)
#define SYMBOL_FINDINFILESDLG_POSITION wxDefaultPosition
#define ffID_BROWSE 10012
#define ffID_MATCHCASE 10013
#define ffID_WHOLEWORD 10014
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
 * FindInFilesDlg class declaration
 */

class FindInFilesDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( FindInFilesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    FindInFilesDlg();
    FindInFilesDlg( wxWindow* parent, wxWindowID id = SYMBOL_FINDINFILESDLG_IDNAME, const wxString& caption = SYMBOL_FINDINFILESDLG_TITLE, const wxPoint& pos = SYMBOL_FINDINFILESDLG_POSITION, const wxSize& size = SYMBOL_FINDINFILESDLG_SIZE, long style = SYMBOL_FINDINFILESDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FINDINFILESDLG_IDNAME, const wxString& caption = SYMBOL_FINDINFILESDLG_TITLE, const wxPoint& pos = SYMBOL_FINDINFILESDLG_POSITION, const wxSize& size = SYMBOL_FINDINFILESDLG_SIZE, long style = SYMBOL_FINDINFILESDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin FindInFilesDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ffID_BROWSE
    void OnBrowseClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnFindClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_OK
    void OnFindUpdate( wxUpdateUIEvent& event );

////@end FindInFilesDlg event handler declarations

////@begin FindInFilesDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end FindInFilesDlg member function declarations

    void AddLookIn( const wxString& path );
    void AddWhat( const wxString& what );
    void AddType( const wxString& type );

    wxArrayString GetPathsFromLookIn( const wxString& lookin ) const;

    wxString GetWhat() const;
    wxString GetLookIn() const;
    wxString GetTypes() const;

    FindThread* m_FindThread;

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin FindInFilesDlg member variables
    wxComboBox* m_WhatCtrl;
    wxComboBox* m_LookInCtrl;
    wxComboBox* m_TypeCtrl;
    wxCheckBox* m_MatchCaseCtrl;
    wxCheckBox* m_WholeWordCtrl;
////@end FindInFilesDlg member variables
};

#endif
    // _FINDINFILESDLG_H_
