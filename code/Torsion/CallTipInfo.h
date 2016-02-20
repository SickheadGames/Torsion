// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_CALLTIPINFO_H
#define TORSION_CALLTIPINFO_H
#pragma once


class CallTipInfo
{
   public:

      CallTipInfo( int tabSize = 3 );
      ~CallTipInfo();

      void SetTip( const wxString& decl, const wxString& desc );

      const wxString& GetTip() const { return m_FormattedTip; }

      bool IsEmpty() const { return m_FormattedTip.IsEmpty(); }

      operator const wxString&() const { return m_FormattedTip; }

      bool GetArgumentRange( int param, int* start, int* end ) const;

      void RemoveThis();

      static wxString FormatTip( const wxString& tip, int tabSize );

   protected:

      const int      m_TabSize;

      wxString       m_FormattedTip;
      wxString       m_Tip;
      wxString       m_Description;

      struct Arg
      {
         int start;
         int end;
      };

      WX_DEFINE_ARRAY( Arg*, ArgArray );
      ArgArray m_Args;
};

WX_DEFINE_ARRAY( CallTipInfo*, CallTipInfoArray );


#endif // TORSION_CALLTIPINFO_H
