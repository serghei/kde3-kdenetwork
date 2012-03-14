/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "page_irccolors.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <kconfig.h>
#include <kcolorbutton.h>

PageIRCColors::PageIRCColors( QWidget *parent, const char *name ) : PageIRCColorsBase( parent, name)
{
}

PageIRCColors::~PageIRCColors()
{
}

void PageIRCColors::saveConfig()
{
    ksopts->ircColors[0] = CBtn_0->color();
    ksopts->ircColors[1] = CBtn_1->color();
    ksopts->ircColors[2] = CBtn_2->color();
    ksopts->ircColors[3] = CBtn_3->color();
    ksopts->ircColors[4] = CBtn_4->color();
    ksopts->ircColors[5] = CBtn_5->color();
    ksopts->ircColors[6] = CBtn_6->color();
    ksopts->ircColors[7] = CBtn_7->color();
    ksopts->ircColors[8] = CBtn_8->color();
    ksopts->ircColors[9] = CBtn_9->color();
    ksopts->ircColors[10] = CBtn_10->color();
    ksopts->ircColors[11] = CBtn_11->color();
    ksopts->ircColors[12] = CBtn_12->color();
    ksopts->ircColors[13] = CBtn_13->color();
    ksopts->ircColors[14] = CBtn_14->color();
    ksopts->ircColors[15] = CBtn_15->color();

    ksopts->nickHighlight[0] = CBox_0->isChecked();
    ksopts->nickHighlight[1] = CBox_1->isChecked();
    ksopts->nickHighlight[2] = CBox_2->isChecked();
    ksopts->nickHighlight[3] = CBox_3->isChecked();
    ksopts->nickHighlight[4] = CBox_4->isChecked();
    ksopts->nickHighlight[5] = CBox_5->isChecked();
    ksopts->nickHighlight[6] = CBox_6->isChecked();
    ksopts->nickHighlight[7] = CBox_7->isChecked();
    ksopts->nickHighlight[8] = CBox_8->isChecked();
    ksopts->nickHighlight[9] = CBox_9->isChecked();
    ksopts->nickHighlight[10] = CBox_10->isChecked();
    ksopts->nickHighlight[11] = CBox_11->isChecked();
    ksopts->nickHighlight[12] = CBox_12->isChecked();
    ksopts->nickHighlight[13] = CBox_13->isChecked();
    ksopts->nickHighlight[14] = CBox_14->isChecked();
    ksopts->nickHighlight[15] = CBox_15->isChecked();


}

void PageIRCColors::readConfig( const KSOColors *opts )
{
    CBtn_0->setColor(opts->ircColors[0]);
    CBtn_1->setColor(opts->ircColors[1]);
    CBtn_2->setColor(opts->ircColors[2]);
    CBtn_3->setColor(opts->ircColors[3]);
    CBtn_4->setColor(opts->ircColors[4]);
    CBtn_5->setColor(opts->ircColors[5]);
    CBtn_6->setColor(opts->ircColors[6]);
    CBtn_7->setColor(opts->ircColors[7]);
    CBtn_8->setColor(opts->ircColors[8]);
    CBtn_9->setColor(opts->ircColors[9]);
    CBtn_10->setColor(opts->ircColors[10]);
    CBtn_11->setColor(opts->ircColors[11]);
    CBtn_12->setColor(opts->ircColors[12]);
    CBtn_13->setColor(opts->ircColors[13]);
    CBtn_14->setColor(opts->ircColors[14]);
    CBtn_15->setColor(opts->ircColors[15]);

    CBox_0->setChecked(opts->nickHighlight[0]);
    CBox_1->setChecked(opts->nickHighlight[1]);
    CBox_2->setChecked(opts->nickHighlight[2]);
    CBox_3->setChecked(opts->nickHighlight[3]);
    CBox_4->setChecked(opts->nickHighlight[4]);
    CBox_5->setChecked(opts->nickHighlight[5]);
    CBox_6->setChecked(opts->nickHighlight[6]);
    CBox_7->setChecked(opts->nickHighlight[7]);
    CBox_8->setChecked(opts->nickHighlight[8]);
    CBox_9->setChecked(opts->nickHighlight[9]);
    CBox_10->setChecked(opts->nickHighlight[10]);
    CBox_11->setChecked(opts->nickHighlight[11]);
    CBox_12->setChecked(opts->nickHighlight[12]);
    CBox_13->setChecked(opts->nickHighlight[13]);
    CBox_14->setChecked(opts->nickHighlight[14]);
    CBox_15->setChecked(opts->nickHighlight[15]);

    TL_0->setPaletteBackgroundColor(opts->backgroundColor);
    TL_1->setPaletteBackgroundColor(opts->backgroundColor);
    TL_2->setPaletteBackgroundColor(opts->backgroundColor);
    TL_3->setPaletteBackgroundColor(opts->backgroundColor);
    TL_4->setPaletteBackgroundColor(opts->backgroundColor);
    TL_5->setPaletteBackgroundColor(opts->backgroundColor);
    TL_6->setPaletteBackgroundColor(opts->backgroundColor);
    TL_7->setPaletteBackgroundColor(opts->backgroundColor);
    TL_8->setPaletteBackgroundColor(opts->backgroundColor);
    TL_9->setPaletteBackgroundColor(opts->backgroundColor);
    TL_10->setPaletteBackgroundColor(opts->backgroundColor);
    TL_11->setPaletteBackgroundColor(opts->backgroundColor);
    TL_12->setPaletteBackgroundColor(opts->backgroundColor);
    TL_13->setPaletteBackgroundColor(opts->backgroundColor);
    TL_14->setPaletteBackgroundColor(opts->backgroundColor);
    TL_15->setPaletteBackgroundColor(opts->backgroundColor);

    TL_0->setPaletteForegroundColor(opts->ircColors[0]);
    TL_1->setPaletteForegroundColor(opts->ircColors[1]);
    TL_2->setPaletteForegroundColor(opts->ircColors[2]);
    TL_3->setPaletteForegroundColor(opts->ircColors[3]);
    TL_4->setPaletteForegroundColor(opts->ircColors[4]);
    TL_5->setPaletteForegroundColor(opts->ircColors[5]);
    TL_6->setPaletteForegroundColor(opts->ircColors[6]);
    TL_7->setPaletteForegroundColor(opts->ircColors[7]);
    TL_8->setPaletteForegroundColor(opts->ircColors[8]);
    TL_9->setPaletteForegroundColor(opts->ircColors[9]);
    TL_10->setPaletteForegroundColor(opts->ircColors[10]);
    TL_11->setPaletteForegroundColor(opts->ircColors[11]);
    TL_12->setPaletteForegroundColor(opts->ircColors[12]);
    TL_13->setPaletteForegroundColor(opts->ircColors[13]);
    TL_14->setPaletteForegroundColor(opts->ircColors[14]);
    TL_15->setPaletteForegroundColor(opts->ircColors[15]);

    TL_0->setFont(ksopts->defaultFont);
    TL_1->setFont(ksopts->defaultFont);
    TL_2->setFont(ksopts->defaultFont);
    TL_3->setFont(ksopts->defaultFont);
    TL_4->setFont(ksopts->defaultFont);
    TL_5->setFont(ksopts->defaultFont);
    TL_6->setFont(ksopts->defaultFont);
    TL_7->setFont(ksopts->defaultFont);
    TL_8->setFont(ksopts->defaultFont);
    TL_9->setFont(ksopts->defaultFont);
    TL_10->setFont(ksopts->defaultFont);
    TL_11->setFont(ksopts->defaultFont);
    TL_12->setFont(ksopts->defaultFont);
    TL_13->setFont(ksopts->defaultFont);
    TL_14->setFont(ksopts->defaultFont);
    TL_15->setFont(ksopts->defaultFont);
}

void PageIRCColors::defaultConfig()
{
    KSOColors opts;
    readConfig( &opts );
}

void PageIRCColors::changed()
{
    TL_0->setPaletteForegroundColor(CBtn_0->color());
    TL_1->setPaletteForegroundColor(CBtn_1->color());
    TL_2->setPaletteForegroundColor(CBtn_2->color());
    TL_3->setPaletteForegroundColor(CBtn_3->color());
    TL_4->setPaletteForegroundColor(CBtn_4->color());
    TL_5->setPaletteForegroundColor(CBtn_5->color());
    TL_6->setPaletteForegroundColor(CBtn_6->color());
    TL_7->setPaletteForegroundColor(CBtn_7->color());
    TL_8->setPaletteForegroundColor(CBtn_8->color());
    TL_9->setPaletteForegroundColor(CBtn_9->color());
    TL_10->setPaletteForegroundColor(CBtn_10->color());
    TL_11->setPaletteForegroundColor(CBtn_11->color());
    TL_12->setPaletteForegroundColor(CBtn_12->color());
    TL_13->setPaletteForegroundColor(CBtn_13->color());
    TL_14->setPaletteForegroundColor(CBtn_14->color());
    TL_15->setPaletteForegroundColor(CBtn_15->color());
    emit modified();
}


#include "page_irccolors.moc"

