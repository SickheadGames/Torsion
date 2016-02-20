// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_IMAGEUTIL_H
#define TORSION_IMAGEUTIL_H
#pragma once

#include <wx/image.h>


void tsColorizeImage( wxImage& image, const wxColour& black, const wxColour& white = *wxWHITE );


#endif // TORSION_IMAGEUTIL_H