#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "alistbox.h"
#include "chanparser.h"
#include "ksopts.h"
#include "control_message.h"
#include "ssfeprompt.h"
#include "toplevel.h"
#include "ksircprocess.h"
#include "ksview.h"

#include <stdio.h>

#include <qregexp.h>
#include <qapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qptrlist.h>

// Static parser table is "initialized"
QDict<parseFunc> ChannelParser::parserTable;


ChannelParser::ChannelParser(KSircTopLevel *_top)
{
  top = _top;

  /*
   * Initial helper variables
   */
  prompt_active = false;
  current_item = -1;
  top_item = 0;

  if(parserTable.isEmpty() == TRUE){
    parserTable.setAutoDelete(TRUE);
    parserTable.insert("`l`", new parseFunc(&ChannelParser::parseSSFEClear));
    parserTable.insert("`s`", new parseFunc(&ChannelParser::parseSSFEStatus));
    parserTable.insert("`i`", new parseFunc(&ChannelParser::parseSSFEInit));
    parserTable.insert("`t`", new parseFunc(&ChannelParser::parseSSFEMsg));
    parserTable.insert("`o`", new parseFunc(&ChannelParser::parseSSFEOut));
    parserTable.insert("`p`", new parseFunc(&ChannelParser::parseSSFEPrompt));
    parserTable.insert("`P`", new parseFunc(&ChannelParser::parseSSFEPrompt));
    parserTable.insert("`R`", new parseFunc(&ChannelParser::parseSSFEReconnect));
    // The rest are *** info message
    parserTable.insert("***", new parseFunc(&ChannelParser::parseINFOInfo));
    parserTable.insert("*E*", new parseFunc(&ChannelParser::parseINFOError));
    parserTable.insert("*!*", new parseFunc(&ChannelParser::parseINFONicks)); // Normal
    parserTable.insert("*C*", new parseFunc(&ChannelParser::parseINFONicks)); // 1st line
    parserTable.insert("*c*", new parseFunc(&ChannelParser::parseINFONicks)); // Last line
    parserTable.insert("*#*", new parseFunc(&ChannelParser::parseINFONicks)); // Non enhanced
    parserTable.insert("*$*", new parseFunc(&ChannelParser::parseINFONicks)); // Enhanced turned off
    parserTable.insert("*>*", new parseFunc(&ChannelParser::parseINFOJoin));
    parserTable.insert("*<*", new parseFunc(&ChannelParser::parseINFOPart));
    parserTable.insert("*N*", new parseFunc(&ChannelParser::parseINFOChangeNick));
    parserTable.insert("*+*", new parseFunc(&ChannelParser::parseINFOMode));
    parserTable.insert("*T*", new parseFunc(&ChannelParser::parseINFOTopic));
    // End of info message
    parserTable.insert("*  ", new parseFunc(&ChannelParser::parseCTCPAction));
  }

}

parseResult *ChannelParser::parse(QString string)
{
   // for older Qts

  parseFunc *pf;
  if(string.length() < 3){
    return new parseError(string, QString("Dumb string, too short"));
  }

  /**
   * Start pre-parsing the strings to make them fit the 3 character
   * parser codes, etc
   */

  /*
   * SSFE control messages are too long, so we convert the
   * messges into a 3 character code, `#ssfe#\S becomes `\S`
   */

  if ((string[0] == '`') & (string.length() > 7))
  {
      QString prefix = "`"+string[7]+"`";
      string = prefix + QString(" ") + string.mid(8).stripWhiteSpace();
  }
  else if((string[0] == '*') && (string[1] == ' ')) {
    string.insert(1, ' ');
  }
  // Pre-parsing is now complete

  pf = parserTable[string.mid(0, 3)];
  if(pf != 0x0){
    parseResult *result = (this->*(pf->parser))(string);

    if (result)
       return result;
  }
  else
  {
    // debug("No handler for: %s", string.data());
  }

  // Little bit of past parsing to catch one we've missed
  if((string[0] == '*') && (string[2] == '*')) {
    string.remove(0, 3);
    return new parseSucc(string, ksopts->infoColor, "user|servinfo");
  }
  // If it's unkown we just fall out of the function
  return 0;
}

parseResult * ChannelParser::parseSSFEClear(QString string)
{
//  top->mainw->clear();
    top->clearWindow();
//  top->mainw->repaint(TRUE);
  string.truncate(0);
  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseSSFEStatus(QString string)
{
  string.remove(0, 12); // strip off the first 12 characters "<junk> [sirc] "
  if(string.length() == 0)
    return new parseError("", i18n("Unable to parse status string"));

  //kdDebug(5008) << "String: " << string << endl;
  QRegExp rx("(\\S+).*\\(*([+-]*[+-\\w\\d]*)\\)*.*on (\\S+) \\((\\S+)\\)");
  if(rx.search(string) == -1){
    return new parseError("", i18n("Unable to parse status (no known format) string"));
  }

  QString nick = rx.cap(1);
  QString modes = rx.cap(2);
  QString chan = rx.cap(3);
  QString chanmode = rx.cap(4);

  /*
   * fix up modes which may have gotten the (away) part
   */
  if(modes.contains("away")){
    modes = "";
  }

  bool away = false;
  if(string.contains("(away)"))
    away = true;

  if(away){
    chan.prepend(i18n("Away-"));
  }

  nickListItem *nickItem = top->nicks->item( top->nicks->findNick( nick ) );
  if ( nickItem ) {
    if(nickItem->away() != away){
      nickItem->setAway( away );
      top->nicks->viewport()->repaint( top->nicks->itemRect( nickItem ), false );
    }
    nickItem->forceColour(&ksopts->ownNickColor);
  }

  top->ksircProcess()->setNick(nick);
  if (chanmode.findRev("t") != -1)
    top->channelButtons->setProtectMode(true);
  else top->channelButtons->setProtectMode(false);
  if (chanmode.findRev("m") != -1)
    top->channelButtons->setModerateMode(true);
  else top->channelButtons->setModerateMode(false);
  if (chanmode.findRev("n") != -1)
    top->channelButtons->setNooutsideMode(true);
  else top->channelButtons->setNooutsideMode(false);
  if (chanmode.findRev("i") != -1)
    top->channelButtons->setMenuItemMode(0, true);
  else top->channelButtons->setMenuItemMode(0, false);
  if (chanmode.findRev("s") != -1)
    top->channelButtons->setMenuItemMode(3, true);
  else top->channelButtons->setMenuItemMode(3, false);

  if (modes.findRev("i") != -1)
    top->channelButtons->setMenuItemMode(4, true);
  else top->channelButtons->setMenuItemMode(4, false);
  if (modes.findRev("w") != -1)
    top->channelButtons->setMenuItemMode(5, true);
  else top->channelButtons->setMenuItemMode(5, false);
  if (modes.findRev("s") != -1)
    top->channelButtons->setMenuItemMode(6, true);
  else top->channelButtons->setMenuItemMode(6, false);

  QString status_line = QString("%1 (%2) %3 (%4) ").arg(chan).arg(chanmode).arg(nick).arg(modes);

  /*
   * Go srearching for key and limit messages
   */
  QRegExp rxKey("<key: (\\S+)>");
  if(rxKey.search(string) >= 0){
    top->channelButtons->setMenuItemMode(2, true);
    status_line += QString("<key: %1>").arg(rxKey.cap(1));
  }
  else {
    top->channelButtons->setMenuItemMode(2, false);
  }

  QRegExp rxLimit("<limit: (\\S+)>");
  if(rxLimit.search(string) >= 0){
    top->channelButtons->setMenuItemMode(1, true);
    status_line += QString("<limit: %1>").arg(rxLimit.cap(1));
  }
  else {
    top->channelButtons->setMenuItemMode(1, false);
  }

  if(ksopts->displayTopic){
    if(top->topic().length() > 0)
      status_line += "T: " + top->topic();
    else
      status_line += "T: " + i18n("<No Topic Set>");
  }

  if(top->caption != status_line){
    if(nick[0] == '@' || (nick[0] == '*' && nick[1] == '@')) {
      // If we're an op,,
      // update the nicks popup menu
      top->channelButtons->setButtonsEnabled(true);  // set the buttons enabled if were an op
      top->opami = TRUE;
    }                  // opami = true sets us to an op
    else {
      top->channelButtons->setButtonsEnabled(false);  // set the buttons enabled if were an op
      top->opami = FALSE;
    }                 // FALSE, were not an ops
    top->UserUpdateMenu();                // update the menu
    top->setCaption(status_line);
    top->setIconText(status_line);
    if(top->ticker) {
      top->ticker->setCaption(status_line);
    }
    top->caption = status_line;           // Make copy so we're not
    // constantly changing the title bar
  }
  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseSSFEInit(QString)
{
  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseSSFEOut(QString)
{
  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseSSFEMsg(QString string)
{

  if(string.length() > 100)
    return new parseError(QString::null, i18n("String length for nick is greater than 100 characters. This is unacceptably long."));

  int l = string.length();
  if (l <= 0)
    return new parseError(string, i18n("String not long enough"));

  return new parseSucc(QString::null); // Null string, don't display anything
}


parseResult * ChannelParser::parseSSFEPrompt(QString string)
{

  if(prompt_active == FALSE){
    QString prompt, caption;
    ssfePrompt *sp;

    // Flush the screen.
    // First remove the prompt message from the Buffer.
    // (it's garunteed to be the first one)
    top->LineBuffer.remove( *top->LineBuffer.begin() );
    top->Buffer = FALSE;
    top->sirc_receive(QString(""));

    // "'[pP]' " gives 4 spaces
    if(string.length() < 5)
      caption = i18n("");
    else
      caption = string.mid(3);
    prompt_active = TRUE;

    // If we use this, then it blows up
    // if we haven't popped up on the remote display yet.

    KSirc::TextParagIterator it = top->mainw->firstParag();
    QString last;
    while(it.atEnd() == FALSE) {
        last = it.plainText();
        ++it;
    }

    if(last[0] == '['){ /* strip time stamp */
        prompt = last.mid(last.find(' '));
    }
    else {
        prompt = last;
    }

    sp = new ssfePrompt(prompt, 0);
    sp->setCaption(caption);
    if(string[1] == 'P')
      sp->setPassword(TRUE);
    sp->exec();
    //	  cerr << "Entered: " << sp->text() << endl;
    prompt = sp->text();
    prompt += "\n";
    emit top->outputUnicodeLine(prompt);
    delete sp;
    prompt_active = FALSE;
  }

  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseSSFEReconnect(QString)
{

  if(top->channelInfo().channel()[0] == '#' ||
     top->channelInfo().channel()[0] == '&'){
    QString str = "/join " + QString(top->channelInfo().channel()) + "\n";
    emit top->outputUnicodeLine(str);
  }

  return new parseSucc(QString::null); // Null string, don't display anything
}

parseResult * ChannelParser::parseINFOInfo(QString string)
{
  string.remove(0, 3);                // takes off the junk

  return new parseSucc(string, ksopts->infoColor, "user|servinfo"); // Null string, don't display anything
}

parseResult * ChannelParser::parseINFOError(QString string)
{
  string.remove(0, 3);               // strip the junk

  return new parseSucc(string,ksopts->errorColor, "user|error"); // Null string, don't display anything
}

parseResult * ChannelParser::parseINFONicks(QString in_string)
{
  QString string = in_string;
  QString channel_name;
  bool clear_box = FALSE;

  // Check to see if it's a continued line
  if(string[1] == 'C'){
    string[1] = '!';
    clear_box = TRUE;
  }
  if(string[1] == '#'){
    string[1] = '!';
    clear_box = FALSE;
  }
  else if(string[1] == 'c'){
    if(current_item > 0)
      top->nicks->setCurrentItem(current_item);
    top->nicks->setTopItem(top_item);
    top->nicks->repaint(TRUE);
    return new parseSucc(QString::null);           // Parsing ok, don't print anything though
  }
  else if(string[1] == '$'){
    top->nicks->clearAdvOps();
    //kdDebug(5008) << "Turning off advanced ops" << endl;
    return new parseSucc(QString::null);           // Parsing ok, don't print anything though
  }


  // Get the channel name portion of the string
  // Search for the first space, since : can be embeded into channel names.
  //count = sscanf(string, "*!* Users on %100[^ ]", channelInfo().channel());
  // *!* Users on #TEST: boren asj asj_
  QRegExp rx("\\*\\S\\* Users on (\\S+): (.+)");
  if(rx.search(string) == -1){
    return new parseError(string, i18n("Could not find channel name"));
  }
  channel_name = rx.cap(1);


  if (channel_name.lower() != top->channelInfo().channel().lower()){
    string.remove(0,3);
    return new parseSucc(string,ksopts->infoColor,"user|misc4");
  }

  if(clear_box == TRUE){
    current_item = top->nicks->currentItem();
    top_item = top->nicks->topItem();
    top->nicks->clear();
  }

  //int start = string.find(": ", 0, FALSE); // Find start of nicks
  //if (start < 0)
  //  return new parseError(string, i18n("Could not find start of nicks"));
  //
  //place_holder = new char[string.length()];
  //strcpy(place_holder, string.ascii()+start+2);
  //nick = strtok(place_holder, " ");
  //  while(nick != 0x0){                     // While there's nick to go...
  QStringList nicks = QStringList::split(QRegExp("\\s+"), rx.cap(2));

  for ( QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it ) {
    QString nick = *it;
    nickListItem *irc = new nickListItem();

    bool done = FALSE;
    uint i;

    for(i = 0; i < nick.length();i++){
      switch(nick[0].unicode()){
      case '@':
	irc->setOp(TRUE);
        nick.remove(0,1);
	break;
      case '+':
	irc->setVoice(TRUE);
	nick.remove(0,1);
	break;
      case '#':
	irc->setAway(TRUE);
	nick.remove(0,1);
	break;
      case '*':
	irc->setIrcOp(TRUE);
	nick.remove(0,1);
	break;
      default:
	done = TRUE;
      }
      if(done == TRUE)
	break;
    }

    if(nick == top->ksircProcess()->getNick()){
        irc->forceColour(&ksopts->ownNickColor);
    }
    irc->setText(nick);
    top->nicks->inSort(irc);
  }

  return new parseSucc(QString::null);           // Parsing ok, don't print anything though
}

parseResult * ChannelParser::parseINFOJoin(QString string)
{
  string.remove(0, 4);                   // strip *>* to save a few compares
  // You have joined channel #Linux
  QRegExp rx("You have joined channel (\\S+)");
  if(rx.search(string) != -1){
      //QString channel = rx.cap(1).lower();
      QString channel = rx.cap(1);
    //kdDebug(5008) << "Channel: " << channel << endl;

    if(top->channelInfo().channel() != channel) {
        KSircChannel ci(top->channelInfo().server(), channel);
	kdDebug(5008) << "Warning: we got a channel join yet me don't belong to it!!!  Assuming no key!" << endl;
	kdDebug(5008) << "String was: " << string << endl;
	kdDebug(5008) << "We think the channel is: " << channel << " we are: " << top->channelInfo().channel()<< endl;
	emit top->open_toplevel(ci);
    }
    return new parseJoinPart(" " + string, ksopts->channelColor, "user|join");
  }

  // reef-diddy (nenernener@xnet-3B34A9E2.snet.net) has joined channel #aquaria
  rx.setPattern("(\\S+) .+ has joined channel (\\S+)");
  if(rx.search(string) != -1){
    QString nick = rx.cap(1);
    QString channel = rx.cap(2).lower();
    //kdDebug(5008) << "Channel: " << channel << " nick: " << nick << endl;
    if(top->channelInfo().channel().lower() != channel){
      return new parseWrongChannel(" " + string, ksopts->errorColor, "user|join");
    }
    //	nicks->insertItem(s3, 0);      // add the sucker
    top->nicks->inSort(nick);
    top->addCompleteNick(nick);
    highlightNick(string, nick);
    return new parseJoinPart(" " + string, ksopts->channelColor, "user|join");
  }

  return 0; // ??
}

parseResult * ChannelParser::parseINFOPart(QString string)
{

  bool foundNick = false;
  QString pixname = "user|kick";
  QString nick("");

  string.remove(0, 4);                // clear junk

  // Multiple type of parts, a signoff or a /part
  // Each get's get nick in a diffrent localtion
  // Set we search and find the nick and the remove it from the nick list
  // 1. /quit, signoff, nick after "^Singoff: "
  // 2. /part, leave the channek, nick after "has left \w+$"
  // 3. /kick, kicked off the channel, nick after "kicked off \w+$"
  //
  // Signoff: looser
  QRegExp rx("Signoff: (\\S+)");
  if(rx.search(string) != -1) {
    nick = rx.cap(1);
    foundNick = true;
    pixname = "user|X";
    highlightNick(string, nick);
  }
  /*
   * Check for "You" before everyone else or else the next
   * case will match it
   * You have left channel <channel>
   */
  rx.setPattern("You have left channel (\\S+)");
  if((foundNick == false) && (rx.search(string) != -1)) {
    QString channel = rx.cap(1);
    if(top->channelInfo().channel().lower() == channel.lower()) {
      QApplication::postEvent(top, new QCloseEvent());
      // WE'RE DEAD
      return new parseSucc(QString::null);
    }
    pixname = "user|part";
  }
  /*
   * Same as above, check your own state first
   * You have been kicked off channel <channel>
   */
  rx.setPattern("You have been kicked off channel (\\S+)");
  if((foundNick == false) && (rx.search(string) != -1)) {
    QString channel = rx.cap(1);
    if(top->channelInfo().channel().lower() != channel.lower())
      return new parseWrongChannel(string, ksopts->errorColor, "user|kick");
    if (ksopts->autoRejoin == TRUE)
      {
       QString str = QString("/join %1\n").arg(top->channelInfo().channel());
       emit top->outputUnicodeLine(str);
       /* if(top->ticker)
	top->ticker->show();
	else*/
         top->show();
    }
    else
    {
      if(top->KickWinOpen != false)
        return new parseError(" " + string, i18n("Kick window open"));
      top->KickWinOpen = true;
      int result = KMessageBox::questionYesNo(top, string, i18n("You Have Been Kicked"), i18n("Rejoin"), i18n("Leave"));
      if (result == KMessageBox::Yes)
      {
        QString str = QString("/join %1\n").arg(top->channelInfo().channel());
	emit top->outputUnicodeLine(str);
	/* if(top->ticker)
	 * top->ticker->show();
	 * else*/
	top->show();
	return new parseJoinPart(" " + string, ksopts->channelColor, "user|kick");
      }
      else
      {
        // WE'RE DEAD
	QApplication::postEvent(top, new QCloseEvent());
        top->KickWinOpen = false;
      }
    }
    pixname = "user|kick";
  }
  /*
   * <nick> has left channel <channel>
   */
  rx.setPattern("(\\S+) has left channel (\\S+)");
  if((foundNick == false) && (rx.search(string) != -1)) {
    nick = rx.cap(1);
    QString channel = rx.cap(2);
//    kdDebug(5008) << "Nick: " << nick << " Channel: " << channel << " top: " << top->channelInfo().channel() << endl;
    if(top->channelInfo().channel().lower() == channel.lower()) {
      foundNick = true;
    }
    else{
      return new parseWrongChannel(QString::null);
    }
    pixname = "user|part";
    highlightNick(string, nick);
  }
  /*
   * "<nick> has been kicked off channel  <channel>"
   */
  rx.setPattern("(\\S+) has been kicked off channel (\\S+)");
  if((foundNick == false) && (rx.search(string) != -1)) {
    nick = rx.cap(1);
    QString channel = rx.cap(2);
    if(top->channelInfo().channel().lower() == channel.lower()) {
      foundNick = true;
    } else {
      return new parseWrongChannel(QString::null);
    }
    highlightNick(string, nick);
    pixname = "user|kick";
  }

  if (foundNick) {

    top->removeCompleteNick(nick);

    int index = top->nicks->findNick(nick);
    if(index >= 0){
      top->nicks->removeItem(index);
      return new parseJoinPart(" " + string, ksopts->channelColor, pixname);
    }
    else {
      return new parseJoinPart(QString::null);
    }
  }
  else {
    return new parseError(" " + string, i18n("Failed to parse part/kick/leave/quit message"));
  }

  return 0;
}

parseResult * ChannelParser::parseINFOChangeNick(QString string)
{
  //char old_nick[101], new_nick[101];
  QString old_nick, new_nick;

  string.remove(0, 4); // Remove the leading *N* and space

  /*
   * *N* asj_ is now known as bleh
   */
  //kdDebug(5008) << "Nick change: " << string << endl;
  QRegExp rx("(\\S+) is now known as (\\S+)");
  if(rx.search(string) == -1){
    if(string.contains("already taken")){
      return new parseSucc(" " + string, ksopts->errorColor, "user|error");
    }

    return new parseError(i18n("Unable to parse: %1").arg(string), i18n("Unable to parse change nick code"));
  }

  old_nick = rx.cap(1);
  new_nick = rx.cap(2);

  // If we have a window open talking to the nick
  // Change the nick to the new one.
  if((top->channelInfo().channel()[0] != '#' || top->channelInfo().channel()[0] != '&') &&
     (top->channelInfo().channel() == old_nick)){
      if(!top->ksircProcess()->mrList()[new_nick.lower()]){
	  top->control_message(CHANGE_CHANNEL, new_nick.lower());
      }
  }

  highlightNick(string, old_nick);
  highlightNick(string, new_nick);

  // search the list for the nick and remove it
  // since the list is source we should do a binary search...
  int found = top->nicks->findNick(old_nick);
  if(found >= 0){ // If the nick's in the nick list, change it and display the change
    // save current selection
    int selection = top->nicks->currentItem();

    // Get the old item, and create a new one
    nickListItem *it = top->nicks->item(found);
    nickListItem *irc  = new nickListItem(*it);
    irc->setText(new_nick);

    top->nicks->removeItem(found);        // remove old nick
    top->nicks->inSort(irc);

    top->changeCompleteNick(old_nick, new_nick);

    top->nicks->setCurrentItem(selection);
    top->nicks->repaint(TRUE);
    // We're done, so let's finish up
    return new parseSucc(" " + string, ksopts->channelColor, "user|join");
  }
  else {
    if(top->channelInfo().channel() == new_nick ||
       top->channelInfo().channel() == old_nick)
        return new parseSucc(" " + string, ksopts->channelColor, "user|elipsis");
    else
	return new parseSucc(QString::null);

  }

  //	  warning("Toplevel-N: nick change search failed on %s", s3.data());
  // return new parseSucc(QString::null);
}

class mode_info {
public:
  mode_info(bool op, QChar mode, QString arg);

  bool op() const;
  QChar mode() const;
  QString arg() const;

private:
  const bool m_op;
  const QChar m_mode;
  const QString m_arg;
};

mode_info::mode_info(bool op, QChar mode, QString arg) :
    m_op(op),
    m_mode(mode),
    m_arg(arg)
{
}

bool mode_info::op() const {
    return m_op;
}

QChar mode_info::mode() const {
    return m_mode;
}

QString mode_info::arg() const {
    return m_arg;
}


parseResult * ChannelParser::parseINFOMode(QString string)
{
  // Basic idea here is simple, go through the mode change and
  // assign each mode a + or a - and an argument or "" if there is
  // none.  After that each mode change it looked at to see if
  // we should handle it in any special way.

  // Strip off leading sirc info

  string.remove(0, 4);


  /*
   * 1k is pretty safe since KProcess returns 1 k blocks, and lines don't get split between reads. This is emprical
   */
  QString modes, args, channel;
  int found = 0;

  if(string.find("for user") >= 0)
    return new parseSucc(" " + string, ksopts->infoColor, "user|mode");

  /*
   * We need to 2 scanf's, one for the case of arguments, and one for no args.
   */
  QRegExp rx("Mode change \"(\\S+) *([^\"]*)\" on channel (\\S+)");
  if(rx.search(string) >= 0){
      modes = rx.cap(1);
      args = rx.cap(2);
      channel = rx.cap(3);
      found = 1;
  }

  rx.setPattern("Mode for channel (\\S+) is \"([^\" ]+)\"");
  if(found == 0 &&
     rx.search(string) >= 0){
      channel = rx.cap(1);
      modes = rx.cap(2);
      found = 1;
  }

  rx.setPattern("Your user mode is");
  if(found == 0 &&
     rx.search(string) >= 0){
      /*
       * Don't parse user mode requests
       */
      return new parseSucc(" " + string, ksopts->infoColor, "user|mode");
  }


  if(found == 0)
      return new parseError(" Failed to parse mode change: " + string, QString::null);

  /*
   * op specifie if it's a + or -.  tru is + false is -
   */
  bool op = true;
  /*
   * arglist is the list of argument
   * we use the itirator to tstep through the list
   * as need be
   */
  QStringList arglist = QStringList::split(" ", args);
  QStringList::Iterator ai = arglist.begin();

  /*
   * the ptr list structure contains the parsed contents
   */
  QPtrList<const mode_info> pmList;
  pmList.setAutoDelete(true);

  for(uint pos = 0; pos < modes.length(); pos++){
    switch(modes.at(pos).unicode()){
    case '+':
      op = true;
      break;
    case '-':
      op = false;
      break;
    case 'l': // Chan limits
      /*
       * -l doesn't take any arguments, so just add the mode and break
       * +l otoh does, so read the argument
       */
      if(op == false){
	pmList.append(new mode_info(op, 'l', QString::null));
	break;
      }
    case 'o': // Op, arg is the nick
    case 'v': // Voice, arg is the nick
    case 'b': // Ban, arg is mask banned
    case 'k': // kcik, arg is nick
      if(ai == NULL)
	return new parseError(i18n("Unable to parse mode change: %1").arg(string), QString::null);
      pmList.append(new mode_info(op, modes.at(pos), *ai));
      ai++;
      break;
    case 'i': // Invite only
    case 'n': // No message to chan
    case 'p': // Private
    case 'm': // Moderated
    case 's': // Secret
    case 't': // Topic setable by ops
    case 'R': // (Dalnet) only registered may join
    case 'r': // (Dalnet) only registered may join or something
      /*
       * Mode changes which don't take args
       */
      pmList.append(new mode_info(op, modes.at(pos), QString::null));
      break;
    default:
      kdDebug(5008) << "Unknown mode change: " << modes.mid(pos, 1) << " Assume no args" << endl;
      pmList.append(new mode_info(op, modes.at(pos), QString::null));
    }
  }
  // We have the modes set in mode and arg, now we go though
  // looking at each mode seeing if we should handle it.
  bool mode_o_plus = false;
  bool mode_o_minus = false;
  bool mode_b_plus = false;
  bool mode_b_minus = false;

  QPtrListIterator<const mode_info> it(pmList);
  const mode_info *mi;
  while ( (mi = it.current()) != 0 ) {
    ++it;
    /*
     * Look at the second character, it's uniq, check for +,- latter
     */
    if(mi->mode().unicode() == 'o'){
      mode_o_plus = mi->op();
      mode_o_minus = !mi->op();

      if(top->ksircProcess()->getNick() == mi->arg())
	  top->channelButtons->setButtonsEnabled(mi->op());

      if(mi->arg().length() == 0){
        qWarning("Invalid nick in +/- o mode change");
        continue;
      }

      int offset = top->nicks->findNick(mi->arg());
      if(offset >= 0){
        nickListItem *irc = new nickListItem();
        *irc = *top->nicks->item(offset);
        top->nicks->removeItem(offset);           // remove old nick
        irc->setOp(mi->op());
        // add new nick in sorted pass,with colour
        top->nicks->inSort(irc);
        top->nicks->repaint(TRUE);
      }
      else{
	kdDebug(5008) << "Toplevel+o: nick search failed on " << mi->arg() << endl;
      }
    }
    else if(mi->mode() == 't'){
      if(mi->op())
        top->channelButtons->setProtectMode(true); // set on
      else
        top->channelButtons->setProtectMode(false); // set off
    }
    else if(mi->mode() == 'm'){
      if(mi->op())
        top->channelButtons->setModerateMode(true); // set on
      else
        top->channelButtons->setModerateMode(false); // set off
    }
    else if(mi->mode() == 'n'){
      if(mi->op())
        top->channelButtons->setNooutsideMode(true); // set on
      else
        top->channelButtons->setNooutsideMode(false); // set off
    }
    else if(mi->mode() == 'v'){
      bool voice;
      if(mi->op())
        voice = TRUE;
      else
        voice = FALSE;

      if(mi->arg().length() == 0){
        qWarning("Invalid nick in +-v mode change");
        continue;
      }

      int offset = top->nicks->findNick(mi->arg());
      if(offset >= 0){
        nickListItem *irc = new nickListItem();
        *irc = *top->nicks->item(offset);
        top->nicks->removeItem(offset);           // remove old nick
        irc->setVoice(voice) ;
        // add new nick in sorted pass,with colour
        top->nicks->inSort(irc);
        top->nicks->repaint();
      }
    }
    else if(mi->mode() == 'b'){
      if(mi->op())
        mode_b_plus = true;
      else
        mode_b_minus = true;
    }
    else if(mi->mode() == 'k'){
      if(mi->op()){
	if(mi->arg().length() == 0){
	  qWarning("Invalid +k mode set, no argument!");
	  continue;
	}
	top->m_channelInfo.setKey(mi->arg());
      }
      else {
	top->m_channelInfo.setKey(""); /* no key set anymore */
      }
    }
    else{
      QChar c(mi->mode());
      QString m(&c, 1);
      QString o;
      if(mi->op())
	o = "+";
      else
        o = "-";
      kdDebug(5008) << "Did not handle: " << o << m << " arg: " << mi->arg() << endl;
    }
  }
  /*
   * We're all done, so output the message and be done with it
   */
  QString pixname = "user|mode";

  if(mode_o_plus)
    pixname = "user|oplus";
  else if(mode_o_minus)
    pixname = "user|ominus";
  else if(mode_b_plus)
    pixname ="user|bplus";
  else if(mode_b_minus)
    pixname = "user|bminus";

  return new parseSucc(" " + string, ksopts->infoColor, pixname);
}

parseResult * ChannelParser::parseCTCPAction(QString string)
{

  string.remove(0, 2);      // * <something> use fancy * pixmap. Remove 2, leave one for space after te *
                            // why? looks cool for dorks
  return new parseSucc(string, ksopts->textColor, "user|action");
}

parseResult * ChannelParser::parseINFOTopic(QString string)
{

  int found = 0;

  string.remove(0, 4); // Remove the leading *T* and space

  //kdDebug(5008) << "Topic parser: " << string << endl;

  // Topic for #boo: this is a nice test
  QRegExp rx( "Topic for (\\S+): (.*)" );
  if(rx.search( string ) != -1){
    QString channel = rx.cap(1);
    QString topic = rx.cap(2);
    topic.replace( QRegExp( "~~" ), "~" );

    /*
     * check where it's going.
     * topic's maybe for other channels since they have no channel routing
     * information, so route it to the right place if need be.
     * If we're not on the channnel just fall through and display it
     * on our channel, maybe the user asked for a topic of a different channel
     */

    if(channel.lower() != top->channelInfo().channel().lower()){
	if(top->ksircProcess()->mrList()[channel.lower()]){
	    KSircTopLevel *t = dynamic_cast<KSircTopLevel *>(top->ksircProcess()->mrList()[channel.lower()]);
	    if(t)
                t->setTopic(topic);
	}
    }
    else {
	//kdDebug(5008) << "New topic: " << topic << endl;
	top->setTopic( topic );
    }
    found = 1;
  }

  rx.setPattern("(\\S+) has changed the topic on channel (\\S+) to (.+)");
  if(found == 0 && rx.search(string) != -1){
      QString nick = rx.cap(1);
      QString channel = rx.cap(2);
      //kdDebug(5008) << "Channel: " << channel << endl;
      if(top->channelInfo().channel().lower() == channel.lower()){
	  QString topic = rx.cap(3);
	  //kdDebug(5008) << "Topic: " << topic << endl;
	  topic.replace(QRegExp("~~"), "~");
	  /*
	   * topic is in double quotes, so remove them
           */
	  top->setTopic( topic.mid(1, topic.length()-2) );
	  QString cmd = "/eval &dostatus();\n";
	  top->sirc_write(cmd);
      }
      highlightNick(string, nick);
  }
  return new parseSucc(" " + string, ksopts->infoColor, "user|topic");
}


void ChannelParser::highlightNick(QString &string, QString &nick)
{
    QRegExp rx(QString("(^|\\s+)%1(\\s+|$)").arg(QRegExp::escape(nick)));
    string.replace(rx, "\\1~n" + nick + "~n\\2");
}

