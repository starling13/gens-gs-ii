/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * RomCartridgeMD.hpp: MD ROM cartridge handler.                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#ifndef __LIBGENS_CPU_ROMCARTRIDGEMD_HPP__
#define __LIBGENS_CPU_ROMCARTRIDGEMD_HPP__

// C includes.
#include <stdint.h>

// SRam and EEPRom.
#include "Save/SRam.hpp"
#include "Save/EEPRom.hpp"

namespace LibGens
{

class RomCartridgeMD
{
	public:
		RomCartridgeMD();
		~RomCartridgeMD();

	private:
		// ROM access.
		template<uint8_t bank>
		inline uint8_t T_readByte_Rom(uint32_t address);
		template<uint8_t bank>
		inline uint16_t T_readWord_Rom(uint32_t address);

	public:
		// Cartridge access functions. ($000000-$9FFFFF)
		uint8_t readByte(uint32_t address);
		uint16_t readWord(uint32_t address);
		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);

		// /TIME register access functions. ($A130xx)
		// Only the low byte of the address is needed here.
		uint8_t readByte_TIME(uint8_t address);
		uint16_t readWord_TIME(uint8_t address);
		void writeByte_TIME(uint8_t address, uint8_t data);
		void writeWord_TIME(uint8_t address, uint16_t data);

	private:
		// ROM data. (Should be allocated in 512 KB blocks.)
		void *m_romData;
		uint32_t m_romData_size;

		// SRam and EEPRom.
		SRam m_SRam;
		EEPRom m_EEPRom;

		// Internal cartridge banking IDs.
		enum CartBank_t {
			// Unused bank. (Return 0xFF)
			// Also used for SRAM-only banks.
			BANK_UNUSED = 0,

			// ROM banks. (512 KB each)
			BANK_ROM_00, BANK_ROM_01, BANK_ROM_02, BANK_ROM_03,
			BANK_ROM_04, BANK_ROM_05, BANK_ROM_06, BANK_ROM_07,
			BANK_ROM_08, BANK_ROM_09, BANK_ROM_0A, BANK_ROM_0B,
			BANK_ROM_0C, BANK_ROM_0D, BANK_ROM_0E, BANK_ROM_0F,
			BANK_ROM_10, BANK_ROM_11, BANK_ROM_12, BANK_ROM_13,
			BANK_ROM_14, BANK_ROM_15, BANK_ROM_16, BANK_ROM_17,
			BANK_ROM_18, BANK_ROM_19, BANK_ROM_1A, BANK_ROM_1B,
			BANK_ROM_1C, BANK_ROM_1D, BANK_ROM_1E, BANK_ROM_1F,

			// MAPPER_MD_CONST_400000: Constant data area.
			BANK_CONST_400000,
		};

		// Physical memory map: 20 banks of 512 KB each.
		uint8_t m_cartBanks[20];

		/**
		 * Initialize the memory map for the loaded ROM.
		 * This identifies the mapper type to use.
		 */
		void initMemoryMap(void);

		// Mapper types.
		enum MapperType_t {
			/**
			 * Flat addressing.
			 * Phys: $000000-$9FFFFF
			 *
			 * Technically, the area above $3FFFFF is "reserved",
			 * but it is usable in MD-only mode.
			 *
			 * SRAM may be enabled if the game attempts to use it.
			 * EEPROM is enabled if the ROM is in the EEPROM database.
			 */
			MAPPER_MD_FLAT,

			/**
			 * Super Street Fighter II.
			 * Phys: $000000-$3FFFFF
			 * Supports up to 32 banks of 512 KB. (total is 16 MB)
			 * Physical bank 0 ($000000-$07FFFF) cannot be reampped.
			 */
			MAPPER_MD_SSF2,

			/**
			 * Realtec mapper.
			 */
			MAPPER_MD_REALTEC,

			/**
			 * Constant registers at $400000.
			 *
			 * This is used by some unlicensed games,
			 * including Ya Se Chuan Shuo and
			 * Super Bubble Bobble MD.
			 *
			 * The specific constants are set in the
			 * MD ROM fixups function.
			 */
			MAPPER_MD_CONST_400000,
		};

		// Mapper information.
		struct {
			MapperType_t type;

			union {
				// Super Street Fighter II mapper.
				struct {
					/**
					 * Map physical banks to virtual bank.
					 * Array index: Physical bank.
					 * Value: Virtual bank.
					 * If the virtual bank is >0x1F, use the default value.
					 */
					uint8_t banks[8];
				} ssf2;

				// Constant registers at $400000.
				struct {
					// Mask of readable bytes at $400000.
					// 1 == readable
					// 0 == not readable (use defaults)
					uint16_t byte_read_mask;

					// Constant registers.
					uint8_t reg[16];
				} const_400000;
			};
		} m_mapper;
};

}

#endif /* __LIBGENS_CPU_ROMCARTRIDGEMD_HPP__ */