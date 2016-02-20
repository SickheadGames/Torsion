// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef _PROJECTPANEL_H_
#define _PROJECTPANEL_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "ProjectPanel.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/toolbar.h"
////@end includes
#include "ProjectCtrl.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class ProjectCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_PROJECTPANEL_STYLE wxCLIP_CHILDREN
#define SYMBOL_PROJECTPANEL_TITLE _("Project")
#define SYMBOL_PROJECTPANEL_IDNAME wxID_ANY
#define SYMBOL_PROJECTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_PROJECTPANEL_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * ProjectPanel class declaration
 */

class ProjectPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( ProjectPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ProjectPanel( );
    ProjectPanel( wxWindow* parent, wxWindowID id = SYMBOL_PROJECTPANEL_IDNAME, const wxPoint& pos = SYMBOL_PROJECTPANEL_POSITION, const wxSize& size = SYMBOL_PROJECTPANEL_SIZE, long style = SYMBOL_PROJECTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_PROJECTPANEL_IDNAME, const wxPoint& pos = SYMBOL_PROJECTPANEL_POSITION, const wxSize& size = SYMBOL_PROJECTPANEL_SIZE, long style = SYMBOL_PROJECTPANEL_STYLE );


    ProjectCtrl* GetCtrl() const { return m_ProjectCtrl; }

protected:

    /// Creates the controls and sizers
    void CreateControls();

////@begin ProjectPanel event handler declarations

////@end ProjectPanel event handler declarations

////@begin ProjectPanel member function declarations

   /// Retrieves bitmap resources
   wxBitmap GetBitmapResource( const wxString& name );

   /// Retrieves icon resources
   wxIcon GetIconResource( const wxString& name );
////@end ProjectPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ProjectPanel member variables
   ProjectCtrl* m_ProjectCtrl;
////@end ProjectPanel member variables
};

#endif
    // _PROJECTPANEL_H_
