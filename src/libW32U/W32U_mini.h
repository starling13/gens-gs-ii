/***************************************************************************
 * libW32U: Win32 Unicode Translation Layer. (Mini Version)                *
 * W32U_mini.h: Main header. (include this!)                               *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __LIBW32U_W32U_MINI_H__
#define __LIBW32U_W32U_MINI_H__

#ifndef _WIN32
#error W32U_mini.h should only be included on Win32!
#endif

// C includes.
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the system is Unicode.
 * @return 1 if the system is Unicode; 0 if the system is ANSI.
 */
int W32U_IsUnicode(void);

/**
 * Convert a null-terminated multibyte string to UTF-16.
 * @param mbs Multibyte string. (null-terminated)
 * @param codepage mbs codepage.
 * @return UTF-16 string, or nullptr on error.
 */
wchar_t *W32U_mbs_to_UTF16(const char *mbs, unsigned int codepage);

/**
 * Convert a null-terminated UTF-16 string to multibyte.
 * @param wcs UTF-16 string. (null-terminated)
 * @param codepage mbs codepage.
 * @return Multibyte string, or nullptr on error.
 */
char *W32U_UTF16_to_mbs(const wchar_t *wcs, unsigned int codepage);

// W32U sub-headers.
#ifndef __IN_W32U__
#define __IN_W32U__

#include "W32U_libc.h"

#undef __IN_W32U__
#endif /* __IN_W32U__ */

#ifdef __cplusplus
}
#endif

#endif /* __LIBW32U_W32U_MINI_H__ */