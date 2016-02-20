// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_SHNOTEBOOK_H
#define TORSION_SHNOTEBOOK_H
#pragma once


#define shNB_NOBUTTONS        0x1

enum
{
    shNB_HITTEST_NOWHERE = 1,   // not on tab
    shNB_HITTEST_ONICON  = 2,   // on icon
    shNB_HITTEST_ONLABEL = 4,   // on label
    shNB_HITTEST_ONBUTTON = 8,   // 0 = close, 1 = right, 2 = left
    shNB_HITTEST_ONITEM  = wxNB_HITTEST_ONICON | wxNB_HITTEST_ONLABEL
};

class WXDLLEXPORT shNotebookEvent : public wxNotifyEvent
{
public:

   shNotebookEvent( int id, wxEventType eventType )
       : wxNotifyEvent( eventType, id ),
         m_Selection( -1 ),
         m_OldSelection( -1 )
    {
    }

    virtual wxEvent* Clone() const
    {
       return new shNotebookEvent( *this );
    }

    void SetOldSelection( int sel ) { m_OldSelection = sel; }
    void SetSelection( int sel ) { m_Selection = sel; }

    int GetSelection() const { return m_Selection; }
    int GetOldSelection() const { return m_OldSelection; }

protected:

    shNotebookEvent( const shNotebookEvent& event )
       : wxNotifyEvent( event ),
         m_Selection( event.m_Selection ),
         m_OldSelection( event.m_OldSelection )
    {
    }

private:

   int   m_Selection;
   int   m_OldSelection;

};

DECLARE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGING, -1) 
DECLARE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGED, -1) 
DECLARE_EVENT_TYPE(EVT_COMMAND_SH_NOTEBOOK_PAGE_CLOSE, -1) 

typedef void (wxEvtHandler::*shNotebookEventFunction)(shNotebookEvent&);

#define shNotebookEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(shNotebookEventFunction, &func)

#define EVT_SH_NOTEBOOK_PAGE_CHANGING(winid, fn) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGING, winid, -1, shNotebookEventHandler(fn), NULL),
#define EVT_SH_NOTEBOOK_PAGE_CHANGED(winid, fn) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_COMMAND_SH_NOTEBOOK_PAGE_CHANGED, winid, -1, shNotebookEventHandler(fn), NULL),
#define EVT_SH_NOTEBOOK_PAGE_CLOSE(winid, fn) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_COMMAND_SH_NOTEBOOK_PAGE_CLOSE, winid, -1, shNotebookEventHandler(fn), NULL),


class shNotebook : public wxControl
{
   DECLARE_CLASS(shNotebook)

   shNotebook( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style = 0 );
   virtual ~shNotebook();

   public:

      int AddPage( wxWindow* page, const wxString& title, const wxString& tooltip = wxEmptyString, bool select = false, int imageId = -1 );

      bool RemovePage( int page );
      wxWindow* GetPage( int page ) const;
      wxWindow* GetCurrentPage() const;
      bool SetSelection( int page );
      int HitTest( const wxPoint& pt, long *flags = NULL );

      void SetPageText( int page, const wxString& title, const wxString& tooltip );
      const wxString& GetPageText( int page ) const;

      int GetPrevPageIndex() const;
      void DeleteAllPages();
      void AssignImageList( wxImageList* imageList );

      int GetSelection() const { return m_Focused; }
      int GetPageCount() const { return m_Pages.GetCount(); }
      int GetLastPageIndex() const { return m_Pages.GetCount() - 1; }

   protected:

      int GetLastVisible() const;
      void OffsetScroll( int Dir );

      virtual bool Destroy();

      void OnPaint( wxPaintEvent& event );
      void OnFocus( wxFocusEvent& event );
      void OnSize( wxSizeEvent& event );
      void OnEraseBg( wxEraseEvent& event );
      void OnScrollTimer( wxTimerEvent& event );
      void OnToolTip( wxTimerEvent& event );

      void OnLeftDown( wxMouseEvent& event );
      void OnLeftUp( wxMouseEvent& event );
      void OnMouseMove( wxMouseEvent& event );
      void OnMouseLeave( wxMouseEvent& event );
      void OnMiddleDown( wxMouseEvent& event );
      void OnMiddleUp( wxMouseEvent& event );

      struct PageInfo 
      {
         wxWindow*   m_Wnd;
         wxString    m_Title;
         wxString    m_ToolTip;
         int         m_ImageId;

         int         m_Width;
         int         m_ActiveWidth;
      };

      void CalcWidths( PageInfo* infos, int count = 1 );

      WX_DEFINE_ARRAY( PageInfo*, PageInfoArray );

      bool           m_HasPainted;

      int            m_Borders;

      wxFont*        m_TabFont;
      wxFont*        m_TabFontBold;

      wxRect         m_TabArea;
      wxRect         m_ClientArea;
      wxSize         m_TabSpacer;

      wxRect         m_ButtonArea;

      int            m_HoveredButton;

      int            m_Scroll;
      int            m_LastVisible;
      wxTimer        m_ScrollTimer;

      wxTimer        m_ToolTipTimer;
      wxString       m_ToolTipText;

      int            m_Focused;
      int            m_Active;
      PageInfoArray  m_Pages;

      DECLARE_EVENT_TABLE()
};

#endif // TORSION_SHNOTEBOOK_H