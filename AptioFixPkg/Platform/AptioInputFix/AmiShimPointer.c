/** @file
  AmiEfiPointer to EfiPointer translator.

Copyright (c) 2016, CupertinoNet. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AmiShim.h"
#include "AmiPointer.h"
#include "AmiShimPointer.h"

//#include <MiscBase.h>

STATIC AMI_SHIM_POINTER mAmiShimPointer;

STATIC UINT8 gIndex = 0;
STATIC UINT8 gXCounts[5] = {0, 0, 0, 0, 0};
STATIC UINT8 gYCounts[5] = {0, 0, 0, 0, 0};

UINT8
EFIAPI
Abs (
  IN INT32 Value
  )
{
  if (Value < 0)
    return -Value;
  return Value;
}

INT32
EFIAPI
InternalClamp (
  IN INT32 Value,
  IN INT32 Min,
  IN INT32 Max
  )
{
  if (Value > Max) {
    return Max;
  } else if (Value < Min) {
    return Min;
  }
  return Value;
}

VOID
EFIAPI
AmiShimPointerFilterOut (
  IN OUT UINT8 *AbsX,
  IN OUT UINT8 *AbsY,
  IN OUT INT32 *X, 
  IN OUT INT32 *Y
  )
{
  UINT8 Index;

  if (*AbsX == 0) {
    gXCounts[gIndex] = 1;
  } else if (*AbsX <= 2) {
    for (Index = 0; Index < 5; Index++) {
      if (!gXCounts[Index])
        break;
    }

    if (Index == 5) {
      *AbsX = 0;
      *X = 0;
    }
    
    gXCounts[gIndex] = 0;
  }

  if (*AbsY == 0) {
    gYCounts[gIndex] = 1;
  } else if (*AbsY <= 2) {
    for (Index = 0; Index < 5; Index++) {
      if (!gYCounts[Index])
        break;
    }

    if (Index == 5) {
      *AbsY = 0;
      *Y = 0;
    }
    
    gYCounts[gIndex] = 0;
  }

  gIndex = gIndex < 4 ? gIndex + 1 : 0;
}

#ifdef AMI_SHIM_POINTER_AMI_SMOOTHING

STATIC UINT8 gAbsRange[8] = {42, 36, 30, 24, 18, 12, 6, 1};
STATIC INT32 gMultipliers[8] = {8, 7, 6, 5, 4, 3, 2, 1};
STATIC INT32 gIndices[8] = {2, 2, 2, 2, 2, 2, 2, 0};

INT32
AmiShimPointerScale (
  IN INT32 Value,
  IN UINT8 AbsValue
  )
{
  UINT8 TmpIndex;

  for (TmpIndex = 0; TmpIndex < 8; TmpIndex++) {
    if (AbsValue >= gAbsRange[TmpIndex]) {
      break;
    }
  }

  if (TmpIndex != 8) {
    if (Value >= 0) {
      Value = gIndices[TmpIndex] + Value * gMultipliers[TmpIndex];
    } else {
      Value = Value * gMultipliers[TmpIndex] - gIndices[TmpIndex];
    }
  }

  return Value;
}

VOID
EFIAPI
AmiShimPointerSmooth (
  IN OUT INT32   *X,
  IN OUT INT32   *Y,
  IN OUT INT32   *Z
  )
{
  UINT8 AbsX, AbsY;
  
  *X = Clamp (*X, -16, 16);
  *Y = Clamp (*Y, -16, 16);
  // According to AMI it should not be reported
  *Z = 0;
  AbsX = Abs (*X);
  AbsY = Abs (*Y);

  if (*X == 0 && *Y == 0) {
    return;
  }

  AmiShimPointerFilterOut (&AbsX, &AbsY, X, Y);

  *X = AmiShimPointerScale(*X, AbsX);
  *Y = AmiShimPointerScale(*Y, AbsY);

  *X *= SCALE_FACTOR;
  *Y *= SCALE_FACTOR;
}

#else

STATIC UINT8 gAbsRange[4] = {80, 64, 48, 1};
STATIC INT32 gMultipliers[4] = {4, 3, 2, 1};

VOID
EFIAPI
AmiShimPointerBoostValue (
  IN OUT INT32   *Value,
  IN INT32       AbsValue
  )
{
  UINTN Index;
  for (Index = 0; Index < 4; Index++) {
    if (gAbsRange[Index] > AbsValue) {
      *Value *= gMultipliers[Index];
      return;
    }
  }
}

VOID
EFIAPI
AmiShimPointerSmooth (
  IN OUT INT32   *X,
  IN OUT INT32   *Y,
  IN OUT INT32   *Z
  )
{
  UINT8 AbsX, AbsY;

  *X = InternalClamp(*X, -96, 96);
  *Y = InternalClamp(*Y, -96, 96);
  *Z = 0;

  AbsX = Abs (*X);
  AbsY = Abs (*Y);

  if (*X == 0 && *Y == 0) {
    return;
  }

  AmiShimPointerFilterOut (&AbsX, &AbsY, X, Y);

  AmiShimPointerBoostValue(X, AbsX);
  AmiShimPointerBoostValue(Y, AbsY);
}

#endif

VOID
EFIAPI
AmiShimPointerPositionHandler (
  IN EFI_EVENT  Event, 
  IN VOID       *Context
  )
{
  UINTN                             Index;
  AMI_SHIM_POINTER_INSTANCE         *Pointer;
  AMI_POINTER_POSITION_STATE_DATA   PositionState;

  // Do not poll until somebody actually starts using the mouse
  // Otherwise first move will be quite random
  if (!mAmiShimPointer.UsageStarted) {
    return;
  }

  // This is important to do quickly and separately, because AMI stores positioning data in INT8.
  // If we move the mouse quickly it will overflow and return invalid data.
  for (Index = 0; Index < MAX_POINTERS; Index++) {
    Pointer = &mAmiShimPointer.PointerMap[Index];
    if (Pointer->DeviceHandle != NULL) {
      PositionState.Changed = 0;
      Pointer->EfiPointer->GetPositionState (Pointer->EfiPointer, &PositionState);
      if (PositionState.Changed == 1) {
        if (PositionState.Absolute == 0) {
          //DEBUG ((EFI_D_ERROR, "Position: %d %d %d %d\n",
          //       PositionState.Changed, PositionState.PositionX, PositionState.PositionY, PositionState.PositionZ));
          AmiShimPointerSmooth(&PositionState.PositionX, &PositionState.PositionY, &PositionState.PositionZ);
          if (PositionState.PositionX != 0 || PositionState.PositionY != 0 || PositionState.PositionZ != 0) {
            Pointer->PositionX += PositionState.PositionX;
            Pointer->PositionY += PositionState.PositionY;
            Pointer->PositionZ += PositionState.PositionZ;
            Pointer->PositionChanged = TRUE;
          }
        } else {
          //FIXME: Add support for devices with absolute positioning
        }
      }
    }
  }
}

EFI_STATUS
EFIAPI
AmiShimPointerUpdateState (
  IN AMI_SHIM_POINTER_INSTANCE    *Pointer,
  OUT EFI_SIMPLE_POINTER_STATE    *State
  )
{
  AMI_POINTER_BUTTON_STATE_DATA     ButtonState;

  if (!mAmiShimPointer.UsageStarted) {
    mAmiShimPointer.UsageStarted = TRUE;
    AmiShimPointerPositionHandler(NULL, NULL);
  }

  ButtonState.Changed = 0;
  Pointer->EfiPointer->GetButtonState (Pointer->EfiPointer, &ButtonState);

  if (ButtonState.Changed == 0 && Pointer->PositionChanged == 0) {
    return EFI_NOT_READY;
  }

  //DEBUG ((EFI_D_ERROR, "Button: %d %d %d %d, Position: %d %d %d %d\n",
  //  ButtonState.Changed, ButtonState.LeftButton, ButtonState.MiddleButton, ButtonState.RightButton,
  //  Pointer->PositionChanged, Pointer->PositionX, Pointer->PositionY, Pointer->PositionZ));

  if (ButtonState.Changed) {
    State->LeftButton = ButtonState.LeftButton;
    State->RightButton = ButtonState.RightButton;
  } else {
    State->LeftButton = State->RightButton = FALSE;
  }

  if (Pointer->PositionChanged) {
    State->RelativeMovementX = Pointer->PositionX;
    State->RelativeMovementY = Pointer->PositionY;
    State->RelativeMovementZ = Pointer->PositionZ;
  } else {
    State->RelativeMovementX = State->RelativeMovementY = State->RelativeMovementZ = 0;
  }

  Pointer->PositionChanged = 0;
  Pointer->PositionX = Pointer->PositionY = Pointer->PositionZ = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimPointerGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN OUT EFI_SIMPLE_POINTER_STATE    *State
  )
{
  EFI_STATUS                 Status;
  UINTN                      Index;
  AMI_SHIM_POINTER_INSTANCE  *Pointer;
  
  if (This == NULL || State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0, Pointer = mAmiShimPointer.PointerMap; Index < MAX_POINTERS; Index++, Pointer++) {
    if (Pointer->SimplePointer == This) {
      break;
    }
  }

  if (Index != MAX_POINTERS) {
    Status = AmiShimPointerUpdateState (Pointer, State);

    if (!EFI_ERROR (Status)) {
      if (State->RelativeMovementX != 0 ||
          State->RelativeMovementY != 0 ||
          State->RelativeMovementZ != 0) {
        //DEBUG ((EFI_D_ERROR, "Received[%p] %d %d %d <%d, %d>\n", This, State->RelativeMovementX, State->RelativeMovementY, State->RelativeMovementZ,
        //        State->LeftButton, State->RightButton));
      } else {
        //DEBUG ((EFI_D_ERROR, "Received[%p] %d %d %d\n", This, State->RelativeMovementX, State->RelativeMovementY, State->RelativeMovementZ));
      }
    }
  } else {
    //DEBUG ((EFI_D_ERROR, "Received unknown this %p\n", This));
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

EFI_STATUS
EFIAPI
AmiShimPointerTimerSetup (
  VOID
  )
{
  EFI_STATUS              Status;

  if (mAmiShimPointer.TimersInitialised) {
    return EFI_ALREADY_STARTED;
  }

  // Refresh rate needs to be increased to poll the mouse frequently enough
  if (mAmiShimPointer.TimerProtocol == NULL) {
    Status = gBS->LocateProtocol(&gEfiTimerArchProtocolGuid, NULL, (VOID **)&mAmiShimPointer.TimerProtocol);
    if (!EFI_ERROR (Status)) {
      Status = mAmiShimPointer.TimerProtocol->GetTimerPeriod(mAmiShimPointer.TimerProtocol, &mAmiShimPointer.OriginalTimerPeriod);
      if (!EFI_ERROR (Status)) {
        if (mAmiShimPointer.OriginalTimerPeriod > AMI_SHIM_TIMER_PERIOD) {
          Status = mAmiShimPointer.TimerProtocol->SetTimerPeriod(mAmiShimPointer.TimerProtocol, AMI_SHIM_TIMER_PERIOD);
          DEBUG((EFI_D_ERROR, "AmiShimPointerTimerSetup changed period %d to %d, error %d\n", mAmiShimPointer.OriginalTimerPeriod, AMI_SHIM_TIMER_PERIOD, Status));
        }
      } else {
        DEBUG((EFI_D_ERROR, "AmiShimPointerTimerSetup failed to obtain previous period %d\n", Status));
      }
    } else {
      DEBUG((EFI_D_ERROR, "AmiShimPointerTimerSetup gEfiTimerArchProtocolGuid not found %d\n", Status));
    }
  }

  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AmiShimPointerPositionHandler, NULL, &mAmiShimPointer.PositionEvent);

  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "AmiShimPointerPositionHandler event creation failed %d\n", Status));
    return Status;
  }

  Status = gBS->SetTimer (mAmiShimPointer.PositionEvent, TimerPeriodic, POSITION_POLL_TIMER_INTERVAL);

  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "AmiShimPointerPositionHandler timer setting failed %d\n", Status));
    gBS->CloseEvent (mAmiShimPointer.PositionEvent);
    return Status;
  }

  mAmiShimPointer.TimersInitialised = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimPointerTimerUninstall (
  VOID
  )
{
  EFI_STATUS              Status;

  if (!mAmiShimPointer.TimersInitialised) {
    return EFI_SUCCESS;
  }

  if (mAmiShimPointer.TimerProtocol != NULL) {
    Status = mAmiShimPointer.TimerProtocol->GetTimerPeriod(mAmiShimPointer.TimerProtocol, &mAmiShimPointer.OriginalTimerPeriod);
    if (!EFI_ERROR (Status)) {
      if (mAmiShimPointer.OriginalTimerPeriod != mAmiShimPointer.OriginalTimerPeriod) {
        Status = mAmiShimPointer.TimerProtocol->SetTimerPeriod(mAmiShimPointer.TimerProtocol, AMI_SHIM_TIMER_PERIOD);
        DEBUG((EFI_D_ERROR, "AmiShimPointerTimerUninstall changed period %d to %d, error %d\n", mAmiShimPointer.OriginalTimerPeriod, AMI_SHIM_TIMER_PERIOD, Status));
      }
      mAmiShimPointer.TimerProtocol = NULL;
    } else {
      DEBUG((EFI_D_ERROR, "AmiShimPointerTimerUninstall failed to obtain current period %d\n", Status));
    }
  }

  if (mAmiShimPointer.PositionEvent != NULL) {
    Status = gBS->SetTimer (mAmiShimPointer.PositionEvent, TimerCancel, 0);
    if (!EFI_ERROR (Status)) {
      gBS->CloseEvent (mAmiShimPointer.PositionEvent);
      mAmiShimPointer.PositionEvent = NULL;
    } else {
      DEBUG((EFI_D_ERROR, "AmiShimPointerPositionHandler timer unsetting failed %d\n", Status));
    }
  }

  mAmiShimPointer.TimersInitialised = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimPointerInstallOnHandle (
  IN EFI_HANDLE                   DeviceHandle,
  IN AMI_EFIPOINTER_PROTOCOL      *EfiPointer,
  IN EFI_SIMPLE_POINTER_PROTOCOL  *SimplePointer
  )
{
  UINTN                      Index;
  AMI_SHIM_POINTER_INSTANCE  *Pointer;
  AMI_SHIM_POINTER_INSTANCE  *FreePointer;
  
  FreePointer = NULL;

  for (Index = 0; Index < MAX_POINTERS; Index++) {
    Pointer = &mAmiShimPointer.PointerMap[Index];
    if (Pointer->DeviceHandle == NULL && FreePointer == NULL) {
      FreePointer = Pointer;
    } else if (Pointer->DeviceHandle == DeviceHandle) {
      return EFI_ALREADY_STARTED;
    }
  }

  if (FreePointer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((EFI_D_ERROR, "Installed onto %X\n", DeviceHandle));
  FreePointer->DeviceHandle = DeviceHandle;
  FreePointer->EfiPointer = EfiPointer;
  FreePointer->SimplePointer = SimplePointer;
  if (FreePointer->SimplePointer->GetState == AmiShimPointerGetState) {
    FreePointer->OriginalGetState = NULL;
    DEBUG ((EFI_D_ERROR, "Function is already hooked\n"));
  } else {
    FreePointer->OriginalGetState = FreePointer->SimplePointer->GetState;
    FreePointer->SimplePointer->GetState = AmiShimPointerGetState;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimPointerInstall (
  VOID
  )
{
  EFI_STATUS                    Status;
  UINTN                         NoHandles;
  EFI_HANDLE                    *Handles;
  UINTN                         Index;
  BOOLEAN                       Installed;
  AMI_EFIPOINTER_PROTOCOL       *EfiPointer;
  EFI_SIMPLE_POINTER_PROTOCOL   *SimplePointer;

  Status = gBS->LocateHandleBuffer (ByProtocol, &gAmiEfiPointerProtocolGuid, NULL, &NoHandles, &Handles);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }
  
  DEBUG ((EFI_D_ERROR, "Found %d Handles located by protocol\n", NoHandles));

  Installed = FALSE;

  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gAmiEfiPointerProtocolGuid, (VOID **)&EfiPointer);
    if (EFI_ERROR(Status)) {
      DEBUG ((EFI_D_ERROR, "Handle %d has no AmiEfiPointerl %d\n", Index, Status));
      continue;
    }

    Status = gBS->HandleProtocol (Handles[Index], &gEfiSimplePointerProtocolGuid, (VOID **)&SimplePointer);
    if (EFI_ERROR(Status)) {
      DEBUG ((EFI_D_ERROR, "Handle %d has no EfiSimplePointer %d\n", Index, Status));
      continue;
    }

    Status = AmiShimPointerInstallOnHandle (Handles[Index], EfiPointer, SimplePointer);
    if (EFI_ERROR(Status)) {
      DEBUG ((EFI_D_ERROR, "Handle %d failed to get installed %d\n", Index, Status));
      continue;
    }

    Installed = TRUE;
  }

  gBS->FreePool (Handles);

  if (!Installed) {
    return EFI_NOT_FOUND;
  }

  if (!mAmiShimPointer.TimersInitialised) {
    return AmiShimPointerTimerSetup();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AmiShimPointerUninstall (
  VOID
  )
{
  UINTN                      Index;
  AMI_SHIM_POINTER_INSTANCE  *Pointer;

  AmiShimPointerTimerUninstall();

  for (Index = 0; Index < MAX_POINTERS; Index++) {
    Pointer = &mAmiShimPointer.PointerMap[Index];
    if (Pointer->DeviceHandle != NULL) {
      Pointer->SimplePointer->GetState = Pointer->OriginalGetState;
      gBS->DisconnectController(Pointer->DeviceHandle, NULL, NULL);
      Pointer->DeviceHandle = NULL;
    }
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
AmiShimPointerArriveHandler (
  IN EFI_EVENT  Event, 
  IN VOID       *Context
  )
{
  AmiShimPointerInstall();
}

EFI_STATUS
EFIAPI
AmiShimPointerInit (
  VOID
  )
{
  EFI_STATUS       Status;
  VOID             *Registration;
  
  Status = gBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AmiShimPointerArriveHandler, NULL, &mAmiShimPointer.ProtocolArriveEvent);

  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "AmiShimPointerArriveHandler event creation failed %d\n", Status));
    return Status;
  }

  // EfiSimplePointer gets installed after AMI proprietary protocol
  Status = gBS->RegisterProtocolNotify (&gEfiSimplePointerProtocolGuid, mAmiShimPointer.ProtocolArriveEvent, &Registration);

  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "AmiShimProtocolArriveHandler protocol registration failed %d\n", Status));
    gBS->CloseEvent (mAmiShimPointer.ProtocolArriveEvent);
    return Status;
  }

  return AmiShimPointerInstall();
}

EFI_STATUS
EFIAPI
AmiShimPointerExit (
  VOID
  )
{
  return AmiShimPointerUninstall();
}