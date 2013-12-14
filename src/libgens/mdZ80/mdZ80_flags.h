/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_flags.h: Z80 flag constants.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/


#ifndef __MDZ80_FLAGS_H__
#define __MDZ80_FLAGS_H__

// CPU flags. (F register)
#define MDZ80_FLAG_C	0x01
#define MDZ80_FLAG_N	0x02
#define MDZ80_FLAG_P	0x04
#define MDZ80_FLAG_3	0x08
#define MDZ80_FLAG_H	0x10
#define MDZ80_FLAG_5	0x20
#define MDZ80_FLAG_Z	0x40
#define MDZ80_FLAG_S	0x80

// Status flags.
#define MDZ80_STATUS_RUNNING	0x01
#define MDZ80_STATUS_HALTED	0x02
#define MDZ80_STATUS_FAULTED	0x10

#endif /* __MDZ80_FLAGS_H__ */
