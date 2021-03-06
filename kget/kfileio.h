/***************************************************************************
*                                kfileio.h
*                             -------------------
*
*    Revision     : $Id: kfileio.h 476275 2005-11-01 01:30:58Z thiago $
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
*
***************************************************************************/

/* Load / save entire (local) files with nice diagnostics dialog messages.
 * These functions load/save the whole buffer in one i/o call, so they
 * should be pretty efficient.
 *
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL.
 */
#ifndef kfileio_h
#define kfileio_h

/** Load a file. Returns a pointer to the memory-block that contains
 * the loaded file. Returns a NULL string if the file could not be loaded.
 * If withDialogs is FALSE no warning dialogs are opened if there are
 * problems.
 * The string returned is always zero-terminated and therefore one
 * byte longer than the file itself.
 * If ensureNewline is TRUE the string will always have a trailing newline.
 */
QString kFileToString(const QString & fileName, bool ensureNewline = TRUE, bool withDialogs = TRUE);

/** Save a file. If withDialogs is FALSE no warning dialogs are opened if
 * there are problems. Returns TRUE on success and FALSE on failure.
 * Replaces existing files without warning if askIfExists==FALSE.
 * Makes a copy if the file exists to filename~ if createBackup==TRUE.
 */
bool kCStringToFile(const QCString & buffer, const QString & fileName, bool askIfExists = FALSE, bool createBackup = TRUE, bool withDialogs = TRUE);

// Does not stop at NUL
bool kByteArrayToFile(const QByteArray & buffer, const QString & fileName, bool askIfExists = FALSE, bool createBackup = TRUE, bool withDialogs = TRUE);


#endif                          /* kfileio_h */
