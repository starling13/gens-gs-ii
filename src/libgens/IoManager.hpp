/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoManager.hpp: I/O manager.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_IOMANAGER_HPP__
#define __LIBGENS_IOMANAGER_HPP__

#include <stdint.h>

#include "GensInput/GensKey_t.h"

namespace LibGens
{

class IoManager
{
	public:
		IoManager();
		~IoManager();

		/**
		 * Reset all controllers.
		 */
		void reset(void);

		/**
		 * @name Physical port numbers.
		 */
		enum PhysPort {
			PHYSPORT_1	= 0,	// Player 1
			PHYSPORT_2	= 1,	// Player 2
			PHYSPORT_EXT	= 2,	// Extension port

			PHYSPORT_MAX
		};

		/**
		 * MD I/O functions.
		 * NOTE: port must be either 0, 1, or 2.
		 * These correspond to physical ports.
		 */
		uint8_t readDataMD(int physPort) const;
		void writeDataMD(int physPort, uint8_t data);

		uint8_t readCtrlMD(int physPort) const;
		void writeCtrlMD(int physPort, uint8_t ctrl);

		uint8_t readSerCtrl(int physPort) const;
		void writeSerCtrl(int physPort, uint8_t serCtrl);

		uint8_t readSerTx(int physPort) const;
		void writeSerTx(int physPort, uint8_t serTx);

		uint8_t readSerRx(int physPort) const;

		/**
		 * I/O device update function.
		 */
		void update(void);

		/**
		 * Update the scanline counter for all controllers.
		 * This is used by the 6-button controller,
		 * which resets its internal counter after
		 * around 25 scanlines of no TH rising edges.
		 */
		void doScanline(void);
		static const int SCANLINE_COUNT_MAX_6BTN = 25;

		/**
		 * @name Virtual port numbers.
		 */
		enum VirtPort {
			VIRTPORT_1	= 0,	// Player 1
			VIRTPORT_2	= 1,	// Player 2
			VIRTPORT_EXT	= 2,	// Extension port

			// Team Player 1
			VIRTPORT_TP1A	= 3,	// TP-1A
			VIRTPORT_TP1B	= 4,	// TP-1B
			VIRTPORT_TP1C	= 5,	// TP-1C
			VIRTPORT_TP1D	= 6,	// TP-1D

			// Team Player 2
			VIRTPORT_TP2A	= 7,	// TP-1A
			VIRTPORT_TP2B	= 8,	// TP-1B
			VIRTPORT_TP2C	= 9,	// TP-1C
			VIRTPORT_TP2D	= 10,	// TP-1D

			// 4-Way Play
			VIRTPORT_4WPA	= 11,	// 4WP-A
			VIRTPORT_4WPB	= 12,	// 4WP-B
			VIRTPORT_4WPC	= 13,	// 4WP-C
			VIRTPORT_4WPD	= 14,	// 4WP-D

			// J-Cart (TODO)
			VIRTPORT_JCART1	= 15,
			VIRTPORT_JCART2	= 16,

			VIRTPORT_MAX
		};

		enum IoType {
			IOT_NONE	= 0,
			IOT_3BTN	= 1,
			IOT_6BTN	= 2,
			IOT_2BTN	= 3,
			IOT_MEGA_MOUSE	= 4,
			IOT_TEAMPLAYER	= 5,
			IOT_4WP_MASTER	= 6,
			IOT_4WP_SLAVE	= 7,

			IOT_MAX
		};

		// Controller configuration.

		// Set/get keymap.
		int setKeymap(int virtPort, const GensKey_t *keymap, int siz);
		int keymap(int virtPort, GensKey_t *keymap, int siz) const;

		/*
		IoType devType(IoPort port) const;
		void setDevType(IoPort port, IoType newDevType);
		
		int numButtons(IoPort port) const;
		int nextLogicalButton(IoPort port, int button) const;
		
		// Logical button names.
		// These are used for button name trnaslation in the UI.
		enum ButtonName_t
		{
			BTNNAME_UNKNOWN	= -1,
			
			// Standard controller buttons.
			BTNNAME_UP	= 0,
			BTNNAME_DOWN	= 1,
			BTNNAME_LEFT	= 2,
			BTNNAME_RIGHT	= 3,
			BTNNAME_B	= 4,
			BTNNAME_C	= 5,
			BTNNAME_A	= 6,
			BTNNAME_START	= 7,
			BTNNAME_Z	= 8,
			BTNNAME_Y	= 9,
			BTNNAME_X	= 10,
			BTNNAME_MODE	= 11,
			
			// SMS/GG buttons.
			BTNNAME_1	= 12,
			BTNNAME_2	= 13,
			
			// Sega Mega Mouse buttons.
			BTNNAME_MOUSE_LEFT	= 14,
			BTNNAME_MOUSE_RIGHT	= 15,
			BTNNAME_MOUSE_MIDDLE	= 16,
			BTNNAME_MOUSE_START	= 17,
			
			BTNNAME_MAX
		};
		
		// Get button names.
		static ButtonName_t ButtonName(int button);
		virtual ButtonName_t buttonName(int button) const;
		*/
		
		/** ZOMG savestate functions. **/
		// TODO: Move this struct to libzomg.
		/*
		struct Zomg_MD_IoSave_int_t
		{
			uint8_t data;
			uint8_t ctrl;
			uint8_t ser_tx;
			uint8_t ser_rx;
			uint8_t ser_ctrl;
		};
		void zomgSaveMD(Zomg_MD_IoSave_int_t *state) const;
		void zomgRestoreMD(const Zomg_MD_IoSave_int_t *state);
		*/
	
	private:
		/**
		 * Update an I/O device's state based on ctrl/data lines.
		 * @param physPort Physical port number.
		 */
		void updateDevice(int physPort);

		void updateDevice_3BTN(int virtPort);
		void updateDevice_6BTN(int virtPort, bool oldSelect);
		void updateDevice_2BTN(int virtPort);
		
		// I/O pin definitions.
		enum IoPinDefs {
			IOPIN_UP	= 0x01,	// D0
			IOPIN_DOWN	= 0x02,	// D1
			IOPIN_LEFT	= 0x04,	// D2
			IOPIN_RIGHT	= 0x08,	// D3
			IOPIN_TL	= 0x10,	// D4
			IOPIN_TR	= 0x20,	// D5
			IOPIN_TH	= 0x40	// D6
		};

		// Button bitfield values.
		enum ButtonBitfield {
			BTN_UP		= 0x01,
			BTN_DOWN	= 0x02,
			BTN_LEFT	= 0x04,
			BTN_RIGHT	= 0x08,
			BTN_B		= 0x10,
			BTN_C		= 0x20,
			BTN_A		= 0x40,
			BTN_START	= 0x80,
			BTN_Z		= 0x100,
			BTN_Y		= 0x200,
			BTN_X		= 0x400,
			BTN_MODE	= 0x800,

			// SMS/GG buttons.
			BTN_1		= 0x10,
			BTN_2		= 0x20,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTN_MOUSE_LEFT		= 0x01,
			BTN_MOUSE_RIGHT		= 0x02,
			BTN_MOUSE_MIDDLE	= 0x04,
			BTN_MOUSE_START		= 0x08	// Start
		};

		// Button index values.
		enum ButtonIndex {
			BTNI_UNKNOWN	= -1,

			// Standard controller buttons.
			BTNI_UP		= 0,
			BTNI_DOWN	= 1,
			BTNI_LEFT	= 2,
			BTNI_RIGHT	= 3,
			BTNI_B		= 4,
			BTNI_C		= 5,
			BTNI_A		= 6,
			BTNI_START	= 7,
			BTNI_Z		= 8,
			BTNI_Y		= 9,
			BTNI_X		= 10,
			BTNI_MODE	= 11,

			// SMS/GG buttons.
			BTNI_1		= 4,
			BTNI_2		= 5,

			// Sega Mega Mouse buttons.
			// NOTE: Mega Mouse buttons are active high,
			// and they use a different bitfield layout.
			BTNI_MOUSE_LEFT		= 0,
			BTNI_MOUSE_RIGHT	= 1,
			BTNI_MOUSE_MIDDLE	= 2,
			BTNI_MOUSE_START	= 3,	// Start

			BTNI_MAX	= 12
		};

		/** Serial I/O definitions and variables. **/

		/**
		 * @name Serial I/O control bitfield.
		 */
		enum SerCtrl {
			 SERCTRL_TFUL	= 0x01,		// TxdFull (1 == full)
			 SERCTRL_RRDY	= 0x02,		// RxdReady (1 == ready)
			 SERCTRL_RERR	= 0x04,		// RxdError (1 == error)
			 SERCTRL_RINT	= 0x08,		// Rxd Interrupt (1 == on)
			 SERCTRL_SOUT	= 0x10,		// TL mode. (1 == serial out; 0 == parallel)
			 SERCTRL_SIN	= 0x20,		// TR mode. (1 == serial in; 0 == parallel)
			 SERCTRL_BPS0	= 0x40,
			 SERCTRL_BPS1	= 0x80
		};

		/**
		 * @name Serial I/O baud rate values.
		 */
		enum SerBaud {
			SERBAUD_4800	= 0x00,
			SERBAUD_2400	= 0x01,
			SERBAUD_1200	= 0x02,
			SERBAUD_300	= 0x03
		};

		struct IoDevice
		{
			IoDevice()
				: type(IOT_3BTN)
				, counter(0)
				, scanlines(0)
				, ctrl(0)
				, mdData(0xFF)
				, deviceData(0xFF)
				, select(false)
				, buttons(~0)
				, serCtrl(0)
				, serLastTx(0xFF)
			{ }

			void reset(void) {
				counter = 0;
				scanlines = 0;
				ctrl = 0;
				mdData = 0xFF;
				deviceData = 0xFF;
				select = false;
				buttons = ~0;
				serCtrl = 0;
				serLastTx = 0xFF;
			}

			IoType type;		// Device type.
			int counter;		// Internal counter.
			int scanlines;		// Scanline counter.

			uint8_t ctrl;		// Tristate control.
			uint8_t mdData;		// Data written from the MD.
			uint8_t deviceData;	// Device data.
			bool select;		// Select line state.

			/**
			 * Controller bitfield.
			 * Format:
			 * - 2-button:          ??CBRLDU
			 * - 3-button:          SACBRLDU
			 * - 6-button: ????MXYZ SACBRLDU
			 * NOTE: ACTIVE LOW! (1 == released; 0 == pressed)
			 */
			unsigned int buttons;

			/**
			 * Determine the SELECT line state.
			 */
			inline void updateSelectLine(void) {
				// TODO: Apply the device data.
				select = (!(ctrl & IOPIN_TH) ||
					    (mdData & IOPIN_TH));
			}

			inline bool isSelect(void) const
				{ return select; }

			/**
			 * Read the last data value, with tristates applied.
			 * @return Data value with tristate settings applied.
			 */
			inline uint8_t readData(void) const {
				return applyTristate(deviceData);
			}

			/**
			 * Apply the Tristate settings to the data value.
			 * @param data Data value.
			 * @return Data value with tristate settings applied.
			 */
			inline uint8_t applyTristate(uint8_t data) const {
				data &= (~ctrl & 0x7F);		// Mask output bits.
				data |= (mdData & (ctrl | 0x80));	// Apply data buffer.
				return data;
			}

			// Serial I/O variables.
			// TODO: Serial data buffer.
			uint8_t serCtrl;	// Serial control.
			uint8_t serLastTx;	// Last transmitted data byte.

			// Button mapping.
			GensKey_t keyMap[BTNI_MAX];
		};

		IoDevice m_ioDevices[VIRTPORT_MAX];
};


/** Controller configuration. **/
/*
// Controller configuration. (STATIC functions)
inline IoBase::IoType IoBase::DevType(void)
	{ return IOT_NONE; }
inline int IoBase::NumButtons(void)
	{ return 0; }
inline int IoBase::NextLogicalButton(int button)
	{ ((void)button); return -1; }

// Controller configuration. (virtual functions)
inline IoBase::IoType IoBase::devType(void) const
	{ return DevType(); }
inline int IoBase::numButtons(void) const
	{ return NumButtons(); }
inline int IoBase::nextLogicalButton(int button) const
	{ return NextLogicalButton(button); }
*/

/**
 * ButtonName(): Get the name for a given button index. (STATIC function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
/*
inline IoBase::ButtonName_t IoBase::ButtonName(int button)
{
	((void)button);
	return BTNNAME_UNKNOWN;
}
*/

/**
 * buttonName(): Get the name for a given button index. (virtual function)
 * @param button Button index.
 * @return Button name, or BTNNAME_UNKNOWN if the button index is invalid.
 */
/*
inline IoBase::ButtonName_t IoBase::buttonName(int button) const
	{ return ButtonName(button); }
*/

}

#endif /* __LIBGENS_IOMANAGER_HPP__ */
