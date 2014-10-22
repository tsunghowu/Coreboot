/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <types.h>
#include <string.h>
#include <cpu/x86/stack.h>
#include <console/console.h>
#include <bootstate.h>
#include <cbmem.h>
#include <device/device.h>
//#include <southbridge_pci_devs.h>
#include <fsp_util.h>
#include "../chip.h"
#include <reset.h>
#include <arch/io.h>
#include "northbridge/intel/fsp_haswell/haswell.h"
#ifndef CONFIG_ENABLE_FSP_FAST_BOOT
# error "CONFIG_ENABLE_FSP_FAST_BOOT must be set."
#endif

#ifdef __PRE_RAM__
void dump_memory(unsigned char *address, unsigned int size);
void dump_pci_context(unsigned char bus, unsigned char device, unsigned function, unsigned int size);

#if 1	//IS_ENABLED(CONFIG_SOUTHBRIDGE_INTEL_FSP_I89XX)
static void GetUpdDefaultFromFsp (FSP_INFO_HEADER *FspInfo, UPD_DATA_REGION   *UpdData)
{
	VPD_DATA_REGION *VpdDataRgnPtr;
	UPD_DATA_REGION *UpdDataRgnPtr;
	VpdDataRgnPtr = (VPD_DATA_REGION *)(UINT32)(FspInfo->CfgRegionOffset  + FspInfo->ImageBase);
	UpdDataRgnPtr = (UPD_DATA_REGION *)(UINT32)(VpdDataRgnPtr->PcdUpdRegionOffset + FspInfo->ImageBase);
	memcpy((void*)UpdData, (void*)UpdDataRgnPtr, sizeof(UPD_DATA_REGION));
}
void dump_memory(unsigned char *address, unsigned int size)
{
	unsigned int i,j;
	volatile unsigned char *pMmioAddress = (volatile unsigned char *)address;

	for(i=0;i<size/16;i++) {
		for(j=0;j<16;j++){
			printk(BIOS_DEBUG, "%-2x ", pMmioAddress[i*16+j]);
		}
		printk(BIOS_DEBUG, "\n");
		if( i*16+j > size )
			return;
	}
}
void dump_pci_context(unsigned char bus, unsigned char device, unsigned function, unsigned int size)
{
	unsigned int i,j;
	
	for(i=0;i<size/16;i++) {
		for(j=0;j<16;j++){
			printk(BIOS_DEBUG, "%-2x ", pci_read_config8(PCI_DEV(bus, device, function), i*16+j) );
		}
		printk(BIOS_DEBUG, "\n");
		if( i*16+j > size )
			return;
	}	
}
#if 0
VOID
ProgramSaBars (
  IN    SA_PLATFORM_POLICY_PPI   *SaPlatformPolicyPpi
  )
/**
  Programs SA Bars

  @param[in] SaPlatformPolicyPpi - Instance of SA_PLATFORM_POLICY_PPI
**/
{
  CPU_FAMILY  CpuFamilyId;

  CpuFamilyId = GetCpuFamily();

  ///
  /// Program SA MchBar, DmiBar and EpBar
  ///
  McD0PciCfg64 (R_SA_MCHBAR)   = (UINT64) (SaPlatformPolicyPpi->PlatformData->MchBar        | BIT0);
  McD0PciCfg64 (R_SA_DMIBAR)   = (UINT64) (SaPlatformPolicyPpi->PlatformData->DmiBar        | BIT0);
  McD0PciCfg64 (R_SA_PXPEPBAR) = (UINT64) (SaPlatformPolicyPpi->PlatformData->EpBar         | BIT0);

  ///
  /// Program SA GdxcBar
  ///
  Mmio64 ((UINTN) (SaPlatformPolicyPpi->PlatformData->MchBar), R_SA_MCHBAR_GDXCBAR_OFFSET) = (UINT64)
    (SaPlatformPolicyPpi->PlatformData->GdxcBar | BIT0);

  if (CpuFamilyId == EnumCpuCrw) {
    ///
    /// Program SA EdramBar
    ///
    Mmio64 ((UINTN) (SaPlatformPolicyPpi->PlatformData->MchBar), R_SA_MCHBAR_EDRAMBAR_OFFSET) = (UINT64)
      (SaPlatformPolicyPpi->PlatformData->EdramBar | BIT0);
  }
}

VOID
EarlyBiosPostProgramming (
  IN    SA_PLATFORM_POLICY_PPI   *SaPlatformPolicyPpi
  )
/**
  Do Early BIOS POST Programming

  @param[in] SaPlatformPolicyPpi - Instance of SA_PLATFORM_POLICY_PPI
**/
{
  /// SA BS 11.1 Early BIOS POST Programming
  /// 1.  Enable System Agent Clock Gating by setting the MCHBAR offset 5F00h [0] = '1b'.
  MmioOr32 ((UINTN) (SaPlatformPolicyPpi->PlatformData->MchBar + 0x5F00), BIT0);
}
#endif
#define NB_GDXC_BASE_ADDRESS	0xfed84000
static void ConfigureDefaultUpdData(UPD_DATA_REGION   *UpdData)
{/*
	UpdData->HTEnable = TRUE;
	UpdData->TurboEnable = FALSE;
	UpdData->MemoryDownEnable = FALSE;
	UpdData->FastBootEnable = CONFIG_ENABLE_FSP_FAST_BOOT;
*/
	/*
	UpdData->SerialDebugPortAddress = 0x3F8;
	UpdData->SerialDebugPortType = 1;
	UpdData->PcdMrcDebugMsg = TRUE;
	UpdData->PcdEnableRmt = TRUE;
	UpdData->PcdUserCrbBoardType = 0;
	*/
	printk(BIOS_DEBUG, "PcdFastBoot %x\n", UpdData->PcdFastBoot );
//UpdData->PcdFastBoot = 0;

printk(BIOS_DEBUG, "PcdMmioSize %x\n", UpdData->PcdMmioSize );
printk(BIOS_DEBUG, "PcdApertureSize %x\n", UpdData->PcdApertureSize );
printk(BIOS_DEBUG, "PcdIgdDvmt50PreAlloc %x\n", UpdData->PcdIgdDvmt50PreAlloc );
printk(BIOS_DEBUG, "PcdTsegSize %x\n", UpdData->PcdTsegSize );

printk(BIOS_DEBUG, "Dump B0, D0, F0:  \n");
dump_memory((unsigned char *)0xF8000000, 0x100 );
dump_pci_context(0,0,0, 0x100);
#if 0
{
	volatile unsigned long *ddTemp;
	ddTemp = (volatile unsigned long *)(0xFED10000 + 0x5420);	//R_SA_MCHBAR_GDXCBAR_OFFSET
	*ddTemp = (unsigned long)(NB_GDXC_BASE_ADDRESS | 0x01);	//Or BIT0

printk(BIOS_DEBUG, "GDXC :%lx\n", *ddTemp );

	ddTemp = (volatile unsigned long *)(0xFED10000 + 0x5F00);	//1.  Enable System Agent Clock Gating by setting the MCHBAR offset 5F00h [0] = '1b'.
	*ddTemp |= (1 << 9)|(1 << 10);
printk(BIOS_DEBUG, "0x5F00 :%lx\n", *ddTemp );
	*ddTemp |= 0x01;
printk(BIOS_DEBUG, "0x5F00 :%lx\n", *ddTemp );


/*
{
	unsigned long reg1 = 5;

	reg1 |= 0x80000000;	// set running bit
	MCHBAR32(0x5e00) = reg1;
	while (reg1 & 0x80000000) {
		printk(BIOS_DEBUG, " PLL busy...");
		reg1 = MCHBAR32(0x5e00);
	}
}
*/
	/* Erratum workarounds */
	//reg32 = MCHBAR32(0x5f00);
	//reg32 |= (1 << 9)|(1 << 10);
	//MCHBAR32(0x5f00) = reg32;

	/* Enable SA Clock Gating */
	//reg32 = MCHBAR32(0x5f00);
	//MCHBAR32(0x5f00) = reg32 | 1;

}
#endif
/*
printk(BIOS_DEBUG, "Dump fed10000 \n");
dump_memory((unsigned char *)0xfed10000, 0x100 );
printk(BIOS_DEBUG, "Dump fed15000 \n");
dump_memory((unsigned char *)(0xfed10000 + 0x5000), 0x100 );
*/
	printk(BIOS_DEBUG, "PcdRegionTerminator %x\n", UpdData->PcdRegionTerminator );
	
}
#else	/* IS_ENABLED(CONFIG_SOUTHBRIDGE_INTEL_FSP_I89XX) */
const PLATFORM_CONFIG DefaultPlatformConfig = {
#if IS_ENABLED(CONFIG_DISABLE_SANDYBRIDGE_HYPERTHREADING)
	FALSE,  /* Hyperthreading */
#else
	TRUE,  /* Hyperthreading */
#endif
	FALSE, /* Turbo Mode */
	FALSE, /* Memory Down */
#if IS_ENABLED(CONFIG_ENABLE_FSP_FAST_BOOT)
	TRUE,  /* Fast Boot */
#else
	FALSE, /* Fast Boot */
#endif	/* CONFIG_ENABLE_FSP_FAST_BOOT */
};
#endif	/* IS_ENABLED(CONFIG_SOUTHBRIDGE_INTEL_FSP_I89XX) */

/*
 *
 * Call the FSP to do memory init. The FSP doesn't return to this function.
 * The FSP returns to the romstage_main_continue().
 *
 */
void chipset_fsp_early_init(FSP_INIT_PARAMS *FspInitParams,
		FSP_INFO_HEADER *fsp_ptr)
{
	FSP_INIT_RT_BUFFER *pFspRtBuffer = FspInitParams->RtBufferPtr;
#if 1	//IS_ENABLED(CONFIG_SOUTHBRIDGE_INTEL_FSP_I89XX)
	UPD_DATA_REGION *fsp_upd_data = pFspRtBuffer->Common.UpdDataRgnPtr;
#else
	MEM_CONFIG MemoryConfig;
	memset((void*)&MemoryConfig, 0, sizeof(MEM_CONFIG));
#endif
	FspInitParams->NvsBufferPtr = NULL;

#if 1	//IS_ENABLED(CONFIG_SOUTHBRIDGE_INTEL_FSP_I89XX)
	/* Initialize the UPD Data */
	GetUpdDefaultFromFsp (fsp_ptr, fsp_upd_data);
	ConfigureDefaultUpdData(fsp_upd_data);
#else
	pFspRtBuffer->Platform.MemoryConfig = &MemoryConfig;
	pFspRtBuffer->PlatformConfiguration.PlatformConfig = &DefaultPlatformConfig;
#endif

#if IS_ENABLED(CONFIG_ENABLE_FSP_FAST_BOOT)
	/* Find the fastboot cache that was saved in the ROM */
	FspInitParams->NvsBufferPtr = find_and_set_fastboot_cache();
#endif
printk(BIOS_DEBUG, "GetUpdDefaultFromFsp %d, NvsBuffer Ptr %x\n", __LINE__, (unsigned int)FspInitParams->NvsBufferPtr );
	pFspRtBuffer->Common.BootMode = 0x00;	//BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS
printk(BIOS_DEBUG, "GetUpdDefaultFromFsp %d\n", __LINE__ );	
}

/* The FSP returns here after the fsp_early_init call */
void ChipsetFspReturnPoint(EFI_STATUS Status,
		VOID *HobListPtr)
{
printk(BIOS_DEBUG, "ChipsetFspReturnPoint %lx\n", (long unsigned int)Status );
	if (Status == 0xFFFFFFFF) {
		hard_reset();
	}
printk(BIOS_DEBUG, "ChipsetFspReturnPoint HobListPtr:%lx\n", (long unsigned int)HobListPtr );
	romstage_main_continue(Status, HobListPtr);
}

#endif	/* __PRE_RAM__ */
