/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
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

#include <arch/io.h>
#include <console/console.h>
#include <device/pci_ids.h>
#include <device/pci_def.h>
#include "pch.h"
#include "smbus.h"

#define SMB_IOREG_HST_STS        0x00   // Host Status Reg.
#define     HST_STS_HOST_BUSY        0x01    // R/WC
#define     HST_STS_INTR             0x02    // R/WC
#define     HST_STS_DEV_ERR          0x04    // R/WC
#define     HST_STS_BUS_ERR          0x08    // R/WC
#define     HST_STS_FAILED           0x10    // R/WC
#define     HST_STS_SMBALERT_STS     0x20    // R/WC
#define     HST_STS_INUSE_STS        0x40    // R/WC
#define     HST_STS_BDS              0x80    // R/WC
#define     HST_STS_ALL              0xff    // R/WC
#define     HST_STS_ERROR            0x1c    // R/WC
#define SMB_IOREG_HST_CNT        0x02   // Host Control Reg.
#define     HST_CNT_INTREN           0x01    // RW
#define     HST_CNT_KILL             0x02    // RW
#define     HST_CNT_SMB_CMD          0x1C    // RW
#define         SMB_CMD_BYTE             0x04
#define         SMB_CMD_BYTE_DATA        0x08
#define         SMB_CMD_WORD_DATA        0x0c
#define         SMB_CMD_BLOCK            0x14
#define     HST_CNT_LAST_BYTE        0x20    // RW
#define     HST_CNT_START            0x40    // RW
#define     HST_CNT_PEC_EN           0x80    // RW
#define SMB_IOREG_HST_CMD        0x03   // Host Command Reg.
#define SMB_IOREG_XMIT_SLVA      0x04   // Transmit Slave Address Reg.
#define     XMIT_SLVA_RW             0x01    // RW
#define SMB_IOREG_HST_D0         0x05   // Host Data 0
#define SMB_IOREG_HST_D1         0x06   // Host Data 1
#define SMB_IOREG_HOST_BLOCK_DB  0x07   // Host Block Data Byte Reg.
#define SMB_IOREG_PEC            0x08   // Packet Error Check Reg.
#define SMB_IOREG_RCV_SLVA       0x09   // Receive Slave Address Reg.
#define SMB_IOREG_SLV_DATA0      0x0a   // Receive Slave Data 0 Reg.
#define SMB_IOREG_SLV_DATA1      0x0b   // Receive Slave Data 1 Reg.
#define SMB_IOREG_AUX_STS        0x0c   // Auxiliary Status Reg.
#define     AUX_STS_CRC_ERR          0x01    // R/WC
#define     AUX_STS_STCO             0x02    // RO
#define SMB_IOREG_AUX_CTL        0x0d   // Auxiliary Control Reg.
#define     AUX_CTL_AAC              0x01    // R/W
#define     AUX_CTL_E32B             0x02    // R/W
#define SMB_IOREG_SMLINK_PIN_CTL 0x0e   // SMLink Pin Control Reg.
#define SMB_IOREG_SMBUS_PIN_CTL  0x0f   // SMBus Pin Control Reg.
#define SMB_IOREG_SLV_STS        0x10   // Slave Status Reg.
#define SMB_IOREG_SLV_CMD        0x11   // Slave Command Reg.
#define SMB_IOREG_NOTIFY_DADDR   0x14   // Notify Device Address Reg.
#define SMB_IOREG_NOTIFY_DLOW    0x16   // Notify Data Low Reg.
#define SMB_IOREG_NOTIFY_DHIGH   0x17   // Notify Data High Reg.

#define IO_DELAY_PORT           0xed    // Use for I/O delay
void enable_smbus(void)
{
#if 0      //has been initialized by FSP.
	device_t dev;

	/* Set the SMBus device statically. */
	dev = PCI_DEV(0x0, 0x1f, 0x3);

	/* Check to make sure we've got the right device. */
	if (pci_read_config16(dev, 0x0) != 0x8086) {
		die("SMBus controller not found!");
	}
	/* Set SMBus I/O base. */
	pci_write_config32(dev, SMB_BASE,
			   SMBUS_IO_BASE | PCI_BASE_ADDRESS_SPACE_IO);
    
    
	/* Set SMBus enable. */
	pci_write_config8(dev, HOSTC, HST_EN);

    
	/* Set SMBus I/O space enable. */
	pci_write_config16(dev, PCI_COMMAND, PCI_COMMAND_IO);
    
	/* Disable interrupt generation. */
	outb(0, SMBUS_IO_BASE + SMBHSTCTL);

	/* Clear any lingering errors, so transactions can run. */
	outb(inb(SMBUS_IO_BASE + SMBHSTSTAT), SMBUS_IO_BASE + SMBHSTSTAT);
#endif    
	print_debug("SMBus controller enabled.\n");
}

int smbus_read_byte(unsigned device, unsigned address)
{
	return do_smbus_read_byte(SMBUS_IO_BASE, device, address);
}

static void native_1_byte_block_write(unsigned char device, unsigned char address, unsigned char *pBuffer, unsigned char length) 
{
    #if 1
    outb(0xFF, SMBUS_IO_BASE);
    inb(SMBUS_IO_BASE + 2);

    outb(0x14, SMBUS_IO_BASE + 2);

    outb(device, SMBUS_IO_BASE + 4);
    outb(address, SMBUS_IO_BASE + 3);
    outb(length, SMBUS_IO_BASE + 5);
    outb(0, SMBUS_IO_BASE + 6);
    outb(pBuffer[0], SMBUS_IO_BASE + 7);
    outb(0, SMBUS_IO_BASE + 9);

    outb(0x54, SMBUS_IO_BASE + 2);

    outb(0x80, SMBUS_IO_BASE);
    outb(0x42, SMBUS_IO_BASE);
    outb(0xFF, SMBUS_IO_BASE);
    #endif
}
int smbus_block_write(unsigned char device, unsigned char address, unsigned char *pBuffer, unsigned char *length)
{
	int Status;
    native_1_byte_block_write(device, address, pBuffer, *length);
    Status = 0;
    *length = inb( SMBUS_IO_BASE + SMB_IOREG_HST_D0 );
	return Status;
}
