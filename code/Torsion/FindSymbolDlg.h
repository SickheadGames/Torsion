// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _FINDSYMBOLDLG_H_
#define _FINDSYMBOLDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "FindSymbolDlg.h"
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
#define SYMBOL_FINDSYMBOLDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FINDSYMBOLDLG_TITLE _("Find Symbol")
#define SYMBOL_FINDSYMBOLDLG_IDNAME wxID_ANY
#define SYMBOL_FINDSYMBOLDLG_SIZE wxSize(400, 300)
#define SYMBOL_FINDSYMBOLDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * FindSymbolDlg class declaration
 */

class FindSymbolDlg: public wxDialog
{    
   DECLARE_DYNAMIC_CLASS( FindSymbolDlg )
   DECLARE_EVENT_TABLE()

public:
   /// Constructors
   FindSymbolDlg( );
   FindSymbolDlg( wxWindow* parent, wxWindowID id = SYMBOL_FINDSYMBOLDLG_IDNAME, const wxString& caption = SYMBOL_FINDSYMBOLDLG_TITLE, const wxPoint& pos = SYMBOL_FINDSYMBOLDLG_POSITION, const wxSize& size = SYMBOL_FINDSYMBOLDLG_SIZE, long style = SYMBOL_FINDSYMBOLDLG_STYLE );

   /// Creation
   bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FINDSYMBOLDLG_IDNAME, const wxString& caption = SYMBOL_FINDSYMBOLDLG_TITLE, const wxPoint& pos = SYMBOL_FINDSYMBOLDLG_POSITION, const wxSize& size = SYMBOL_FINDSYMBOLDLG_SIZE, long style = SYMBOL_FINDSYMBOLDLG_STYLE );

   /// Initialises member variables
   void Init();

   /// Creates the controls and sizers
   void CreateControls();

////@begin FindSymbolDlg event handler declarations

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
   void OnFindClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for wxID_OK
   void OnFindUpdate( wxUpdateUIEvent& event );

////@end FindSymbolDlg event handler declarations

    void AddWhat( const wxString& what );

////@begin FindSymbolDlg member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end FindSymbolDlg member function declarations

   /// Should we show tooltips?
   static bool ShowToolTips();

////@begin FindSymbolDlg member variables
   wxComboBox* m_WhatCtrl;
////@end FindSymbolDlg member variables
   wxString m_What;
};

#endif
   // _FINDSYMBOLDLG_H_
