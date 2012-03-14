/**********************************************************************

 The IO DCC Window

 $$Id: ioDCC.cpp 234524 2003-06-29 15:21:53Z mlaurent $$

 Display DCC progress, etc.  This in the future should be expanded.

**********************************************************************/

#include "ioDCC.h"
#include "ksircprocess.h"
#include "displayMgr.h"
#include "control_message.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpassivepopup.h>

#include <qregexp.h>

extern DisplayMgr *displayMgr;

KSircIODCC::KSircIODCC(KSircProcess *_proc) :
    QObject(),
    KSircMessageReceiver(_proc)
{
    proc = _proc;
    setBroadcast(FALSE);
    mgr = new dccTopLevel(0x0, "dccTopLevel Manager");
    displayMgr->newTopLevel(mgr, FALSE);
    displayMgr->setCaption(mgr, proc->serverName() + i18n(" DCC Controller"));

    connect(mgr->mgr(), SIGNAL(dccConnectClicked(dccItem *)), this, SLOT(dccConnectClicked(dccItem *)));
    connect(mgr->mgr(), SIGNAL(dccResumeClicked(dccItem *)), this, SLOT(dccResumeClicked(dccItem *)));
    connect(mgr->mgr(), SIGNAL(dccRenameClicked(dccItem *)), this, SLOT(dccRenameClicked(dccItem *)));
    connect(mgr->mgr(), SIGNAL(dccAbortClicked(dccItem *)), this, SLOT(dccAbortClicked(dccItem *)));
    connect(mgr->mgr(), SIGNAL(outputLine(QCString)), this, SIGNAL(outputLine(QCString)));
}


KSircIODCC::~KSircIODCC()
{
    //    displayMgr->removeTopLevel(mgr);
    if(mgr)
	delete (dccTopLevel *) mgr;
}

void KSircIODCC::sirc_receive(QCString str, bool )
{
  if(!mgr)
        return;
  // Parse the string to find out what type it is.
  // Note the order here.
  // Most people tend to receive files, so let's
  // parse the byte xfered messages first since there's lot's of them.
  // The we get lots of send bytexfer messages so parse them second.
  // Then we look at the one time start/stop messages.  They only arrive
  // once in a long long time (compared to the byte messages) so if it takes
  // a few extra clock cycles to find them, who cares?
  if(str.find("DCC GET read:", 0) != -1){ /*fold01*/
      QRegExp rx("read: (.+) who: (.+) bytes: (.*)");
      if(rx.search(str)){
	  QString key = QString("%1/%2").arg(rx.cap(1)).arg(rx.cap(2));
	  uint bytesXfer = rx.cap(3).toUInt();

	  //
	  // Only update the display for 1% intervals.
	  // LastSize + 1%Size must be less than the total xfered bytes.
	  //
	  if(DCCGetItems[key]){
	      DCCGetItems[key]->setReceivedBytes(bytesXfer/1024);
	  }
      }
  }
  else if(str.find("DCC SEND write:", 0) != -1){ /*fold01*/
      QRegExp rx("write: (.+) who: (.+) bytes: (.*)");
      if(rx.search(str)){
	  QString key = QString("%1/%2").arg(rx.cap(1)).arg(rx.cap(2));
	  uint bytesXfer = rx.cap(3).toUInt();

	  if(DCCSendItems[key]){
	      DCCSendItems[key]->setReceivedBytes(bytesXfer/1024);
	  }
      }
  }
  else if(str.find("INBOUND DCC SEND", 0) != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) file: (.+) size: (.*) ip: (.+) port: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
          QString size = rx.cap(3);
          QString ip = rx.cap(4);
          //QSTring port = rx.cap(5);

          int fileSize = size.toInt(); // Bytes per step
          fileSize /= 1024;

          // setup dcc dialog
          displayMgr->show(mgr);
	  dccItem *it = mgr->mgr()->newGetItem(filename, who, dccItem::dccGotOffer, fileSize);
	  connect(it, SIGNAL(itemRenamed(dccItem *, QString, QString)),
                  this, SLOT(dccRenameDone(dccItem *, QString, QString)));
	  it->setWhoPostfix("(" + ip + ")");

          QString key = QString("%1/%2").arg(filename).arg(who);
	  if(DCCGetItems[key]){
	      /*
	       * don't add duplicates, this cuases real headaches
	       * for both us and the user.
               */
              QString renamed;
	      dccItem *old = DCCGetItems.take(key);
	      int i;
	      for(i = 0; i <= (int) DCCGetItems.count()+1; i++){
		  renamed = QString("%1 (finished %2)/%3").arg(filename).arg(i+1).arg(old->who());
		  if( DCCGetItems[renamed] == 0x0)
                      break;
	      }
	      old->changeFilename(QString("%1 (finished %2)").arg(filename).arg(i+1));
              DCCGetItems.insert(renamed, it);
	  }

          DCCGetItems.insert(key, it);
      }
      else {
          kdDebug(5008) << "DCC SEND was unable to parse: " << str << endl;
      }
  }
  else if(str.find("Sent DCC SEND", 0) != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) file: (.+) size: (.*)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
          QString size = rx.cap(3);
          int fileSize = size.toInt(); // Bytes per step
          fileSize /= 1024;

          // setup dcc dialog
          displayMgr->show(mgr);
	  dccItem *it = mgr->mgr()->newSendItem(filename, who, dccItem::dccSentOffer, fileSize);
	  QString key = QString("%1/%2").arg(filename).arg(who);
	  if(DCCSendItems[key]){
	      /*
	       * don't add duplicates, this cuases real headaches
	       * for both us and the user.
	       * We can get these on repeated sends or old files,
	       * just renamed them, their finished anyways
               */
              QString renamed;
	      dccItem *old = DCCSendItems.take(key);
	      int i;
	      for(i = 0; i <= (int) DCCSendItems.count()+1; i++){
		  renamed = QString("%1 (sent %2)/%3").arg(filename).arg(i+1).arg(old->who());
		  if( DCCSendItems[renamed] == 0x0)
                      break;
	      }
	      old->changeFilename(QString("%1 (sent %2)").arg(filename).arg(i+1));
              DCCSendItems.insert(renamed, it);
	  }
          DCCSendItems.insert(key, it);
      }
      else {
          kdDebug(5008) << "DCC SENT was unable to parse: " << str << endl;
      }
  }
  else if(str.find("DCC CHAT OFFERED", 0) != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) ip: (.+) port: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString ip = rx.cap(2);
          QString port = rx.cap(3);
          // setup dcc dialog
          displayMgr->show(mgr);
	  dccItem *it = mgr->mgr()->newChatItem(who, dccItem::dccGotOffer);
	  connect(it, SIGNAL(itemRenamed(dccItem *, QString, QString)),
		  this, SLOT(dccRenameDone(dccItem *, QString, QString)));
          it->setWhoPostfix("(" + ip + ")");
          DCCChatItems.insert(who, it);
      }
      else {
          kdDebug(5008) << "DCC CHAT SEND was unable to parse: " << str << endl;
      }
  }
  else if(str.find("DCC CHAT SEND", 0) != -1){ /*FOLD01*/
      QRegExp rx("who: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);

          // setup dcc dialog
          displayMgr->show(mgr);
	  dccItem *it = mgr->mgr()->newChatItem(who, dccItem::dccSentOffer);
	  connect(it, SIGNAL(itemRenamed(dccItem *, QString, QString)),
		  this, SLOT(dccRenameDone(dccItem *, QString, QString)));
          DCCChatItems.insert(who, it);
      }
      else {
          kdDebug(5008) << "DCC CHAT SEND was unable to parse: " << str << endl;
      }
  }
  else if(str.find("DCC SEND terminated") != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) file: (.+) reason: (.*)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
          QString error = rx.cap(3);
          enum dccItem::dccStatus status = dccItem::dccDone;

	  if(error == "CLOSE"){
              status = dccItem::dccCancel;
	  }
          if(error != "OK"){
              status = dccItem::dccError;
              KPassivePopup::message(i18n("DCC SEND with %1 for %2 failed because of %3").arg(who).arg(filename).arg(error), mgr->mgr());
	  }
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCSendItems[key]){
              kdDebug(5008) << "SendPercent: " << DCCSendItems[key]->getPercent() << endl;
              if((status == dccItem::dccDone) && (DCCSendItems[key]->getPercent() < 100))
                  status = dccItem::dccError;
              DCCSendItems[key]->changeStatus(status);
          }
      }
  }
  else if(str.find ("DCC GET terminated") != -1){ /*fold01*/
      kdDebug(5008) << "Term: " << str << endl;
      QRegExp rx("who: (.+) file: (.+) reason: (.*)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
          QString error = rx.cap(3);
          enum dccItem::dccStatus status = dccItem::dccDone;

          if(error != "OK"){
              status = dccItem::dccError;
              KPassivePopup::message(i18n("DCC GET with %1 for %2 failed because of %3").arg(who).arg(filename).arg(error), mgr->mgr());
	  }
                    QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCGetItems[key]){
              kdDebug(5008) << "GetPercent: " << DCCGetItems[key]->getPercent() << endl;
              if((status == dccItem::dccDone) && (DCCGetItems[key]->getPercent() < 100))
                  status = dccItem::dccError;
              DCCGetItems[key]->changeStatus(status);
          }
      }
      else {
          kdDebug(5008) << "DCC Get term failed to parse: " << str << endl;
      }
  }
  else if(str.find("DCC GET resumed") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
	  QString filename = rx.cap(2);
	  QString key = QString("%1/%2").arg(filename).arg(who);
	  if(DCCGetItems[key]){
	      dccItem *it = DCCGetItems[key];
	      kdDebug(5008) << "Got DCC GET resumed message..." << it->file() << endl;
	      if(it->status() == dccItem::dccWaitOnResume)
                  dccConnectClicked(it);
              it->changeStatus(dccItem::dccResumed);
          }
      }
  }
  else if(str.find("DCC SEND resumed") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
	  QString filename = rx.cap(2);
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCSendItems[key]){
              DCCSendItems[key]->changeStatus(dccItem::dccResumed);
          }
      }
  }
  else if(str.find("DCC GET established") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
	  QString filename = rx.cap(2);
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCGetItems[key]){
              DCCGetItems[key]->changeStatus(dccItem::dccRecving);
          }
      }
  }
  else if(str.find("DCC SEND established") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+) ip: (\\S+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
	  QString ip = rx.cap(3);
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCSendItems[key]){
              DCCSendItems[key]->setWhoPostfix("(" + ip + ")");
              DCCSendItems[key]->changeStatus(dccItem::dccSending);
          }
      }
  }
  else if(str.find("DCC CHAT established") != -1){ /*FOLD01*/
      QRegExp rx("who: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          if(DCCChatItems[who]){
	      DCCChatItems[who]->changeStatus(dccItem::dccOpen);
              proc->new_toplevel(KSircChannel(proc->serverName(), QString("=") + who.lower()));
          }
      }
  }
  else if(str.find("DCC CHAT inbound established") != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) ip: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString ip = rx.cap(2);
          if(DCCChatItems[who]){
              DCCChatItems[who]->setWhoPostfix("(" + ip + ")");
	      DCCChatItems[who]->changeStatus(dccItem::dccOpen);
	      proc->new_toplevel(KSircChannel(proc->serverName(), QString("=") + who.lower()));
          }
      }
  }
  else if(str.find("DCC GET failed") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+) reason: (.*)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);
	  QString error = rx.cap(3);
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCGetItems[key]){
              DCCGetItems[key]->changeStatus(dccItem::dccError);
          }
          KPassivePopup::message(i18n("DCC Get with %1 for %2 failed because of %3").arg(who).arg(filename).arg(error), mgr->mgr());
      }
  }
  else if(str.find("DCC CHAT failed") != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) reason: (.*)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString error = rx.cap(2);
          if(DCCChatItems[who]){
              DCCChatItems[who]->changeStatus(dccItem::dccError);
          }
          KPassivePopup::message(i18n("DCC Chat with %1 failed because of %2").arg(who).arg(error), mgr->mgr());
      }
  }
  else if(str.find("DCC CHAT renamed") != -1){ /*FOLD01*/
      QRegExp rx("who: (.+) to: (.+)");
      if(rx.search(str)){
	  QString oldwho = rx.cap(1);
	  QString newwho = rx.cap(2);

          QString oldwin = "=" + oldwho;
	  if(proc->getWindowList().find(oldwin)){
	      proc->getWindowList().find(oldwin)->control_message(CHANGE_CHANNEL,QString("=" +newwho).lower());
	  }
      }
  }
  else if(str.find("Closing DCC GET") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
	  QString filename = rx.cap(2);
	  QString key = QString("%1/%2").arg(filename).arg(who);
          if(DCCGetItems[key]){
              DCCGetItems[key]->changeStatus(dccItem::dccCancel);
          }
      }
  }
//  else if(str.find("Closing DCC SEND") != -1){ /*FOLD01*/
//      QRegExp rx("who: (.+) file: (.+)");
//      if(rx.search(str)){
//          QString who = rx.cap(1);
//	  QString filename = rx.cap(2);
//	  QString key = QString("%1/%2").arg(filename).arg(who);
//          if(DCCSendItems[key]){
//              DCCSendItems[key]->changeStatus(dccItem::dccCancel);
//          }
//      }
//  }
  else if(str.find("Closing DCC CHAT") != -1){ /*fold01*/
      QRegExp rx("who: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          if(DCCChatItems[who]){
              DCCChatItems[who]->changeStatus(dccItem::dccDone);
          }
      }
  }
  else if(str.find("No DCC SEND") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
	  QString filename = rx.cap(2);

	  //QString key = QString("%1/%2").arg(filename).arg(who);

          QPtrList<dccItem> toDel;
          QDict<dccItem> new_list;
          QDictIterator<dccItem> it( DCCSendItems );
          for(;it.current(); ++it){
              if((it.current()->who() == who) &&
                 (it.current()->file() == filename)
                ){
                  toDel.append(it.current());
              }
              else {
                  new_list.insert(it.currentKey(), it.current());
              }
          }
          DCCSendItems = new_list;
          toDel.setAutoDelete(true);
          toDel.clear();
      }
  }
  else if(str.find("No DCC GET") != -1){ /*fold01*/
      QRegExp rx("who: (.+) file: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);
          QString filename = rx.cap(2);

          QPtrList<dccItem> toDel;
          QDict<dccItem> new_list;
          QDictIterator<dccItem> it( DCCGetItems );
          for(;it.current(); ++it){
              if((it.current()->who() == who) &&
                 (it.current()->file() == filename)
                ){
                  toDel.append(it.current());
              }
              else {
                  new_list.insert(it.currentKey(), it.current());
              }
          }
          DCCGetItems = new_list;
          toDel.setAutoDelete(true);
          toDel.clear();
      }
  }
  else if(str.find("No DCC CHAT") != -1){ /*fold01*/
      QRegExp rx("who: (.+)");
      if(rx.search(str)){
          QString who = rx.cap(1);

          QPtrList<dccItem> toDel;
          QDict<dccItem> new_list;
          QDictIterator<dccItem> it( DCCChatItems );
          for(;it.current(); ++it){
              if(it.current()->who() == who){
                  toDel.append(it.current());
              }
              else {
                  new_list.insert(it.currentKey(), it.current());
              }
          }
          DCCChatItems = new_list;
          toDel.setAutoDelete(true);
          toDel.clear();
      }
  }
  else{ /*FOLD01*/
    proc->getWindowList()["!default"]->sirc_receive(str);
  }

}

void KSircIODCC::control_message(int, QString)
{
}

void KSircIODCC::cancelTransfer(QString filename)
{
    //if(DlgList[filename]){
    //emit outputLine(DCCStatus[filename]->cancelMessage.ascii());
    //delete DlgList[filename];
    //DlgList.remove(filename);
    //delete DCCStatus[filename];
    //DCCStatus.remove(filename);
  //}
}

void KSircIODCC::getFile()
{
    /*
    QString text = pending->fileListing()->text(pending->fileListing()->currentItem());
  int pos = text.find(" ", 0);
  QString nick = text.mid(0, pos);
  pos = text.find(" ", pos+1) + 1;
  QString filename = text.mid(pos, text.length() - pos);
  //if(DlgList[filename]->isVisible() == FALSE)
  //  DlgList[filename]->show();
  QString command = "/dcc get " + nick + " " + filename + "\n";
  emit outputLine(command.ascii());
  for(uint i = 0; i < pending->fileListing()->count(); i++)
    if(QString(pending->fileListing()->text(i)) == (nick + " offered " + filename))
      pending->fileListing()->removeItem(i);

  if(pending->fileListing()->count() == 0)
  pending->hide();
  */
}

void KSircIODCC::forgetFile()
{
    /*
  QString text = pending->fileListing()->text(pending->fileListing()->currentItem());
  int pos = text.find(" ", 0);
  QString nick = text.mid(0, pos);
  pos = text.find(" ", pos+1) + 1;
  QString filename = text.mid(pos, text.length() - pos);
  QString command = "/dcc close get " + nick + " " + filename + "\n";
  emit outputLine(command.ascii());
  for(uint i = 0; i < pending->fileListing()->count(); i++)
    if(QString(pending->fileListing()->text(i)) == (nick + " offered " + filename))
      pending->fileListing()->removeItem(i);

  if(pending->fileListing()->count() == 0)
  pending->hide();
  */

}

filterRuleList *KSircIODCC::defaultRules()
{

//  filterRule *fr;
  filterRuleList *frl = new filterRuleList();
  frl->setAutoDelete(TRUE);
  /*
  fr = new filterRule();
  fr->desc = "Capture DCC IO Messages";
  fr->search = "^\\*D\\*";
  fr->from = "^";
  fr->to = "~!dcc~";
  frl->append(fr);
  */
  return frl;

}

void KSircIODCC::dccConnectClicked(dccItem *it)
{
    QString str;
    kdDebug(5008) << "Got connect click on " << it->who() << " " << it->type() << endl;
    switch(it->type()){
    case dccItem::dccGet:
        str = "/dcc get " + it->who() + " " + it->file() + "\n";
        emit outputLine(str.ascii());
        break;
    case dccItem::dccChat:
        str = "/dcc chat " + it->who() + "\n";
        emit outputLine(str.ascii());
        break;
    default:
        break;
    }

}
void KSircIODCC::dccResumeClicked(dccItem *it)
{
    QString str;
    kdDebug(5008) << "Got resume click on " << it->who() << " " << it->type() << endl;
    switch(it->type()){
    case dccItem::dccGet:
	it->changeStatus(dccItem::dccWaitOnResume);
        str = "/resume " + it->who() + " " + it->file() + "\n";
        emit outputLine(str.ascii());
        break;
    default:
        break;
    }

}
void KSircIODCC::dccRenameClicked(dccItem *it)
{
    it->doRename();
}
void KSircIODCC::dccAbortClicked(dccItem *it)
{
    QString str;
    switch(it->status()){
    case dccItem::dccDone:
    case dccItem::dccCancel:
    case dccItem::dccError:
        switch(it->type()) {
	case dccItem::dccGet:
            DCCGetItems.remove(QString("%1/%2").arg(it->file()).arg(it->who()));
            break;
        case dccItem::dccSend:
            DCCSendItems.remove(QString("%1/%2").arg(it->file()).arg(it->who()));
            break;
        case dccItem::dccChat:
            DCCChatItems.remove(it->who());
            break;
        }
        delete it;
        break;
    default:
        switch(it->type()) {
        case dccItem::dccGet:
            str = "/dcc close get " + it->who() + " " + it->file() + "\n";
            emit outputLine(str.ascii());
            break;
        case dccItem::dccSend:
            str = "/dcc close send " + it->who() + " " + it->file() + "\n";
            emit outputLine(str.ascii());
            break;
        case dccItem::dccChat:
            str = "/dcc close chat " + it->who() + "\n";
            emit outputLine(str.ascii());
            break;
        }
    }
}

void KSircIODCC::dccRenameDone(dccItem *it, QString oldWho, QString oldFile)
{
    if(it->type() == dccItem::dccGet){
	QString str = QString("/dcc rename %1 %2 %3\n").arg(oldWho).arg(oldFile).arg(it->file());
	QString key = QString("%1/%2").arg(oldFile).arg(oldWho);
	if(DCCGetItems[key]){
            DCCGetItems.take(key);
	    QString newkey = QString("%1/%2").arg(it->file()).arg(it->who());
            DCCGetItems.insert(newkey, it);
	}
	emit outputLine(str.ascii());
    }
    else if(it->type() == dccItem::dccChat){
	if(DCCChatItems[oldWho]){
	    DCCChatItems.take(oldWho);
	    DCCChatItems.insert(it->who(), it);
	}
	QString str = QString("/dcc rchat %1 %2\n").arg(oldWho).arg(it->who());
	emit outputLine(str.ascii());

    }
}

void KSircIODCC::showMgr()
{
    displayMgr->show(mgr);
}

#include "ioDCC.moc"
