/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * EmuManager.cpp: Emulation manager.                                         *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2011 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include <config.h>

#include "EmuManager.hpp"
#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/Util/Timing.hpp"
#include "libgens/MD/EmuMD.hpp"

// LibGens Sound Manager.
// Needed for LibGens::SoundMgr::MAX_SAMPLING_RATE.
#include "libgens/sound/SoundMgr.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

// LibGens video includes.
#include "libgens/MD/VdpPalette.hpp"
#include "libgens/MD/VdpRend.hpp"
#include "libgens/MD/VdpIo.hpp"
#include "libgens/MD/TAB336.h"

// M68K_Mem.hpp has SysRegion.
#include "libgens/cpu/M68K_Mem.hpp"

// Multi-File Archive Selection Dialog.
#include "ZipSelectDialog.hpp"

// libzomg. Needed for savestate preview images.
#include "libzomg/Zomg.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// Qt key handler.
#include "Input/KeyHandlerQt.hpp"


namespace GensQt4
{

EmuManager::EmuManager()
{
	// Initialize timing information.
	m_lastTime = 0.0;
	m_lastTime_fps = 0.0;
	m_frames = 0;
	
	// No ROM is loaded at startup.
	m_rom = NULL;
	m_paused.data = 0;
	
	// Save slot.
	m_saveSlot = gqt4_config->saveSlot();
	
	// TODO: Load initial VdpPalette settings.
	
	// Create the Audio Backend.
	// TODO: Allow selection of all available audio backend classes.
	// NOTE: Audio backends are NOT QWidgets!
	m_audio = new GensPortAudio();
	
	// Connect the configuration slots.
	connect(gqt4_config, SIGNAL(saveSlot_changed(int)),
		this, SLOT(saveSlot_changed_slot(int)));
	connect(gqt4_config, SIGNAL(autoFixChecksum_changed(bool)),
		this, SLOT(autoFixChecksum_changed_slot(bool)));
	
	// Graphics settings.
	connect(gqt4_config, SIGNAL(contrast_changed(int)),
		this, SLOT(contrast_changed_slot(int)));
	connect(gqt4_config, SIGNAL(brightness_changed(int)),
		this, SLOT(brightness_changed_slot(int)));
	connect(gqt4_config, SIGNAL(grayscale_changed(bool)),
		this, SLOT(grayscale_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(inverted_changed(bool)),
		this, SLOT(inverted_changed_slot(bool)));
	connect(gqt4_config, SIGNAL(colorScaleMethod_changed(int)),
		this, SLOT(colorScaleMethod_changed_slot(int)));
	connect(gqt4_config, SIGNAL(interlacedMode_changed(GensConfig::InterlacedMode_t)),
		this, SLOT(interlacedMode_changed_slot(GensConfig::InterlacedMode_t)));
	
	// Region code settings.
	connect(gqt4_config, SIGNAL(regionCode_changed(GensConfig::ConfRegionCode_t)),
		this, SLOT(regionCode_changed_slot(GensConfig::ConfRegionCode_t)));
	connect(gqt4_config, SIGNAL(regionCodeOrder_changed(uint16_t)),
		this, SLOT(regionCodeOrder_changed_slot(uint16_t)));
}

EmuManager::~EmuManager()
{
	// Delete the audio backend.
	m_audio->close();
	delete m_audio;
	m_audio = NULL;
	
	// Delete the ROM.
	// TODO
	
	// TODO: Do we really need to clear this?
	m_paused.data = 0;
}


/**
 * openRom(): Open a ROM file.
 * Prompts the user to select a ROM file, then opens it.
 * @param parent Parent window for the modal dialog box.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(QWidget *parent)
{
	// TODO: Proper compressed file support.
	#define ZLIB_EXT " *.zip *.zsg *.gz"
	#define LZMA_EXT " *.7z"
	#define RAR_EXT " *.rar"
	
	// TODO: Set the default filename.
	QString filename = QFileDialog::getOpenFileName(parent,
			tr("Open ROM"),		// Dialog title
			QString(),		// Default filename.
			tr("Sega Genesis ROM images") +
			QLatin1String(
				" (*.bin *.gen *.md *.smd"
#ifdef HAVE_ZLIB
				ZLIB_EXT
#endif /* HAVE_ZLIB */
#ifdef HAVE_LZMA
				LZMA_EXT
#endif /* HAVE_LZMA */
				RAR_EXT
				");;"
				) +
#if 0
			tr("Sega Genesis / 32X ROMs; Sega CD disc images") +
			"(*.bin *.smd *.gen *.32x *.cue *.iso *.raw" ZLIB_EXT LZMA_EXT RAR_EXT ");;" +
#endif
			tr("All Files") + QLatin1String(" (*.*)"));
	
	if (filename.isEmpty())
		return -1;
	
	// Convert the filename to native separators.
	filename = QDir::toNativeSeparators(filename);
	
	// Open the ROM file.
	return openRom(filename);
}


/**
 * openRom(): Open a ROM file.
 * @param filename ROM filename. (Must have native separators!)
 * TODO: Add a z_entry parameter?
 * @return 0 on success; non-zero on error.
 */
int EmuManager::openRom(const QString& filename)
{
	// Open the file using the LibGens::Rom class.
	// TODO: This won't work for KIO...
	LibGens::Rom *rom = new LibGens::Rom(filename.toUtf8().constData());
	if (!rom->isOpen())
	{
		// Couldn't open the ROM file.
		fprintf(stderr, "Error opening ROM file. (TODO: Get error information.)\n");
		delete rom;
		return -2;
	}
	
	// Check if this is a multi-file ROM archive.
	if (rom->isMultiFile())
	{
		// Multi-file ROM archive.
		// Prompt the user to select a file.
		ZipSelectDialog *zipsel = new ZipSelectDialog();
		zipsel->setFileList(rom->get_z_entry_list());
		int ret = zipsel->exec();
		if (ret != QDialog::Accepted || zipsel->selectedFile() == NULL)
		{
			// Dialog was rejected.
			delete rom;
			delete zipsel;
			return -3;
		}
		
		// Get the selected file.
		rom->select_z_entry(zipsel->selectedFile());
		delete zipsel;
	}
	
	// Load the selected ROM file.
	return loadRom(rom);
}


/**
 * loadRom(): Load a ROM file.
 * Loads a ROM file previously opened via LibGens::Rom().
 * @param rom [in] Previously opened ROM object.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::loadRom(LibGens::Rom *rom)
{
	if (gqt4_emuThread || m_rom)
	{
		// Close the ROM first.
		// Don't emit a stateChanged() signal, since
		// we're opening a ROM immediately afterwards.
		closeRom(false);
		
		// HACK: Set a QTimer for 100 ms to actually load the ROM to make sure
		// the emulation thread is shut down properly.
		// If we don't do that, then loading savestates causes
		// video corruption due to a timing mismatch.
		// TODO: Figure out how to fix this.
		// TODO: Proper return code.
		m_loadRom_int_tmr_rom = rom;
		QTimer::singleShot(100, this, SLOT(sl_loadRom_int()));
		return 0;
	}
	
	// ROM isn't running. Open the ROM directly.
	return loadRom_int(rom);
}


/**
 * loadRom_int(): Load a ROM file. (Internal function.)
 * Loads a ROM file previously opened via LibGens::Rom().
 * @param rom [in] Previously opened ROM object.
 * @return 0 on success; non-zero on error.
 */
int EmuManager::loadRom_int(LibGens::Rom *rom)
{
	if (m_rom)
		return -1;
	
	// Check if this is a multi-file ROM archive.
	if (rom->isMultiFile() && !rom->isRomSelected())
	{
		// Multi-file ROM archive, but a ROM hasn't been selected.
		return -2;
	}
	
	// Make sure the ROM is supported.
	const QChar chrBullet(0x2022);  // U+2022: BULLET
	const QChar chrNewline(L'\n');
	const QChar chrSpace(L' ');
	
	// Check the system ID.
	if (rom->sysId() != LibGens::Rom::MDP_SYSTEM_MD)
	{
		// Only MD ROM images are supported.
		const LibGens::Rom::MDP_SYSTEM_ID errSysId = rom->sysId();
		delete rom;
		
		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what system the ROM is for...)
		QMessageBox::critical(NULL,
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error title)
				tr("Unsupported System"),
				//: A ROM image was selected for a system that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is designed for a system that"
				   " is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what system the ROM image is for.
				tr("Selected ROM's system: %1").arg(SysName_l(errSysId)) +
				chrNewline + chrNewline +
				//: List of systems that Gens/GS II currently supports.
				tr("Supported systems:") + chrNewline +
				chrBullet + chrSpace + SysName_l(LibGens::Rom::MDP_SYSTEM_MD)
				);
		
		return 3;
	}
	
	// Check the ROM format.
	if (rom->romFormat() != LibGens::Rom::RFMT_BINARY)
	{
		// Only binary ROM images are supported.r
		LibGens::Rom::RomFormat errRomFormat = rom->romFormat();
		delete rom;
		
		// TODO: Split this into another function?
		QString romFormat;
		switch (errRomFormat)
		{
			case LibGens::Rom::RFMT_BINARY:
				romFormat = tr("Binary", "rom-format");
				break;
			case LibGens::Rom::RFMT_SMD:
				romFormat = tr("Super Magic Drive", "rom-format");
				break;
			case LibGens::Rom::RFMT_SMD_SPLIT:
				romFormat = tr("Super Magic Drive (split)", "rom-format");
				break;
			case LibGens::Rom::RFMT_MGD:
				romFormat = tr("Multi Game Doctor", "rom-format");
				break;
			case LibGens::Rom::RFMT_CD_CUE:
				romFormat = tr("CD-ROM cue sheet", "rom-format");
				break;
			case LibGens::Rom::RFMT_CD_ISO_2048:
			case LibGens::Rom::RFMT_CD_ISO_2352:
				romFormat = tr("ISO-9660 CD-ROM image (%1-byte sectors)", "rom-format")
						.arg(errRomFormat == LibGens::Rom::RFMT_CD_ISO_2048 ? 2048 : 2352);
				break;
			case LibGens::Rom::RFMT_CD_BIN_2048:
			case LibGens::Rom::RFMT_CD_BIN_2352:
				romFormat = tr("Raw CD-ROM image (%1-byte sectors)", "rom-format")
						.arg(errRomFormat == LibGens::Rom::RFMT_CD_BIN_2048 ? 2048 : 2352);
				break;
			default:
				romFormat = tr("Unknown", "rom-format");
				break;
		}
		
		// TODO: Specify GensWindow as parent window.
		// TODO: Move this out of EmuManager and simply use return codes?
		// (how would we indicate what format the ROM was in...)
		QMessageBox::critical(NULL,
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error title)
				tr("Unsupported ROM Format"),
				//: A ROM image was selected in a format that Gens/GS II does not currently support. (error description)
				tr("The selected ROM image is in a format that is not currently supported by Gens/GS II.") +
				chrNewline + chrNewline +
				//: Indicate what format the ROM image is in.
				tr("Selected ROM image format: %1").arg(romFormat) +
				chrNewline + chrNewline +
				//: List of ROM formats that Gens/GS II currently supports.
				tr("Supported ROM formats:") + chrNewline +
				chrBullet + chrSpace + tr("Binary", "rom-format")
				);
		
		return 4;
	}
	
	// Determine the system region code.
	LibGens::SysVersion::RegionCode_t lg_region = GetLgRegionCode(
				gqt4_config->regionCode(), rom->regionCode(),
				gqt4_config->regionCodeOrder());
	
	if (gqt4_config->regionCode() == GensConfig::CONFREGION_AUTODETECT)
	{
		// Print the auto-detected region.
		const QString detect_str = LgRegionCodeStr(lg_region);
		if (!detect_str.isEmpty())
		{
			const QString auto_str = tr("ROM region detected as %1.");
			emit osdPrintMsg(1500, auto_str.arg(detect_str));
		}
	}
	
	// Create a new MD emulation context.
	delete gqt4_emuContext;
	gqt4_emuContext = new LibGens::EmuMD(rom, lg_region);
	rom->close();	// TODO: Let EmuMD handle this...
	
	if (!gqt4_emuContext->isRomOpened())
	{
		// Error loading the ROM image in EmuMD.
		// TODO: EmuMD error code constants.
		// TODO: Show an error message.
		fprintf(stderr, "Error: Initialization of gqt4_emuContext failed. (TODO: Error code.)\n");
		delete gqt4_emuContext;
		gqt4_emuContext = NULL;
		delete rom;
		return 5;
	}
	
	// Save the Rom class pointer as m_rom.
	m_rom = rom;
	
	// m_rom isn't deleted, since keeping it around
	// indicates that a game is running.
	// TODO: Use gqt4_emuContext instead?
	
	// Open audio.
	m_audio->open();
	
	// Initialize timing information.
	m_lastTime = LibGens::Timing::GetTimeD();
	m_lastTime_fps = m_lastTime;
	m_frames = 0;
	
	// Start the emulation thread.
	m_paused.data = 0;
	gqt4_emuThread = new EmuThread();
	QObject::connect(gqt4_emuThread, SIGNAL(frameDone(bool)),
			 this, SLOT(emuFrameDone(bool)));
	gqt4_emuThread->start();
	
	// Update the Gens title.
	emit stateChanged();
	return 0;
}


/**
 * GetLgRegionCode(): Determine the LibGens region code to use.
 * @param confRegionCode Current GensConfig region code.
 * @param mdHexRegionCode ROM region code, in MD hex format.
 * @param regionCodeOrder Region code order for auto-detection. (MSN == highest priority)
 * @return LibGens region code to use.
 */
LibGens::SysVersion::RegionCode_t EmuManager::GetLgRegionCode(
		GensConfig::ConfRegionCode_t confRegionCode,
		int mdHexRegionCode, uint16_t regionCodeOrder)
{
	// TODO: Consolidate LibGens::SysVersion::RegionCode_t and GensConfig::ConfRegion_t.
	LibGens::SysVersion::RegionCode_t lg_region;
	
	switch (confRegionCode)
	{
		case GensConfig::CONFREGION_JP_NTSC:
			lg_region = LibGens::SysVersion::REGION_JP_NTSC;
			break;
		case GensConfig::CONFREGION_ASIA_PAL:
			lg_region = LibGens::SysVersion::REGION_ASIA_PAL;
			break;
		case GensConfig::CONFREGION_US_NTSC:
			lg_region = LibGens::SysVersion::REGION_US_NTSC;
			break;
		case GensConfig::CONFREGION_EU_PAL:
			lg_region = LibGens::SysVersion::REGION_EU_PAL;
			break;
		case GensConfig::CONFREGION_AUTODETECT:
		default:
		{
			// Use the detected region from the ROM image.
			int regionMatch = 0;
			int orderTmp = regionCodeOrder;
			for (int i = 0; i < 4; i++, orderTmp <<= 4)
			{
				int orderN = ((orderTmp >> 12) & 0xF);
				if (mdHexRegionCode & orderN)
				{
					// Found a match.
					regionMatch = orderN;
					break;
				}
			}
			
			if (regionMatch == 0)
			{
				// No region matched.
				// Use the highest-priority region.
				regionMatch = ((regionCodeOrder >> 12) & 0xF);
			}
			
			switch (regionMatch & 0xF)
			{
				default:
				case 0x4:	lg_region = LibGens::SysVersion::REGION_US_NTSC; break;
				case 0x8:	lg_region = LibGens::SysVersion::REGION_EU_PAL; break;
				case 0x1:	lg_region = LibGens::SysVersion::REGION_JP_NTSC; break;
				case 0x2:	lg_region = LibGens::SysVersion::REGION_ASIA_PAL; break;
			}
		}
	}
	
	// Return the region code.
	return lg_region;
}


/**
 * closeRom(): Close the open ROM file and stop emulation.
 * @param emitStateChanged If true, emits the stateChanged() signal after the ROM is closed.
 */
int EmuManager::closeRom(bool emitStateChanged)
{
	if (!isRomOpen())
		return 0;
	
	if (gqt4_emuThread)
	{
		// Disconnect the emuThread's signals.
		gqt4_emuThread->disconnect();
		
		// Stop and delete the emulation thread.
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	if (gqt4_emuContext)
	{
		// Make sure SRam/EEPRom data is saved.
		// (SaveData() will call the LibGens OSD handler if necessary.)
		gqt4_emuContext->saveData();
		
		// Delete the emulation context.
		delete gqt4_emuContext;
		gqt4_emuContext = NULL;
		
		// Delete the Rom instance.
		// TODO: Handle this in gqt4_emuContext.
		delete m_rom;
		m_rom = NULL;
	}
	
	// Close audio.
	m_audio->close();
	m_paused.data = 0;
	
	// Only clear the screen if we're emitting stateChanged().
	// If we're not emitting stateChanged(), this usually means
	// we're loading a new ROM immediately afterwards, so
	// clearing the screen is a waste of time.
	if (emitStateChanged)
	{
		if (gqt4_config->introStyle() == 0)
		{
			// Intro Effect is disabled.
			// Clear the screen.
			LibGens::VdpIo::Reset();
			emit updateVideo();
		}
		
		// Update the Gens title.
		emit stateChanged();
	}
	
	return 0;
}


/**
 * romName(): Get the ROM name.
 * @return ROM name, or empty string if no ROM is loaded.
 */
QString EmuManager::romName(void)
{
	if (!m_rom)
		return QString();
	
	// TODO: This is MD/MCD/32X only!
	
	// Check the active system region.
	const char *s_romName;
	if (LibGens::M68K_Mem::ms_SysVersion.isEast())
	{
		// East (JP). Return the domestic ROM name.
		s_romName = m_rom->romNameJP();
		if (!s_romName || s_romName[0] == 0x00)
		{
			// Domestic ROM name is empty.
			// Return the overseas ROM name.
			s_romName = m_rom->romNameUS();
		}
	}
	else
	{
		// West (US/EU). Return the overseas ROM name.
		s_romName = m_rom->romNameUS();
		if (!s_romName || s_romName[0] == 0x00)
		{
			// Overseas ROM name is empty.
			// Return the domestic ROM name.
			s_romName = m_rom->romNameJP();
		}
	}
	
	// Return the ROM name.
	if (!s_romName)
		return QString();
	return QString::fromUtf8(s_romName);
}


/**
 * sysName(): Get the system name for the active ROM, based on ROM region.
 * @return System name, or empty string if unknown or no ROM is loaded.
 */
QString EmuManager::sysName(void)
{
	if (!m_rom)
		return QString();
	
	return SysName(m_rom->sysId(), LibGens::M68K_Mem::ms_SysVersion.region());
}


/**
 * SysName(): Get the system name for the specified system ID and region.
 * @param sysID System ID.
 * @param region Region.
 * @return System name, or empty string if unknown.
 */
QString EmuManager::SysName(LibGens::Rom::MDP_SYSTEM_ID sysId, LibGens::SysVersion::RegionCode_t region)
{
	switch (sysId)
	{
		case LibGens::Rom::MDP_SYSTEM_MD:
			// Genesis / Mega Drive.
			if (region == LibGens::SysVersion::REGION_US_NTSC)
			{
				//: MD ROM region is US/NTSC. System name should be the equivalent of "Genesis".
				return tr("Genesis", "rom-region");
			}
			else
			{
				//: MD ROM region is not US/NTSC. System name should be the equivalent of "Mega Drive".
				return tr("Mega Drive", "rom-region");
			}
		
		case LibGens::Rom::MDP_SYSTEM_MCD:
			if (region == LibGens::SysVersion::REGION_US_NTSC)
			{
				//: MCD disc region is US/NTSC. System name should be the equivalent of "Sega CD".
				return tr("Sega CD", "rom-region");
			}
			else
			{
				//: MCD disc region is not US/NTSC. System name should be the equivalent of "Mega CD".
				return tr("Mega CD", "rom-region");
			}
		
		case LibGens::Rom::MDP_SYSTEM_32X:
			switch (region)
			{
				default:
				case LibGens::SysVersion::REGION_US_NTSC:
					//: 32X ROM is US/NTSC. System name should be the equivalent of "Sega 32X".
					return tr("Sega 32X", "rom-region");
				case LibGens::SysVersion::REGION_EU_PAL:
					//: 32X ROM is EU/PAL. System name should be the equivalent of "Mega Drive 32X".
					return tr("Mega Drive 32X", "rom-region");
				case LibGens::SysVersion::REGION_JP_NTSC:
				case LibGens::SysVersion::REGION_ASIA_PAL:
					//: 32X ROM is JP or ASIA. System name should be the equivalent of "Super 32X".
					return tr("Super 32X", "rom-region");
			}
		
		case LibGens::Rom::MDP_SYSTEM_MCD32X:
			if (region == LibGens::SysVersion::REGION_US_NTSC)
			{
				//: Sega CD 32X disc region is US/NTSC. System name should be the equivalent of "Sega CD 32X".
				return tr("Sega CD 32X", "rom-region");
			}
			else
			{
				//: Sega CD 32X disc region is US/NTSC. System name should be the equivalent of "Mega CD 32X".
				return tr("Mega CD 32X", "rom-region");
			}
		
		case LibGens::Rom::MDP_SYSTEM_SMS:
			//: Master System. (No localized names yet...)
			return tr("Master System", "rom-region");
		
		case LibGens::Rom::MDP_SYSTEM_GG:
			//: Game Gear. (No localized names yet...)
			return tr("Game Gear", "rom-region");
		
		case LibGens::Rom::MDP_SYSTEM_SG1000:
			//: SG-1000. (No localized names yet...)
			return tr("SG-1000", "rom-region");
		
		case LibGens::Rom::MDP_SYSTEM_PICO:
			//: Pico. (No localized names yet...)
			return tr("Pico", "rom-region");
		
		default:
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * SysName_l(): Get the localized system name for the specified system ID.
 * @param sysID System ID.
 * @return Localized system name, or empty string if unknown.
 */
QString EmuManager::SysName_l(LibGens::Rom::MDP_SYSTEM_ID sysId)
{
	switch (sysId)
	{
		case LibGens::Rom::MDP_SYSTEM_MD:
			//: Localized name of Sega Genesis.
			return tr("Genesis", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_MCD:
			//: Localized name of Sega CD.
			return tr("Sega CD", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_32X:
			//: Localized name of Sega 32X.
			return tr("Sega 32X", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_MCD32X:
			//: Localized name of Sega CD 32X.
			return tr("Sega CD 32X", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_SMS:
			//: Localized name of Master System.
			return tr("Master System", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_GG:
			//: Localized name of Game Gear.
			return tr("Game Gear", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_SG1000:
			//: Localized name of SG-1000.
			return tr("SG-1000", "local-region");
		
		case LibGens::Rom::MDP_SYSTEM_PICO:
			//: Localized name of Pico.
			return tr("Pico", "local-region");
		
		default:
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * LgRegionCodeStr(): Get a string identifying a given LibGens region code.
 * @param region Region code. (1, 2, 4, 8)
 * @return Region code string, or NULL on error.
 */
QString EmuManager::LgRegionCodeStr(LibGens::SysVersion::RegionCode_t region)
{
	switch (region)
	{
		case LibGens::SysVersion::REGION_JP_NTSC:	return tr("Japan (NTSC)");
		case LibGens::SysVersion::REGION_ASIA_PAL:	return tr("Asia (PAL)");
		case LibGens::SysVersion::REGION_US_NTSC:	return tr("USA (NTSC)");
		case LibGens::SysVersion::REGION_EU_PAL:	return tr("Europe (PAL)");
		default:	return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * LgRegionCodeStr(): Get a string identifying a given GensConfig region code.
 * TODO: Combine ConfRegionCode_t with RegionCode_t.
 * @param region Region code.
 * @return Region code string, or empty string on error.
 */
QString EmuManager::GcRegionCodeStr(GensConfig::ConfRegionCode_t region)
{
	switch (region)
	{
		case GensConfig::CONFREGION_AUTODETECT:	return tr("Auto-Detect");
		case GensConfig::CONFREGION_JP_NTSC:	return tr("Japan (NTSC)");
		case GensConfig::CONFREGION_ASIA_PAL:	return tr("Asia (PAL)");
		case GensConfig::CONFREGION_US_NTSC:	return tr("USA (NTSC)");
		case GensConfig::CONFREGION_EU_PAL:	return tr("Europe (PAL)");
		default:	return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * LgRegionCodeStrMD(): Get a string identifying a given region code. (MD hex code)
 * @param region Region. (1, 2, 4, 8)
 * @return Region code string, or NULL on error.
 */
QString EmuManager::LgRegionCodeStrMD(int region)
{
	switch (region & 0xF)
	{
		case 0x1:	return tr("Japan (NTSC)");
		case 0x2:	return tr("Asia (PAL)");
		case 0x4:	return tr("USA (NTSC)");
		case 0x8:	return tr("Europe (PAL)");
		default:	return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * getSaveStateFilename(): Get the savestate filename.
 * TODO: Move savestate code to another file?
 * @return Savestate filename, or empty string if no ROM is loaded.
 */
QString EmuManager::getSaveStateFilename(void)
{
	if (!m_rom)
		return QString();
	
	const QString filename =
		gqt4_config->userPath(GensConfig::GCPATH_SAVESTATES) +
		QString::fromUtf8(m_rom->filenameBaseNoExt()) +
		QChar(L'.') + QString::number(m_saveSlot) +
		QLatin1String(".zomg");
	return filename;
}


/**
 * emuFrameDone(): Emulation thread is finished rendering a frame.
 * @param wasFastFrame The frame was rendered "fast", i.e. no VDP updates.
 */
void EmuManager::emuFrameDone(bool wasFastFrame)
{
	// Make sure the emulation thread is still running.
	if (!gqt4_emuThread || gqt4_emuThread->isStopRequested())
		return;
	
	if (!wasFastFrame)
		m_frames++;
	
	double timeDiff;
	double thisTime = LibGens::Timing::GetTimeD();
	
	if (m_lastTime < 0.001)
	{
		m_lastTime = thisTime;
		m_lastTime_fps = m_lastTime;
		timeDiff = 0.0;
	}
	else
	{
		// Get the time difference.
		timeDiff = (thisTime - m_lastTime);
		
		// Check the FPS counter.
		double timeDiff_fps = (thisTime - m_lastTime_fps);
		if (timeDiff_fps >= 0.250)
		{
			// Push the current fps.
			// (Updated four times per second.)
			double fps = ((double)m_frames / timeDiff_fps);
			emit updateFps(fps);
			
			// Reset the timer and frame counter.
			m_lastTime_fps = thisTime;
			m_frames = 0;
		}
	}
	
	// Check for SRam/EEPRom autosave.
	// TODO: Frames elapsed.
	gqt4_emuContext->autoSaveData(1);
	
	// Check for requests in the emulation queue.
	if (!m_qEmuRequest.isEmpty())
		processQEmuRequest();
	
	// Update the GensQGLWidget.
	if (!wasFastFrame)
		emit updateVideo();
	
	// If emulation is paused, don't resume the emulation thread.
	if (m_paused.data)
		return;
	
	/** Auto Frame Skip **/
	// TODO: Figure out how to properly implement the old Gens method of synchronizing to audio.
#if 0
	// Check if we should do a fast frame.
	// TODO: Audio stutters a bit if the video drops below 60.0 fps.
	m_audio->wpSegWait();	// Wait for the buffer to empty.
	m_audio->write();	// Write audio.
	bool doFastFrame = !m_audio->isBufferEmpty();
#else
	// TODO: Remove the ring buffer and just use the classic SDL-esque method.
	m_audio->write();	// Write audio.
#endif
	
	// Check if we're higher or lower than the required framerate.
	bool doFastFrame = false;
	const double frameRate = (1.0 / (LibGens::M68K_Mem::ms_SysVersion.isPal() ? 50.0 : 60.0));
	const double threshold = 0.001;
	if (timeDiff > (frameRate + threshold))
	{
		// Lower than the required framerate.
		// Do a fast frame.
		//printf("doing fast frame; ");
		doFastFrame = true;
	}
	else if (timeDiff < (frameRate - threshold))
	{
		// Higher than the required framerate.
		// Wait for the required amount of time.
		do
		{
			thisTime = LibGens::Timing::GetTimeD();
			timeDiff = (thisTime - m_lastTime);
		} while (timeDiff < frameRate);
		
		// TODO: This causes some issues...
		if (timeDiff > (frameRate + threshold))
			doFastFrame = true;
	}
	// Update the last time value.
	m_lastTime = thisTime;
	
	// Tell the emulation thread that we're ready for another frame.
	if (gqt4_emuThread)
		gqt4_emuThread->resume(doFastFrame);
}

}
