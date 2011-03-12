/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * CtrlConfig.cpp: Controller configuration.                               *
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

#include "CtrlConfig.hpp"

// I/O devices.
#include "libgens/IO/IoBase.hpp"
#include "libgens/IO/Io3Button.hpp"
#include "libgens/IO/Io6Button.hpp"
#include "libgens/IO/Io2Button.hpp"
#include "libgens/IO/IoMegaMouse.hpp"

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QLatin1String>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

namespace GensQt4
{

/** Static CtrlConfig members. **/
const int CtrlConfig::MAX_BTNS;		// initialized in CtrlConfig.hpp

class CtrlConfigPrivate
{
	public:
		CtrlConfigPrivate(CtrlConfig *q);
		
		// Dirty flag.
		bool isDirty(void) const;
		void clearDirty(void);
		
		// Controller types.
		LibGens::IoBase::IoType ctrlTypes[CtrlConfig::PORT_MAX];
		
		// Key configuration.
		GensKey_t ctrlKeys[CtrlConfig::PORT_MAX][CtrlConfig::MAX_BTNS];
		
		// Get the number of buttons for a given controller type.
		static int NumButtons(LibGens::IoBase::IoType ioType);
		
		// Get an internal port name. (non-localized)
		static QString PortName(CtrlConfig::CtrlPort_t port);
		
		// Load/Save functions.
		int load(const QSettings& settings);
		int save(QSettings& settings);
	
	private:
		CtrlConfig *const q;
		Q_DISABLE_COPY(CtrlConfigPrivate)
		
		// Dirty flag.
		bool m_dirty;
		
		// Key value separator in the config file.
		static const QChar chrKeyValSep;
};

/********************************
 * CtrlConfigPrivate functions. *
 ********************************/

// Key value separator in the config file.
const QChar CtrlConfigPrivate::chrKeyValSep = QChar(L':');

CtrlConfigPrivate::CtrlConfigPrivate(CtrlConfig* q)
	: q(q)
	, m_dirty(true)
{
	// Clear the controller types and key configuration arrays.
	// TODO: Load defaults.
	memset(ctrlTypes, 0x00, sizeof(ctrlTypes));
	memset(ctrlKeys, 0x00, sizeof(ctrlKeys));
}


/**
 * CtrlConfigPrivate::isDirty(): Check if the controller configuration is dirty.
 * @return True if the controller configuration is dirty; false otherwise.
 */
inline bool CtrlConfigPrivate::isDirty(void) const
	{ return m_dirty; }

/**
 * CtrlConfigPrivate::clearDirty(): Clear the dirty flag.
 */
inline void CtrlConfigPrivate::clearDirty(void)
	{ m_dirty = false; }


/**
 * NumButtons(): Get the number of buttons for a given controller type.
 * @param ioType I/O type.
 * @return Number of buttons, or 0 if none or error. (TODO: -1 for error?)
 * TODO: Make this a CtrlConfig function?
 */
int CtrlConfigPrivate::NumButtons(LibGens::IoBase::IoType ioType)
{
	switch (ioType)
	{
		default:
		case LibGens::IoBase::IOT_NONE:		return LibGens::IoBase::NumButtons();
		case LibGens::IoBase::IOT_3BTN:		return LibGens::Io3Button::NumButtons();
		case LibGens::IoBase::IOT_6BTN:		return LibGens::Io6Button::NumButtons();
		case LibGens::IoBase::IOT_2BTN:		return LibGens::Io2Button::NumButtons();
		case LibGens::IoBase::IOT_MEGA_MOUSE:	return LibGens::IoMegaMouse::NumButtons();
		
		// TODO: Other devices.
#if 0
		IOT_TEAMPLAYER	= 5,
		IOT_4WP_MASTER	= 6,
		IOT_4WP_SLAVE	= 7,
#endif
	}
	
	// Should not get here...
	return 0;
}


/**
 * CtrlConfigPrivate::PortName(): Get an internal port name. (non-localized)
 * @param port Port number.
 * @return Port name, or empty string on error.
 */
QString CtrlConfigPrivate::PortName(CtrlConfig::CtrlPort_t port)
{
	switch (port)
	{
		// System controller ports.
		case CtrlConfig::PORT_1:	return QLatin1String("port1");
		case CtrlConfig::PORT_2:	return QLatin1String("port2");
		
		// Team Player, Port 1.
		case CtrlConfig::PORT_TP1A:	return QLatin1String("portTP1A");
		case CtrlConfig::PORT_TP1B:	return QLatin1String("portTP1B");
		case CtrlConfig::PORT_TP1C:	return QLatin1String("portTP1C");
		case CtrlConfig::PORT_TP1D:	return QLatin1String("portTP1D");

		// Team Player, Port 2.
		case CtrlConfig::PORT_TP2A:	return QLatin1String("portTP2A");
		case CtrlConfig::PORT_TP2B:	return QLatin1String("portTP2B");
		case CtrlConfig::PORT_TP2C:	return QLatin1String("portTP2C");
		case CtrlConfig::PORT_TP2D:	return QLatin1String("portTP2D");
		
		// 4-Way Play.
		case CtrlConfig::PORT_4WPA:	return QLatin1String("port4WPA");
		case CtrlConfig::PORT_4WPB:	return QLatin1String("port4WPB");
		case CtrlConfig::PORT_4WPC:	return QLatin1String("port4WPC");
		case CtrlConfig::PORT_4WPD:	return QLatin1String("port4WPD");
		
		default:
			// Unknown port.
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * CtrlConfigPrivate::load(): Load controller configuration from a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfigPrivate::load(const QSettings& settings)
{
	for (int i = CtrlConfig::PORT_1; i < CtrlConfig::PORT_MAX; i++)
	{
		// Get the controller type.
		// TODO: Allow ASCII controller types?
		const QString portName = PortName((CtrlConfig::CtrlPort_t)i);
		ctrlTypes[i] = (LibGens::IoBase::IoType)
				(settings.value(portName + QLatin1String("/type"), LibGens::IoBase::IOT_NONE).toInt());
		if (ctrlTypes[i] < LibGens::IoBase::IOT_NONE ||
		    ctrlTypes[i] >= LibGens::IoBase::IOT_MAX)
		{
			ctrlTypes[i] = LibGens::IoBase::IOT_NONE;
		}
		
		// Clear the controller keys.
		memset(ctrlKeys[i], 0x00, sizeof(ctrlKeys[i]));
		
		// Read the controller keys from the configuration file.
		const QStringList keyData =
			settings.value(portName + QLatin1String("/keys"), QString()).toString().split(chrKeyValSep);
		
		// Copy the controller keys into ctrlKeys[].
		for (int j = qMin(keyData.size(), NumButtons(ctrlTypes[i])) - 1; j >= 0; j--)
		{
			ctrlKeys[i][j] = keyData.at(j).toUInt(NULL, 0);
		}
	}
	
	// Controller configuration loaded.
	m_dirty = true;
	return 0;
}


/**
 * CtrlConfigPrivate::save(): Save controller configuration to a settings file.
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfigPrivate::save(QSettings& settings)
{
	for (int i = CtrlConfig::PORT_1; i < CtrlConfig::PORT_MAX; i++)
	{
		// Save the controller type.
		// TODO: Allow ASCII controller types?
		const QString portName = PortName((CtrlConfig::CtrlPort_t)i);
		settings.setValue(portName + QLatin1String("/type"), (int)ctrlTypes[i]);
		
		// Save the controller keys.
		QString keyData;
		QString keyHex;
		const int numButtons = NumButtons(ctrlTypes[i]);
		
		// Write the buttons to the configuration file.
		for (int j = 0; j < qMin(numButtons, CtrlConfig::MAX_BTNS); j++)
		{
			if (j > 0)
				keyData += QChar(chrKeyValSep);
			keyHex = QString::number(ctrlKeys[i][j], 16);
			keyData += QLatin1String("0x");
			if (ctrlKeys[i][j] <= 0xFFFF)
				keyData += keyHex.rightJustified(4, QChar(L'0'));
			else
				keyData += keyHex.rightJustified(8, QChar(L'0'));
		}
		
		settings.setValue(portName + QLatin1String("/keys"), keyData);
	}
	
	// Controller configuration saved.
	return 0;
}


/*************************
 * CtrlConfig functions. *
 *************************/

CtrlConfig::CtrlConfig(QObject *parent)
	: QObject(parent)
	, d(new CtrlConfigPrivate(this))
{
}

CtrlConfig::~CtrlConfig()
{
	delete d;
}


/**
 * CtrlConfig::isDirty(): Check if the controller configuration is dirty.
 * WRAPPER FUNCTION for CtrlConfigPrivate::isDirty().
 * @return True if the controller configuration is dirty; false otherwise.
 */
bool CtrlConfig::isDirty(void) const
	{ return d->isDirty(); }

/**
 * CtrlConfig::clearDirty(): Clear the dirty flag.
 * WRAPPER FUNCTION for CtrlConfigPrivate::clearDirty().
 */
void CtrlConfig::clearDirty(void)
	{ d->clearDirty(); }


/**
 * CtrlConfig::load(): Load controller configuration from a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::load(const QSettings& settings)
{
	return d->load(settings);
}


/**
 * save(): Save controller configuration to a settings file.
 * WRAPPER FUNCTION for CtrlConfigPrivate::load().
 * NOTE: The group must be selected in the QSettings before calling this function!
 * @param settings Settings file.
 * @return 0 on success; non-zero on error.
 */
int CtrlConfig::save(QSettings& settings)
{
	return d->save(settings);
}


/** Controller update functions. **/


/**
 * updatePort1(): Update controller port 1.
 * @param ppOldPort Pointer to IoBase variable, possibly containing an IoBase object.
 * ppOldPort may be updated with the address to the new IoBase object.
 */
void CtrlConfig::updatePort1(LibGens::IoBase **ppOldPort) const
{
	LibGens::IoBase *oldPort = *ppOldPort;
	LibGens::IoBase *newPort;
	// TODO: Team Player / 4WP support.
	
	if (!oldPort || oldPort->devType() != d->ctrlTypes[PORT_1])
	{
		// New port needs to be created.
		switch (d->ctrlTypes[PORT_1])
		{
			default:
			case LibGens::IoBase::IOT_NONE:
				newPort = new LibGens::IoBase(oldPort);
				break;
			case LibGens::IoBase::IOT_3BTN:
				newPort = new LibGens::Io3Button(oldPort);
				break;
			case LibGens::IoBase::IOT_6BTN:
				newPort = new LibGens::Io6Button(oldPort);
				break;
			case LibGens::IoBase::IOT_2BTN:
				newPort = new LibGens::Io2Button(oldPort);
				break;
			case LibGens::IoBase::IOT_MEGA_MOUSE:
				newPort = new LibGens::IoMegaMouse(oldPort);
				break;
		}
		
		// Delete the old port.
		delete oldPort;
	}
	else
	{
		// Device type is the same.
		newPort = oldPort;
	}
	
	// Assign the new keymap.
	// TODO
	
	// Update the port variable.
	*ppOldPort = newPort;
}

}
