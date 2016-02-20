// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _CODEBROWSERPANEL_H_
#define _CODEBROWSERPANEL_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "CodeBrowserPanel.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/toolbar.h"
#include "CodeBrowserCtrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CodeBrowserCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CODEBROWSERPANEL_STYLE wxCLIP_CHILDREN
#define SYMBOL_CODEBROWSERPANEL_TITLE _("Code Browser")
#define SYMBOL_CODEBROWSERPANEL_IDNAME wxID_ANY
#define SYMBOL_CODEBROWSERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CODEBROWSERPANEL_POSITION wxDefaultPosition
#define ID_CLASSES 10001
#define ID_DATABLOCKS 10002
#define ID_FUNCTIONS 10003
#define ID_GLOBALS 10004
#define ID_NAMESPACES 10005
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * CodeBrowserPanel class declaration
 */

class CodeBrowserPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CodeBrowserPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CodeBrowserPanel( );
    CodeBrowserPanel( wxWindow* parent, wxWindowID id = SYMBOL_CODEBROWSERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CODEBROWSERPANEL_POSITION, const wxSize& size = SYMBOL_CODEBROWSERPANEL_SIZE, long style = SYMBOL_CODEBROWSERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CODEBROWSERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CODEBROWSERPANEL_POSITION, const wxSize& size = SYMBOL_CODEBROWSERPANEL_SIZE, long style = SYMBOL_CODEBROWSERPANEL_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CodeBrowserPanel event handler declarations

   /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_CLASSES
   void OnFilterClick( wxCommandEvent& event );

   /// wxEVT_UPDATE_UI event handler for ID_CLASSES
   void OnFilterUpdate( wxUpdateUIEvent& event );

////@end CodeBrowserPanel event handler declarations

////@begin CodeBrowserPanel member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end CodeBrowserPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CodeBrowserPanel member variables
   CodeBrowserCtrl* m_CodeTree;
   wxTextCtrl* m_DescCtrl;
////@end CodeBrowserPanel member variables
};

#endif
    // _CODEBROWSERPANEL_H_
