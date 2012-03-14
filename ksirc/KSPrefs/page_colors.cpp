/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "page_colors.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qlabel.h>

#include <kconfig.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kdebug.h>

PageColors::PageColors( QWidget *parent, const char *name ) : PageColorsBase( parent, name)
{
    changing = 0;
    m_dcol.setAutoDelete(true);
}

PageColors::~PageColors()
{
}

void PageColors::saveConfig()
{
    ksopts->backgroundColor = backCBtn->color();
    ksopts->selBackgroundColor = selBackCBtn->color();
    ksopts->selForegroundColor = selForeCBtn->color();
    ksopts->errorColor = errorCBtn->color();
    ksopts->infoColor = infoCBtn->color();
    ksopts->textColor = genericTextCBtn->color();
    ksopts->channelColor = chanMsgCBtn->color();
    ksopts->linkColor = linkCBtn->color();

    ksopts->ownNickColor = ownNickCBtn->color();
    ksopts->ownNickBold = ownNickBoldCB->isChecked();
    ksopts->ownNickRev = ownNickRevCB->isChecked();
    ksopts->ownNickUl = ownNickUlCB->isChecked();

    if(fixedOtherColRB->isChecked()){
	ksopts->nickColourization = false;
	ksopts->nickForeground = nickFGCBtn->color();
	ksopts->nickBackground = nickBGCBtn->color();
    }
    else if(autoOtherColRB->isChecked()){
	ksopts->nickColourization = true;
	ksopts->nickForeground = QColor();
	ksopts->nickBackground = QColor();
    }
    else {
	ksopts->nickColourization = false;
	ksopts->nickForeground = QColor();
	ksopts->nickBackground = QColor();
    }

    ksopts->msgContainNick = ownContainNickCBtn->color();
    if(msg1LE->text().length() > 0){
	ksopts->msg1Contain = msg1CBtn->color();
	ksopts->msg1String = msg1LE->text();
	ksopts->msg1Regex = msg1Regex->isChecked();
    }
    else {
	ksopts->msg1Contain = QColor();
	ksopts->msg1String = msg1LE->text();
	ksopts->msg1Regex = false;
    }
    if(msg2LE->text().length() > 0){
	ksopts->msg2Contain = msg2CBtn->color();
	ksopts->msg2String = msg2LE->text();
        ksopts->msg2Regex = msg2Regex->isChecked();
    }
    else {
	ksopts->msg2Contain = QColor();
	ksopts->msg2String = msg2LE->text();
	ksopts->msg2Regex = false;
    }

    ksopts->ksircColors = allowKSircColorsCB->isChecked();
    ksopts->mircColors = allowMIRCColorsCB->isChecked();
    ksopts->colourTheme = themeLB->currentText();

    KConfig *conf = kapp->config();
    QDictIterator<KSOColors> it(m_dcol);
    QStringList names;
    for(; it.current(); ++it){
        names << it.currentKey();
        conf->setGroup("ColourSchemes-"+ it.currentKey());
        conf->writeEntry("Background", it.current()->backgroundColor);
        conf->writeEntry("SelBackground", it.current()->selBackgroundColor);
        conf->writeEntry("SelForeground", it.current()->selForegroundColor);
        conf->writeEntry("Error", it.current()->errorColor);
        conf->writeEntry("Info", it.current()->infoColor);
        conf->writeEntry("Text", it.current()->textColor);
        conf->writeEntry("Channel", it.current()->channelColor);
        conf->writeEntry("Link", it.current()->linkColor);
        conf->writeEntry("OwnNick", it.current()->ownNickColor);
        conf->writeEntry("NickForeground", it.current()->nickForeground);
        conf->writeEntry("NickBackground", it.current()->nickBackground);
    }
    conf->setGroup("ColourSchemes");
    conf->writeEntry("Names", names);

}

void PageColors::readConfig( const KSOColors *opts )
{
    backCBtn->setColor( opts->backgroundColor );
    selBackCBtn->setColor( opts->selBackgroundColor );
    selForeCBtn->setColor( opts->selForegroundColor );
    errorCBtn->setColor( opts->errorColor );
    infoCBtn->setColor( opts->infoColor );
    genericTextCBtn->setColor( opts->textColor );
    chanMsgCBtn->setColor( opts->channelColor );
    linkCBtn->setColor( opts->linkColor );

    ownNickCBtn->setColor( opts->ownNickColor );
    ownNickBoldCB->setChecked( opts->ownNickBold );
    ownNickRevCB->setChecked( opts->ownNickRev );
    ownNickUlCB->setChecked( opts->ownNickUl );

    if( opts->nickColourization ){
	autoOtherColRB->setChecked( true );
	nickFGCBtn->setColor( QColor() );
        nickBGCBtn->setColor( QColor() );

    }
    else if( opts->nickForeground.isValid() ||
	     opts->nickBackground.isValid() ){
	fixedOtherColRB->setChecked( true );
	nickFGCBtn->setColor( opts->nickForeground );
        nickBGCBtn->setColor( opts->nickBackground );
    }
    else {
	noOtherColRB->setChecked( true );
	nickFGCBtn->setColor( QColor() );
        nickBGCBtn->setColor( QColor() );

    }

    ownContainNickCBtn->setColor( opts->msgContainNick );
    msg1CBtn->setColor( opts->msg1Contain );
    msg1LE->setText( opts->msg1String );
    msg1Regex->setChecked( opts->msg1Regex );
    msg2CBtn->setColor( opts->msg2Contain );
    msg2LE->setText( opts->msg2String );
    msg2Regex->setChecked( opts->msg2Regex );

    coloursSetEnable();

    allowKSircColorsCB->setChecked( opts->ksircColors );
    allowMIRCColorsCB->setChecked( opts->mircColors );

    KConfig *conf = kapp->config();
    conf->setGroup("ColourSchemes");
    themeLB->clear();
    QStringList names = conf->readListEntry("Names");
    if(names.contains("Custom")){
        names.remove(names.find("Custom"));
    }
    names.prepend("Custom");
    themeLB->insertStringList(names);
    if(themeLB->findItem(ksopts->colourTheme, Qt::ExactMatch))
        themeLB->setCurrentItem(themeLB->findItem(ksopts->colourTheme, Qt::ExactMatch));
    else
        themeLB->setCurrentItem(0);
    themeLE->setText(themeLB->currentText());

    m_dcol.clear();

    QStringList::Iterator it = names.begin();
    for( ; it != names.end(); ++it){
        conf->setGroup("ColourSchemes-"+ *it);
        m_dcol.insert(*it, new KSOColors);
        m_dcol[*it]->backgroundColor = conf->readColorEntry( "Background");
        m_dcol[*it]->selBackgroundColor = conf->readColorEntry( "SelBackground");
        m_dcol[*it]->selForegroundColor = conf->readColorEntry( "SelForeground");
        m_dcol[*it]->errorColor = conf->readColorEntry( "Error");
        m_dcol[*it]->infoColor = conf->readColorEntry( "Info");
        m_dcol[*it]->textColor = conf->readColorEntry( "Text");
        m_dcol[*it]->channelColor = conf->readColorEntry( "Channel");
        m_dcol[*it]->ownNickColor = conf->readColorEntry( "OwnNick");
        m_dcol[*it]->nickForeground = conf->readColorEntry( "NickForeground");
        m_dcol[*it]->nickBackground = conf->readColorEntry( "NickBackground");
        m_dcol[*it]->linkColor = conf->readColorEntry("Link");
    }
}

void PageColors::defaultConfig()
{
    KSOColors opts;
    readConfig( &opts );
}

void PageColors::changed()
{
    if(changing == 0)
	themeLB->setSelected(0, TRUE);
    coloursSetEnable();
    emit modified();
}

void PageColors::theme_clicked(QListBoxItem *li)
{
    if(li == 0x0)
        return;

    QString name = li->text();

    changing = 1;
    backCBtn->setColor( m_dcol[name]->backgroundColor );
    selBackCBtn->setColor( m_dcol[name]->selBackgroundColor );
    selForeCBtn->setColor( m_dcol[name]->selForegroundColor );
    errorCBtn->setColor( m_dcol[name]->errorColor );
    infoCBtn->setColor( m_dcol[name]->infoColor );
    genericTextCBtn->setColor( m_dcol[name]->textColor );
    chanMsgCBtn->setColor( m_dcol[name]->channelColor );
    linkCBtn->setColor( m_dcol[name]->linkColor );

    ownNickCBtn->setColor( m_dcol[name]->ownNickColor );
    nickFGCBtn->setColor( m_dcol[name]->nickForeground );
    nickBGCBtn->setColor( m_dcol[name]->nickBackground );
    changing = 0;

    themeLE->setText(li->text());
}

void PageColors::themeNewPB_clicked()
{
    themeLE->clear();
}

void PageColors::themeAddPB_clicked()
{
    QString name = themeLE->text();

    kdDebug(5008) << "Got add: " << themeLB->currentText() << endl;

    m_dcol.replace(name, new KSOColors());

    m_dcol[name]->backgroundColor =  backCBtn->color();
    m_dcol[name]->selBackgroundColor =  selBackCBtn->color();
    m_dcol[name]->selForegroundColor =  selForeCBtn->color();
    m_dcol[name]->errorColor = errorCBtn->color();
    m_dcol[name]->infoColor =  infoCBtn->color();
    m_dcol[name]->textColor = genericTextCBtn->color();
    m_dcol[name]->channelColor = chanMsgCBtn->color();
    m_dcol[name]->linkColor =  linkCBtn->color();
    m_dcol[name]->ownNickColor = ownNickCBtn->color();
    m_dcol[name]->nickForeground = nickFGCBtn->color();
    m_dcol[name]->nickBackground =  nickBGCBtn->color();

    if(themeLB->findItem(name, Qt::ExactMatch) == 0){
        themeLB->insertItem(name);
    }

    themeLB->setCurrentItem(themeLB->findItem(name, Qt::ExactMatch));

}

void PageColors::themeDelPB_clicked()
{
    m_dcol.remove(themeLB->currentText());
    themeLB->removeItem(themeLB->currentItem());
}

void PageColors::theme_sel()
{
    themeLE->setText(themeLB->currentText());
}

void PageColors::coloursSetEnable()
{
    nickFGCBtn->setEnabled( fixedOtherColRB->isChecked() );
    nickBGCBtn->setEnabled( fixedOtherColRB->isChecked() );
    nickFGColorLabel->setEnabled( fixedOtherColRB->isChecked() );
    nickBGColorLabel->setEnabled( fixedOtherColRB->isChecked() );

    bool msgEn = msg1LE->text().length() > 0;
    msg1CBtn->setEnabled(msgEn);
    msg1Regex->setEnabled(msgEn);
    msgEn = msg2LE->text().length() > 0;
    msg2CBtn->setEnabled(msgEn);
    msg2Regex->setEnabled(msgEn);

}

#include "page_colors.moc"
