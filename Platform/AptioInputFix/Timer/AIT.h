/** @file
  Timer booster

Copyright (c) 2018, vit9696. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef AIT_SELF_H
#define AIT_SELF_H

#include <Library/UefiLib.h>

//
// AMI has EFI_TIMER_PERIOD_MILLISECONDS(5) here, but we sync with AppleEvent.
//
#define AIT_TIMER_PERIOD  EFI_TIMER_PERIOD_MILLISECONDS(10)

EFI_STATUS
AITInit (
  VOID
  );

EFI_STATUS
AITExit (
  VOID
  );

#endif
