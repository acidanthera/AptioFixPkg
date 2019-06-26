## @file
# Copyright (C) 2017, vit9696.  All rights reserved.<BR>
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##

[Defines]
  PLATFORM_NAME           = AptioFixPkg
  PLATFORM_GUID           = 4FBFC810-9D3C-4BF3-ABF5-89A793C235B1
  PLATFORM_VERSION        = 1.0
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = RELEASE|DEBUG|NOOPT
  SKUID_IDENTIFIER        = DEFAULT
  DSC_SPECIFICATION       = 0x00010006

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseRngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  DebugLib|OcSupportPkg/Library/OcDebugLogLib/OcDebugLogLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  OcAppleBootPolicyLib|OcSupportPkg/Library/OcBootManagementLib/OcBootManagementLib.inf
  OcAppleChunklistLib|OcSupportPkg/Library/OcAppleChunklistLib/OcAppleChunklistLib.inf
  OcAppleDiskImageLib|OcSupportPkg/Library/OcAppleDiskImageLib/OcAppleDiskImageLib.inf
  OcAppleKeysLib|OcSupportPkg/Library/OcAppleKeysLib/OcAppleKeysLib.inf
  OcAppleRamDiskLib|OcSupportPkg/Library/OcAppleRamDiskLib/OcAppleRamDiskLib.inf
  OcBootManagementLib|OcSupportPkg/Library/OcBootManagementLib/OcBootManagementLib.inf
  OcCompressionLib|OcSupportPkg/Library/OcCompressionLib/OcCompressionLib.inf
  OcCryptoLib|OcSupportPkg/Library/OcCryptoLib/OcCryptoLib.inf
  OcDataHubLib|OcSupportPkg/Library/OcDataHubLib/OcDataHubLib.inf
  OcDevicePathLib|OcSupportPkg/Library/OcDevicePathLib/OcDevicePathLib.inf
  OcDeviceTreeLib|OcSupportPkg/Library/OcDeviceTreeLib/OcDeviceTreeLib.inf
  OcFileLib|OcSupportPkg/Library/OcFileLib/OcFileLib.inf
  OcGuardLib|OcSupportPkg/Library/OcGuardLib/OcGuardLib.inf
  OcMachoLib|OcSupportPkg/Library/OcMachoLib/OcMachoLib.inf
  OcMiscLib|OcSupportPkg/Library/OcMiscLib/OcMiscLib.inf
  OcRtcLib|OcSupportPkg/Library/OcRtcLib/OcRtcLib.inf
  OcStringLib|OcSupportPkg/Library/OcStringLib/OcStringLib.inf
  OcTimerLib|OcSupportPkg/Library/OcTimerLib/OcTimerLib.inf
  OcXmlLib|OcSupportPkg/Library/OcXmlLib/OcXmlLib.inf
  SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf
  PlatformHookLib|MdeModulePkg/Library/BasePlatformHookLibNull/BasePlatformHookLibNull.inf

[Components]
  AptioFixPkg/Platform/AptioInputFix/AptioInputFix.inf
  AptioFixPkg/Platform/AptioMemoryFix/AptioMemoryFix.inf
  AptioFixPkg/Platform/VariableRuntime/VariableRuntime.inf
  AptioFixPkg/Application/VerifyMsrE2/VerifyMsrE2.inf
  AptioFixPkg/Application/CleanNvram/CleanNvram.inf

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|0
!if $(TARGET) == RELEASE
  # DEBUG_PRINT_ENABLED
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|2
  # DEBUG_ERROR | DEBUG_WARN
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000002
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x80000002
!else
  # DEBUG_ASSERT_ENABLED | DEBUG_PRINT_ENABLED | DEBUG_CODE_ENABLED | CLEAR_MEMORY_ENABLED | ASSERT_DEADLOOP_ENABLED
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2f
  # DEBUG_ERROR | DEBUG_WARN | DEBUG_INFO
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x80000042
!endif

[BuildOptions]
  # While there are no PCDs as of now, there at least are some custom macros.
  # See AptioMemoryFix/Config.h for more details.
  DEFINE APTIOFIXPKG_BUILD_OPTIONS_GEN = -D DISABLE_NEW_DEPRECATED_INTERFACES $(APTIOFIXPKG_BUILD_OPTIONS)

  GCC:DEBUG_*_*_CC_FLAGS     = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  GCC:NOOPT_*_*_CC_FLAGS     = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  GCC:RELEASE_*_*_CC_FLAGS   = $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  MSFT:DEBUG_*_*_CC_FLAGS    = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  MSFT:NOOPT_*_*_CC_FLAGS    = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  MSFT:RELEASE_*_*_CC_FLAGS  = $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  XCODE:DEBUG_*_*_CC_FLAGS   = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  XCODE:NOOPT_*_*_CC_FLAGS   = -D APTIOFIX_DEBUG_TARGET $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
  XCODE:RELEASE_*_*_CC_FLAGS = -Oz -flto $(APTIOFIXPKG_BUILD_OPTIONS_GEN)
