// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_FILETYPEIMAGELIST_H
#define TORSION_FILETYPEIMAGELIST_H
#pragma once

#include <wx/imaglist.h>

WX_DECLARE_STRING_HASH_MAP( int, StringToIntMap );

class FileTypeImageList : public wxImageList
{
public:

	FileTypeImageList();
   virtual ~FileTypeImageList();

public:

   int AddFileIcon( const wxString& file, wxIcon& icon );
   int AddFileIcon( const wxString& file );

public:

   StringToIntMap m_TypeToIndex;
};


#endif // TORSION_FILETYPEIMAGELIST_H
