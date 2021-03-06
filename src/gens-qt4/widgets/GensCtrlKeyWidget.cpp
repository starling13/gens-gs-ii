/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlKeyWidget.hpp: Controller key input widget.                     *
 * Based on KKeySequenceWidget from KDE 4.6.0.                             *
 *                                                                         *
 * Copyright (c) 1998 Mark Donohoe <donohoe@kde.org>                       *
 * Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>                      *
 * Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>               *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#include "GensCtrlKeyWidget.hpp"
#include "GensCtrlKeyWidget_p.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeySequence>
#include <QtGui/QToolButton>
#include <QtGui/QIcon>
#include <QtGui/QLabel>

// Qt events.
#include <QtGui/QKeyEvent>
#include <QtGui/QFocusEvent>

// GensQApplication::IconFromTheme()
#include "../GensQApplication.hpp"
#include "Input/KeyHandlerQt.hpp"

// Key names.
#include "KeyNames.hpp"

// Needed for GetAsyncKeyState().
#if defined(Q_WS_WIN)
#include <windows.h>
#endif

namespace GensQt4 {

class GensCtrlKeyWidgetPrivate
{
	public:
		GensCtrlKeyWidgetPrivate(GensCtrlKeyWidget *q, QLabel *lblDisplay = 0);

	private:
		GensCtrlKeyWidget *const q_ptr;
		Q_DECLARE_PUBLIC(GensCtrlKeyWidget)
	private:
		Q_DISABLE_COPY(GensCtrlKeyWidgetPrivate)

	public:
		void init(void);
		static QIcon GetClearButtonIcon(void);

		void updateShortcutDisplay(void);
		void startRecording(void);

		void cancelRecording(void)
		{
			gensKey = oldGensKey;
			doneRecording();
		}

	public:
		// private slot
		void doneRecording(void);
		void blinkLabel(void);

		// members
		QHBoxLayout *layout;
		GensCtrlKeyButton *keyButton;
		QToolButton *clearButton;
		QLabel *lblDisplay;

		QTimer keyChangeTimeout;
		QTimer blinkTimer;
		static const int ms_KeyChangeTimeout = 10000;	// milliseconds
		static const int ms_BlinkTimer = 500;		// milliseconds

		// key sequence values
		GensKey_t gensKey;
		GensKey_t oldGensKey;
		bool isRecording;
};

/***************************************
 * GensCtrlKeyWidgetPrivate functions. *
 ***************************************/

GensCtrlKeyWidgetPrivate::GensCtrlKeyWidgetPrivate(GensCtrlKeyWidget *q, QLabel *lblDisplay)
	: q_ptr(q)
	, layout(nullptr)
	, keyButton(nullptr)
	, clearButton(nullptr)
	, lblDisplay(lblDisplay)
	, gensKey(0)
	, oldGensKey(0)
	, isRecording(false)
{ }

/**
 * Get the clear button icon.
 * @return Clear button icon.
 */
inline QIcon GensCtrlKeyWidgetPrivate::GetClearButtonIcon(void)
{
	// Determine which icon to use for the clear button.
	QString iconName;
	if (QApplication::isLeftToRight())
		iconName = QLatin1String("edit-clear-locationbar-rtl");
	else
		iconName = QLatin1String("edit-clear-locationbar-ltr");

	return GensQApplication::IconFromTheme(iconName);
}

/**
 * Initialize the private data members.
 */
void GensCtrlKeyWidgetPrivate::init(void)
{
	Q_Q(GensCtrlKeyWidget);
	layout = new QHBoxLayout(q);
	layout->setMargin(0);

	// Create the key button.
	keyButton = new GensCtrlKeyButton(this, q);
	keyButton->setFocusPolicy(Qt::StrongFocus);
	keyButton->setIcon(GensQApplication::IconFromTheme(QLatin1String("configure")));
	layout->addWidget(keyButton);

	// Create the clear button.
	clearButton = new QToolButton(q);
	layout->addWidget(clearButton);

	// Set the clear button icon.
	clearButton->setIcon(GetClearButtonIcon());
}

/**
 * Start recording a key sequence.
 */
void GensCtrlKeyWidgetPrivate::startRecording()
{
	oldGensKey = gensKey;
	gensKey = 0;

	isRecording = true;
	keyButton->grabKeyboard();

	if (!QWidget::keyboardGrabber()) {
		// TODO: Output a warning.
		//kWarning() << "Failed to grab the keyboard! Most likely qt's nograb option is active";
	}

	keyButton->setDown(true);
	updateShortcutDisplay();

	// Start the timers.
	keyChangeTimeout.start(ms_KeyChangeTimeout);
	blinkTimer.start(ms_BlinkTimer);
}

/**
 * Done recording a key sequence.
 */
void GensCtrlKeyWidgetPrivate::doneRecording(void)
{
	isRecording = false;
	keyChangeTimeout.stop();
	blinkTimer.stop();
	if (lblDisplay)
		lblDisplay->setVisible(true);

	// Release the keyboard grabber.
	keyButton->releaseKeyboard();
	keyButton->setDown(false);

	Q_Q(GensCtrlKeyWidget);
	if (gensKey == oldGensKey) {
		// The sequence hasn't changed
		updateShortcutDisplay();
		// TODO: Separate signal for "cancelled".
		emit q->keyUnchanged();
		return;
	}

	// Key sequence has changed.
        emit q->keyChanged(gensKey);
	updateShortcutDisplay();
}

/**
 * Blink the label.
 */
void GensCtrlKeyWidgetPrivate::blinkLabel(void)
{
	if (!lblDisplay)
		return;

	if (!isRecording)
		lblDisplay->setVisible(true);
	else
		lblDisplay->setVisible(!lblDisplay->isVisible());
}

/**
 * Update the shortcut display.
 */
void GensCtrlKeyWidgetPrivate::updateShortcutDisplay(void)
{
	if (!lblDisplay)
		return;

	if (isRecording) {
		// Shortcut display should blink "Press a key..."
		//: Key input is currently being recorded.
		lblDisplay->setText(GensCtrlKeyWidget::tr("Press a key..."));
		lblDisplay->setTextFormat(Qt::PlainText);
	} else {
		// Display the key.
		// TODO: Move LibGens::DevManager::KeyName to the UI.
		QString keyName = KeyNames::keyName(gensKey);
		if (keyName.isEmpty()) {
			//: No key is assigned.
			keyName = GensCtrlKeyWidget::tr("None", "key-name");
		}

		lblDisplay->setText(keyName);
		lblDisplay->setTextFormat(Qt::PlainText);
	}
}

/********************************
 * GensCtrlKeyWidget functions. *
 ********************************/

GensCtrlKeyWidget::GensCtrlKeyWidget(QWidget *parent, QLabel *label)
	: super(parent)
	, d_ptr(new GensCtrlKeyWidgetPrivate(this, label))
{
	// Initialize the private members.
	Q_D(GensCtrlKeyWidget);
	d->init();
	setFocusProxy(d->keyButton);
	
	// Connect signals to the buttons.
	connect(d->keyButton, SIGNAL(clicked()), this, SLOT(captureKey()));
	connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearKey()));
	connect(&d->keyChangeTimeout, SIGNAL(timeout()), this, SLOT(captureKeyTimeout()));
	connect(&d->blinkTimer, SIGNAL(timeout()), this, SLOT(blinkLabel()));

	//TODO: how to adopt style changes at runtime?
	/*QFont modFont = d->clearButton->font();
	modFont.setStyleHint(QFont::TypeWriter);
	d->clearButton->setFont(modFont);*/
	d->updateShortcutDisplay();

	if (d->lblDisplay) {
		// Make sure the label is unreferenced if it's destroyed.
		connect(d->lblDisplay, SIGNAL(destroyed(QObject*)),
			this, SLOT(labelDestroyed()));
	}
}

GensCtrlKeyWidget::~GensCtrlKeyWidget()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void GensCtrlKeyWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LayoutDirectionChange) {
		// Update the clear button icon.
		Q_D(GensCtrlKeyWidget);
		d->clearButton->setIcon(GensCtrlKeyWidgetPrivate::GetClearButtonIcon());
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/**
 * Set the display label.
 */
void GensCtrlKeyWidget::setLabel(QLabel *label)
{
	Q_D(GensCtrlKeyWidget);

	if (d->lblDisplay) {
		// Disconnected the destroyed() signal from the current label.
		disconnect(d->lblDisplay, SIGNAL(destroyed(QObject*)),
			   this, SLOT(labelDestroyed()));
	}

	d->lblDisplay = label;
	if (d->lblDisplay) {
		// Connect the destroyed() signal from the new label.
		connect(d->lblDisplay, SIGNAL(destroyed(QObject*)),
			this, SLOT(labelDestroyed()));
	}

	// Update the shortcut display.
	d->updateShortcutDisplay();
}

/**
 * captureKey() has timed out.
 * Forwards to GensCtrlKeyWidgetPrivate::doneRecording().
 * We can't use Q_PRIVATE_SLOT() because the private class isn't available,
 * and we're not using KDE's automoc4 to generate mocs without linking
 * them into the executable separately.
 * 
 * (automoc4 links mocs using #include.)
 */
void GensCtrlKeyWidget::captureKeyTimeout(void)
{
	Q_D(GensCtrlKeyWidget);
	d->gensKey = d->oldGensKey;
	d->doneRecording();
}

/**
 * Blink the label.
 * Forwards to GensCtrlKeyWidgetPrivate::blinkLabel().
 * We can't use Q_PRIVATE_SLOT() because the private class isn't available,
 * and we're not using KDE's automoc4 to generate mocs without linking
 * them into the executable separately.
 */
void GensCtrlKeyWidget::blinkLabel(void)
{
	Q_D(GensCtrlKeyWidget);
	d->blinkLabel();
}

/**
 * Label was destroyed.
 */
void GensCtrlKeyWidget::labelDestroyed(void)
{
	Q_D(GensCtrlKeyWidget);
	d->lblDisplay = nullptr;
}

/**
 * Return the currently selected GensKey_t.
 * @return Currently selected GensKey_t.
 */
GensKey_t GensCtrlKeyWidget::key(void) const
{
	Q_D(const GensCtrlKeyWidget);
	return d->gensKey;
}

/**
 * Capture a GensKey_t.
 */
void GensCtrlKeyWidget::captureKey(void)
{
	Q_D(GensCtrlKeyWidget);
	d->startRecording();
}

/**
 * Set the current GensKey_t.
 * @param seq Key sequence.
 */
void GensCtrlKeyWidget::setKey(GensKey_t key)
{
	// oldKeySequence holds the key sequence before recording started, if setKeySequence()
	// is called while not recording then set oldKeySequence to the existing sequence so
	// that the keySequenceChanged() signal is emitted if the new and previous key
	// sequences are different
	Q_D(GensCtrlKeyWidget);
	if (!d->isRecording)
		d->oldGensKey = d->gensKey;

	d->gensKey = key;
	d->doneRecording();
}

/**
 * GensCtrlKeyWidget::clearKey(): Clear the current GensKey_t.
 */
void GensCtrlKeyWidget::clearKey()
{
	setKey(0);
}

/************************************
 * GensCtrlKeyButton functions. *
 ************************************/

/**
 * GensCtrlKeyButton::event(): Handle events.
 * This is mainly to prevent Qt from special-casing Tab and Backtab.
 * @param e QEvent.
 */
bool GensCtrlKeyButton::event(QEvent* e)
{
	if (d->isRecording && e->type() == QEvent::KeyPress) {
		keyPressEvent(static_cast<QKeyEvent*>(e));
		return true;
	}

	// The shortcut 'alt+c' ( or any other dialog local action shortcut )
	// ended the recording and triggered the action associated with the
	// action. In case of 'alt+c' ending the dialog.  It seems that those
	// ShortcutOverride events get sent even if grabKeyboard() is active.
	if (d->isRecording && e->type() == QEvent::ShortcutOverride) {
		e->accept();
		return true;
	}

	return QPushButton::event(e);
}

/**
 * GensCtrlKeyButton::keyPressEvent(): Key press event.
 * @param e QKeyEvent.
 */
void GensCtrlKeyButton::keyPressEvent(QKeyEvent *e)
{
	int keyQt = e->key();
	if (keyQt == -1) {
		// Qt sometimes returns garbage keycodes, I observed -1, if it doesn't know a key.
		// We cannot do anything useful with those (several keys have -1, indistinguishable)
		// and QKeySequence.toString() will also yield a garbage string.
		
		// TODO: Show an error message.
#if 0
		KMessageBox::sorry(this,
			i18n("The key you just pressed is not supported by Qt."),
			i18n("Unsupported Key"));
#endif
		d->cancelRecording();
	}

	//don't have the return or space key appear as first key of the sequence when they
	//were pressed to start editing - catch and them and imitate their effect
	if (!d->isRecording && ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space))) {
		d->startRecording();
		d->updateShortcutDisplay();
		return;
	}

	// We get events even if recording isn't active.
	if (!d->isRecording)
		return QPushButton::keyPressEvent(e);

	e->accept();

	// Check for modifier keys.
	GensKey_t keyVal = 0;
	if (e->modifiers() != 0 && e->modifiers() != Qt::KeypadModifier) {
		// Modifier key is pressed.
		keyVal = KeyHandlerQt::NativeModifierToKeyVal(e);
		if (keyVal == 0) {
			// Unable to convert the native modifier key.
#ifdef Q_WS_WIN
			// Check for Win32 modifiers.
			// NOTE: This prefers left modifiers in the case that both are pressed.
			// TODO: Split into a separate function.
			switch (e->key()) {
				case Qt::Key_Shift:
					if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
						keyVal = KEYV_LSHIFT;
					else if (GetAsyncKeyState(VK_RSHIFT) & 0x8000)
						keyVal = KEYV_RSHIFT;
					break;
				
				case Qt::Key_Control:
					if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
						keyVal = KEYV_LCTRL;
					else if (GetAsyncKeyState(VK_RCONTROL) & 0x8000)
						keyVal = KEYV_RCTRL;
					break;
				
				case Qt::Key_Alt:
					if (GetAsyncKeyState(VK_LMENU) & 0x8000)
						keyVal = KEYV_LALT;
					else if (GetAsyncKeyState(VK_RMENU) & 0x8000)
						keyVal = KEYV_RALT;
					break;
				
				default:
					break;
			}
#endif
		}
	} else {
		// No unexpected modifier keys.
		keyVal = KeyHandlerQt::QKeyEventToKeyVal(e);
	}

	// Save the key value.
	d->gensKey = keyVal;
	if (keyVal != 0)
		d->doneRecording();
}

/**
 * Focus out event.
 * @param e QFocusEvent.
 */
void GensCtrlKeyButton::focusOutEvent(QFocusEvent *e)
{
	Q_UNUSED(e)

	// Key sequence recording should be cancelled if the
	// GensCtrlKeyButton loses focus.
	if (d->isRecording)
		d->cancelRecording();
}

}
