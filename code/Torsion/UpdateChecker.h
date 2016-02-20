// Torsion TorqueScript IDE - Copyright (C) Sickhead Games, LLC
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.

#ifndef TORSION_UPDATECHECKER_H
#define TORSION_UPDATECHECKER_H
#pragma once


class UpdateChecker
{
public:

   UpdateChecker();
   virtual ~UpdateChecker();

   bool CheckAvailable( wxString* version );

};

#endif // TORSION_UPDATECHECKER_H
