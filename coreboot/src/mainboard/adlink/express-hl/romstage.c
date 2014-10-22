/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
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

#include <stdint.h>
#include <stddef.h>
#include <arch/cpu.h>
#include <lib.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <device/pnp_def.h>
#include <arch/stages.h>
#include <console/console.h>
#include <cbmem.h>
#include <ramstage_cache.h>
#include <romstage_handoff.h>
#include <reset.h>
#include <cpu/x86/lapic.h>
#include <fsp_util.h>
#include "cpu/intel/fsp_haswell/haswell.h"
#include "northbridge/intel/fsp_haswell/haswell.h"
#include "northbridge/intel/fsp_haswell/raminit.h"
#include "southbridge/intel/fsp_lynxpoint/pch.h"
#include "southbridge/intel/fsp_lynxpoint/gpio.h"
#include "southbridge/intel/fsp_lynxpoint/me.h"
#include "gpio.h"
#include <console/cbmem_console.h>
#include "superio/nuvoton/common/nuvoton.h"		//<CHW+>
#include <superio/nuvoton/nct5104d/nct5104d.h>	//<CHW+>
#define NUVOTON_SERIAL_DEV_A PNP_DEV(0x4e, NCT5104D_SP1)
#define NUVOTON_SERIAL_DEV_B PNP_DEV(0x4e, NCT5104D_SP2)

const struct rcba_config_instruction rcba_config[] = {

	/*
	 *             GFX    INTA -> PIRQA (MSI)
	 * D28IP_P1IP  WLAN   INTA -> PIRQB
	 * D28IP_P4IP  ETH0   INTB -> PIRQC
	 * D29IP_E1P   EHCI1  INTA -> PIRQD
	 * D20IP_XHCI  XHCI   INTA -> PIRQA
	 * D31IP_SIP   SATA   INTA -> PIRQF (MSI)
	 * D31IP_SMIP  SMBUS  INTB -> PIRQG
	 * D31IP_TTIP  THRT   INTC -> PIRQH
	 * D27IP_ZIP   HDA    INTA -> PIRQG (MSI)
	 */

	/* Device interrupt pin register (board specific) */
	RCBA_SET_REG_32(D31IP, (INTC << D31IP_TTIP) | (NOINT << D31IP_SIP2) |
			(INTB << D31IP_SMIP) | (INTA << D31IP_SIP)),
	RCBA_SET_REG_32(D29IP, (INTA << D29IP_E1P)),
	RCBA_SET_REG_32(D28IP, (INTA << D28IP_P1IP) | (INTC << D28IP_P3IP) |
			(INTB << D28IP_P4IP)),
	RCBA_SET_REG_32(D27IP, (INTA << D27IP_ZIP)),
	RCBA_SET_REG_32(D26IP, (INTA << D26IP_E2P)),
	RCBA_SET_REG_32(D25IP, (NOINT << D25IP_LIP)),
	RCBA_SET_REG_32(D22IP, (NOINT << D22IP_MEI1IP)),
	RCBA_SET_REG_32(D20IR, (INTA << D20IP_XHCI)),

	/* Device interrupt route registers */
	RCBA_SET_REG_32(D31IR, DIR_ROUTE(PIRQF, PIRQG, PIRQH, PIRQA)),
	RCBA_SET_REG_32(D29IR, DIR_ROUTE(PIRQD, PIRQE, PIRQF, PIRQG)),
	RCBA_SET_REG_32(D28IR, DIR_ROUTE(PIRQB, PIRQC, PIRQD, PIRQE)),
	RCBA_SET_REG_32(D27IR, DIR_ROUTE(PIRQG, PIRQH, PIRQA, PIRQB)),
	RCBA_SET_REG_32(D26IR, DIR_ROUTE(PIRQE, PIRQF, PIRQG, PIRQH)),
	RCBA_SET_REG_32(D25IR, DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD)),
	RCBA_SET_REG_32(D22IR, DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD)),
	RCBA_SET_REG_32(D21IR, DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD)),
	RCBA_SET_REG_32(D20IR, DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD)),
	RCBA_SET_REG_32(D23IR, DIR_ROUTE(PIRQA, 0, 0, 0)),

	/* Disable unused devices (board specific) */
	RCBA_RMW_REG_32(FD, ~0, PCH_DISABLE_ALWAYS),

	RCBA_END_CONFIG,
};

const struct rcba_config_instruction pch_early_config[] = {
	/* Enable IOAPIC */
	RCBA_SET_REG_16(OIC, 0x0100),
	/* PCH BWG says to read back the IOAPIC enable register */
	RCBA_READ_REG_16(OIC),

	RCBA_END_CONFIG,
};
#if 0
void mainboard_romstage_entry(unsigned long bist)
{
	struct pei_data pei_data = {
		.pei_version = PEI_VERSION,
		.mchbar = DEFAULT_MCHBAR,
		.dmibar = DEFAULT_DMIBAR,
		.epbar = DEFAULT_EPBAR,
		.pciexbar = DEFAULT_PCIEXBAR,
		.smbusbar = SMBUS_IO_BASE,
		.wdbbar = 0x4000000,
		.wdbsize = 0x1000,
		.hpet_address = HPET_ADDR,
		.rcba = DEFAULT_RCBA,
		.pmbase = DEFAULT_PMBASE,
		.gpiobase = DEFAULT_GPIOBASE,
		.temp_mmio_base = 0xfed08000,
		.system_type = 5, /* ULT */
		.tseg_size = CONFIG_SMM_TSEG_SIZE,
		.spd_addresses = { 0xa2, 0x00, 0xa2, 0x00 },
		.ec_present = 1,
		// 0 = leave channel enabled
		// 1 = disable dimm 0 on channel
		// 2 = disable dimm 1 on channel
		// 3 = disable dimm 0+1 on channel
		.dimm_channel0_disabled = 2,
		.dimm_channel1_disabled = 2,
		.max_ddr3_freq = 1600,
		.usb2_ports = {
			/* Length, Enable, OCn# */
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P0: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P1: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P2: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P3: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P4: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P5: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 1, USB_OC_PIN_SKIP, /* P6: */
			  USB_PORT_FRONT_PANEL },
			{ 0x40, 0, USB_OC_PIN_SKIP, /* P7: */
			  USB_PORT_FRONT_PANEL },
		},
		.usb3_ports = {
			/* Enable, OCn# */
			{ 1, USB_OC_PIN_SKIP }, /* P1; */
			{ 1, USB_OC_PIN_SKIP }, /* P2; */
			{ 1, USB_OC_PIN_SKIP }, /* P3; */
			{ 1, USB_OC_PIN_SKIP }, /* P4; */
		},
	};

	struct romstage_params romstage_params = {
		.pei_data = &pei_data,
		.gpio_map = &mainboard_gpio_map,
		.rcba_config = &rcba_config[0],
		.bist = bist,
		.copy_spd = NULL,
	};

	/* Call into the real romstage main with this board's attributes. */
	romstage_common(&romstage_params);
}
#endif

static inline void reset_system(void)
{
	hard_reset();
	while (1) {
		hlt();
	}
}

void main(FSP_INFO_HEADER *fsp_info_header)
{
#if IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)
	int boot_mode = 0;
#endif
	u32 pm1_cnt;
	u16 pm1_sts;

	post_code(0x40);
	
#if CONFIG_COLLECT_TIMESTAMPS
	tsc_t start_romstage_time;
	tsc_t before_initram_time;

	start_romstage_time = rdtsc();

	/* since this mainboard doesn't use audio, we can stuff the TSC values in there */
	pci_write_config32(PCI_DEV(0, 27, 0), 0x2c,  start_romstage_time.lo >> 4 |
			start_romstage_time.lo << 28);
#endif
#if 0
	pch_enable_lpc();

	/* Enable GPIOs */
	pci_write_config32(PCH_LPC_DEV, GPIO_BASE, DEFAULT_GPIOBASE|1);
	pci_write_config8(PCH_LPC_DEV, GPIO_CNTL, 0x10);
	setup_pch_gpios(&mainboard_gpio_map);
	//setup_sio_gpios();

	console_init();
#endif	
#if 1
	pch_enable_lpc();

	nuvoton_enable_serial(NUVOTON_SERIAL_DEV_A, CONFIG_TTYS0_BASE);
	pch_enable_bars();

#if CONFIG_INTEL_LYNXPOINT_LP
	setup_pch_lp_gpios(mainboard_gpio_map);
#else
	//TBD setup_pch_gpios(mainboard_gpio_map);
#endif

	console_init();

	pch_generic_setup();

	/* Enable SMBus for reading SPDs. */
	enable_smbus();
#if 1		//Send "I'm alive" to ADLINK BMC.
	{
		unsigned short smbusbar;
		smbusbar = pci_read_config16(PCI_DEV(0, 0x1f, 3), SMB_BASE);
		smbusbar &= 0xFFF0;
		printk(BIOS_DEBUG, "smbusbar %x\n", smbusbar );
		{
			{
				unsigned char bBMC_ALIVE_Cmd[1] = {0x03};
				unsigned char bBMCSlaveAddr = 0x50;
				unsigned char bBMCSlaveCmd = 0x25;
				unsigned char nLength = 0x01;
				smbus_block_write(bBMCSlaveAddr, bBMCSlaveCmd, bBMC_ALIVE_Cmd, (unsigned char*)&nLength);
			}
		}
	
	}
#endif	
#endif
	/* Early PCH RCBA settings */
	pch_config_rcba(pch_early_config);

	post_code(0x41);

{
	struct cpuid_result cpuid_regs;
	cpuid_regs = cpuid(0x00000001);
	printk(BIOS_DEBUG, "Power Up... CPUID is %lx    ----- \n", (unsigned long int)cpuid_regs.eax );	
}

	post_code(0x42);

	post_code(0x43);
	haswell_early_initialization(HASWELL_MOBILE);

	post_code(0x44);
	/* Wait for ME to be ready */
//printk(BIOS_DEBUG, "%s %d\n", __FUNCTION__, __LINE__ );
	intel_early_me_status();
	intel_early_me_init();
	intel_early_me_uma_size();

	post_code(0x45);
	/* Check PM1_STS[15] to see if we are waking from Sx */
	pm1_sts = inw(DEFAULT_PMBASE + PM1_STS);

	/* Read PM1_CNT[12:10] to determine which Sx state */
	pm1_cnt = inl(DEFAULT_PMBASE + PM1_CNT);
	post_code(0x46);
	if ((pm1_sts & WAK_STS) && ((pm1_cnt >> 10) & 7) == 5) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Resume from S3 detected.\n");
		boot_mode = 2;
		/* Clear SLP_TYPE. This will break stage2 but
		 * we care for that when we get there.
		 */
		outl(pm1_cnt & ~(7 << 10), DEFAULT_PMBASE + PM1_CNT);
#else
		printk(BIOS_DEBUG, "Resume from S3 detected, but disabled.\n");
#endif
	}

	post_code(0x48);

#if CONFIG_COLLECT_TIMESTAMPS
	before_initram_time= rdtsc();
	/* since this mainboard doesn't use audio, we can stuff the TSC values in there */
	pci_write_config32(PCI_DEV(0, 27, 0), 0x14, before_initram_time.lo >> 4 |
			before_initram_time.lo << 28);

#endif

  /*
   * Call early init to initialize memory and chipset. This function returns
   * to the romstage_main_continue function with a pointer to the HOB
   * structure.
   */
	printk(BIOS_DEBUG, "Starting the Intel FSP (early_init)\n");
	fsp_early_init(fsp_info_header);
	die("Uh Oh! fsp_early_init should not return here.\n");
}

/*******************************************************************************
 * The FSP early_init function returns to this function.
 * Memory is setup and the stack is set by the FSP.
 ******************************************************************************/
void romstage_main_continue(EFI_STATUS status, VOID *HobListPtr) {
	int cbmem_was_initted;
	u32 reg32;
	void *cbmem_hob_ptr;
	struct romstage_handoff *handoff;
#if CONFIG_COLLECT_TIMESTAMPS
	tsc_t start_romstage_time;
	tsc_t base_time;
	tsc_t before_initram_time;
	tsc_t after_initram_time = rdtsc();
	u32 timebase = pci_read_config32(PCI_DEV(0, 0x1f, 2), 0xd0);
	u32 time_romstage_start = pci_read_config32(PCI_DEV(0, 27, 0), 0x2c);
	u32 time_before_initram = pci_read_config32(PCI_DEV(0, 27, 0), 0x14);

	base_time.lo = timebase << 4;
	base_time.hi = timebase >> 28;
	start_romstage_time.lo = time_romstage_start << 4;
	start_romstage_time.hi = time_romstage_start >> 28;
	before_initram_time.lo = time_before_initram << 4;
	before_initram_time.hi = time_before_initram >> 28;
#endif

	/*
	 * HD AUDIO is not used on this system, so we're using some registers
	 * in there as temporary registers to save TSC values.  This is complete
	 * now, so disable the audio block.
	 */
	reg32 = RCBA32(FD);
	reg32 |= PCH_DISABLE_HD_AUDIO;
	RCBA32(FD) = reg32;

	post_code(0x49);

#if CONFIG_USBDEBUG
	/* FSP reconfigures USB, so reinit it to have debug */
	early_usbdebug_init();
#endif

	/* For reference print FSP version */
	u32 version = MCHBAR32(0x5034);
	printk(BIOS_DEBUG, "FSP Version %d.%d.%d Build %d\n",
		version >> 24 , (version >> 16) & 0xff,
		(version >> 8) & 0xff, version & 0xff);
	printk(BIOS_DEBUG, "FSP Status: 0x%0x\n", (u32)status);

	intel_early_me_init_done(ME_INIT_STATUS_SUCCESS);

	report_memory_config();

	post_code(0x4b);

	//early_pch_init();
	early_pch_init(&mainboard_gpio_map, &rcba_config[0]);

	post_code(0x4c);

	//rcba_config();
	post_code(0x4d);

	quick_ram_check();
	post_code(0x4e);
	

	cbmem_was_initted = 0;
	cbmem_initialize_empty();

//	if(cbmem_was_initted) {
//		reset_system();
//	}
	handoff = romstage_handoff_find_or_add();
	if (handoff != NULL)
		handoff->s3_resume = 0;
	else
		printk(BIOS_DEBUG, "Romstage handoff structure not added!\n");

	/* Save the HOB pointer in CBMEM to be used in ramstage. */
	cbmem_hob_ptr = cbmem_add (CBMEM_ID_HOB_POINTER, sizeof(*HobListPtr));
	*(u32*)cbmem_hob_ptr = (u32)HobListPtr;
	post_code(0x4f);
#if CONFIG_COLLECT_TIMESTAMPS
	timestamp_init(base_time);
	timestamp_add(TS_START_ROMSTAGE, start_romstage_time );
	timestamp_add(TS_BEFORE_INITRAM, before_initram_time );
	timestamp_add(TS_AFTER_INITRAM, after_initram_time);
	timestamp_add_now(TS_END_ROMSTAGE);
#endif
#if IS_ENABLED(CONFIG_CONSOLE_CBMEM)
//	printk(BIOS_DEBUG, "cbmemc_reinit\n");
//	cbmemc_reinit();
#endif
	/* Load the ramstage. */
	copy_and_run();
	while (1);
}

void romstage_fsp_rt_buffer_callback(FSP_INIT_RT_BUFFER *FspRtBuffer)
{
	/* No overrides needed */
	return;
}
