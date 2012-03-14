/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSPREFS_H
#define KSPREFS_H

#include <kdialogbase.h>

class PageGeneral;
class PageColors;
class PageIRCColors;
class PageStartup;
class PageRMBMenu;
class PageServChan;
class PageAutoConnect;
class PageLooknFeel;
class PageShortcuts;

class PageFont; /* For the font settings */


class KSPrefs : public KDialogBase
{

Q_OBJECT
public:
    KSPrefs(QWidget * parent = 0, const char * name = 0);
    ~KSPrefs();

public slots:
    void saveConfig();
    void defaultConfig();
    void readConfig();
    void modified();

signals:
    void update(int);

private:
    PageColors   *pageColors;
    PageIRCColors   *pageIRCColors;
    PageGeneral  *pageGeneral;
    PageRMBMenu  *pageRMBMenu;
    PageStartup  *pageStartup;
    PageServChan *pageServChan;
    PageAutoConnect *pageAutoConnect;
    PageLooknFeel *pageLooknFeel;
    PageShortcuts *pageShortcuts;

    int pSCDirty;
    int pACDirty;
    int pLFDirty;
    int pShortDirty;

    PageFont *pageFont; /* Font settings page */
    int dirty;
};

#endif
