// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#include "PreCompiled.h"
#include "tsMenu.h"
#include "Icons.h"


#ifdef __WXMSW__
   #include "VSMenuItem.h"
   #define WXMENUITEM VSMenuItem  
#else
   #define WXMENUITEM wxMenuItem  
#endif

#ifdef _DEBUG 
   #define new DEBUG_NEW 
#endif 

tsMenuArray tsMenu::s_AllMenus;


tsMenu::tsMenu( const wxString& title, long style )
   : wxMenu( title, style )
{
   wxASSERT( s_AllMenus.Index( this ) == wxNOT_FOUND );
   s_AllMenus.Add( this );
}


tsMenu::tsMenu( long style )
   : wxMenu( style )
{
   wxASSERT( s_AllMenus.Index( this ) == wxNOT_FOUND );
   s_AllMenus.Add( this );
}


tsMenu::~tsMenu()
{
   wxASSERT( s_AllMenus.Index( this ) != wxNOT_FOUND );
   s_AllMenus.Remove( this );
}


wxMenuItem* tsMenu::NewMenuItem( int itemid,
                           const wxString& text,
                           const wxString& help,
                           wxItemKind kind,
                           wxMenu *subMenu )
{
   return new WXMENUITEM( this, itemid, text, help, kind, subMenu);  
}

wxMenuItem* tsMenu::AppendIconItem( int id, const wxString& text, const wxBitmap& bmp, const wxString& helpString )
{
   WXMENUITEM* menuItem = new WXMENUITEM( this, id, text, helpString, wxITEM_NORMAL );
   menuItem->SetBitmap( bmp );
   return wxMenu::Append( menuItem );
}

wxMenuItem* tsMenu::AppendCheckIconItem( int id, const wxString& text, const wxBitmap& bmp, const wxString& helpString )
{
   WXMENUITEM* menuItem = new WXMENUITEM( this, id, text, helpString, wxITEM_CHECK );
   menuItem->SetBitmaps( bmp, bmp );
   return wxMenu::Append( menuItem );
}

void tsMenu::OnSysColourChanged()
{
   // Loop thru each menu... remove the disabled 
   // bitmap from each menu item.
   for ( size_t i=0; i < s_AllMenus.GetCount(); i++ )
   {
      tsMenu* menu = s_AllMenus[i];
      wxASSERT( menu );

      wxMenuItemList::Node* node = menu->GetMenuItems().GetFirst();
      for ( ; node; node = node->GetNext() )
      {
         WXMENUITEM* item = wxDynamicCast( node->GetData(), WXMENUITEM );
         if ( item && item->GetDisabledBitmap().Ok() )
            item->SetDisabledBitmap( wxNullBitmap );
      }
   }
}


