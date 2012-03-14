#ifndef CHAN_PARSER_H
#define CHAN_PARSER_H

#include <qcolor.h>
#include <qdict.h>

/*
 * This file defines the parser, and all exceptions generated
 * by the toplevel parse_input function.
 */

class ChannelParser; // Main class defined
class QString;
class KSircTopLevel;

class parseResult 
{
public:
  parseResult() { }
  virtual ~parseResult() { }
  // Empty 
};

/*
 * Parsing is ok, this is the string to display
 */
class parseSucc : public parseResult
{
public:
  QString string;       // String to display
  QColor  colour;
  QString pm;
  
  parseSucc(const QString &_s, const QColor &_c = QColor(), const QString &_pm = QString::null){
    string = _s;
    colour = _c;
    pm = _pm;
  }
};

/*
 * parseError is a fatal error message.
 * arg0: if it is not empty, it will get displayed to the channel screen (in error colour with mad smile)
 * arg1: if none empty goes to stderr
 */
class parseError : public parseResult
{
public:
  QString str;
  QString err;

  parseError(const QString &_s, const QString &_e)
    {
      str = _s;
      err = _e;
    }
};

class parseWrongChannel : public parseSucc 
{
public:
  parseWrongChannel(const QString &_s, const QColor &_c = QColor(), const QString &_pm = QString::null)
    : parseSucc(_s, _c, _pm)
    {
    }
  
};

class parseJoinPart : public parseSucc
{
public:
  parseJoinPart(const QString &_s, const QColor &_c = QColor(), const QString &_pm = QString::null)
    : parseSucc(_s, _c, _pm)
    {
    }
  
};

class infoFoundNick {
public:
  char nick[101];
  infoFoundNick(char *_n){
    strncpy(nick, _n, 100);
    nick[100]='\0';
  }
};

class badModeChangeError // Mode change barfed up, exit out
{
public:
  QString str;
  char *msg;

  badModeChangeError(QString _str, char *_msg)
    {
      str = _str;
      msg = _msg;
    }
};

class wrongChannelError // Message was not intended for us, display str and continue
{
public:
  int display;
  
  wrongChannelError(int _display)
    {
      display = _display;
    }
};

class doneModes // Finished parsing modes from the extModes string
{
public:
  doneModes(int /*i*/)
  {
  }
};

// End of exceptions

class parseFunc
{
public:
  parseResult *(ChannelParser::*parser)(QString);
  parseFunc(parseResult *(ChannelParser::*_parser)(QString)){
    parser = _parser;
  }

private:
  parseFunc(); // Disable the default no argument constructor
};


class ChannelParser {

public:
  /**
   * ChannelParser takes a KSircTopLevel as it's first argument so
   * we can refrence it latter.
   *
   * NOTE: the KSircTopLevel befriends the parser so we can have access to.
   * NOTE2: don't abuse this you little wanker.
   */
  ChannelParser(KSircTopLevel *_top);


  /**
   * parse() thanks the string to be parsed and parses it.
   * It returns a class of type 'parseResult' or 0.
   */
  parseResult *parse(QString string);

private:
  KSircTopLevel *top;

  /**
   * These are helper variables used internally in the parsing functions
   */

  bool prompt_active;
  int current_item;
  int top_item;

  /**
   * The parser Table holds a list of functions with parse
   * sepecific commands and arguments.  The key for the dict is the
   * first 3 characters of the "search key" for the string type.
   *
   * Even through the parserTable is static, the functions called manipulate
   * this object.
   *
   * The functions do the following:
   *   - On succesfull compleion: generte a parseSucc exception
   *   - On failure generate: parseFailed, etc
   */
   
  static QDict<parseFunc> parserTable;

  /*
   * Note regarding ssfe control messages:
   *
   * They are converted from the form `#ssfe#<COMMAND> to `<COMMAND>`
   * t/m is converted to t
   */

  /**
   * Start of the praser function definitions
   *
   * Each function returns void (since they never return it does matter)
   * 1 argument, the string to parse
   *
   */

  /**
   * SSFE clear 'l' function, clears main window
   */
  parseResult * parseSSFEClear(QString string);

  /**
   * SSFE Status is used to update caption, and op status
   *
   */
  parseResult * parseSSFEStatus(QString string);

  /**
   * SSFE Init is called by ssfe when it's time to init and setup
   */
  parseResult * parseSSFEInit(QString string);

  /**
   * SSFE msg is called for each /msg sent to a diffrent user
   */
  parseResult * parseSSFEMsg(QString string);

  /**
   * SSFE Out, not used
   */
  parseResult * parseSSFEOut(QString string);
  
  /**
   * SSFE Prompt, same function used for p and P,  gives a password prompt
   */
  parseResult * parseSSFEPrompt(QString string);

  /**
   * SSFE Reconnect called when (re)connected to a server
   */
  parseResult * parseSSFEReconnect(QString string);

  /**
   * That's all the SSFE control messages, the rest are info *\S* message
   */

  /**
   * *** is a generic infomation messge
   */
  parseResult * parseINFOInfo(QString string);

  /**
   * *E* is a error message
   */
  parseResult * parseINFOError(QString string);

  /**
   * *#* is a list of nicks, used to update the nick list if it's for
   * the current channel.
   */
  parseResult * parseINFONicks(QString in_string);

  /**
   * *>* is a join message.  If it's for the current channel
   */
  parseResult * parseINFOJoin(QString string);

  /**
   * *<* is a part message, if it's for the current channel remove it.
   * There's multiple part types, so parsing it more complicated.
   */
  parseResult * parseINFOPart(QString string);

  /**
   * *N* Is a nick change, update the nick list if needed
   * Nick changes go outto all windows, so the nick may not be on
   * out current channel.
   */
  parseResult * parseINFOChangeNick(QString string);

  /*
   * *M* is a mode change.  Parsing is mode changes is probably the most
   * complicated of all the parsings
   */
  parseResult * parseINFOMode(QString string);

  /*
   * *  is a ctcp actiion.  Simply print a pretty * for the user
   */
  parseResult * parseCTCPAction(QString string);

  /*
   * *T* is a topic message, catch it and update the status is required
   */
  parseResult * parseINFOTopic(QString string);

private:
  void highlightNick(QString &string, QString &nick);
};

#endif
