/** @file
  Header file for Ami input translators.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#ifndef _AMI_SHIM_H_
#define _AMI_SHIM_H_

//#include <AppleUefi.h>

#include <IndustryStandard/AppleHid.h>

#include <Protocol/AppleKeyMapDatabase.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/Timer.h>

#include <Library/DebugLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiUsbLib.h>
#include <Library/HiiLib.h>

EFI_STATUS
EFIAPI
AmiShimTranslatorEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
AmiShimPointerInit (
  VOID
  );

EFI_STATUS
EFIAPI
AmiShimPointerExit (
  VOID
  );

EFI_STATUS
EFIAPI
AmiShimKeycodeInit (
  VOID
  );

EFI_STATUS
EFIAPI
AmiShimKeycodeExit (
  VOID
  );

enum {
  AmiShimRightShift,
  AmiShimLeftShift,
  AmiShimRightControl,
  AmiShimLeftControl,
  AmiShimRightAlt,
  AmiShimLeftAlt,
  AmiShimRightGui,
  AmiShimLeftGui,
  AmiShimModifierMax
};

extern APPLE_MODIFIER_MAP         gModifierRemap[AmiShimModifierMax];

#endif

