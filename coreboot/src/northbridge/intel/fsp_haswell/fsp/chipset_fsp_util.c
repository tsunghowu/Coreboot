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

const UINT32 AzaliaTableData[] =  {
    //===================================================================================================
    //
    //                               Realtek Semiconductor Corp.
    //
    //===================================================================================================
    
    //Realtek High Definition Audio Configuration
    //Realtek HD Audio Codec : ALC892
    //PCI PnP ID : PCI\VEN_0000&DEV_0000&SUBSYS_00000000
    //HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0892&SUBSYS_00000000
    //The number of verb command block : 16
    
    //    NID 0x11 : 0x411110F0
    //    NID 0x12 : 0x411111F0
    //    NID 0x14 : 0x0101F030
    //    NID 0x15 : 0x01011031
    //    NID 0x16 : 0x01016032
    //    NID 0x17 : 0x41012033
    //    NID 0x18 : 0x01A19850
    //    NID 0x19 : 0x42A19C80
    //    NID 0x1A : 0x01813051
    //    NID 0x1B : 0x42214C40
    //    NID 0x1C : 0x5993105F
    //    NID 0x1D : 0x40000100
    //    NID 0x1E : 0x01442070
    //    NID 0x1F : 0x41C46060
    
    
    //===== HDA Codec Subsystem ID Verb-table =====
    //HDA Codec Subsystem ID  : 0x80867270
    0x00172070,
    0x00172172,
    0x00172286,
    0x00172380,
    
    
    //===== Pin Widget Verb-table =====
    //Widget node 0x01 :
    0x0017FF00,
    0x0017FF00,
    0x0017FF00,
    0x0017FF00,
    //Pin widget 0x11 S/PDIF-OUT2
    0x01171CF0,
    0x01171D10,
    0x01171E11,
    0x01171F41,
    //Pin widget 0x12 DMIC
    0x01271CF0,
    0x01271D11,
    0x01271E11,
    0x01271F41,
    //Pin widget 0x14 FRONT (Port-D)
    0x01471C30,
    0x01471DF0,
    0x01471E01,
    0x01471F01,
    //Pin widget 0x15 SURR (Port-A)
    0x01571C31,
    0x01571D10,
    0x01571E01,
    0x01571F01,
    //Pin widget 0x16 CEN/LFE (Port-G)
    0x01671C32,
    0x01671D60,
    0x01671E01,
    0x01671F01,
    //Pin widget 0x17 SIDESURR (Port-H)
    0x01771C33,
    0x01771D20,
    0x01771E01,
    0x01771F41,
    //Pin widget 0x18 MIC1 (Port-B)
    0x01871C50,
    0x01871D98,
    0x01871EA1,
    0x01871F01,
    //Pin widget 0x19 MIC2 (Port-F)
    0x01971C80,
    0x01971D9C,
    0x01971EA1,
    0x01971F42,
    //Pin widget 0x1A LINE1 (Port-C)
    0x01A71C51,
    0x01A71D30,
    0x01A71E81,
    0x01A71F01,
    //Pin widget 0x1B LINE2 (Port-E)
    0x01B71C40,
    0x01B71D4C,
    0x01B71E21,
    0x01B71F42,
    //Pin widget 0x1C CD-IN
    0x01C71C5F,
    0x01C71D10,
    0x01C71E93,
    0x01C71F59,
    //Pin widget 0x1D BEEP-IN
    0x01D71C00,
    0x01D71D01,
    0x01D71E00,
    0x01D71F40,
    //Pin widget 0x1E S/PDIF-OUT
    0x01E71C70,
    0x01E71D20,
    0x01E71E44,
    0x01E71F02,
};
const UINT32 CrbHdaVerbTblADLINK[] = {	
//HDACfg Version : 3.99
//HD Audio Codec : ALC888/ALC888S
//PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_088810EC
//HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0888&SUBSYS_10EC0888
//Pin Widget Count : 14

//    NID 0x14 : 0x01014410
//    NID 0x15 : 0x01011412
//    NID 0x16 : 0x01016411
//    NID 0x17 : 0x01012414
//    NID 0x18 : 0x01A19C30
//    NID 0x19 : 0x02A19C31
//    NID 0x1A : 0x01813C3F
//    NID 0x1B : 0x02214C1F
//    NID 0x1C : 0x411111F0
//    NID 0x1D : 0x4005E601
//    NID 0x1E : 0x014B1120
//    NID 0x1F : 0x01CB1140
//    NID 0x11 : 0x411111F0
//    NID 0x12 : 0x411111F0


//===== HDA Codec Subsystem ID Verb-table =====
//HDA Codec Subsystem ID  : 0x10EC0888
0x00172088,
0x00172108,
0x001722EC,
0x00172310,

//===== Pin Widget Verb-table =====
//Pin Complex (NID 0x14 )
0x01471C10,
0x01471D44,
0x01471E01,
0x01471F01,
//Pin Complex (NID 0x15 )
0x01571C12,
0x01571D14,
0x01571E01,
0x01571F01,
//Pin Complex (NID 0x16 )
0x01671C11,
0x01671D64,
0x01671E01,
0x01671F01,
//Pin Complex (NID 0x17 )
0x01771C14,
0x01771D24,
0x01771E01,
0x01771F01,
//Pin Complex (NID 0x18 )
0x01871C30,
0x01871D9C,
0x01871EA1,
0x01871F01,
//Pin Complex (NID 0x19 )
0x01971C31,
0x01971D9C,
0x01971EA1,
0x01971F02,
//Pin Complex (NID 0x1A )
0x01A71C3F,
0x01A71D3C,
0x01A71E81,
0x01A71F01,
//Pin Complex (NID 0x1B )
0x01B71C1F,
0x01B71D4C,
0x01B71E21,
0x01B71F02,
//Pin Complex (NID 0x1C )
0x01C71CF0,
0x01C71D11,
0x01C71E11,
0x01C71F41,
//Pin Complex (NID 0x1D )
0x01D71C01,
0x01D71DE6,
0x01D71E05,
0x01D71F40,
//Pin Complex (NID 0x1E )
0x01E71C20,
0x01E71D11,
0x01E71E4B,
0x01E71F01,
//Pin Complex (NID 0x1F )
0x01F71C40,
0x01F71D11,
0x01F71ECB,
0x01F71F01,
//Pin Complex (NID 0x11 )
0x01171CF0,
0x01171D11,
0x01171E11,
0x01171F41,
//Pin Complex (NID 0x12 )
0x01271CF0,
0x01271D11,
0x01271E11,
0x01271F41
};

const AUDIO_AZALIA_VERB_TABLE AzaliaVerbTable = {
  //
  //  VerbTable: (RealTek ALC892)
  //  Revision ID = 0xff
  //  Codec Verb Table For Azalia on Lava Canyon
  //  Codec Address: CAd value (0/1/2)
  //  Codec Vendor: 0x10EC0892
  //
    {
    0x10EC,     // Vendor ID
    0x0888,     // Device ID
    0xFF,       // Revision ID (any)
    0xFF,       // SDI number (any)
    15 * 4,     // Data size in DWORDs
    0x00000000  // Reserved
    },
    (UINT32 *)CrbHdaVerbTblADLINK
};

const UINT32 SaAudioTableData[] = {
  ///
  /// Audio Verb Table - 0x80862807
  ///
  /// Pin Widget 5 - PORT B
  ///
  0x00571C10,
  0x00571D00,
  0x00571E56,
  0x00571F18,
  ///
  /// Pin Widget 6 - PORT C
  ///
  0x00671C20,
  0x00671D00,
  0x00671E56,
  0x00671F19,

};


const AUDIO_SA_VERB_TABLE SaVerbTable = {
  ///
  ///  VerbTable:
  ///  Revision ID = 0xFF
  ///  Codec Vendor: 0x80862807
  ///
    {
    0x80862807, ///< Vendor ID/Device ID
    0x0000,     ///< SubSystem ID
    0xFF,       ///< Revision ID
    0x02,       ///< Front panel support (1=yes, 2=no)
    0x0002,     ///< Number of Rear Jacks
    0x0000,     ///< Number of Front Jacks
    },
    (UINT32 *)SaAudioTableData
};

static void ConfigureDefaultUpdData(UPD_DATA_REGION   *UpdData)
{/*
	UpdData->HTEnable = TRUE;
	UpdData->TurboEnable = FALSE;
	UpdData->MemoryDownEnable = FALSE;
	UpdData->FastBootEnable = CONFIG_ENABLE_FSP_FAST_BOOT;
*/
	
	UpdData->SerialDebugPortAddress = 0x3F8;
	UpdData->SerialDebugPortType = 1;
	/*
	UpdData->PcdMrcDebugMsg = TRUE;
	UpdData->PcdEnableRmt = TRUE;
	UpdData->PcdUserCrbBoardType = 0;
	*/
	printk(BIOS_DEBUG, "PcdFastBoot %x\n", UpdData->PcdFastBoot );
	UpdData->PcdEnableAzalia = TRUE;
	UpdData->SaHdaVerbTablePtr  = (AUDIO_SA_VERB_TABLE *)&SaVerbTable;
  UpdData->AzaliaVerbTablePtr = (AUDIO_AZALIA_VERB_TABLE *)&AzaliaVerbTable;
  
  UpdData->PcdInternalGfx = 1;
  UpdData->PcdPrimaryDisplay = 0;
//UpdData->PcdFastBoot = 0;

//printk(BIOS_DEBUG, "PcdMmioSize %x\n", UpdData->PcdMmioSize );
//printk(BIOS_DEBUG, "PcdApertureSize %x\n", UpdData->PcdApertureSize );
//printk(BIOS_DEBUG, "PcdIgdDvmt50PreAlloc %x\n", UpdData->PcdIgdDvmt50PreAlloc );
//printk(BIOS_DEBUG, "PcdTsegSize %x\n", UpdData->PcdTsegSize );

//printk(BIOS_DEBUG, "Dump B0, D0, F0:  \n");
//dump_memory((unsigned char *)0xF8000000, 0x100 );
//dump_pci_context(0,0,0, 0x100);
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
