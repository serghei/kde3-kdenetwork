
#include "baserules.h"
#include "ksopts.h"
#include "ksircprocess.h"

#include <qregexp.h>

#include <stdio.h>

void KSMBaseRules::sirc_receive(QCString, bool)
{
}

void KSMBaseRules::control_message(int, QString)
{
}

filterRuleList *KSMBaseRules::defaultRules()
{
  filterRule *fr;
  filterRuleList *frl = new filterRuleList();
  frl->setAutoDelete(TRUE);
  if( ksopts->ksircColors ){
    // 3 stage rule to filter:
    // ~blah~<something> stuff with ~ here and ~ there
    // Here's the path follows
    // =>~blah~;;;<something> with ~ here and ~ there
    // =>~blah~;;;<something> with ~~ here and ~~ there
    // =>~blah~<something> with ~~ here and ~ there
    // Basic problem was getting it not to change ~blah~ into ~~blah~~
    fr = new filterRule();
    fr->desc = "Add marker to second ~";
    fr->search = "^~\\S+~";
    fr->from = "^~(\\S+)~";
    fr->to = "~;;;$1~;;;";
    frl->append(fr);
    fr = new filterRule();
    fr->desc = "Escape kSirc Control Codes";
    fr->search = "~";
    fr->from = "(?g)~(?!;;;)";
    fr->to = "$1~~";
    frl->append(fr);
    fr = new filterRule();
    fr->desc = "Remove marker to second";
    fr->search = "^~;;;\\S+~;;;";
    fr->from = "^~;;;(\\S+)~;;;";
    fr->to = "~$1~";
    frl->append(fr);

  }
  else{
    // If we don't escape all ~'s at least esacpe the ones in
    // email/part joins etc.
    fr = new filterRule();
    fr->desc = "Search for dump ~'s";
    fr->search = "\\W~\\S+@\\S+\\W";
    fr->from = "~(\\S+@)";
    fr->to = "~~$1";
    frl->append(fr);
  }
  if(ksopts->mircColors){
    fr = new filterRule();
    fr->desc = "Remove mirc Colours";
    fr->search = "\\x03";
    fr->from = "(?g)\\x03(?:\\d{0,2},{0,1}\\d{0,2})";
    fr->to = "";
    frl->append(fr);
  }
  if( ksopts->nickForeground.isValid() || ksopts->nickColourization){
    fr = new filterRule();
    fr->desc = "Highlight nicks in colour";
    fr->search = "(?:~\\S+~)[<>|\\[\\]\\-]\\S+[<>|\\[\\]\\-]";
    fr->from = "^((?:~\\S+~))([<>|\\[\\]\\-])(\\S+)([<>|\\[\\]\\-])";
    fr->to = "$1$2~n$3~n$4";
    frl->append(fr);
    fr = new filterRule();
    fr->desc = "Highlight nicks in colour in CTCP ACTION";
    fr->search = "(?:~\\S+~)\\* (\\S+)";
    fr->from = "^((?:~\\S+~))\\* (\\S+)";
    fr->to = "$1* ~n$2~n";
    frl->append(fr);
  }
  if( ksopts->msgContainNick.isValid() ){
      //QString nick = ksopts->nick;
    QString nick = ksircProcess()->getNick();
    if ( !nick.isEmpty() ) {
	if(nick.length() > 83){
	    qDebug("Nick too long");
	    nick.truncate( 83 );
	}
	/*
	 * Since the nick is used in the regex we must
	 * escape all regex words
	 */
	nick = QRegExp::escape(nick);

	sprintf(match_us,
		"(?i)<\\S+>.*\\s%s(,.*|\02:.*|:.*|\\s.*|$)", nick.latin1());
	sprintf(to_us,
		"$1~o");//, ksopts->ownNickColor);
	fr = new filterRule();
	fr->desc = "Highlight our nick";
	fr->search = match_us;
	fr->from = "(<\\S+>)";
	fr->to = to_us;
	frl->append(fr);
    }
  }
  if( ksopts->msg1Contain.isValid() ){
      //QString nick = ksopts->nick;
      /*
       * Since the nick is used in the regex we must
       * escape all regex words
       */
      QString msg = ksopts->msg1String;

      if(msg.length() > 0){
	  if(ksopts->msg1Regex == false)
	      msg = QRegExp::escape(msg);

	  snprintf(msg1_match, 100,
		   "(?i)<\\S+>.*\\s%s(,.*|\02:.*|:.*|\\s.*|$)", msg.latin1());
	  snprintf(msg1_to, 100,
		   "$1~%s", ksopts->msg1Contain.name().latin1());//, ksopts->ownNickColor);
	  fr = new filterRule();
	  fr->desc = "Highlight our msg1";
	  fr->search = msg1_match;
	  fr->from = "(<\\S+>)";
	  fr->to = msg1_to;
	  frl->append(fr);
      }
  }
  if( ksopts->msg2Contain.isValid() ){
      //QString nick = ksopts->nick;
      /*
       * Since the nick is used in the regex we must
       * escape all regex words
       */
      QString msg = ksopts->msg2String;

      if(msg.length() > 0){
	  if(ksopts->msg2Regex == false)
	      msg = QRegExp::escape(msg);

	  snprintf(msg2_match, 100,
		   "(?i)<\\S+>.*\\s%s(,.*|\02:.*|:.*|\\s.*|$)", msg.latin1());
	  snprintf(msg2_to, 100,
		   "$1~%s", ksopts->msg2Contain.name().latin1());//, ksopts->ownNickColor);
	  fr = new filterRule();
	  fr->desc = "Highlight our msg2";
	  fr->search = msg2_match;
	  fr->from = "(<\\S+>)";
	  fr->to = msg2_to;
	  frl->append(fr);
      }
  }

  // Default rules alays in place
  fr = new filterRule();
  fr->desc = "Remove Just bold in parts and joins";
  fr->search = "\\*\\x02\\S+\\x02\\*";
  fr->from = "\\*\\x02(\\S+)\\x02\\*";
  fr->to = "\\*$1\\*";
  frl->append(fr);

  return frl;
}
