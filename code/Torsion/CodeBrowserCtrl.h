// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_CODEBROWSERCTRL_H
#define TORSION_CODEBROWSERCTRL_H
#pragma once

#include <wx/treectrl.h>
#include <wx/datetime.h>
#include <map>


class AutoCompPage;
class AutoCompClass;
class AutoCompClassArray;
class AutoCompFunctionArray;
class AutoCompVarArray;
class AutoCompVar;
class AutoCompData;

class CodeBrowserCtrl : public wxTreeCtrl
{
   DECLARE_CLASS( CodeBrowserCtrl )

   public:

      CodeBrowserCtrl( wxWindow* parent, wxWindowID id, 
         const wxPoint& pos, const wxSize& size );
      virtual ~CodeBrowserCtrl();

      enum
      {
         FILTER_CLASSES    = (1<<0),
         FILTER_DATABLOCKS = (1<<1),
         FILTER_FUNCTIONS  = (1<<2),
         FILTER_GLOBALS    = (1<<3),
         FILTER_NAMESPACES = (1<<4),

         FILTER_DATABLOCKOBJECT  = FILTER_DATABLOCKS | FILTER_GLOBALS,
         FILTER_OBJECT           = FILTER_CLASSES | FILTER_GLOBALS,

         FILTER_ALL        =  FILTER_CLASSES | 
                              FILTER_DATABLOCKS | 
                              FILTER_FUNCTIONS | 
                              FILTER_GLOBALS | 
                              FILTER_NAMESPACES
      };

      void SetFilterValue( bool enabled, unsigned int bit );
      unsigned int GetFilterValue() const { return m_Filter; }

      void SetDescCtrl( wxTextCtrl* descCtrl ) { m_DescCtrl = descCtrl; }

   protected:

      void OnItemExpanding( wxTreeEvent& event );
      //void OnItemCollapsing( wxTreeEvent& event );
      void OnRightClickItem( wxTreeEvent& event );
	   void OnItemActivated( wxTreeEvent& event );
      void OnSelChanged( wxTreeEvent& event );

      // To fix flicker!
      bool m_DonePaint;
      #ifdef __WXMSW__
         void OnEraseBackground( wxEraseEvent& event );
         void OnPaint( wxPaintEvent& event );
      #endif // __WXMSW__

      void OnDataUpdated( wxCommandEvent& event );
      void UpdateData();

      void OnSetFocus( wxFocusEvent& event );
      void OnKillFocus( wxFocusEvent& event );

      virtual bool Enable( bool enable = true );

      #ifdef __WXMSW__
         virtual bool MSWOnNotify( int idCtrl, WXLPARAM lParam, WXLPARAM* result );
      #endif

      /*
      void RefreshRoot();


      void Select( const wxString& path );

      void OpenItem( const wxTreeItemId& item );
      void OpenItemWith( const wxTreeItemId& item );

      wxString GetSelectedPath() const;
      */

      //void OnToolTip( wxTreeEvent& Event );

      void OnInternalIdle();

   protected:

      enum ITYPE {
         ITYPE_NONE = -1,
         ITYPE_VAR,
         ITYPE_FUNC,
         ITYPE_CLASS,
         ITYPE_DATABLOCK,
         ITYPE_OBJECT,
         ITYPE_DATABLOCKOBJECT,
         ITYPE_NAMESPACE,
         ITYPE_PACKAGE,
      };

      int OnCompareItems( const wxTreeItemId& item1, const wxTreeItemId& item2 );

      void MergeFunctions( wxTreeItemId parent, const AutoCompFunctionArray& functions, const wxString& namespace_ );
      void MergeVars( wxTreeItemId parent, const AutoCompVarArray& vars, bool globalsOnly );
      void MergeVar( wxTreeItemId parent, const AutoCompVar* var );
      wxTreeItemId MergeClass( wxTreeItemId parent, const AutoCompClass* class_, ITYPE type );
      void MergeClassMembers( wxTreeItemId parent, const AutoCompClass* class_ );

      wxTreeItemId ReviveItemFromCache( const wxString& sid );
      wxTreeItemId AppendItem( const wxString& sid, wxTreeItemId parent, const wxString& name, ITYPE type );

      static inline void MakeSid( wxString* sid, const wxString& parentSid, const wxString& name, ITYPE type )
      {
         wxASSERT( sid );
         *sid = parentSid;
         *sid << name << "?" << (int)type;
         sid->MakeLower();
      }

      inline const wxString& GetSid( wxTreeItemId item ) const
      {
         wxASSERT( item.IsOk() );

         ItemData* data = (ItemData*)GetItemData( item );
         wxASSERT( data );
         return data->sid;
      }

      class ItemData : public wxTreeItemData
      {
      public:

         ItemData() : 
            wxTreeItemData(),
            type( ITYPE_NONE ),
            base( false )
         {}

         wxString name;
         wxString sid;
         ITYPE    type;
         bool     base;
      };

      typedef std::map<wxString,wxTreeItemId> StringToIdMap;
      //typedef std::map<wxTreeItemId,wxString> IdToStringMap;

      StringToIdMap  m_CurrentIds;
      StringToIdMap  m_CachedIds;

      wxColor        m_ExportColor;

      bool           m_IsActivate;
      bool           m_IsActive;
      bool           m_NeedUpdate;

      wxTextCtrl*    m_DescCtrl;

      const AutoCompData* m_Data;

      unsigned int   m_Filter;

      DECLARE_EVENT_TABLE()
};

#endif // TORSION_CODEBROWSERCTRL_H