/* -------------------------------------------------------------

   queryview.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

    BrowseData    data structure used for caching definitions
    DictHTMLPart  handling of middle mouse button clicks
    QueryView     widget that displays the definitions

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qtimer.h>

#include <kfiledialog.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kcursor.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <khtml_events.h>
#include <khtmlview.h>

#include "actions.h"
#include "options.h"
#include "dict.h"
#include "queryview.h"
#include <kapplication.h>
#include <kiconloader.h>


//********  SaveHelper  *******************************************

QString SaveHelper::lastPath;

SaveHelper::SaveHelper(const QString &saveName, const QString &filter, QWidget *parent)
  : p_arent(parent), s_aveName(saveName), f_ilter(filter), file(0), tmpFile(0)
{
}


SaveHelper::~SaveHelper()
{
  if (file) {       // local filesystem, just close the file
    delete file;
  } else
    if (tmpFile) {      // network location, initiate transaction
      tmpFile->close();
      if (KIO::NetAccess::upload(tmpFile->name(),url, p_arent) == false)
         KMessageBox::error(global->topLevel, i18n("Unable to save remote file."));
      tmpFile->unlink();   // delete temp file
      delete tmpFile;
    }
}


QFile* SaveHelper::getFile(const QString &dialogTitle)
{
  url = KFileDialog::getSaveURL(lastPath+s_aveName,f_ilter,p_arent,dialogTitle);

  if (url.isEmpty())
    return 0;

  lastPath = url.url(-1);
  lastPath.truncate(lastPath.length()-url.fileName().length());

  if (url.isLocalFile()) {
    if (QFileInfo(url.path()).exists() &&
        (KMessageBox::warningContinueCancel(global->topLevel,
                                            i18n("A file named %1 already exists.\nDo you want to replace it?").arg(url.path()),
                                            dialogTitle, i18n("&Replace")) != KMessageBox::Continue)) {
      return 0;
    }

    file = new QFile(url.path());
    if(!file->open(IO_WriteOnly)) {
      KMessageBox::error(global->topLevel, i18n("Unable to save file."));
      delete file;
      file = 0;
    }
    return file;
  } else {
    tmpFile = new KTempFile();
    if (tmpFile->status()!=0)
      KMessageBox::error(global->topLevel, i18n("Unable to create temporary file."));     {
      delete tmpFile;
      tmpFile = 0;
      return 0;
    }
    return tmpFile->file();
  }
}


//****  BrowseData  ****************************************************


BrowseData::BrowseData(const QString &Nhtml, const QString &NqueryText)
: html(Nhtml),queryText(NqueryText),xPos(0),yPos(0)
{}


//*********  DictHTMLPart  ******************************************

DictHTMLPart::DictHTMLPart(QWidget *parentWidget, const char *widgetname)
  : KHTMLPart(parentWidget,widgetname)
{}


DictHTMLPart::~DictHTMLPart()
{}


void DictHTMLPart::khtmlMouseReleaseEvent(khtml::MouseReleaseEvent *event)
{
  if (event->qmouseEvent()->button()==MidButton)
    emit(middleButtonClicked());
  else
    KHTMLPart::khtmlMouseReleaseEvent(event);
}


//*********  QueryView  ******************************************


QueryView::QueryView(QWidget *_parent)
: QVBox( _parent), actBack(0L), actForward(0L), actQueryCombo(0L), browsePos(0), isRendering(false)
{
  browseList.setAutoDelete(true);

  part=new DictHTMLPart(this);
  part->setDNDEnabled(false);
  part->setJScriptEnabled(false);
  part->setJavaEnabled(false);
  part->setURLCursor(KCursor::handCursor());
  setFocusPolicy(QWidget::NoFocus);
  connect(part, SIGNAL(completed()), SLOT(partCompleted()));
  connect(part, SIGNAL(middleButtonClicked()), SLOT(middleButtonClicked()));
  rightBtnMenu = new KPopupMenu(this);
  connect(part,SIGNAL(popupMenu(const QString &, const QPoint &)),this,SLOT(buildPopupMenu(const QString &, const QPoint &)));
  connect(part->browserExtension(),SIGNAL(openURLRequest(const KURL &,const KParts::URLArgs &)),
          this,SLOT(slotURLRequest(const KURL &,const KParts::URLArgs &)));
  connect(part->browserExtension(),SIGNAL(enableAction(const char *,bool)),this,SLOT(enableAction(const char *,bool)));
  QTimer::singleShot( 0, this, SLOT(optionsChanged()) );
  connect(interface,SIGNAL(resultReady(const QString &, const QString &)), SLOT(resultReady(const QString &, const QString &)));
}


QueryView::~QueryView()
{}


void QueryView::setActions(KToolBarPopupAction* NactBack, KToolBarPopupAction* NactForward, DictComboAction* NactQueryCombo)
{
  actBack = NactBack;
  connect(actBack->popupMenu(),SIGNAL(activated(int)),SLOT(browseBack(int)));
  actForward = NactForward;
  connect(actForward->popupMenu(),SIGNAL(activated(int)),SLOT(browseForward(int)));
  actQueryCombo = NactQueryCombo;
}


bool QueryView::browseBackPossible() const
{
  return (browsePos > 0)? true:false;
}


bool QueryView::browseForwardPossible() const
{
  return (browsePos+1 < browseList.count()) ? true:false;
}


void QueryView::optionsChanged()
{
  saveCurrentResultPos();

  currentHTMLHeader = QString("<html><head><style type=\"text/css\">\n");
  currentHTMLHeader += QString("body { background-color:%1; color:%2; }\n").arg(global->backgroundColor().name()).arg(global->textColor().name());
  currentHTMLHeader += QString("a:link, a:active { color:%1; text-decoration:none; }\n").arg(global->linksColor().name());
  currentHTMLHeader += QString("a:visited { color:%1; text-decoration:none; }\n").arg(global->visitedLinksColor().name());
  currentHTMLHeader += QString("a:hover { color:%1; text-decoration:underline; }\n").arg(global->linksColor().name());
  currentHTMLHeader += QString("p.heading {  margin-left:0mm; margin-top:2mm; margin-bottom:2mm; padding:1mm; background-color:%1; color:%2; font-family:%3; font-size:%4pt; ").arg(global->headingsBackgroundColor().name()).arg(global->headingsTextColor().name()).arg(global->headingsFont().family()).arg(global->headingsFont().pointSize());
  if (global->headingsFont().italic())
    currentHTMLHeader += QString("font-style:italic; ");
  if (global->headingsFont().bold())
    currentHTMLHeader += QString("font-weight:bold; ");
  currentHTMLHeader += QString("}\n");
  currentHTMLHeader += QString("p.definition { margin-left:1.5mm; margin-top:2mm; margin-bottom:2mm;}\n");
  currentHTMLHeader += QString("table { margin-left:1.5mm; margin-top:2mm; margin-bottom:2mm;}\n");
  currentHTMLHeader += QString("pre { font-family:%1; font-size:%2pt; ").arg(global->textFont().family()).arg(global->textFont().pointSize());
  if (global->textFont().italic())
    currentHTMLHeader += QString("font-style:italic; ");
  if (global->textFont().bold())
    currentHTMLHeader += QString("font-weight:bold; ");
  currentHTMLHeader += QString("}\n");
  currentHTMLHeader += QString("</style></head>\n");

  showResult();         // apply changes directly
}


void QueryView::stop()
{
  if (isRendering == true) {
    part->closeURL();
    isRendering = false;
    emit(renderingStopped());
  }
}


// print current result
void QueryView::printQuery()
{
  part->view()->print();
}


// save the current result in an .html file
void QueryView::saveQuery()
{
  if (!browseList.isEmpty()) {
    BrowseData* brw = browseList.at(browsePos);
    QString fName = brw->queryText+".html";
    fName.replace(QRegExp("[\\s/]"),"_");
    SaveHelper helper(fName,"*.html",global->topLevel);
    QFile *file = helper.getFile(QString::null);

    if (file) {
      QTextStream stream(file);
      stream.setEncoding(QTextStream::Locale);
      stream << currentHTMLHeader+brw->html;
    }
  }
}


void QueryView::browseBack()
{
  if (browseBackPossible()) {
    saveCurrentResultPos();
    browsePos--;
    actQueryCombo->setEditText(browseList.at(browsePos)->queryText);
    showResult();
    updateBrowseActions();
  }
}


void QueryView::browseForward()
{
  if (browseForwardPossible()) {
    saveCurrentResultPos();
    browsePos++;
    actQueryCombo->setEditText(browseList.at(browsePos)->queryText);
    showResult();
    updateBrowseActions();
  }
}


void QueryView::selectAll()
{
  part->selectAll();
}


void QueryView::copySelection()
{
  kapp->clipboard()->setText(part->selectedText());
}


void QueryView::showFindDialog()
{
  KAction *act = part->actionCollection()->action("find");
  if (act)
    act->activate();
}


void QueryView::paletteChange ( const QPalette & )
{

  optionsChanged();
}


void QueryView::fontChange ( const QFont & )
{
  optionsChanged();
}


void QueryView::saveCurrentResultPos()
{
  if (!browseList.isEmpty()) {
    browseList.at(browsePos)->xPos = part->view()->contentsX();
    browseList.at(browsePos)->yPos = part->view()->contentsY();
  }
}


void QueryView::showResult()
{
  if (!isRendering) {
    isRendering = true;
    emit(renderingStarted());
  }

  part->begin();
  if (browseList.isEmpty()) {
    part->write(currentHTMLHeader+"<body></body></html>");
    part->end();
  } else {
    BrowseData* brw = browseList.at(browsePos);
    emit(newCaption(getShortString(brw->queryText.simplifyWhiteSpace(),70)));
    part->write(currentHTMLHeader+brw->html);
    part->end();
    part->view()->setFocus();
  }
}


void QueryView::resultReady(const QString &result, const QString &query)
{
  BrowseData* brw = new BrowseData(result,query);

  if (browseList.isEmpty()) {
    browsePos = 0;
    browseList.append(brw);
  } else {
    saveCurrentResultPos();
    while (browseList.count()>browsePos+1)
      browseList.removeLast();
    browseList.append(brw);
    browsePos++;
    while (browseList.count()>global->maxBrowseListEntrys) {
      browseList.removeFirst();
      browsePos--;
    }
  }

  showResult();
  emit(enablePrintSave());
  actQueryCombo->selectAll();
  updateBrowseActions();
}


void QueryView::partCompleted()
{
  if (!browseList.isEmpty())
    part->view()->setContentsPos(browseList.at(browsePos)->xPos,browseList.at(browsePos)->yPos);
  if (isRendering) {
    emit(renderingStopped());
    isRendering = false;
  }
}


void QueryView::slotURLRequest (const KURL &url, const KParts::URLArgs &)
{
  QString type = url.host();
  QString urlText = url.prettyURL();
  urlText.remove(0,8+type.length());

  if (type.length()) {             // valid url
    if(type=="define")
      emit(defineRequested(urlText));
    if(type=="dbinfo")
      interface->showDbInfo(urlText.utf8());
    if(type=="realhttp")
      kapp->invokeBrowser("http://"+urlText);
    if(type=="realftp")
      kapp->invokeBrowser("ftp://"+urlText);
  }
}


void QueryView::middleButtonClicked()
{
  emit(clipboardRequested());
}


// construct the right-mouse-button-popup-menu on demand
void QueryView::buildPopupMenu(const QString &url, const QPoint &point)
{
  rightBtnMenu->clear();

  if (!url.isEmpty()) {                // menuitem if mouse is over link
    KURL u(url);
    QString type = u.host();
    popupLink = u.prettyURL();
    popupLink.remove(0,8+type.length());

    if (type.length()) {             // valid url
      if(type=="define") {
        rightBtnMenu->insertItem(i18n("Define &Synonym"),
                                 this,SLOT(popupDefineLink()));
        rightBtnMenu->insertItem(i18n("M&atch Synonym"),
                                 this,SLOT(popupMatchLink()));
        rightBtnMenu->insertSeparator();
      }
      if(type=="dbinfo") {
        rightBtnMenu->insertItem(i18n("D&atabase Information"),this,SLOT(popupDbInfo()));
        rightBtnMenu->insertSeparator();
      }
      if(type=="realhttp") {
        popupLink.prepend("http://");
        rightBtnMenu->insertItem(SmallIcon("fileopen"),
                                 i18n("&Open Link"),
                                 this,SLOT(popupOpenLink()));
        rightBtnMenu->insertSeparator();
      }
      if(type=="realftp") {
        popupLink.prepend("ftp://");
        rightBtnMenu->insertItem(SmallIcon("fileopen"),
                                 i18n("&Open Link"),
                                 this,SLOT(popupOpenLink()));
        rightBtnMenu->insertSeparator();
      }
    }
  }

  if (part->hasSelection()) {
    popupSelect = part->selectedText();
    rightBtnMenu->insertItem(i18n("&Define Selection"),
                             this,SLOT(popupDefineSelect()));
    rightBtnMenu->insertItem(i18n("&Match Selection"),
                             this,SLOT(popupMatchSelect()));
    rightBtnMenu->insertSeparator();
  } else {
    kapp->clipboard()->setSelectionMode(false);
    QString text = kapp->clipboard()->text();
    if (text.isEmpty()) {
      kapp->clipboard()->setSelectionMode(true);
      text = kapp->clipboard()->text();
    }
    if (!text.isEmpty()) {
      popupSelect = QApplication::clipboard()->text();
      rightBtnMenu->insertItem(SmallIcon("define_clip"),
                               i18n("&Define Clipboard Content"),
                               this,SLOT(popupDefineSelect()));
      rightBtnMenu->insertItem(i18n("&Match Clipboard Content"),
                               this,SLOT(popupMatchSelect()));
      rightBtnMenu->insertSeparator();
    }
  }

  int ID;

  if (browseBackPossible()) {    // if possible, show string
    if (browseList.at(browsePos-1)->queryText.isEmpty())
      rightBtnMenu->insertItem(SmallIcon("back"),
                               i18n("&Back: Information"),
                               this,SLOT(browseBack()));
    else
      rightBtnMenu->insertItem(SmallIcon("back"),
                               i18n("&Back: '%1'").arg(getShortString(browseList.at(browsePos-1)->queryText,25)),
                               this,SLOT(browseBack()));
  } else {
    ID = rightBtnMenu->insertItem(SmallIcon("back"), i18n("&Back"), this, SLOT(browseBack()));
    rightBtnMenu->setItemEnabled(ID,false);
  }

  if (browseForwardPossible()) {  // if possible, show string
    if (browseList.at(browsePos+1)->queryText.isEmpty())
      rightBtnMenu->insertItem(SmallIcon("forward"),
                               i18n("&Forward: Information"),
                               this,SLOT(browseForward()));
    else
      rightBtnMenu->insertItem(SmallIcon("forward"),
                               i18n("&Forward: '%1'").arg(getShortString(browseList.at(browsePos+1)->queryText,25)),
                               this,SLOT(browseForward()));
  } else {
    ID = rightBtnMenu->insertItem(SmallIcon("forward"),i18n("&Forward"),this,SLOT(browseForward()));
    rightBtnMenu->setItemEnabled(ID,false);
  }

  rightBtnMenu->popup(point);
}


void QueryView::popupDefineLink()
{
  emit(defineRequested(popupLink));
}


void QueryView::popupMatchLink()
{
  emit(matchRequested(popupLink));
}


void QueryView::popupOpenLink()
{
  kapp->invokeBrowser(popupLink);
}


void QueryView::popupDefineSelect()
{
  emit(defineRequested(popupSelect));
}


void QueryView::popupMatchSelect()
{
  emit(matchRequested(popupSelect));
}


void QueryView::popupDbInfo()
{

  interface->showDbInfo(popupLink.utf8());
}


void QueryView::enableAction(const char * name, bool enabled)
{
  if (!strcmp(name,"copy"))
    emit(enableCopy(enabled));
}


void QueryView::browseBack(int index)
{
  int x = browsePos-index;
  if (x>=0) {
    saveCurrentResultPos();
    browsePos = x;
    actQueryCombo->setEditText(browseList.at(browsePos)->queryText);
    showResult();
    QTimer::singleShot(0, this, SLOT(updateBrowseActions()));  // don't clear the menu in this slot
  }
}


void QueryView::browseForward(int index)
{
  int x = browsePos+index;
  if (x < (int)(browseList.count())) {
    saveCurrentResultPos();
    browsePos = x;
    actQueryCombo->setEditText(browseList.at(browsePos)->queryText);
    showResult();
    QTimer::singleShot(0, this, SLOT(updateBrowseActions()));  // don't clear the menu in this slot
  }
}


void QueryView::updateBrowseActions()
{
  if (browseBackPossible()) {
    actBack->setEnabled(true);
    if (browseList.at(browsePos-1)->queryText.isEmpty())
      actBack->setText(i18n("&Back: Information"));
    else
      actBack->setText(i18n("&Back: '%1'").arg(getShortString(browseList.at(browsePos-1)->queryText,25)));

    actBack->popupMenu()->clear();
    int i = browsePos-1;
    int num = 1;
    QString s;
    while ((i>=0)&&(num<=10)) {
      s = browseList.at(i)->queryText;
      if (s.isEmpty()) s = i18n("Information");
      actBack->popupMenu()->insertItem(s,num);
      num++;
      i--;
    }
  } else {
    actBack->setEnabled(false);
    actBack->setText(i18n("&Back"));
    actBack->popupMenu()->clear();
  }

  if (browseForwardPossible()) {
    actForward->setEnabled(true);
    if (browseList.at(browsePos+1)->queryText.isEmpty())
      actForward->setText(i18n("&Forward: Information"));
    else
      actForward->setText(i18n("&Forward: '%1'").arg(getShortString(browseList.at(browsePos+1)->queryText,25)));

    actForward->popupMenu()->clear();
    int i = browsePos+1;
    int num = 1;
    QString s;
    while ((i<(int)(browseList.count()))&&(num<=10)) {
      s = browseList.at(i)->queryText;
      if (s.isEmpty()) s = i18n("Information");
      actForward->popupMenu()->insertItem(s,num);
      num++;
      i++;
    }
  } else {
    actForward->setEnabled(false);
    actForward->setText(i18n("&Forward"));
    actForward->popupMenu()->clear();
  }
}

//--------------------------------

#include "queryview.moc"
