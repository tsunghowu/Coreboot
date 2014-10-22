#include <types.h>
#include <string.h>
#include <device/device.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <console/console.h>
#include <delay.h>
#include <x86emu/x86emu.h>
#include <pc80/mc146818rtc.h>
#include <arch/acpi.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <boot/coreboot_tables.h>
#include "hda_verb.h"
#include "onboard.h"
#include "ec.h"
#include <southbridge/intel/bd82x6x/pch.h>
#include <smbios.h>
#include <device/pci.h>
#include <ec/google/chromeec/ec.h>
#include <cbfs_core.h>

#include <cpu/x86/tsc.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/mtrr.h>
#include <cpu/amd/mtrr.h>
#include <cpu/x86/msr.h>
#include "i915_reg.h"

enum {
	vmsg = 1, vio = 2, vspin = 4,
};

static int verbose = 0; //vmsg ;

static unsigned int *mmio;
static unsigned int graphics;
static unsigned short addrport;
static unsigned short dataport;
static unsigned int physbase;

#define READ32(addr) io_i915_READ32(addr)
#define WRITE32(val, addr) io_i915_WRITE32(val, addr)

static unsigned long io_i915_READ32(unsigned long addr)
{
       unsigned long val;
       outl(addr, addrport);
       val = inl(dataport);
       return val;
}

static void io_i915_WRITE32(unsigned long val, unsigned long addr)
{
       outl(addr, addrport);
       outl(val, dataport);
}


/*
2560
4 words per
4 *p
10240
4k bytes per page
4096/p
2.50
1700 lines
1700 * p
4250.00
PTEs
*/
static void
setgtt(int start, int end, unsigned long base, int inc)
{
        int i;

	for(i = start; i < end; i++){
                u32 word = base + i*inc;
                WRITE32(word|1,(i*4)|1);
        }
}

static char *regname(unsigned long addr)
{
	static char name[16];
	snprintf(name, sizeof (name), "0x%lx", addr);
	return name;
}

static unsigned long tickspermicrosecond = 1795;
static unsigned long long globalstart;

static unsigned long
microseconds(unsigned long long start, unsigned long long end)
{
	unsigned long ret;
	ret = ((end - start)/tickspermicrosecond);
	return ret;
}

static unsigned long globalmicroseconds(void)
{
	return microseconds(globalstart, rdtscll());
}

/* One-letter commands for code not mean to be ready for humans.
 * The code was generated by a set of programs/scripts.
 * M print out a kernel message
 * R read a register. We do these mainly to ensure that if hardware wanted
 * the register read, it was read; also, in debug, we can see what was expected
 * and what was found. This has proven *very* useful to get this debugged.
 * The udelay, if non-zero, will make sure there is a
 * udelay() call with the value.
 * The count is from the kernel and tells us how many times this read was done.
 * Also useful for debugging and the state
 * machine uses the info to drive a poll.
 * W Write a register
 * V set verbosity. It's a bit mask.
 *   0 -> nothing
 *   1 -> print kernel messages
 *   2 -> print IO ops
 *   4 -> print the number of times we spin on a register in a poll
 *   8 -> restore whatever the previous verbosity level was
 *   		(only one deep stack)
 *
 * Again, this is not really meant for human consumption. There is not a poll
 * operator as such because, sometimes, there is a read/write/read where the
 * second read is a poll, and this chipset is so touchy I'm reluctant to move
 * things around and/or delete too many reads.
 */
#define M 1
#define R 2
#define W 3
#define V 4
#define I 8

struct iodef {
	unsigned char op;
	unsigned int count;
	const char *msg;
	unsigned long addr;
	unsigned long data;
	unsigned long udelay;
} iodefs[] = {
#include "i915io.c"
};

static int i915_init_done = 0;

int vbe_mode_info_valid(void);
int vbe_mode_info_valid(void)
{
	return i915_init_done;
}

void fill_lb_framebuffer(struct lb_framebuffer *framebuffer);
void fill_lb_framebuffer(struct lb_framebuffer *framebuffer)
{
	printk(BIOS_SPEW, "fill_lb_framebuffer: graphics is %p\n", (void *)graphics);
	framebuffer->physical_address = graphics;
	framebuffer->x_resolution = 2560;
	framebuffer->y_resolution = 1700;
	framebuffer->bytes_per_line = 10240;
	framebuffer->bits_per_pixel = 32;
	framebuffer->red_mask_pos = 16;
	framebuffer->red_mask_size = 8;
	framebuffer->green_mask_pos = 8;
	framebuffer->green_mask_size = 8;
	framebuffer->blue_mask_pos = 0;
	framebuffer->blue_mask_size = 8;
	framebuffer->reserved_mask_pos = 0;
	framebuffer->reserved_mask_size = 0;

}

int i915lightup(unsigned int physbase, unsigned int iobase, unsigned int mmio,
	unsigned int gfx);

int i915lightup(unsigned int pphysbase,
	unsigned int piobase,
	unsigned int pmmio,
	unsigned int pgfx)
{
	int i, prev = 0;
	struct iodef *id, *lastidread = 0;
	unsigned long u, t;
	static unsigned long times[4096];
	mmio = (void *)pmmio;
	addrport = piobase;
	dataport = addrport + 4;
	physbase = pphysbase;
	graphics = pgfx;
	printk(BIOS_SPEW,
		"i915lightup: graphics %p mmio %p"
		"addrport %04x physbase %08x\n",
			(void *)graphics, mmio, addrport, physbase);
	globalstart = rdtscll();

	/* state machine! */
	for(i = 0, id = iodefs; i < ARRAY_SIZE(iodefs); i++, id++){
		switch(id->op){
		case M:
			if (verbose & vmsg) printk(BIOS_SPEW, "%ld: %s\n",
				globalmicroseconds(), id->msg);
			break;
		case R:
			u = READ32(id->addr);
			if (verbose & vio)printk(BIOS_SPEW, "%s: Got %08lx, expect %08lx\n",
				regname(id->addr), u, id->data);
			/* we're looking for something. */
			if (lastidread->addr == id->addr){
				/* they're going to be polling.
				 * just do it 1000 times
				 */
				for(t = 0; t < 1000 && id->data != u; t++){
					u = READ32(id->addr);
				}
				if (verbose & vspin) printk(BIOS_SPEW,
						"%s: # loops %ld got %08lx want %08lx\n",
						regname(id->addr),
						t, u, id->data);
			}
			lastidread = id;
			break;
		case W:
			if (verbose & vio)printk(BIOS_SPEW, "%s: outl %08lx\n", regname(id->addr),
									id->data);
			WRITE32(id->data, id->addr);
			if (id->addr == PCH_PP_CONTROL){
				switch(id->data & 0xf){
					case 8: break;
					case 7: break;
					default: udelay(100000);
				}
			}
			break;
		case V:
			if (id->count < 8){
				prev = verbose;
				verbose = id->count;
			} else {
				verbose = prev;
			}
			break;
		case I:
			break;
		default:
			printk(BIOS_SPEW, "BAD TABLE, opcode %d @ %d\n", id->op, i);
			return -1;
		}
		if (id->udelay)
			udelay(id->udelay);
		times[i] = globalmicroseconds();
	}
	/* optional, we don't even want to take timestamp overhead
	 * if we can avoid it. */
	if (0)
	for(i = 0, id = iodefs; i < ARRAY_SIZE(iodefs); i++, id++){
		switch(id->op){
		case R:
			printk(BIOS_SPEW, "%ld: R %08lx\n", times[i], id->addr);
			break;
		case W:
			printk(BIOS_SPEW, "%ld: W %08lx %08lx\n", times[i],
				id->addr, id->data);
			break;
		}
	}

	setgtt(0, 4520, physbase, 4096);
	printk(BIOS_SPEW, "memset %p to 0 for %d bytes\n",
				(void *)graphics, 4520*4096);
	memset((void *)graphics, 0, 4520*4096);
	printk(BIOS_SPEW, "%ld microseconds\n", globalmicroseconds());
	i915_init_done = 1;
	return i915_init_done;
}
