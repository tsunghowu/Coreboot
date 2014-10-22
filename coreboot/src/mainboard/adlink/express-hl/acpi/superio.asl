/*
 * This file is part of the coreboot project.
 *
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

/* Values should match those defined in devicetree.cb */

#undef NCT5104D_SHOW_UARTA		 // pnp 4e.2: Disable UART A from NCT5104D
#undef NCT5104D_SHOW_UARTB		 // pnp 4e.2: Disable UART B from NCT5104D
#undef NCT5104D_SHOW_UARTC		 // pnp 4e.2: Disable UART C from NCT5104D
#undef NCT5104D_SHOW_UARTD		 // pnp 4e.2: Disable UART D from NCT5104D
#undef SUPERIO_PNP_BASE
#undef SUPERIO_DEV

#define NCT5104D_SHOW_UARTA
#define NCT5104D_SHOW_UARTB
#define SUPERIO_PNP_BASE 0x4e
#define SUPERIO_DEV SIO1
#include "superio/nuvoton/nct5104d/acpi/superio.asl"
