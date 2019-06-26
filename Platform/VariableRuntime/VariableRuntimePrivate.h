/** @file
  Copyright (C) 2019, vit9696. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef VARIABLE_RUNTIME_PRIVATE_H
#define VARIABLE_RUNTIME_PRIVATE_H

VOID
RedirectRuntimeServices (
  VOID
  );

VOID
SetWriteUnprotectorMode (
  IN BOOLEAN  Enable
  );

BOOLEAN
EFIAPI
SetBootVariableRedirect (
  IN BOOLEAN  Enable
  );

EFI_STATUS
EFIAPI
SetCustomGetVariableHandler (
  IN  EFI_GET_VARIABLE  GetVariable,
  OUT EFI_GET_VARIABLE  *OrgGetVariable  OPTIONAL
  );


#endif // VARIABLE_RUNTIME_PRIVATE_H
