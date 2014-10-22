/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
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

#include <console/console.h>
#include <arch/io.h>
#include <device/device.h>
#include <device/pci.h>
#include <watchdog.h>

  //
  //  Disable PCH Watchdog timer at SB_RCBA+0x3410
  //
  //  Mmio32((MmPci32(0, 0, 0x1F, 0, 0xF0) & ~BIT0), 0x3410) |= 0x20;
  //

#define CMOS_RTC_SECONDS         0x00
#define CMOS_RTC_SECONDS_ALARM   0x01
#define CMOS_RTC_MINUTES         0x02
#define CMOS_RTC_MINUTES_ALARM   0x03
#define CMOS_RTC_HOURS           0x04
#define CMOS_RTC_HOURS_ALARM     0x05
#define CMOS_RTC_DAY_WEEK        0x06
#define CMOS_RTC_DAY_MONTH       0x07
#define CMOS_RTC_MONTH           0x08
#define CMOS_RTC_YEAR            0x09
#define CMOS_STATUS_A            0x0a
#define CMOS_STATUS_B            0x0b
#define CMOS_STATUS_C            0x0c
#define CMOS_STATUS_D            0x0d
#define CMOS_RESET_CODE          0x0f
#define CMOS_CENTURY             0x32
void watchdog_off(void)
{
	device_t dev;
	unsigned long value, base;

	/* Turn off the ICH7 watchdog. */
	dev = dev_find_slot(0, PCI_DEVFN(0x1f, 0));

	/* Enable I/O space. */
	value = pci_read_config16(dev, 0x04);
	value |= (1 << 10);
	pci_write_config16(dev, 0x04, value);

	/* Get TCO base. */
	base = (pci_read_config32(dev, 0x40) & 0x0fffe) + 0x60;

	/* Disable the watchdog timer. */
	value = inw(base + 0x08);
	value |= 1 << 11;
	outw(value, base + 0x08);

	/* Clear TCO timeout status. */
	outw(0x0008, base + 0x04);
	outw(0x0002, base + 0x06);
{
	
	outb(CMOS_CENTURY, 0x70);
	printk(BIOS_DEBUG, "CMOS_CENTURY: %d\n", inb(0x71));

	outb(CMOS_RTC_YEAR, 0x70);
	printk(BIOS_DEBUG, "YEAR: %d\n", inb(0x71));

	outb(CMOS_RTC_MONTH, 0x70);
	printk(BIOS_DEBUG, "MONTH: %d\n", inb(0x71));

	outb(CMOS_RTC_DAY_MONTH, 0x70);
	printk(BIOS_DEBUG, "CMOS_RTC_DAY_MONTH: %d\n", inb(0x71));

	outb(CMOS_CENTURY, 0x70);
	outb(0x20, 0x71);
	
outb(CMOS_RTC_YEAR, 0x70);
outb(2014-1980, 0x71);

outb(CMOS_RTC_MONTH, 0x70);
outb(10, 0x71);

outb(CMOS_RTC_DAY_MONTH, 0x70);
outb(16, 0x71);

}
	printk(BIOS_DEBUG, "PCH watchdog disabled\n");
}
