// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "AboutDlg.cpp"
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

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_ABOUTDLG_STYLE wxDEFAULT_DIALOG_STYLE
#define SYMBOL_ABOUTDLG_TITLE _("About")
#define SYMBOL_ABOUTDLG_IDNAME wxID_ANY
#define SYMBOL_ABOUTDLG_SIZE wxDefaultSize
#define SYMBOL_ABOUTDLG_POSITION wxDefaultPosition
#define abID_VERSION 10001
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
 * AboutDlg class declaration
 */

class AboutDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( AboutDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    AboutDlg( );
    AboutDlg( wxWindow* parent, wxWindowID id = SYMBOL_ABOUTDLG_IDNAME, const wxString& caption = SYMBOL_ABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_ABOUTDLG_POSITION, const wxSize& size = SYMBOL_ABOUTDLG_SIZE, long style = SYMBOL_ABOUTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ABOUTDLG_IDNAME, const wxString& caption = SYMBOL_ABOUTDLG_TITLE, const wxPoint& pos = SYMBOL_ABOUTDLG_POSITION, const wxSize& size = SYMBOL_ABOUTDLG_SIZE, long style = SYMBOL_ABOUTDLG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin AboutDlg event handler declarations

////@end AboutDlg event handler declarations

////@begin AboutDlg member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end AboutDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin AboutDlg member variables
   wxStaticText* m_RegInfo;
////@end AboutDlg member variables
};

#endif
    // _ABOUTDLG_H_
