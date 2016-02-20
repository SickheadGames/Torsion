// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _GOTODEFDIALOG_H_
#define _GOTODEFDIALOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "GotoDefDialog.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/listctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListView;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_GOTODEFDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_GOTODEFDIALOG_TITLE _("Go To Definition")
#define SYMBOL_GOTODEFDIALOG_IDNAME wxID_ANY
#define SYMBOL_GOTODEFDIALOG_SIZE wxSize(475, 200)
#define SYMBOL_GOTODEFDIALOG_POSITION wxDefaultPosition
#define ID_SYMBOLSLIST 10023
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * GotoDefDialog class declaration
 */

class GotoDefDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( GotoDefDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    GotoDefDialog( );
    GotoDefDialog( wxWindow* parent, wxWindowID id = SYMBOL_GOTODEFDIALOG_IDNAME, const wxString& caption = SYMBOL_GOTODEFDIALOG_TITLE, const wxPoint& pos = SYMBOL_GOTODEFDIALOG_POSITION, const wxSize& size = SYMBOL_GOTODEFDIALOG_SIZE, long style = SYMBOL_GOTODEFDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_GOTODEFDIALOG_IDNAME, const wxString& caption = SYMBOL_GOTODEFDIALOG_TITLE, const wxPoint& pos = SYMBOL_GOTODEFDIALOG_POSITION, const wxSize& size = SYMBOL_GOTODEFDIALOG_SIZE, long style = SYMBOL_GOTODEFDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    int ShowModal(   const wxArrayString& paths,
                     const wxArrayInt& lines,
                     const wxArrayString& symbols );

    wxString   GetPath() const;
    int        GetLine() const;

protected:

////@begin GotoDefDialog event handler declarations

    /// wxEVT_COMMAND_LIST_ITEM_ACTIVATED event handler for ID_SYMBOLSLIST
    void OnSymbolDoubleClicked( wxListEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_OK
    void OnOkUpdate( wxUpdateUIEvent& event );

////@end GotoDefDialog event handler declarations

////@begin GotoDefDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end GotoDefDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin GotoDefDialog member variables
    wxListView* m_SymbolList;
////@end GotoDefDialog member variables

      wxArrayString     m_Symbols;
      wxArrayString     m_Paths;
      wxArrayInt        m_Lines;
      int               m_Selected;
};

#endif
    // _GOTODEFDIALOG_H_
