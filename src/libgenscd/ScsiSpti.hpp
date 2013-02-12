/***************************************************************************
 * libgenscd: Gens/GS II CD-ROM Handler Library.                           *
 * ScsiSpti.hpp: SPTI (Windows NT) SCSI device handler class.              *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __LIBGENSCD_SCSISPTI_HPP__
#define __LIBGENSCD_SCSISPTI_HPP__

#ifndef _WIN32
#error SPTI is Win32 only.
#endif

// ScsiBase class.
#include "ScsiBase.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace LibGensCD
{

class ScsiSpti : public ScsiBase
{
	public:
		ScsiSpti(const std::string& filename);
		virtual ~ScsiSpti();

		bool isOpen(void) const override final;
		void close(void) override final;

		/**
		 * Check if a disc is present.
		 * @return True if a disc is present; false if not.
		 */
		bool isDiscPresent(void) override final;

		/**
		 * Check if the disc has changed since the last access.
		 * @return True if the disc has changed; false if not.
		 */
		bool hasDiscChanged(void) override final;

		/**
		 * Send a SCSI command descriptor block to the drive.
		 * @param cdb		[in] SCSI command descriptor block.
		 * @param cdb_len	[in] Length of cdb.
		 * @param out		[out] Output buffer, or nullptr if no data is requested.
		 * @param out_len	[out] Length of out.
		 * @param mode		[in] Data direction mode. (IN == receive from device; OUT == send to device)
		 * @return 0 on success, non-zero on error. (TODO: Return SCSI sense key?)
		 */
		int scsi_send_cdb(const void *cdb, uint8_t cdb_len,
				  void *out, size_t out_len,
				  scsi_data_mode mode = SCSI_DATA_IN) override final;

	private:
		// TODO: Move to private class?

		// Drive handle.
		HANDLE m_hDevice;
};

}

#endif /* __LIBGENSCD_SCSISPTI_HPP__ */
