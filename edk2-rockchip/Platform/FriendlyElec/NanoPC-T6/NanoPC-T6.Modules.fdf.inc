## @file
#
#  Copyright (c) 2023, Mario Bălănică <mariobalanica02@gmail.com>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

  # ACPI Support
  INF RuleOverride = ACPITABLE $(PLATFORM_DIRECTORY)/AcpiTables/AcpiTables.inf

  # Device Tree Support
  FILE FREEFORM = gDtPlatformDefaultDtbFileGuid {
    SECTION RAW = Platform/Rockchip/DeviceTree/rk3588-nanopc-t6.dtb
  }

  # Splash screen logo
  INF MdeModulePkg/Logo/LogoDxe.inf
