/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
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
#include <stdlib.h>
#include <device/device.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <console/console.h>
#include <delay.h>
#include <pc80/mc146818rtc.h>
#include <arch/acpi.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <boot/coreboot_tables.h>
#include "hda_verb.h"
#include <smbios.h>
#include <device/pci.h>
#include <ec/google/chromeec/ec.h>
#include <cbfs_core.h>

#include <cpu/x86/tsc.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/msr.h>
#include <edid.h>
#include <drivers/intel/gma/i915.h>
#include "mainboard.h"

/*
 * Here is the rough outline of how we bring up the display:
 *  1. Upon power-on Sink generates a hot plug detection pulse thru HPD
 *  2. Source determines video mode by reading DPCD receiver capability field
 *     (DPCD 00000h to 0000Dh) including eDP CP capability register (DPCD
 *     0000Dh).
 *  3. Sink replies DPCD receiver capability field.
 *  4. Source starts EDID read thru I2C-over-AUX.
 *  5. Sink replies EDID thru I2C-over-AUX.
 *  6. Source determines link configuration, such as MAX_LINK_RATE and
 *     MAX_LANE_COUNT. Source also determines which type of eDP Authentication
 *     method to use and writes DPCD link configuration field (DPCD 00100h to
 *     0010Ah) including eDP configuration set (DPCD 0010Ah).
 *  7. Source starts link training. Sink does clock recovery and equalization.
 *  8. Source reads DPCD link status field (DPCD 00200h to 0020Bh).
 *  9. Sink replies DPCD link status field. If main link is not stable, Source
 *     repeats Step 7.
 * 10. Source sends MSA (Main Stream Attribute) data. Sink extracts video
 *     parameters and recovers stream clock.
 * 11. Source sends video data.
 */

/* how many bytes do we need for the framebuffer?
 * Well, this gets messy. To get an exact answer, we have
 * to ask the panel, but we'd rather zero the memory
 * and set up the gtt while the panel powers up. So,
 * we take a reasonable guess, secure in the knowledge that the
 * MRC has to overestimate the number of bytes used.
 * 8 MiB is a very safe guess. There may be a better way later, but
 * fact is, the initial framebuffer is only very temporary. And taking
 * a little long is ok; this is done much faster than the AUX
 * channel is ready for IO.
 */
#define FRAME_BUFFER_BYTES (8*MiB)
/* how many 4096-byte pages do we need for the framebuffer?
 * There are hard ways to get this, and easy ways:
 * there are FRAME_BUFFER_BYTES/4096 pages, since pages are 4096
 * on this chip (and in fact every Intel graphics chip we've seen).
 */
#define FRAME_BUFFER_PAGES (FRAME_BUFFER_BYTES/(4096))

static unsigned int *mmio;
static unsigned int graphics;
static unsigned int physbase;

void ug1(int);
void ug2(int);
void ug22(int);
void ug3(int);

/* GTT is the Global Translation Table for the graphics pipeline.
 * It is used to translate graphics addresses to physical
 * memory addresses. As in the CPU, GTTs map 4K pages.
 * The setgtt function adds a further bit of flexibility:
 * it allows you to set a range (the first two parameters) to point
 * to a physical address (third parameter);the physical address is
 * incremented by a count (fourth parameter) for each GTT in the
 * range.
 * Why do it this way? For ultrafast startup,
 * we can point all the GTT entries to point to one page,
 * and set that page to 0s:
 * memset(physbase, 0, 4096);
 * setgtt(0, 4250, physbase, 0);
 * this takes about 2 ms, and is a win because zeroing
 * the page takes a up to 200 ms.
 * This call sets the GTT to point to a linear range of pages
 * starting at physbase.
 */

#define GTT_PTE_BASE (2 << 20)

static void
setgtt(int start, int end, unsigned long base, int inc)
{
	int i;

	for(i = start; i < end; i++){
		u32 word = base + i*inc;
		/* note: we've confirmed by checking
		 * the values that mrc does no
		 * useful setup before we run this.
		 */
		gtt_write(GTT_PTE_BASE + i * 4, word|1);
		gtt_read(GTT_PTE_BASE + i * 4);
	}
}

static int i915_init_done = 0;

/* fill the palette. */
static void palette(void)
{
	int i;
	unsigned long color = 0;

	for(i = 0; i < 256; i++, color += 0x010101){
		gtt_write(_LGC_PALETTE_A + (i<<2),color);
	}
}

void dp_init_dim_regs(struct intel_dp *dp);
void dp_init_dim_regs(struct intel_dp *dp)
{
	struct edid *edid = &(dp->edid);

	dp->bytes_per_pixel = edid->bpp / 8;

	dp->stride = edid->bytes_per_line;

	dp->htotal = (edid->ha - 1) | ((edid->ha + edid->hbl - 1) << 16);

	dp->hblank = (edid->ha - 1) | ((edid->ha + edid->hbl - 1) << 16);

	dp->hsync = (edid->ha + edid->hso - 1) |
		((edid->ha + edid->hso + edid->hspw - 1) << 16);

	dp->vtotal = (edid->va - 1) | ((edid->va + edid->vbl - 1) << 16);

	dp->vblank = (edid->va - 1) | ((edid->va + edid->vbl - 1) << 16);

	dp->vsync = (edid->va + edid->vso - 1) |
		((edid->va + edid->vso + edid->vspw - 1) << 16);

	/* PIPEASRC is wid-1 x ht-1 */
	dp->pipesrc = (edid->ha-1)<<16 | (edid->va-1);

	dp->pfa_pos = 0;

	dp->pfa_ctl = 0x80800000;

	dp->pfa_sz = (edid->ha << 16) | (edid->va);

	dp->flags = intel_ddi_calc_transcoder_flags(3 * 6,  /* bits per color is 6 */
						    dp->port,
						    dp->pipe,
						    dp->type,
						    dp->lane_count,
						    dp->pfa_sz,
						    dp->edid.phsync == '+'?1:0,
						    dp->edid.pvsync == '+'?1:0);

	dp->transcoder = intel_ddi_get_transcoder(dp->port,
						  dp->pipe);

	intel_dp_compute_m_n(dp->bpp,
			     dp->lane_count,
			     dp->edid.pixel_clock,
			     dp->edid.link_clock,
			     &dp->m_n);

	printk(BIOS_SPEW, "dp->stride  = 0x%08x\n",dp->stride);
	printk(BIOS_SPEW, "dp->htotal  = 0x%08x\n", dp->htotal);
	printk(BIOS_SPEW, "dp->hblank  = 0x%08x\n", dp->hblank);
	printk(BIOS_SPEW, "dp->hsync   = 0x%08x\n", dp->hsync);
	printk(BIOS_SPEW, "dp->vtotal  = 0x%08x\n", dp->vtotal);
	printk(BIOS_SPEW, "dp->vblank  = 0x%08x\n", dp->vblank);
	printk(BIOS_SPEW, "dp->vsync   = 0x%08x\n", dp->vsync);
	printk(BIOS_SPEW, "dp->pipesrc = 0x%08x\n", dp->pipesrc);
	printk(BIOS_SPEW, "dp->pfa_pos = 0x%08x\n", dp->pfa_pos);
	printk(BIOS_SPEW, "dp->pfa_ctl = 0x%08x\n", dp->pfa_ctl);
	printk(BIOS_SPEW, "dp->pfa_sz  = 0x%08x\n", dp->pfa_sz);
	printk(BIOS_SPEW, "dp->link_m  = 0x%08x\n", dp->m_n.link_m);
	printk(BIOS_SPEW, "dp->link_n  = 0x%08x\n", dp->m_n.link_n);
	printk(BIOS_SPEW, "0x6f030     = 0x%08x\n", TU_SIZE(dp->m_n.tu) | dp->m_n.gmch_m);
	printk(BIOS_SPEW, "0x6f030     = 0x%08x\n", dp->m_n.gmch_m);
	printk(BIOS_SPEW, "0x6f034     = 0x%08x\n", dp->m_n.gmch_n);
	printk(BIOS_SPEW, "dp->flags   = 0x%08x\n", dp->flags);
}

int intel_dp_bw_code_to_link_rate(u8 link_bw);

int intel_dp_bw_code_to_link_rate(u8 link_bw)
{
	switch (link_bw) {
        case DP_LINK_BW_1_62:
        default:
		return 162000;
        case DP_LINK_BW_2_7:
		return 270000;
        case DP_LINK_BW_5_4:
                return 540000;
	}
}

void mainboard_train_link(struct intel_dp *intel_dp)
{
	u8 read_val;
	u8 link_status[DP_LINK_STATUS_SIZE];

	gtt_write(DP_TP_CTL(intel_dp->port),DP_TP_CTL_ENABLE | DP_TP_CTL_ENHANCED_FRAME_ENABLE);
	gtt_write(DP_A, DP_PORT_EN | DP_LINK_TRAIN_PAT_1 | DP_LINK_TRAIN_PAT_1_CPT | DP_VOLTAGE_0_4 | DP_PRE_EMPHASIS_0 | DP_PORT_WIDTH_1 | DP_PLL_FREQ_270MHZ | DP_SYNC_VS_HIGH |0x80000011);

	intel_dp_get_training_pattern(intel_dp, &read_val);
	intel_dp_set_training_pattern(intel_dp, DP_TRAINING_PATTERN_1 | DP_LINK_QUAL_PATTERN_DISABLE | DP_SYMBOL_ERROR_COUNT_BOTH);
	intel_dp_get_lane_count(intel_dp, &read_val);
	intel_dp_set_training_lane0(intel_dp, DP_TRAIN_VOLTAGE_SWING_400 | DP_TRAIN_PRE_EMPHASIS_0);
	intel_dp_get_link_status(intel_dp, link_status);

	gtt_write(DP_TP_CTL(intel_dp->port),DP_TP_CTL_ENABLE | DP_TP_CTL_ENHANCED_FRAME_ENABLE | DP_TP_CTL_LINK_TRAIN_PAT2);

	intel_dp_get_training_pattern(intel_dp, &read_val);
	intel_dp_set_training_pattern(intel_dp, DP_TRAINING_PATTERN_2 | DP_LINK_QUAL_PATTERN_DISABLE | DP_SYMBOL_ERROR_COUNT_BOTH);
	intel_dp_get_link_status(intel_dp, link_status);
	intel_dp_get_lane_align_status(intel_dp, &read_val);
	intel_dp_get_training_pattern(intel_dp, &read_val);
	intel_dp_set_training_pattern(intel_dp, DP_TRAINING_PATTERN_DISABLE | DP_LINK_QUAL_PATTERN_DISABLE | DP_SYMBOL_ERROR_COUNT_BOTH);
}

#define TEST_GFX 0

#if TEST_GFX
static void test_gfx(struct intel_dp *dp)
{
	int i;

	/* This is a sanity test code which fills the screen with two bands --
	   green and blue. It is very useful to ensure all the initializations
	   are made right. Thus, to be used only for testing, not otherwise
	*/
	for (i = 0; i < (dp->edid.va - 4); i++) {
		u32 *l;
		int j;
		u32 tcolor = 0x0ff;
		for (j = 0; j < (dp->edid.ha-4); j++) {
			if (j == (dp->edid.ha/2)) {
				tcolor = 0xff00;
			}
			l = (u32*)(graphics + i * dp->stride + j * sizeof(tcolor));
			memcpy(l,&tcolor,sizeof(tcolor));
		}
	}
}
#else
static void test_gfx(struct intel_dp *dp) {}
#endif


void mainboard_set_port_clk_dp(struct intel_dp *intel_dp)
{
	u32 ddi_pll_sel = 0;

	switch (intel_dp->link_bw) {
	case DP_LINK_BW_1_62:
		ddi_pll_sel = PORT_CLK_SEL_LCPLL_810;
		break;
	case DP_LINK_BW_2_7:
		ddi_pll_sel = PORT_CLK_SEL_LCPLL_1350;
		break;
	case DP_LINK_BW_5_4:
		ddi_pll_sel = PORT_CLK_SEL_LCPLL_2700;
		break;
	default:
		printk(BIOS_ERR, "invalid link bw %d\n", intel_dp->link_bw);
		return;
	}

	gtt_write(PORT_CLK_SEL(intel_dp->port), ddi_pll_sel);
}

int i915lightup(unsigned int pphysbase, unsigned int pmmio,
		unsigned int pgfx, unsigned int init_fb)
{
	int must_cycle_power = 0;
	struct intel_dp adp, *dp = &adp;
	int i;
	int edid_ok;
	int pixels = FRAME_BUFFER_BYTES/64;

	mmio = (void *)pmmio;
	physbase = pphysbase;
	graphics = pgfx;
	printk(BIOS_SPEW,
	       "i915lightup: graphics %p mmio %p"
	       "physbase %08x\n",
	       (void *)graphics, mmio, physbase);

	void runio(struct intel_dp *dp);
	void runlinux(struct intel_dp *dp);
	dp->gen = 8; // This is gen 8 which we believe is Haswell
	dp->is_haswell = 1;
	dp->DP = 0x2;
	/* These values are used for training the link */
	dp->lane_count = 2;
	dp->link_bw = DP_LINK_BW_2_7;
	dp->panel_power_down_delay = 600;
	dp->panel_power_up_delay = 200;
	dp->panel_power_cycle_delay = 600;
	dp->pipe = PIPE_A;
	dp->port = PORT_A;
	dp->plane = PLANE_A;
	dp->clock = 160000;
	dp->bpp = 32;
	dp->type = INTEL_OUTPUT_EDP;
	dp->output_reg = DP_A;
	/* observed from YABEL. */
	dp->aux_clock_divider = 0xe1;
	dp->precharge = 3;

	/* 1. Normal mode: Set the first page to zero and make
	   all GTT entries point to the same page
	   2. Developer/Recovery mode: We do not zero out all
	   the pages pointed to by GTT in order to avoid wasting time */
        if (init_fb)
                setgtt(0, FRAME_BUFFER_PAGES, physbase, 4096);
        else {
                setgtt(0, FRAME_BUFFER_PAGES, physbase, 0);
                memset((void*)graphics, 0, 4096);
        }

	dp->address = 0x50;

	if ( !intel_dp_get_dpcd(dp) )
		goto fail;

	intel_dp_i2c_aux_ch(dp, MODE_I2C_WRITE, 0, NULL);
	for(dp->edidlen = i = 0; i < sizeof(dp->rawedid); i++){
		if (intel_dp_i2c_aux_ch(dp, MODE_I2C_READ,
					0x50, &dp->rawedid[i]) < 0)
			break;
		dp->edidlen++;
	}

	edid_ok = decode_edid(dp->rawedid, dp->edidlen, &dp->edid);
	printk(BIOS_SPEW, "decode edid returns %d\n", edid_ok);

	dp->edid.link_clock = intel_dp_bw_code_to_link_rate(dp->link_bw);

	printk(BIOS_SPEW, "pixel_clock is %i, link_clock is %i\n",dp->edid.pixel_clock, dp->edid.link_clock);

	dp_init_dim_regs(dp);

	intel_ddi_set_pipe_settings(dp);

	runio(dp);

	palette();

	pixels = dp->edid.ha * (dp->edid.va-4) * 4;
	printk(BIOS_SPEW, "ha=%d, va=%d\n",dp->edid.ha, dp->edid.va);

	test_gfx(dp);

	set_vbe_mode_info_valid(&dp->edid, graphics);
	i915_init_done = 1;
	return i915_init_done;

fail:
	printk(BIOS_SPEW, "Graphics could not be started;");
	if (0 && must_cycle_power){
		printk(BIOS_SPEW, "Turn off power and wait ...");
		gtt_write(PCH_PP_CONTROL,0xabcd0000);
		udelay(600000);
		gtt_write(PCH_PP_CONTROL,0xabcd000f);
	}
	printk(BIOS_SPEW, "Returning.\n");
	return 0;
}
