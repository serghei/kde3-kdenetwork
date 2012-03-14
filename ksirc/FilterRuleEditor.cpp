/**********************************************************************

	--- Qt Architect generated file ---

	File: FilterRuleEditor.cpp
	Last generated: Mon Dec 15 18:14:27 1997

 *********************************************************************/

#include "FilterRuleEditor.h"

#include <qregexp.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

FilterRuleEditor::FilterRuleEditor
(
	QWidget* parent,
	const char* name
)
    : KDialogBase( parent, name, true, i18n( "Edit Filter Rules" ),
        Close, Close, true )
{
  filter = new FilterRuleWidget( this, name );

  setMainWidget( filter );

  updateListBox();
  newHighlight(0);

  // ### split "OkPressed()" into 2 slots
  connect( filter->ModifyButton, SIGNAL(clicked()), SLOT(OkPressed()) );
  connect( filter->InsertButton, SIGNAL(clicked()), SLOT(OkPressed()) );

  connect( filter->NewButton, SIGNAL(clicked()), SLOT(newRule()) );
  connect( filter->DeleteButton, SIGNAL(clicked()), SLOT(deleteRule()) );

  connect( filter->UpButton, SIGNAL(clicked()), SLOT(raiseRule()) );
  connect( filter->DownButton, SIGNAL(clicked()), SLOT(lowerRule()) );

  connect( filter->RuleList, SIGNAL(highlighted(int)), SLOT(newHighlight(int)) );
  connect( filter->RuleList, SIGNAL(selected(int)), SLOT(newHighlight(int)) );


  filter->RuleList->setHScrollBarMode( QListBox::AlwaysOff );
  filter->RuleList->setMultiSelection( FALSE );

  filter->DownButton->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
  filter->UpButton->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
}


FilterRuleEditor::~FilterRuleEditor()
{
}

void FilterRuleEditor::newRule()
{
  filter->LineTitle->setText( QString::null );
  filter->LineSearch->setText( QString::null );
  filter->LineFrom->setText( QString::null );
  filter->LineTo->setText( QString::null );
  filter->LineTitle->setFocus();

  filter->InsertButton->setEnabled( true );
  filter->ModifyButton->setEnabled( false );
}

void FilterRuleEditor::OkPressed()
{
  int number, after;
  KConfig *kConfig = kapp->config();

  if(filter->InsertButton->isEnabled()){
    number = kConfig->readNumEntry("Rules", 0) + 1;
    after = number - 1;
    kConfig->writeEntry("Rules", number);
  }
  else if(filter->ModifyButton->isEnabled()){
    number = filter->RuleList->currentItem() + 1;
    after = number - 1;
  }
  else{
    return;
  }

  QString name = filter->LineTitle->text();
  QString search = filter->LineSearch->text();
  QString from = filter->LineFrom->text();
  QString to = filter->LineTo->text();

  if( name.isEmpty() || search.isEmpty() || from.isEmpty() || to.isEmpty() ){
      KMessageBox::error(this, i18n("Cannot create the rule since not\n"
          "all the fields are filled in."), i18n("Error"));

  }
  else{
    kConfig->setGroup("FilterRules");
    QString key;
    key.sprintf("name-%d", number);
    kConfig->writeEntry(key, convertSpecial(name));
    key.sprintf("search-%d", number);
    kConfig->writeEntry(key, convertSpecial(search));
    key.sprintf("from-%d", number);
    kConfig->writeEntry(key, convertSpecial(from));
    key.sprintf("to-%d", number);
    kConfig->writeEntry(key, convertSpecial(to));
    //    kConfig->sync();
    updateListBox(after);
  }
}

void FilterRuleEditor::updateListBox(int citem )
{
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  int number = kConfig->readNumEntry("Rules", 0);
  filter->RuleList->clear();

  for(; number > 0; number--){
    QString key;
    key.sprintf("name-%d", number);
    filter->RuleList->insertItem(kConfig->readEntry(key), 0);
  }
  if(filter->RuleList->count() > 0)
    filter->RuleList->setCurrentItem(citem);
  filter->RuleList->repaint();

  filter->DeleteButton->setEnabled( filter->RuleList->currentItem() > -1 );
  filter->ModifyButton->setEnabled( filter->RuleList->currentItem() > -1 );
  filter->InsertButton->setEnabled( false );
  filter->NewButton->setEnabled( true );
}

void FilterRuleEditor::moveRule(int from, int to)
{
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  QString src;
  QString dest;
  src.sprintf("name-%d", from);
  dest.sprintf("name-%d", to);
  kConfig->writeEntry(dest, kConfig->readEntry(src));
  kConfig->deleteEntry(src);
  src.sprintf("search-%d", from);
  dest.sprintf("search-%d", to);
  kConfig->writeEntry(dest, kConfig->readEntry(src));
  kConfig->deleteEntry(src);
  src.sprintf("from-%d", from);
  dest.sprintf("from-%d", to);
  kConfig->writeEntry(dest, kConfig->readEntry(src));
  kConfig->deleteEntry(src);
  src.sprintf("to-%d", from);
  dest.sprintf("to-%d", to);
  kConfig->writeEntry(dest, kConfig->readEntry(src));
  kConfig->deleteEntry(src);
}

void FilterRuleEditor::deleteRule()
{
  int number = filter->RuleList->currentItem();

  if( number >= 0){
    KConfig *kConfig = kapp->config();
    kConfig->setGroup("FilterRules");
    int max = kConfig->readNumEntry("Rules");
    for(int i = number+2; i <= max; i++){
      moveRule(i, i-1);
    }
    max--;
    kConfig->writeEntry("Rules", max);

    updateListBox();
  }
}

void FilterRuleEditor::newHighlight(int i)
{
  i++;
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  QString key;
  key.sprintf("name-%d", i);
  filter->LineTitle->setText(convertSpecialBack(kConfig->readEntry(key)));
  key.sprintf("search-%d", i);
  filter->LineSearch->setText(convertSpecialBack(kConfig->readEntry(key)));
  key.sprintf("from-%d", i);
  filter->LineFrom->setText(convertSpecialBack(kConfig->readEntry(key)));
  key.sprintf("to-%d", i);
  filter->LineTo->setText(convertSpecialBack(kConfig->readEntry(key)));
}

void FilterRuleEditor::raiseRule()
{
  int item = filter->RuleList->currentItem();
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  int max = kConfig->readNumEntry("Rules");
  if(item > 0){
    moveRule(item, max+1);
    moveRule(item+1, item);
    moveRule(max+1, item+1);
    updateListBox(item - 1);
  }
}

void FilterRuleEditor::lowerRule()
{
  int item = filter->RuleList->currentItem();
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("FilterRules");
  int max = kConfig->readNumEntry("Rules");
  if(item < max-1){
    moveRule(item+2, max+1);
    moveRule(item+1, item+2);
    moveRule(max+1, item+1);
    updateListBox(item+1);
  }
}

QString FilterRuleEditor::convertSpecial(QString str)
{
  str.replace(QRegExp("\\$"), "$$");
  return str;
}

QString FilterRuleEditor::convertSpecialBack(QString str)
{
  str.replace(QRegExp("\\$\\$"), "$");
  return str;
}

#include "FilterRuleEditor.moc"

// vim:tabstop=2:shiftwidth=2:expandtab:cinoptions=(s,U1,m1

