/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication.hpp: QApplication subclass.                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
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

#ifndef __GENS_QT4_GENSQAPPLICATION_HPP__
#define __GENS_QT4_GENSQAPPLICATION_HPP__

#include <gens-qt4/config.gens-qt4.h>

#include "SigHandler.hpp"

// Qt includes.
#include <QtGui/QApplication>
#include <QtGui/QIcon>
#include <QtCore/QThread>
#include <QtGui/QStyle>

namespace GensQt4 {

class GensQApplicationPrivate;

class GensQApplication : public QApplication
{
	Q_OBJECT

	public:
		GensQApplication(int &argc, char **argv);
		GensQApplication(int &argc, char **argv, bool GUIenabled);
		GensQApplication(int &argc, char **argv, Type type);
		virtual ~GensQApplication();

	private:
		typedef QApplication super;
		friend class GensQApplicationPrivate;
		GensQApplicationPrivate *const d;
	private:
		Q_DISABLE_COPY(GensQApplication)

	public:
		/**
		 * Check if the current thread is the GUI thread.
		 * @return True if it is; false if it isn't.
		 */
		bool isGuiThread(void);

		/**
		 * Get an icon from the system theme.
		 * @param name Icon name.
		 * @return QIcon.
		 */
		static QIcon IconFromTheme(const QString &name);

		/**
		 * Get an icon from the Gens/GS II icon set.
		 * @param name Icon name.
		 * @param subcategory Subcategory.
		 * @return QIcon.
		 */
		static QIcon IconFromProgram(const QString &name, const QString &subcategory);

		/**
		 * Get an icon from the Gens/GS II icon set.
		 * @param name Icon name.
		 * @return QIcon.
		 */
		static QIcon IconFromProgram(const QString &name);

		/**
		 * Get a standard icon.
		 * @param standardIcon Standard pixmap.
		 * @param option QStyleOption.
		 * @param widget QWidget.
		 * @return QIcon.
		 */
		static QIcon StandardIcon(QStyle::StandardPixmap standardIcon,
				const QStyleOption *option = 0,
				const QWidget *widget = 0);

#ifdef Q_OS_WIN32
		// Win32 event filter.
		bool winEventFilter(MSG *msg, long *result);
#endif /* Q_OS_WIN32 */

		/**
		 * HACK: The following mess is a hack to get the
		 * custom signal handler dialog to work across
		 * multiple threads. Don't mess around with it!
		 */

	signals:
#ifdef HAVE_SIGACTION
		void signalCrash(int signum, siginfo_t *info, void *context);
#else /* HAVE_SIGACTION */
		void signalCrash(int signum);
#endif /* HAVE_SIGACTION */
	
	private:
#ifdef Q_OS_WIN32
		/**
		 * Set the Qt font to match the system font.
		 */
		static void SetFont_Win32(void);
#endif /* Q_OS_WIN32 */

		friend class SigHandler; // Allow SigHandler to call doCrash().
#ifdef HAVE_SIGACTION
		inline void doCrash(int signum, siginfo_t *info, void *context)
			{ emit signalCrash(signum, info, context); }
#else /* HAVE_SIGACTION */
		inline void doCrash(int signum)
			{ emit signalCrash(signum); }
#endif /* HAVE_SIGACTION */

	private slots:
#ifdef HAVE_SIGACTION
		inline void slotCrash(int signum, siginfo_t *info, void *context)
			{ SigHandler::SignalHandler(signum, info, context); }
#else /* HAVE_SIGACTION */
		inline void slotCrash(int signum)
			{ SigHandler::SignalHandler(signum); }
#endif /* HAVE_SIGACTION */
};

}

#endif /* __GENS_QT4_GENSQAPPLICATION_HPP__ */
