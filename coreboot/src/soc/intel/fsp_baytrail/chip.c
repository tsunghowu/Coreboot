/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <baytrail/pci_devs.h>
#include <baytrail/ramstage.h>
#include <drivers/intel/fsp/fsp_util.h>
#include "chip.h"

static void pci_domain_set_resources(device_t dev)
{
	assign_resources(dev->link_list);
}

static void finalize_dev (device_t dev)
{
	/*
	 * Notify FSP for PostPciEnumeration.
	 * Northbridge APIC init should be early and late enough...
	 */
	printk(BIOS_DEBUG, "FspNotify(EnumInitPhaseAfterPciEnumeration)\n");
	FspNotify(EnumInitPhaseAfterPciEnumeration);
}

static struct device_operations pci_domain_ops = {
	.read_resources   = pci_domain_read_resources,
	.set_resources    = pci_domain_set_resources,
	.enable_resources = NULL,
	.init             = NULL,
	.final            = &finalize_dev,
	.scan_bus         = pci_domain_scan_bus,
	.ops_pci_bus      = pci_bus_default_ops,
};

static void cpu_bus_noop(device_t dev) { }

static struct device_operations cpu_bus_ops = {
	.read_resources   = cpu_bus_noop,
	.set_resources    = cpu_bus_noop,
	.enable_resources = cpu_bus_noop,
	.init             = baytrail_init_cpus,
	.scan_bus         = NULL,
};

static void enable_dev(device_t dev)
{
	printk(BIOS_DEBUG, "enable_dev(%s, %d)\n",
	       dev_name(dev), dev->path.type);

	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	} else if (dev->path.type == DEVICE_PATH_PCI) {
		/* Handle south cluster enablement. */
		if (PCI_SLOT(dev->path.pci.devfn) > GFX_DEV &&
		    (dev->ops == NULL || dev->ops->enable == NULL)) {
			southcluster_enable_dev(dev);
		}
	}
}

static void finalize_chip(void *chip_info)
{
	/* Notify FSP for ReadyToBoot */
	printk(BIOS_DEBUG, "FspNotify(EnumInitPhaseReadyToBoot)\n");
	FspNotify(EnumInitPhaseReadyToBoot);

}

/* Called at BS_DEV_INIT_CHIPS time -- very early. Just after BS_PRE_DEVICE. */
static void soc_init(void *chip_info)
{
	baytrail_init_pre_device();
}

struct chip_operations soc_intel_fsp_baytrail_ops = {
	CHIP_NAME("Intel BayTrail SoC")
	.enable_dev = enable_dev,
	.init = soc_init,
	.final = &finalize_chip,
};

static void pci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

struct pci_operations soc_pci_ops = {
	.set_subsystem = &pci_set_subsystem,
};
