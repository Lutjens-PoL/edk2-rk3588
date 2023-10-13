/** @file
*
*  Copyright (c) 2021, Rockchip Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/GpioLib.h>
#include <Library/RK806.h>
#include <Library/Rk3588Pcie.h>
#include <Library/PWMLib.h>
#include <Soc.h>

static struct regulator_init_data rk806_init_data[] = {
  /* Master PMIC */
  RK8XX_VOLTAGE_INIT(MASTER_BUCK1, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK3, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK4, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK5, 850000),
  //RK8XX_VOLTAGE_INIT(MASTER_BUCK6, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK7, 2000000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK8, 3300000),
  RK8XX_VOLTAGE_INIT(MASTER_BUCK10, 1800000),

  RK8XX_VOLTAGE_INIT(MASTER_NLDO1, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_NLDO2, 850000),
  RK8XX_VOLTAGE_INIT(MASTER_NLDO3, 750000),
  RK8XX_VOLTAGE_INIT(MASTER_NLDO4, 850000),
  RK8XX_VOLTAGE_INIT(MASTER_NLDO5, 750000),

  RK8XX_VOLTAGE_INIT(MASTER_PLDO1, 1800000),
  RK8XX_VOLTAGE_INIT(MASTER_PLDO2, 1800000),
  RK8XX_VOLTAGE_INIT(MASTER_PLDO3, 1200000),
  RK8XX_VOLTAGE_INIT(MASTER_PLDO4, 3300000),
  RK8XX_VOLTAGE_INIT(MASTER_PLDO5, 3300000),
  RK8XX_VOLTAGE_INIT(MASTER_PLDO6, 1800000),

  /* No dual PMICs on this platform */
};

VOID
EFIAPI
SdmmcIoMux (
  VOID
  )
{
  /* sdmmc0 iomux (microSD socket) */
  BUS_IOC->GPIO4D_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111); //SDMMC_D0,SDMMC_D1,SDMMC_D2,SDMMC_D3
  BUS_IOC->GPIO4D_IOMUX_SEL_H = (0x00FFUL << 16) | (0x0011); //SDMMC_CLK,SDMMC_CMD
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x000FUL << 16) | (0x0001); //SDMMC_DET
}

VOID
EFIAPI
SdhciEmmcIoMux (
  VOID
  )
{
  /* sdhci0 iomux (eMMC onboard) */
  BUS_IOC->GPIO2A_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111); //EMMC_CMD,EMMC_CLKOUT,EMMC_DATASTROBE,EMMC_RSTN
  BUS_IOC->GPIO2D_IOMUX_SEL_L = (0xFFFFUL << 16) | (0x1111); //EMMC_D0,EMMC_D1,EMMC_D2,EMMC_D3
  BUS_IOC->GPIO2D_IOMUX_SEL_H = (0xFFFFUL << 16) | (0x1111); //EMMC_D4,EMMC_D5,EMMC_D6,EMMC_D7
}

#define NS_CRU_BASE         0xFD7C0000
#define CRU_CLKSEL_CON59    0x03EC
#define CRU_CLKSEL_CON78    0x0438

VOID
EFIAPI
Rk806SpiIomux (
  VOID
  )
{
  /* io mux */
  //BUS_IOC->GPIO1A_IOMUX_SEL_H = (0xFFFFUL << 16) | 0x8888;
  //BUS_IOC->GPIO1B_IOMUX_SEL_L = (0x000FUL << 16) | 0x0008;
  PMU1_IOC->GPIO0A_IOMUX_SEL_H = (0x0FF0UL << 16) | 0x0110;
  PMU1_IOC->GPIO0B_IOMUX_SEL_L = (0xF0FFUL << 16) | 0x1011;
  MmioWrite32(NS_CRU_BASE + CRU_CLKSEL_CON59, (0x00C0UL << 16) | 0x0080);
}

VOID
EFIAPI
Rk806Configure (
  VOID
  )
{
  UINTN RegCfgIndex;

  RK806Init();

  for (RegCfgIndex = 0; RegCfgIndex < ARRAY_SIZE(rk806_init_data); RegCfgIndex++)
    RK806RegulatorInit(rk806_init_data[RegCfgIndex]);
}

VOID
EFIAPI
SetCPULittleVoltage (
  IN UINT32 Microvolts
  )
{
  struct regulator_init_data Rk806CpuLittleSupply =
        RK8XX_VOLTAGE_INIT(MASTER_BUCK2, Microvolts);

  RK806RegulatorInit(Rk806CpuLittleSupply);
}

VOID
EFIAPI
NorFspiIomux (
  VOID
  )
{
  /* io mux */
  /* Do not override, set by earlier boot stages. */
}

VOID
EFIAPI
GmacIomux (
  IN UINT32 Id
  )
{
  switch (Id) {
    case 0:
      /* gmac0 iomux */
      BUS_IOC->GPIO2A_IOMUX_SEL_H = (0xFF00UL << 16) | 0x1100;
      BUS_IOC->GPIO2B_IOMUX_SEL_L = (0xFFFFUL << 16) | 0x1111;
      BUS_IOC->GPIO2B_IOMUX_SEL_H = (0xFF00UL << 16) | 0x1100;
      BUS_IOC->GPIO2C_IOMUX_SEL_L = (0x0FFFUL << 16) | 0x0111;
      BUS_IOC->GPIO4C_IOMUX_SEL_L = (0xFF00UL << 16) | 0x1100;
      BUS_IOC->GPIO4C_IOMUX_SEL_H = (0x00FFUL << 16) | 0x0011;

      /* phy0 reset */
      GpioPinSetDirection (0, GPIO_PIN_PB0, GPIO_PIN_OUTPUT);
      break;
    case 1:
      /* gmac1 iomux */
      BUS_IOC->GPIO3B_IOMUX_SEL_H = (0x0FFFUL << 16) | 0x0111;
      BUS_IOC->GPIO3A_IOMUX_SEL_L = (0xFFFFUL << 16) | 0x1111;
      BUS_IOC->GPIO3B_IOMUX_SEL_L = (0xF0FFUL << 16) | 0x1011;
      BUS_IOC->GPIO3A_IOMUX_SEL_H = (0xF0FFUL << 16) | 0x1011;
      BUS_IOC->GPIO3C_IOMUX_SEL_L = (0xFF00UL << 16) | 0x1100;

      /* phy1 reset */
      GpioPinSetDirection (1, GPIO_PIN_PB4, GPIO_PIN_OUTPUT);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
GmacIoPhyReset (
  UINT32   Id,
  BOOLEAN  Enable
  )
{
  switch (Id) {
    case 0:
      /* phy0 reset */
      GpioPinWrite (0, GPIO_PIN_PB0, !Enable);
      break;
    case 1:
      /* phy1 reset */
      GpioPinWrite (1, GPIO_PIN_PB4, !Enable);
      break;
    default:
      break;
  }
}

VOID
EFIAPI
NorFspiEnableClock (
  UINT32 *CruBase
  )
{
  UINTN BaseAddr = (UINTN) CruBase;

  MmioWrite32(BaseAddr + 0x087C, 0x0E000000);
}

VOID
EFIAPI
I2cIomux (
   UINT32 id
  )
{
  switch (id) {
  case 0:
    /* io mux M2 */
    PMU2_IOC->GPIO0D_IOMUX_SEL_L = (0x0F00UL << 16) | 0x0300;
    PMU2_IOC->GPIO0D_IOMUX_SEL_L = (0x00F0UL << 16) | 0x0030;
    break;
  case 1:
    /* io mux */
    //BUS_IOC->GPIO0B_IOMUX_SEL_H = (0x0FF0UL << 16) | 0x0990;
    //PMU2_IOC->GPIO0B_IOMUX_SEL_H = (0x0FF0UL << 16) | 0x0880;
    break;
  case 2:
    /* io mux */
    BUS_IOC->GPIO0B_IOMUX_SEL_H = (0xF000UL << 16) | 0x9000;
    BUS_IOC->GPIO0C_IOMUX_SEL_L = (0x000FUL << 16) | 0x0009;
    PMU2_IOC->GPIO0B_IOMUX_SEL_H = (0xF000UL << 16) | 0x8000;
    PMU2_IOC->GPIO0C_IOMUX_SEL_L = (0x000FUL << 16) | 0x0008;
    break;
  case 3:
    break;
  case 4:
    break;
  case 5:
    /* io mux M2 */
    BUS_IOC->GPIO4A_IOMUX_SEL_H = (0xF000UL << 16) | 0x9000;
    BUS_IOC->GPIO4A_IOMUX_SEL_H = (0x0F00UL << 16) | 0x0900;
    break;
  case 6:
    break;
  default:
    break;
  }
}

VOID
EFIAPI
UsbPortPowerEnable (
  VOID
  )
{
  DEBUG((DEBUG_INFO, "UsbPortPowerEnable called\n"));
  /* Set GPIO0 PB0 (TYPEC0_PWREN) output high to power TYPEC0 */
  GpioPinWrite (0, GPIO_PIN_PD0, TRUE);
  GpioPinSetDirection (0, GPIO_PIN_PD0, GPIO_PIN_OUTPUT);
  /* Set GPIO0 PB0 (TYPEC1_PWREN) output high to power TYPEC1 */
  GpioPinWrite (0, GPIO_PIN_PD3, TRUE);
  GpioPinSetDirection (0, GPIO_PIN_PD3, GPIO_PIN_OUTPUT);

  // DEBUG((DEBUG_INFO, "Trying to enable blue led\n"));
  // GpioPinWrite (0, GPIO_PIN_PB7, TRUE);
  // GpioPinSetDirection (0, GPIO_PIN_PB7, GPIO_PIN_OUTPUT);
}

VOID
EFIAPI
Usb2PhyResume (
  VOID
  )
{
  MmioWrite32(0xfd5d0008, 0x20000000);
  MmioWrite32(0xfd5d4008, 0x20000000);
  MmioWrite32(0xfd5d8008, 0x20000000);
  MmioWrite32(0xfd5dc008, 0x20000000);
  MmioWrite32(0xfd7f0a10, 0x07000700);
  MmioWrite32(0xfd7f0a10, 0x07000000);
}

VOID
EFIAPI
PcieIoInit (
  UINT32 Segment
  )
{
  /* Set reset and power IO to gpio output mode */
  switch(Segment) {
    case PCIE_SEGMENT_PCIE30X4:
      GpioPinSetDirection (4, GPIO_PIN_PB6, GPIO_PIN_OUTPUT);
      // PciePinmuxInit(Segment, 1);
      break;
    case PCIE_SEGMENT_PCIE20L0:
      GpioPinSetDirection (4, GPIO_PIN_PA5, GPIO_PIN_OUTPUT);
      // PciePinmuxInit(Segment, 1); // PCIE30x1_0_{CLKREQN,WAKEN,PERSTN}_M1
      break;
    case PCIE_SEGMENT_PCIE20L1:
      break;
    case PCIE_SEGMENT_PCIE20L2:
      GpioPinSetDirection (3, GPIO_PIN_PD1, GPIO_PIN_OUTPUT);
      // PciePinmuxInit(Segment, 0); // PCIE20x1_2_{CLKREQN,WAKEN,PERSTN}_M0
      break;
    default:
      break;
  }
}

VOID
EFIAPI
PciePowerEn (
  UINT32 Segment,
  BOOLEAN Enable
  )
{
/* nothing to power on */
}

VOID
EFIAPI
PciePeReset (
  UINT32 Segment,
  BOOLEAN Enable
  )
{
  switch(Segment) {
    case PCIE_SEGMENT_PCIE30X4:
      GpioPinWrite (4, GPIO_PIN_PB6, !Enable);
      break;
    case PCIE_SEGMENT_PCIE20L0:
      GpioPinWrite (4, GPIO_PIN_PA5, !Enable);
      break;
    case PCIE_SEGMENT_PCIE20L1:
      break;
    case PCIE_SEGMENT_PCIE20L2: // m.2 a+e key
      GpioPinWrite (3, GPIO_PIN_PD1, !Enable);
      break;
    default:
      break;
  }
}

PWM_DATA pwm_data = {
  .ControllerID = PWM_CONTROLLER0,
  .ChannelID = PWM_CHANNEL2,
  .PeriodNs = 4000000,
  .DutyNs = 4000000,
  .Polarity = FALSE,
}; // PWM0_CH2

VOID
EFIAPI
PwmFanIoSetup (
  VOID
  )
{
  GpioPinSetFunction (0, GPIO_PIN_PC4, 0x3); // PWM2_M0
  RkPwmSetConfig (&pwm_data);
  RkPwmEnable (&pwm_data);
}

VOID
EFIAPI
PwmFanSetSpeed (
  IN UINT32 Percentage
  )
{
  pwm_data.DutyNs = pwm_data.PeriodNs * Percentage / 100;
  RkPwmSetConfig (&pwm_data);
}

VOID
EFIAPI
PlatformInitLeds (
  VOID
  )
{
  /* Status indicator */
  GpioPinWrite (0, GPIO_PIN_PA0, FALSE);
  GpioPinSetDirection (0, GPIO_PIN_PA0, GPIO_PIN_OUTPUT);
}

VOID
EFIAPI
PlatformSetStatusLed (
  IN BOOLEAN Enable
  )
{
  GpioPinWrite (0, GPIO_PIN_PA0, Enable);
}

VOID
EFIAPI
PlatformEarlyInit (
  VOID
  )
{
  // Configure various things specific to this platform
}
