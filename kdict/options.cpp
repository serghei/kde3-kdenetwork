/* -------------------------------------------------------------

   options.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   GlobalData      manages all global data of Kdict

   OptionsDialog   the "Preferences" dialog

   DbSetsDialog    dialog for editing the user defined database sets

 ------------------------------------------------------------- */

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qpainter.h>

#include <kcolordialog.h>
#include <kconfig.h>
#include <klocale.h>
#include <knumvalidator.h>
#include <knuminput.h>
#include <klineedit.h>
#include <kcharsets.h>
#include <kfontdialog.h>
#include <kiconloader.h>

#include "options.h"


//*********  DictOptions ******************************************


void GlobalData::read()
{
  KConfig *config=KGlobal::config();

  // general...
  config->setGroup("General");
  defineClipboard = config->readBoolEntry("Lookup_Clipboard",false);
  headLayout = config->readNumEntry("Heading_Layout",0);
  if ((headLayout > 2)||(headLayout < 0))
    headLayout = 0;
  saveHistory = config->readBoolEntry("Save_History",true);
  maxHistEntrys = config->readNumEntry("Max_History_Entrys",500);
  if ((maxHistEntrys < 10)||(maxHistEntrys >5000))
    maxHistEntrys = 200;
  maxBrowseListEntrys = config->readNumEntry("Max_Browse_Entrys",15);
  if ((maxBrowseListEntrys < 1)||(maxBrowseListEntrys > 50))
    maxBrowseListEntrys = 15;
  maxDefinitions = config->readNumEntry("Max_Definitions",2000);
  if ((maxDefinitions < 100)||(maxDefinitions > 10000))
    maxDefinitions = 2000;

  //colors
  config->setGroup("Colors");
  useCustomColors=config->readBoolEntry("customColors", false);

  QColor defCol=KGlobalSettings::textColor();
  c_olors[Ctext]=config->readColorEntry("textColor",&defCol);
  c_olorNames[Ctext]=i18n("Text");

  defCol=KGlobalSettings::baseColor();
  c_olors[Cbackground]=config->readColorEntry("backgroundColor",&defCol);
  c_olorNames[Cbackground]=i18n("Background");

  defCol=KGlobalSettings::highlightedTextColor();
  c_olors[CheadingsText]=config->readColorEntry("headingsTextColor",&defCol);
  c_olorNames[CheadingsText]=i18n("Heading Text");

  defCol=KGlobalSettings::highlightColor();
  c_olors[CheadingsBackground]=config->readColorEntry("headingsBackgroundColor",&defCol);
  c_olorNames[CheadingsBackground]=i18n("Heading Background");

  defCol=KGlobalSettings::linkColor();
  c_olors[Clinks]=config->readColorEntry("linksColor",&defCol);
  c_olorNames[Clinks]=i18n("Link");

  defCol=KGlobalSettings::visitedLinkColor();
  c_olors[CvisitedLinks]=config->readColorEntry("linksColor",&defCol);
  c_olorNames[CvisitedLinks]=i18n("Followed Link");

  //fonts
  config->setGroup("Fonts");
  useCustomFonts=config->readBoolEntry("customFonts", false);

  QFont defFont=KGlobalSettings::generalFont();
  f_onts[Ftext]=config->readFontEntry("textFont",&defFont);
  f_ontNames[Ftext]=i18n("Text");

  defFont.setBold( true );
  defFont.setPointSize(defFont.pointSize()+2);
  f_onts[Fheadings]=config->readFontEntry("headingsFont",&defFont);
  f_ontNames[Fheadings]=i18n("Headings");

  // geometry...
  config->setGroup("Geometry");
  QSize invalid(-1,-1);
  optSize = config->readSizeEntry("Opt_Size",&invalid);
  setsSize = config->readSizeEntry("Sets_Size",&invalid);
  matchSize = config->readSizeEntry("Match_Size",&invalid);
  showMatchList = config->readBoolEntry("Show_MatchList",false);
  splitterSizes = config->readIntListEntry("Splitter_Sizes");

  config->setGroup("Query Combo");
  queryComboCompletionMode = (KGlobalSettings::Completion)config->readNumEntry("Completion_mode",
                                                                               KGlobalSettings::completionMode());

  config->setGroup("Query_History");
  queryHistory = config->readListEntry("History");

  config->setGroup("DictServer");
  server = config->readEntry("Server", "dict.org");
  port = config->readNumEntry("Port", 2628);
  if (port < 0)
    port = 2628;
  timeout = config->readNumEntry("Timeout",60);
  if (timeout < 0)
    timeout = 60;
  pipeSize = config->readNumEntry("Pipe_Size",256);
  if (pipeSize < 0)
    pipeSize = 256;
  idleHold = config->readNumEntry("Idle_Hold",30);
  if (idleHold < 0)
    idleHold = 30;
  encoding=config->readEntry("encoding", "utf8");
  authEnabled = config->readBoolEntry("Auth_Enabled",false);
  user = config->readEntry("User");
  secret = encryptStr(config->readEntry("Secret"));
  serverDatabases = config->readListEntry("Server_Databases");
  currentDatabase = config->readNumEntry("Current_Database",0);
  strategies = config->readListEntry("Strategies");
  if (strategies.isEmpty()) {
    strategies.append(i18n("Spell Check"));
    strategies.append(i18n("Exact"));
    strategies.append(i18n("Prefix"));
  } else {
    strategies.remove(strategies.begin());
    strategies.prepend(i18n("Spell Check"));
  }

  currentStrategy = config->readNumEntry("Current_Strategy",0);
  if (currentStrategy >= strategies.count())
      currentStrategy = 0;

  config->setGroup("Database_Sets");
  databaseSets.setAutoDelete(true);
  int num = config->readNumEntry("Num_Sets",0);
  QStringList* temp;
  QString strNum;
  while (num > 0) {
    temp = new QStringList();
    strNum.setNum(num);
    *temp = config->readListEntry(strNum);
    if (!temp->isEmpty()) {
      databaseSets.prepend(temp);
      num--;
    } else {
      delete temp;
      num = 0;       // stop reading...
    }
  }
  databases = serverDatabases;
  for (int i = databaseSets.count()-1;i>=0;i--)
    databases.prepend(databaseSets.at(i)->first());
  databases.prepend(i18n("All Databases"));
  if (currentDatabase >= databases.count())
    currentDatabase = 0;
}


void GlobalData::write()
{
  KConfig *config=KGlobal::config();

  config->setGroup("General");
  config->writeEntry("Lookup_Clipboard",defineClipboard);
  config->writeEntry("Heading_Layout",headLayout);
  config->writeEntry("Save_History",saveHistory);
  config->writeEntry("Max_History_Entrys",maxHistEntrys);
  config->writeEntry("Max_Browse_Entrys",maxBrowseListEntrys);
  config->writeEntry("Max_Definitions",maxDefinitions);

  config->setGroup("Colors");
  config->writeEntry("customColors",useCustomColors);
  config->writeEntry("textColor", c_olors[Ctext]);
  config->writeEntry("backgroundColor", c_olors[Cbackground]);
  config->writeEntry("headingsTextColor", c_olors[CheadingsText]);
  config->writeEntry("headingsBackgroundColor", c_olors[CheadingsBackground]);
  config->writeEntry("linksColor", c_olors[Clinks]);
  config->writeEntry("visitedLinksColor", c_olors[CvisitedLinks]);

  config->setGroup("Fonts");
  config->writeEntry("customFonts", useCustomFonts);
  config->writeEntry("textFont", f_onts[Ftext]);
  config->writeEntry("headingsFont", f_onts[Fheadings]);

  config->setGroup("Geometry");
  config->writeEntry("Opt_Size",optSize);
  config->writeEntry("Sets_Size",setsSize);
  config->writeEntry("Match_Size",matchSize);
  config->writeEntry("Show_MatchList",showMatchList);
  config->writeEntry("Splitter_Sizes",splitterSizes);

  config->setGroup("Query Combo");
  config->writeEntry("Completion_mode", (int)queryComboCompletionMode);

  config->setGroup("Query_History");
  QStringList copy;
  if (saveHistory)
    copy = queryHistory;
  config->writeEntry("History",copy);

  config->setGroup("DictServer");
  config->writeEntry("Server", server);
  config->writeEntry("Port", port);
  config->writeEntry("Timeout",timeout);
  config->writeEntry("Pipe_Size",pipeSize);
  config->writeEntry("Idle_Hold",idleHold);
  config->writeEntry("encoding", encoding);
  config->writeEntry("Auth_Enabled",authEnabled);
  config->writeEntry("User", user);
  config->writeEntry("Secret", encryptStr(secret));
  config->writeEntry("Server_Databases",serverDatabases);
  config->writeEntry("Current_Database",currentDatabase);
  config->writeEntry("Strategies",strategies);
  config->writeEntry("Current_Strategy",currentStrategy);

  config->setGroup("Database_Sets");
  config->writeEntry("Num_Sets",databaseSets.count());
  QString strNum;
  for (unsigned int i = 0;i<databaseSets.count();i++)
    config->writeEntry(strNum.setNum(i+1),*databaseSets.at(i));
}


QColor GlobalData::defaultColor(int i)
{
  switch(i) {
    case Ctext:
      return KGlobalSettings::textColor();
    break;

    case Cbackground:
      return KGlobalSettings::baseColor();
    break;

    case CheadingsText:
      return KGlobalSettings::highlightedTextColor();
    break;

    case CheadingsBackground:
      return KGlobalSettings::highlightColor();
    break;

    case Clinks:
      return KGlobalSettings::linkColor();
    break;

    case CvisitedLinks:
      return KGlobalSettings::visitedLinkColor();
    break;

  }

  return KGlobalSettings::baseColor();
}


QColor GlobalData::textColor()
{
  if(useCustomColors)
    return c_olors[Ctext];
  else
    return defaultColor(Ctext);
}


QColor GlobalData::backgroundColor()
{
  if(useCustomColors)
    return c_olors[Cbackground];
  else
    return defaultColor(Cbackground);
}


QColor GlobalData::headingsTextColor()
{
  if(useCustomColors)
    return c_olors[CheadingsText];
  else
    return defaultColor(CheadingsText);
}


QColor GlobalData::headingsBackgroundColor()
{
  if(useCustomColors)
    return c_olors[CheadingsBackground];
  else
    return defaultColor(CheadingsBackground);
}


QColor GlobalData::linksColor()
{
  if(useCustomColors)
    return c_olors[Clinks];
  else
    return defaultColor(Clinks);
}


QColor GlobalData::visitedLinksColor()
{
  if(useCustomColors)
    return c_olors[CvisitedLinks];
  else
    return defaultColor(CvisitedLinks);
}


QFont GlobalData::defaultFont(int i)
{
  QFont font = KGlobalSettings::generalFont();

  if (font.pointSize() < 5)
    font.setPointSize(12);

  if (i==Fheadings)
    font.setPointSize(font.pointSize()+5);

  return font;
}


QFont GlobalData::textFont()
{
  if(useCustomFonts)
    return f_onts[Ftext];
  else
    return defaultFont(Ftext);
}


QFont GlobalData::headingsFont()
{
  if(useCustomFonts)
    return f_onts[Fheadings];
  else
    return defaultFont(Fheadings);
}


// simple password scrambling...
QString GlobalData::encryptStr(const QString& aStr)
{
  uint i,val,len = aStr.length();
  QString result;

  for (i=0; i<len; i++)
  {
    val = aStr[i] - ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}


//*********  OptionsDialog::DialogListBox  *****************************


OptionsDialog::DialogListBox::DialogListBox(bool alwaysIgnore, QWidget * parent, const char * name)
 : QListBox(parent, name), a_lwaysIgnore(alwaysIgnore)
{
}


OptionsDialog::DialogListBox::~DialogListBox()
{
}


void OptionsDialog::DialogListBox::keyPressEvent(QKeyEvent *e)
{
  if ((a_lwaysIgnore || !(hasFocus()&&isVisible()))&&((e->key()==Key_Enter)||(e->key()==Key_Return)))
    e->ignore();
  else
    QListBox::keyPressEvent(e);
}


//*********  OptionsDialog::ColorListItem  *****************************


OptionsDialog::ColorListItem::ColorListItem( const QString &text, const QColor &color )
  : QListBoxText(text), mColor( color )
{
}


OptionsDialog::ColorListItem::~ColorListItem()
{
}


void OptionsDialog::ColorListItem::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( 30+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, 30, h-1 );
  p->fillRect( 4, 2, 28, h-3, mColor );
}


int OptionsDialog::ColorListItem::height(const QListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int OptionsDialog::ColorListItem::width(const QListBox *lb ) const
{
  return( 30 + lb->fontMetrics().width( text() ) + 6 );
}


//*********  OptionsDialog::FontListItem  *****************************


OptionsDialog::FontListItem::FontListItem( const QString &name, const QFont &font )
  : QListBoxText(name), f_ont(font)
{
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


OptionsDialog::FontListItem::~FontListItem()
{
}


void OptionsDialog::FontListItem::setFont(const QFont &font)
{
  f_ont = font;
  fontInfo = QString("[%1 %2]").arg(f_ont.family()).arg(f_ont.pointSize());
}


void OptionsDialog::FontListItem::paint( QPainter *p )
{
  QFont fnt = p->font();
  fnt.setWeight(QFont::Bold);
  p->setFont(fnt);
  int fontInfoWidth = p->fontMetrics().width(fontInfo);
  int h = p->fontMetrics().ascent() + p->fontMetrics().leading()/2;
  p->drawText(2, h, fontInfo );
  fnt.setWeight(QFont::Normal);
  p->setFont(fnt);
  p->drawText(5 + fontInfoWidth, h, text() );
}


int OptionsDialog::FontListItem::width(const QListBox *lb ) const
{
  return( lb->fontMetrics().width(fontInfo) + lb->fontMetrics().width(text()) + 20 );
}


//*********  OptionsDialog  ******************************************


OptionsDialog::OptionsDialog(QWidget *parent, const char *name)
  : KDialogBase(IconList, i18n("Configure"), Help|Default|Ok|Apply|Cancel, Ok, parent, name, false, true)
{

  //******** Server ************************************
  serverTab = addPage(i18n("Server"),i18n("DICT Server Configuration"), BarIcon("network", KIcon::SizeMedium ));
  QGridLayout* grid = new QGridLayout(serverTab,10,3,0,spacingHint());

  w_server = new KLineEdit(serverTab);
  w_server->setText(global->server);
  QLabel *l = new QLabel(w_server, i18n("Host&name:"), serverTab);
  grid->addWidget(l,0,0);
  grid->addMultiCellWidget(w_server,0,0,1,2);
  connect( w_server, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );

  w_port = new KLineEdit(serverTab);
  w_port->setValidator(new KIntValidator(0,65536,this));
  w_port->setText(QString::number(global->port));
  l = new QLabel(w_port, i18n("&Port:"), serverTab);
  grid->addWidget(l,1,0);
  grid->addWidget(w_port,1,1);
  connect( w_port, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );

  w_idleHold = new KIntSpinBox(0,300,5,0,10,serverTab);
  w_idleHold->setSuffix(i18n(" sec"));
  w_idleHold->setValue(global->idleHold);
  l = new QLabel(w_idleHold, i18n("Hold conn&ection for:"), serverTab);
  grid->addWidget(l,2,0);
  grid->addWidget(w_idleHold,2,1);
  connect( w_idleHold, SIGNAL( valueChanged(int) ), this, SLOT( slotChanged() ) );

  w_timeout = new KIntSpinBox(5,600,5,5,10,serverTab);
  w_timeout->setSuffix(i18n(" sec"));
  w_timeout->setValue(global->timeout);
  l = new QLabel(w_timeout, i18n("T&imeout:"), serverTab);
  grid->addWidget(l,3,0);
  grid->addWidget(w_timeout,3,1);
  connect( w_timeout, SIGNAL( valueChanged(int) ), this, SLOT( slotChanged() ) );

  w_pipesize = new KIntSpinBox(100,5000,2,2,10,serverTab);
  w_pipesize->setSuffix(i18n(" bytes"));
  w_pipesize->setValue(global->pipeSize);
  l = new QLabel(w_pipesize, i18n("Command &buffer:"), serverTab);
  grid->addWidget(l,4,0);
  grid->addWidget(w_pipesize,4,1);
  connect( w_pipesize, SIGNAL( valueChanged(int) ), this, SLOT( slotChanged() ) );

  QStringList encodingNames = KGlobal::charsets()->descriptiveEncodingNames();
  int i=0,x=0;
  for ( QStringList::Iterator it = encodingNames.begin(); it != encodingNames.end(); ++it ) {
    if (KGlobal::charsets()->encodingForName(*it)==global->encoding) {
      x = i;
      break;
    }
    i++;
  }
  w_encoding = new QComboBox(serverTab);
  w_encoding->insertStringList(encodingNames);
  w_encoding->setCurrentItem(x);
  l = new QLabel(w_encoding, i18n("Encod&ing:"), serverTab);
  grid->addWidget(l,5,0);
  grid->addMultiCellWidget(w_encoding,5,5,1,2);
  connect( w_encoding, SIGNAL( activated(int) ), this, SLOT( slotChanged() ) );

  w_auth = new QCheckBox(i18n("Server requires a&uthentication"),serverTab);
  w_auth->setChecked(global->authEnabled);
  grid->addMultiCellWidget(w_auth,6,6,0,2);
  connect( w_auth, SIGNAL( toggled(bool) ), this, SLOT( slotChanged() ) );
  connect(w_auth,SIGNAL(toggled(bool)),SLOT(slotAuthRequiredToggled(bool)));

  w_user = new KLineEdit(serverTab);
  w_user->setText(global->user);
  l_user = new QLabel(w_user, i18n("U&ser:"),serverTab);
  grid->addWidget(l_user,7,0);
  grid->addMultiCellWidget(w_user,7,7,1,2);
  connect( w_user, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );

  w_secret = new KLineEdit(serverTab);
  w_secret->setEchoMode(QLineEdit::Password);
  w_secret->setText(global->secret);
  l_secret = new QLabel(w_secret, i18n("Pass&word:"), serverTab);
  grid->addWidget(l_secret,8,0);
  grid->addMultiCellWidget(w_secret,8,8,1,2);
  connect( w_secret, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );

  slotAuthRequiredToggled( w_auth->isChecked() );

  grid->setColStretch(1,2);
  grid->setColStretch(2,2);

  //************ Appearance ***************************
  appTab = addPage(i18n("Appearance"),i18n("Customize Visual Appearance"), BarIcon("appearance", KIcon::SizeMedium ));

  QGridLayout *topL=new QGridLayout(appTab, 8, 3, 0, spacingHint());

  //color-list
  c_List = new DialogListBox(false, appTab);
  topL->addMultiCellWidget(c_List,1,3,0,1);
  connect(c_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotColItemSelected(QListBoxItem*)));
  connect(c_List, SIGNAL(selectionChanged()), SLOT(slotColSelectionChanged()));

  c_olorCB = new QCheckBox(i18n("&Use custom colors"),appTab);
  topL->addWidget(c_olorCB,0,0);
  connect(c_olorCB, SIGNAL(toggled(bool)), this, SLOT(slotColCheckBoxToggled(bool)));
  connect(c_olorCB, SIGNAL(toggled(bool) ), this, SLOT( slotChanged()));

  c_olChngBtn=new QPushButton(i18n("Cha&nge..."), appTab);
  connect(c_olChngBtn, SIGNAL(clicked()), SLOT(slotColChangeBtnClicked()));
  topL->addWidget(c_olChngBtn,1,2);

  c_olDefBtn=new QPushButton(i18n("Default&s"), appTab);
  connect(c_olDefBtn, SIGNAL(clicked()), SLOT(slotColDefaultBtnClicked()));
  topL->addWidget(c_olDefBtn,2,2);
  connect(c_olDefBtn, SIGNAL(clicked()), SLOT(slotChanged()));

  //font-list
  f_List = new DialogListBox(false, appTab);
  topL->addMultiCellWidget(f_List,5,7,0,1);
  connect(f_List, SIGNAL(selected(QListBoxItem*)),SLOT(slotFontItemSelected(QListBoxItem*)));
  connect(f_List, SIGNAL(selectionChanged()),SLOT(slotFontSelectionChanged()));

  f_ontCB = new QCheckBox(i18n("Use custom &fonts"),appTab);
  topL->addWidget(f_ontCB,4,0);
  connect(f_ontCB, SIGNAL(toggled(bool)), SLOT(slotFontCheckBoxToggled(bool)));
  connect(f_ontCB, SIGNAL(toggled(bool)), SLOT(slotChanged()));

  f_ntChngBtn=new QPushButton(i18n("Chang&e..."), appTab);
  connect(f_ntChngBtn, SIGNAL(clicked()), SLOT(slotFontChangeBtnClicked()));
  topL->addWidget(f_ntChngBtn,5,2);

  f_ntDefBtn=new QPushButton(i18n("Defaul&ts"), appTab);
  connect(f_ntDefBtn, SIGNAL(clicked()), SLOT(slotFontDefaultBtnClicked()));
  topL->addWidget(f_ntDefBtn,6,2);
  connect(f_ntDefBtn, SIGNAL(clicked()), SLOT(slotChanged()));

  topL->setColStretch(1,2);
  topL->setColStretch(2,0);
  topL->setRowStretch(3,1);
  topL->setRowStretch(7,1);
  topL->setResizeMode(QLayout::Minimum);

  //init
  c_olorCB->setChecked(global->useCustomColors);
  slotColCheckBoxToggled(global->useCustomColors);
  for(int i=0; i<global->colorCount(); i++)
    c_List->insertItem(new ColorListItem(global->colorName(i), global->color(i)));

  f_ontCB->setChecked(global->useCustomFonts);
  slotFontCheckBoxToggled(global->useCustomFonts);
  for(int i=0; i<global->fontCount(); i++)
    f_List->insertItem(new FontListItem(global->fontName(i), global->font(i)));

  //************ Layout ***************************
  layoutTab = addPage(i18n("Layout"),i18n("Customize Output Format"), BarIcon("text_left", KIcon::SizeMedium ));

  QVBoxLayout *vbox = new QVBoxLayout(layoutTab, 0, spacingHint());

  QButtonGroup *bGroup = new QButtonGroup(i18n("Headings"),layoutTab);
  QVBoxLayout *bvbox = new QVBoxLayout(bGroup,8,5);

  bvbox->addSpacing(fontMetrics().lineSpacing()-4);
  w_layout[0] = new QRadioButton(i18n("O&ne heading for each database"),bGroup);
  w_layout[0]->setChecked(global->headLayout == 0);
  bvbox->addWidget(w_layout[0],1);
  w_layout[1] = new QRadioButton(i18n("A&s above, with separators between the definitions"),bGroup);
  w_layout[1]->setChecked(global->headLayout == 1);
  bvbox->addWidget(w_layout[1],1);
  w_layout[2] = new QRadioButton(i18n("A separate heading for &each definition"),bGroup);
  w_layout[2]->setChecked(global->headLayout == 2);
  bvbox->addWidget(w_layout[2],1);
  connect(w_layout[0], SIGNAL(toggled(bool)), SLOT(slotChanged()));
  connect(w_layout[1], SIGNAL(toggled(bool)), SLOT(slotChanged()));
  connect(w_layout[2], SIGNAL(toggled(bool)), SLOT(slotChanged()));

  vbox->addWidget(bGroup,0);
  vbox->addStretch(1);

  //************ Other ***************************
  otherTab = addPage(i18n("Miscellaneous"),i18n("Various Settings"), BarIcon("misc", KIcon::SizeMedium ));

  vbox = new QVBoxLayout(otherTab, 0, spacingHint());

  QGroupBox *group = new QGroupBox(i18n("Limits"),otherTab);

  grid = new QGridLayout(group,4,2,8,5);
  grid->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  w_MaxDefinitions = new KIntSpinBox(100,10000,100,100,10,group);
  w_MaxDefinitions->setValue(global->maxDefinitions);
  l = new QLabel(w_MaxDefinitions, i18n("De&finitions:"), group);
  grid->addWidget(l,1,0);
  grid->addWidget(w_MaxDefinitions,1,1);
  connect(w_MaxDefinitions, SIGNAL(valueChanged(int)), SLOT(slotChanged()));

  w_Maxbrowse = new KIntSpinBox(1,100,1,1,10,group);
  w_Maxbrowse->setValue(global->maxBrowseListEntrys);
  l = new QLabel(w_Maxbrowse, i18n("Cached &results:"), group);
  grid->addWidget(l,2,0);
  grid->addWidget(w_Maxbrowse,2,1);
  connect(w_Maxbrowse, SIGNAL(valueChanged(int)), SLOT(slotChanged()));

  w_Maxhist = new KIntSpinBox(10,5000,10,10,10,group);
  w_Maxhist->setValue(global->maxHistEntrys);
  l = new QLabel(w_Maxhist, i18n("Hi&story entries:"), group);
  grid->addWidget(l,3,0);
  grid->addWidget(w_Maxhist,3,1);
  connect(w_Maxhist, SIGNAL(valueChanged(int)), SLOT(slotChanged()));

  grid->setColStretch(1,1);

  vbox->addWidget(group,0);

  group = new QGroupBox(i18n("Other"),otherTab);

  QVBoxLayout *vbox2 = new QVBoxLayout(group, 8, 5);

  vbox2->addSpacing(fontMetrics().lineSpacing()-4);

  w_Savehist = new QCheckBox(i18n("Sa&ve history on exit"),group);
  w_Savehist->setChecked(global->saveHistory);
  vbox2->addWidget(w_Savehist,0);
  connect(w_Savehist, SIGNAL(toggled(bool)), SLOT(slotChanged()));

  w_Clipboard = new QCheckBox(i18n("D&efine selected text on start"),group);
  w_Clipboard->setChecked(global->defineClipboard);
  vbox2->addWidget(w_Clipboard,1);
  connect(w_Clipboard, SIGNAL(toggled(bool)), SLOT(slotChanged()));

  vbox->addWidget(group,0);
  vbox->addStretch(2);

  setHelp("preferences");

  if (global->optSize.isValid())
    resize(global->optSize);
  else
    resize(300,200);
  enableButton( Apply, false );
  configChanged = false;
}


OptionsDialog::~OptionsDialog()
{
  global->optSize = size();
}


void OptionsDialog::slotApply()
{
  global->server = w_server->text();
  global->port = w_port->text().toInt();
  global->timeout = w_timeout->value();
  global->idleHold = w_idleHold->value();
  global->pipeSize = w_pipesize->value();
  global->encoding = KGlobal::charsets()->encodingForName(w_encoding->currentText());
  global->authEnabled = w_auth->isChecked();
  global->user = w_user->text();
  global->secret = w_secret->text();
  global->useCustomColors=c_olorCB->isChecked();
  for(int i=0; i<global->colorCount(); i++)
    global->c_olors[i] = (static_cast<ColorListItem*>(c_List->item(i)))->color();

  global->useCustomFonts=f_ontCB->isChecked();
  for(int i=0; i<global->fontCount(); i++)
    global->f_onts[i] = (static_cast<FontListItem*>(f_List->item(i)))->font();
  if (w_layout[0]->isChecked())
    global->headLayout = 0;
  else
    if (w_layout[1]->isChecked())
      global->headLayout = 1;
    else
      global->headLayout = 2;
  global->maxDefinitions = w_MaxDefinitions->value();
  global->maxBrowseListEntrys = w_Maxbrowse->value();
  global->maxHistEntrys = w_Maxhist->value();
  global->defineClipboard = w_Clipboard->isChecked();
  global->saveHistory = w_Savehist->isChecked();
  emit(optionsChanged());
  enableButton( Apply, false );
  configChanged = false;
}


void OptionsDialog::slotOk()
{
  if( configChanged )
    slotApply();
  KDialogBase::slotOk();
}


void OptionsDialog::slotDefault()
{
  QStringList encodingNames;
  int i=0,x=0;

  switch(activePageIndex()) {
  case 0:
    w_server->setText("dict.org");
    w_port->setText("2628");
    w_idleHold->setValue(30);
    w_timeout->setValue(60);
    w_pipesize->setValue(256);
    encodingNames = KGlobal::charsets()->descriptiveEncodingNames();
    for ( QStringList::Iterator it = encodingNames.begin(); it != encodingNames.end(); ++it ) {
      if (KGlobal::charsets()->encodingForName(*it)=="utf8")
        x = i;
      i++;
    }
    w_encoding->setCurrentItem(x);
    w_auth->setChecked(false);
    w_user->clear();
    w_user->setEnabled(false);
    w_secret->clear();
    w_secret->setEnabled(false);
    break;
  case 1:
    c_olorCB->setChecked(false);
    slotColCheckBoxToggled(false);
    slotColDefaultBtnClicked();
    f_ontCB->setChecked(false);
    slotFontCheckBoxToggled(false);
    slotFontDefaultBtnClicked();
    break;
  case 2:
    w_layout[0]->setChecked(true);
    break;
  case 3:
    w_MaxDefinitions->setValue(2000);
    w_Maxbrowse->setValue(15);
    w_Maxhist->setValue(500);
    w_Savehist->setChecked(true);
    w_Clipboard->setChecked(false);
  }
}


void OptionsDialog::slotAuthRequiredToggled( bool enable )
{
  l_user->setEnabled( enable );
  l_secret->setEnabled( enable );
  w_user->setEnabled( enable );
  w_secret->setEnabled( enable );
}


void OptionsDialog::slotColCheckBoxToggled(bool b)
{
  c_List->setEnabled(b);
  c_olDefBtn->setEnabled(b);
  c_olChngBtn->setEnabled(b && (c_List->currentItem()!=-1));
  if (b) c_List->setFocus();
}


// show color dialog for the entry
void OptionsDialog::slotColItemSelected(QListBoxItem *it)
{
  if (it) {
    ColorListItem *colorItem = static_cast<ColorListItem*>(it);
    QColor col = colorItem->color();
    int result = KColorDialog::getColor(col,this);

    if (result == KColorDialog::Accepted) {
      colorItem->setColor(col);
      c_List->triggerUpdate(false);
      slotChanged();
    }
  }
}


void OptionsDialog::slotColDefaultBtnClicked()
{
  ColorListItem *colorItem;
  for(int i=0; i < global->colorCount(); i++) {
    colorItem=static_cast<ColorListItem*>(c_List->item(i));
    colorItem->setColor(global->defaultColor(i));
  }
  c_List->triggerUpdate(true);
  c_List->repaint(true);
}


void OptionsDialog::slotColChangeBtnClicked()
{
  if(c_List->currentItem()!=-1)
    slotColItemSelected(c_List->item(c_List->currentItem()));
}


void OptionsDialog::slotColSelectionChanged()
{
  c_olChngBtn->setEnabled(c_List->currentItem()!=-1);
}


void OptionsDialog::slotFontCheckBoxToggled(bool b)
{
  f_List->setEnabled(b);
  f_ntDefBtn->setEnabled(b);
  f_ntChngBtn->setEnabled(b && (f_List->currentItem()!=-1));
  if (b) f_List->setFocus();
}


// show font dialog for the entry
void OptionsDialog::slotFontItemSelected(QListBoxItem *it)
{
  if (it) {
    FontListItem *fontItem = static_cast<FontListItem*>(it);
    QFont font = fontItem->font();
    int result = KFontDialog::getFont(font,false,this);

    if (result == KFontDialog::Accepted) {
      fontItem->setFont(font);
      f_List->triggerUpdate(false);
      slotChanged();
    }
  }
}


void OptionsDialog::slotFontDefaultBtnClicked()
{
  FontListItem *fontItem;
  for(int i=0; i < global->fontCount(); i++) {
    fontItem=static_cast<FontListItem*>(f_List->item(i));
    fontItem->setFont(global->defaultFont(i));
  }
  f_List->triggerUpdate(false);
}


void OptionsDialog::slotFontChangeBtnClicked()
{
  if(f_List->currentItem()!=-1)
    slotFontItemSelected(f_List->item(f_List->currentItem()));
}


void OptionsDialog::slotFontSelectionChanged()
{
  f_ntChngBtn->setEnabled(f_List->currentItem()!=-1);
}

void OptionsDialog::slotChanged()
{
  enableButton( Apply, true );
  configChanged = true;
}


//--------------------------------

#include "options.moc"
