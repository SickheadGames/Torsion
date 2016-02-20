// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _EXTERNALTOOLSDLG_H_
#define _EXTERNALTOOLSDLG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "ExternalToolsDlg.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes
#include "ToolCommand.h"


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define etID_TOOLLIST 10020
#define etID_CMDMENU 10024
#define etID_ARGMENU 10021
#define etID_DIRMENU 10022
#define SYMBOL_EXTERNALTOOLSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_EXTERNALTOOLSDLG_TITLE _("External Tools")
#define SYMBOL_EXTERNALTOOLSDLG_IDNAME wxID_ANY
#define SYMBOL_EXTERNALTOOLSDLG_SIZE wxSize(400, 400)
#define SYMBOL_EXTERNALTOOLSDLG_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * ExternalToolsDlg class declaration
 */

class ExternalToolsDlg: public wxDialog
{    
   DECLARE_DYNAMIC_CLASS( ExternalToolsDlg )
   DECLARE_EVENT_TABLE()

public:
   /// Constructors
   ExternalToolsDlg( );
   ExternalToolsDlg( wxWindow* parent, wxWindowID id = SYMBOL_EXTERNALTOOLSDLG_IDNAME, const wxString& caption = SYMBOL_EXTERNALTOOLSDLG_TITLE, const wxPoint& pos = SYMBOL_EXTERNALTOOLSDLG_POSITION, const wxSize& size = SYMBOL_EXTERNALTOOLSDLG_SIZE, long style = SYMBOL_EXTERNALTOOLSDLG_STYLE );

   /// Creation
   bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EXTERNALTOOLSDLG_IDNAME, const wxString& caption = SYMBOL_EXTERNALTOOLSDLG_TITLE, const wxPoint& pos = SYMBOL_EXTERNALTOOLSDLG_POSITION, const wxSize& size = SYMBOL_EXTERNALTOOLSDLG_SIZE, long style = SYMBOL_EXTERNALTOOLSDLG_STYLE );

   /// Creates the controls and sizers
   void CreateControls();

////@begin ExternalToolsDlg event handler declarations

   /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for etID_TOOLLIST
   void OnToolSelected( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_ADD
   void OnAddClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
   void OnDeleteClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for wxID_DELETE
   void OnDeleteUpdate( wxUpdateUIEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_UP
   void OnUpClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for wxID_UP
   void OnUpUpdate( wxUpdateUIEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DOWN
   void OnDownClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for wxID_DOWN
   void OnDownUpdate( wxUpdateUIEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for etID_CMDMENU
   void OnCmdMenuClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for etID_ARGMENU
   void OnArgMenuClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for etID_DIRMENU
   void OnDirMenuClick( wxCommandEvent& event );

   /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
   void OnOkClick( wxCommandEvent& event );

////@end ExternalToolsDlg event handler declarations

////@begin ExternalToolsDlg member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end ExternalToolsDlg member function declarations

   /// Should we show tooltips?
   static bool ShowToolTips();

////@begin ExternalToolsDlg member variables
   wxListBox* m_ToolList;
   wxTextCtrl* m_TitleCtrl;
   wxTextCtrl* m_CmdCtrl;
   wxTextCtrl* m_ArgCtrl;
   wxTextCtrl* m_DirCtrl;
////@end ExternalToolsDlg member variables

   void UpdateToolList();

   int m_LastSel;
   ToolCommandArray m_ToolCmds;
};

#endif
   // _EXTERNALTOOLSDLG_H_
