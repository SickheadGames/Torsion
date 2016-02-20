// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _FINDREPLACEDLG_H_
#define _FINDREPLACEDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "FindReplaceDlg.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */
class wxFindReplaceData;

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_FINDREPLACEDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FINDREPLACEDLG_TITLE _("Replace")
#define SYMBOL_FINDREPLACEDLG_IDNAME wxID_ANY
#define SYMBOL_FINDREPLACEDLG_SIZE wxSize(400, 300)
#define SYMBOL_FINDREPLACEDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * FindReplaceDlg class declaration
 */

class FindReplaceDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( FindReplaceDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    FindReplaceDlg( );
    FindReplaceDlg( wxWindow* parent, wxFindReplaceData* data, wxWindowID id = SYMBOL_FINDREPLACEDLG_IDNAME, const wxString& caption = SYMBOL_FINDREPLACEDLG_TITLE, const wxPoint& pos = SYMBOL_FINDREPLACEDLG_POSITION, const wxSize& size = SYMBOL_FINDREPLACEDLG_SIZE, long style = SYMBOL_FINDREPLACEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxFindReplaceData* data, wxWindowID id = SYMBOL_FINDREPLACEDLG_IDNAME, const wxString& caption = SYMBOL_FINDREPLACEDLG_TITLE, const wxPoint& pos = SYMBOL_FINDREPLACEDLG_POSITION, const wxSize& size = SYMBOL_FINDREPLACEDLG_SIZE, long style = SYMBOL_FINDREPLACEDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool Destroy();

    bool ShowFind( bool show = true );
    bool ShowReplace( bool show = true );
    virtual bool Show( bool show = true );

////@begin FindReplaceDlg event handler declarations

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FIND
   void OnFindClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for wxID_FIND
   void OnFindUpdate( wxUpdateUIEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REPLACE
   void OnReplaceClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_REPLACE_ALL
   void OnReplaceAllClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
   void OnCloseClick( wxCommandEvent& event );

////@end FindReplaceDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

////@begin FindReplaceDlg member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end FindReplaceDlg member function declarations

   wxFindReplaceData* m_Data;

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin FindReplaceDlg member variables
   wxComboBox* m_WhatCtrl;
   wxBoxSizer* m_WithSizer;
   wxComboBox* m_WithCtrl;
   wxCheckBox* m_MatchCaseCtrl;
   wxCheckBox* m_WholeWordCtrl;
   wxCheckBox* m_SearchUpCtrl;
   wxBoxSizer* m_ButtonSizer;
////@end FindReplaceDlg member variables
};

#endif
    // _FINDREPLACEDLG_H_
