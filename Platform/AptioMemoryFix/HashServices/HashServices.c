/**
 * Hash service fix for AMI EFIs with broken SHA implementations.
 *
 * Forcibly reinstalls EFI_HASH_PROTOCOL with working MD5, SHA-1,
 * SHA-256 implementations.
 *
 * Author: Joel Höner <athre0z@zyantific.com>
 */

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/ServiceBinding.h>
#include <Protocol/Hash.h>

#include "HashServices.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"

STATIC EFI_SERVICE_BINDING_PROTOCOL mHashBindingProto = {
  &HSCreateChild,
  &HSDestroyChild
};

EFI_STATUS
EFIAPI
HSGetHashSize (
  IN  CONST EFI_HASH_PROTOCOL *This,
  IN  CONST EFI_GUID          *HashAlgorithm,
  OUT UINTN                   *HashSize
  )
{
  if (!HashAlgorithm || !HashSize) {
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&gEfiHashAlgorithmMD5Guid, HashAlgorithm)) {
    *HashSize = sizeof (EFI_MD5_HASH);
    return EFI_SUCCESS;
  } else if (CompareGuid (&gEfiHashAlgorithmSha1Guid, HashAlgorithm)) {
    *HashSize = sizeof (EFI_SHA1_HASH);
    return EFI_SUCCESS;
  } else if (CompareGuid (&gEfiHashAlgorithmSha256Guid, HashAlgorithm)) {
    *HashSize = sizeof (EFI_SHA256_HASH);
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
HSHash (
  IN CONST EFI_HASH_PROTOCOL  *This,
  IN CONST EFI_GUID           *HashAlgorithm,
  IN BOOLEAN                  Extend,
  IN CONST UINT8              *Message,
  IN UINT64                   MessageSize,
  IN OUT EFI_HASH_OUTPUT      *Hash
  )
{
  HS_PRIVATE_DATA  *PrivateData;
  HS_CONTEXT_DATA  CtxCopy;

  if (!This || !HashAlgorithm || !Message || !Hash || !MessageSize) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = HS_PRIVATE_FROM_PROTO (This);

  if (CompareGuid (&gEfiHashAlgorithmMD5Guid, HashAlgorithm)) {
    if (!Extend) {
      md5_init (&PrivateData->Ctx.Md5);
    }

    md5_update (&PrivateData->Ctx.Md5, Message, MessageSize);
    CopyMem (&CtxCopy, &PrivateData->Ctx, sizeof (PrivateData->Ctx));
    md5_final (&CtxCopy.Md5, *Hash->Md5Hash);
    return EFI_SUCCESS;
  } else if (CompareGuid (&gEfiHashAlgorithmSha1Guid, HashAlgorithm)) {
    if (!Extend) {
      sha1_init (&PrivateData->Ctx.Sha1);
    }

    sha1_update (&PrivateData->Ctx.Sha1, Message, MessageSize);
    CopyMem (&CtxCopy, &PrivateData->Ctx, sizeof (PrivateData->Ctx));
    sha1_final (&CtxCopy.Sha1, *Hash->Sha1Hash);
    return EFI_SUCCESS;
  } else if (CompareGuid (&gEfiHashAlgorithmSha256Guid, HashAlgorithm)) {
    if (!Extend) {
      sha256_init (&PrivateData->Ctx.Sha256);
    }

    sha256_update (&PrivateData->Ctx.Sha256, Message, MessageSize);
    CopyMem (&CtxCopy, &PrivateData->Ctx, sizeof (PrivateData->Ctx));
    sha256_final (&CtxCopy.Sha256, *Hash->Sha256Hash);
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
HSCreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                    *ChildHandle
  )
{
  HS_PRIVATE_DATA  *PrivateData;
  EFI_STATUS       Status;

  PrivateData = AllocateZeroPool (sizeof (*PrivateData));
  if (!PrivateData) {
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->Signature         = HS_PRIVATE_SIGNATURE;
  PrivateData->Proto.GetHashSize = HSGetHashSize;
  PrivateData->Proto.Hash        = HSHash;

  Status = gBS->InstallProtocolInterface (
    ChildHandle,
    &gEfiHashProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &PrivateData->Proto
    );

  if (EFI_ERROR (Status)) {
    FreePool (PrivateData);
  }

  return Status;
}

EFI_STATUS
EFIAPI
HSDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   ChildHandle
  )
{
  EFI_HASH_PROTOCOL *HashProto;
  EFI_STATUS        Status;

  if (!This || !ChildHandle) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->HandleProtocol (
    ChildHandle,
    &gEfiHashProtocolGuid,
    (VOID **) &HashProto
    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->UninstallProtocolInterface (
    ChildHandle,
    &gEfiHashProtocolGuid,
    HashProto
    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  FreePool (HS_PRIVATE_FROM_PROTO (HashProto));
  return EFI_SUCCESS;
}

BOOLEAN
EFIAPI
HSSelfTest (
  VOID
  )
{
  EFI_SERVICE_BINDING_PROTOCOL  *BindingProto;
  EFI_HASH_PROTOCOL             *HashProto;
  EFI_HASH_OUTPUT               HashOut;
  EFI_MD5_HASH                  Md5Hash;
  EFI_SHA1_HASH                 Sha1Hash;
  EFI_SHA256_HASH               Sha256Hash;
  EFI_STATUS                    Status;
  EFI_HANDLE                    Child;
  UINTN                         HashIter;
  BOOLEAN                       Failed;

  //
  // Create binding.
  //
  Status = gBS->LocateProtocol (
    &gEfiHashServiceBindingProtocolGuid,
    NULL,
    (VOID **) &BindingProto
    );

  if (EFI_ERROR (Status)) {
    Print (L"AMF: Binding lookup failure - %r\n", Status);
    return FALSE;
  }

  Child = NULL;

  Status = BindingProto->CreateChild (BindingProto, &Child);
  if (EFI_ERROR (Status)) {
    Print (L"AMF: Child creation failure - %r\n", Status);
    return FALSE;
  }

  //
  // Obtain protocol.
  //
  HashProto = NULL;
  Status = gBS->LocateProtocol (
    &gEfiHashProtocolGuid,
    NULL,
    (VOID **) &HashProto
    );

  if (EFI_ERROR (Status)) {
    Print (L"AMF: Locate hash protocol failure - %r\n", Status);
    BindingProto->DestroyChild (BindingProto, Child);
    return FALSE;
  }

  Failed = FALSE;

  //
  // Do two iterations of test-hashing.
  //
  HashOut.Md5Hash = &Md5Hash;
  for (HashIter = 0; HashIter < 2; ++HashIter) {
    Status = HashProto->Hash (
      HashProto,
      &gEfiHashAlgorithmMD5Guid,
      (BOOLEAN)HashIter,
      (CONST UINT8 *)"ABCDEFGHIJKLMNOP",
      16,
      &HashOut
      );

    if (EFI_ERROR (Status)) {
      Print (L"AMF: MD5 failure - %r\n", Status);
      Failed = TRUE;
      break;
    }
  }

  HashOut.Sha1Hash = &Sha1Hash;
  for (HashIter = 0; HashIter < 2; ++HashIter) {
    Status = HashProto->Hash (
      HashProto,
      &gEfiHashAlgorithmSha1Guid,
      (BOOLEAN)HashIter,
      (CONST UINT8 *)"ABCDEFGHIJKLMNOP",
      16,
      &HashOut
      );

    if (EFI_ERROR (Status)) {
      Print (L"AMF: Sha1 failure - %r\n", Status);
      Failed = TRUE;
      break;
    }
  }

  HashOut.Sha256Hash = &Sha256Hash;
  for (HashIter = 0; HashIter < 2; ++HashIter) {
    Status = HashProto->Hash (
      HashProto,
      &gEfiHashAlgorithmSha256Guid,
      (BOOLEAN)HashIter,
      (CONST UINT8 *)"ABCDEFGHIJKLMNOP",
      16,
      &HashOut
      );

    if (EFI_ERROR (Status)) {
      Print (L"AMF: Sha256 failure - %r\n", Status);
      Failed = TRUE;
      break;
    }
  }

  BindingProto->DestroyChild (BindingProto, Child);

  if (Failed) {
    return FALSE;
  }

  //
  // Verify result.
  //
  if (CompareMem (
    Md5Hash,
    "\x3f\xe2\x2a\x38\x1b\xac\x54\x3f\xe4\x11"
    "\xce\x24\xcc\x15\x2e\x71",
    sizeof (Md5Hash)
    )) {
    Print (L"AMF: MD5 mismatch!\n");
    return FALSE;
  }

  if (CompareMem (
    Sha1Hash,
    "\x14\x80\x6E\x23\xB4\xCE\xB6\x5D\xDF\x01"
    "\xE5\xEA\x7F\xBC\xDD\x03\xAA\xFA\xF5\xCD",
    sizeof (Sha1Hash)
    )) {
    Print (L"AMF: Sha1 mismatch!\n");
    return FALSE;
  }

  if (CompareMem (
    Sha256Hash,
    "\xf3\x5b\xd8\xfe\x5e\x0f\xbf\xcd\xc3\x5b"
    "\x85\xb3\x79\x88\x71\x70\xbb\xc8\xfc\xb7"
    "\x03\xdd\x6c\xca\x2d\xa4\x67\x2c\x40\xc7"
    "\x97\xfa",
    sizeof (Sha256Hash)
    )) {
    Print (L"AMF: Sha256 mismatch!\n");
    return FALSE;
  }

  Print (L"AMF: Basic hash tests are ok\n");

  return TRUE;
}

EFI_STATUS
EFIAPI
InitializeHashServices (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS      Status;
  EFI_HANDLE      *OriginalHandles;
  VOID            *OriginalProto;
  UINTN           Index;
  UINTN           HandleBufferSize;

  //
  // Uninstall all the existing protocol instances (which are not to be trusted).
  //
  HandleBufferSize = 0;

  Status = gBS->LocateHandle (
    ByProtocol,
    &gEfiHashServiceBindingProtocolGuid,
    NULL,
    &HandleBufferSize,
    NULL
    );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    OriginalHandles = (EFI_HANDLE *)AllocateZeroPool (HandleBufferSize);

    if (!OriginalHandles) {
      return EFI_OUT_OF_RESOURCES;
    }

    for (Index = 0; Index < HandleBufferSize / sizeof (EFI_HANDLE); Index++) {
      Status = gBS->HandleProtocol (
        OriginalHandles[Index],
        &gEfiHashServiceBindingProtocolGuid,
        &OriginalProto
        );

      if (EFI_ERROR (Status)) {
        break;
      }

      Status = gBS->UninstallProtocolInterface (
        OriginalHandles[Index],
        &gEfiHashServiceBindingProtocolGuid,
        OriginalProto
        );

      if (EFI_ERROR(Status)) {
        break;
      }
    }

    FreePool ((VOID *) OriginalHandles);
  }

  //
  // Install our own protocol binding
  //
  Status = gBS->InstallProtocolInterface (
    &ImageHandle,
    &gEfiHashServiceBindingProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &mHashBindingProto
    );

  return Status;
}
