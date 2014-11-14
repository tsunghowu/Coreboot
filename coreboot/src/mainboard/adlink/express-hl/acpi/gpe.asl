/*
 * This file is part of the coreboot project.
 *
 * Copyright (c) 2011 Sven Schnelle <svens@stackframe.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */


Scope (\_GPE)
{  
	//
	// Add SCI handler as required
 	//
        Method (_L12, 0, NotSerialized)
        {
        //    Store (0x12, DBG0)
        }
        
        Method (_L18, 0, NotSerialized)
        {
        //    Store (0x18, DBG0)
        }

        Method (_L1B, 0, NotSerialized)
        {
        //    Store (0x1B, DBG0)
        }

        Method(_L06)
        {
         //    If(LAnd(\_SB.PCI0.GFX0.GSSE, LNot(GSMI))) // Graphics software SCI event?
         //       {
         //               \_SB.PCI0.GFX0.GSCI() // Handle the SWSCI
         //       }
         DBGP("_L06....\n")
        }
}
