#ifndef KSIRCTOPLEVEL_H
#define KSIRCTOPLEVEL_H

#include <qdatetime.h>
#include <kmainwindow.h>

#include "ahistlineedit.h"
#include "messageReceiver.h"
#include "chanButtons.h"
#include "KSTicker/ksticker.h"
#include "ksircchannel.h"

#define KST_CHANNEL_ID 2

class KSircTopic;
class QSplitter;
class KSircView;
class aListBox;
class UserControlMenu;
class ChannelParser;
class charSelector;
class LogFile;
class KSelectAction;
class QLabel;
class QVBox;
class QListBoxItem;

class QPopupMenu;

class KSircTopLevel : public KMainWindow,
		      public UnicodeMessageReceiver
{
  Q_OBJECT
    friend class ChannelParser;
public:
  /**
    * Constructor, needs the controlling ksircprocess passed to it, so
    * we can make the ksircmessage receiver happy.
    */
  KSircTopLevel(KSircProcess *_proc, const KSircChannel &channelInfo, const char *
		name=0);
  /**
    * Destructor, destroys itself.
    */
  ~KSircTopLevel();

  /**
    * Reimplement show() to popup the menu bars and toolbar items
    */
  virtual void show();

  /**
   * Line recieved that should be printed on the screen. Unparsed, and
   * ready processing.  This is a single line, and it NOT \n
   * terminated.
   */
  virtual void sirc_receive(QString str, bool broadcast = false);

  /**
    * Reimplement the ksircmessagereceiver control messages.  These
    * are parsed and dealt with quickly.
    */
  virtual void control_message(int, QString);

  QWidget *lineEdit() const { return linee; }

  void setTopic( const QString &topic );
  QString topic() const { return m_topic; }

  /**
   * This returns the current channel information for a toplevel
   */
  const KSircChannel &channelInfo() const { return m_channelInfo; }

signals:
  /**
    * signals thats the toplevel widget wishes to
    * output a new line.  The line is correctly
    * linefeed terminated, etc.
    */
  void outputLine(QCString);
  /**
    * open a new toplevel widget with for the
    * channel/user QString.
    */
  void open_toplevel(const KSircChannel &);
  /**
    * emittedon shutdown, indicating that
    * this window is closing. Refrence to outselves
    * is include.
    */
  void closing(KSircTopLevel *, QString channel);
  /**
   * emitted when the user typed /quit, will disconnect from this
   * server and close all toplevels belonging to it.
   */
  void requestQuit( const QCString& command );
  /**
    * emitted when we change channel name
    * on the fly.  old is the old channel name, new
    * is the new one.
    */
  void changeChannel(const QString &oldName, const QString &newName);
  /**
    * emitted to say that KSircTopLevel
    * (this) is now the window with focus.  Used by
    * servercontroller to set the !default window.
    */
  void currentWindow(KSircTopLevel *);
  /**
   * Stop updating list item sizes, majour changes comming through
   *
   */
  void freezeUpdates(bool);
  /**
   * Emitted when a new message appeared in the irc window
   */
  void changed(bool, QString);

public slots:
  /**
   * When enter is pressed, we read the SLE and output the results
   * after processing via emitting outputLine.
   */
  virtual void sirc_line_return(const QString &s);

   /**
    * Clears the message window
    */
  void clearWindow();

  /**
   * If the focus shifts this should get called with
   * the widget that just got focus.  Used in MDI modes
   * where the toplevel doesn't get focus in/out events.
   */
  void focusChange(QWidget *w);

protected slots:
    /**
      * When the rightMouse button is pressed in the nick list, popup
      * the User popup menu at his cursor location.
      */
   void UserSelected(int index);
   /**
     * Menu items was selected, now read the UserControlMenu and
     * reupdate the required menus.
     */
   void UserParseMenu(int id);
   /**
     * Page down accel key.  Tells the mainw to scroll down 1 page.
     */
   void AccelScrollDownPage();
   /**
     * Page Up accell key.  Tells the mainw to scroll down 1 page.
     */
   void AccelScrollUpPage();
   /**
     * Slot to termiate (close) the window.
     */
   void terminate() { close( true /*alsoDelete*/ ); }
   /**
     * Called when the user menu is finished and the popup menu needs
     * to be updated.
     */
   void UserUpdateMenu();
   /**
     * Open the new channel/window selector.
     */
   void newWindow();
   /**
     * We've received focus, let's make us the default, and issue a
     * /join such that we default to the right channel.
     */
   void gotFocus();
   /**
     * We've lost focus, set ourselves as loosing focus.
     */
   void lostFocus();
   /**
     * Create and popup the ticker.  Make sure to restore it to the
     * last position is was at.
     */
//   void showTicker();
   /**
     * toggle the timestamp from the channel window menu (and with keyaccel)
     */
   void toggleTimestamp();
   /**
     * toggle the filtering of join/part message
     */
   void toggleFilterJoinPart();

    /**
     * toggle the beep from the channel window menu (and with keyaccel)
     */
   void toggleBeep();

   /**
    * show the dcc manager
    */
   void showDCCMgr();

   /**
     * Delete the ticker and ppoup the main window
     */
//   void unHide();
   /**
     * On a TAB key press we call TabNickCompletion which
     * reads the last thing in linee matches it with a nick and
     * puts it back into the line.
     */
   void TabNickCompletionShift();
   /**
     * On a TAB key press we call TabNickCompletion which
     * reads the last thing in linee matches it with a nick and
     * puts it back into the line.
     */
   void TabNickCompletionNormal();

   /**
     * Signals a Line Change in linee
     */
   void lineeNotTab();
   /**
     *  Move the display to or from the root window
     *
     */
   void toggleRootWindow();
   /**
    * On a middle mouse button press we call pasteToWindow which
    * reads the clip board and pastes into the main listbox.
    */
   void pasteToWindow();
   /**
    * On a midle mouse button press to the nick list we open
    * a query window and paste the text there
    */
   void pasteToNickList(int button, QListBoxItem *item, const QPoint &pos);
   /**
    * dnd paste to nick list
    */
   void dndTextToNickList(const QListBoxItem *itom, const QString& text);
   /**
    * open a toplevel on double click
    */
   void openQueryFromNick(const QString &);

   /**
    * Some text was dropped on the listbox
    */
   void slotTextDropped(const QString&);

   /**
    * Calls slotDccURLs with the current nick we're talking to
    */
   void slotDropURLs(const QStringList& urls);

   /**
    * Sends the list of urls to nick
    */
   void slotDccURLs(const QStringList& urls, const QString& nick );

   /**
    * Re-apply color settings.
    */
   void initColors();

   /**
   *  Dumps current content of mainw into a logfile
   */
   void saveCurrLog();

    /**
     * Makes a beep when a change happens (if the user has this on)
     * the bool is TRUE if changed line contained user's nick
     */
   void doChange(bool, QString);

   /**
    * show the ticker window
    */
   void toggleTicker();

   /**
    * ksirc command menu clicked on
    */
   void cmd_process(int id);

protected:
   /**
     * On a TAB key press we call TabNickCompletion which
     * reads the last thing in linee matches it with a nick and
     * puts it back into the line.
     */
   void TabNickCompletion(int dir);

   /**
    * Make sure to update colors correctly.
    */
   virtual bool event( QEvent *e);

    /**
     * Redfine closeEvent to emit closing and delete itself.
     */
   virtual void closeEvent(QCloseEvent *);
   /**
     * Searches through the nick list and finds the nick with the best match.
     * which, arg2, selects nicks other than the first shouldarg1 match more
     * than 1 nick.
     */
   virtual QString findNick(QString, uint which = 0);
   /**
    * remove a nick from the prefered in nick completion list
    */
   virtual void removeCompleteNick(const QString &);
   /**
	* Adds a nick to the list of nicks to prefer in nick completion
	* or moves it to the top of the list if it was already there
	*/
   virtual void addCompleteNick(const QString &);
   /**
	* Changes a nick in the completion list if it has been listed,
	* otherwise does nothing
	*/
   virtual void changeCompleteNick(const QString &, const QString &);

   virtual void setupCommandMenu();

   /**
    * Returns true if this window is a private conversation (query or dcc chat)
    *, instead of a public chat in a channel or a special window.
    */
   bool isPrivateChat() const;

   /**
    * Returns true if this is a public channel window
    */
   bool isPublicChat() const;

   /**
    * Returns true if this is a special window
    */
   bool isSpecialWindow() const;

private slots:
    void setTopicIntern( const QString &topic );
    void insertText();
    void setMode(QString, int, QString currentNick=0);
    void setEncoding();
    void returnPressed();
    void toggleTopic();

private:

    void outputUnicodeLine( const QString &message );

    bool atBottom();

    bool continued_line;
    charSelector *selector;
    chanButtons *channelButtons;
    QSplitter *pan;
    QVBox *nicks_box;
    QVBox *top;
    KMenuBar *kmenu;
    QLabel *lag;
    enum {PING = 10, TOPIC = 20};
    KSircView *mainw;
    aHistLineEdit *linee;
    aListBox *nicks;

    //QString m_currentNick;

    //KSircProcess *proc;

    int lines;

    struct BufferedLine
    {
        BufferedLine() { wasBroadcast = false; }
        BufferedLine( const QString &msg, bool broadc )
            { message = msg; wasBroadcast = broadc; }
        bool operator==( const BufferedLine &other )
            { return message == other.message &&
                wasBroadcast == other.wasBroadcast; }

        QString message;
        bool wasBroadcast;
    };

  bool Buffer;
  QValueList<BufferedLine> LineBuffer;

  // QPopupMenu's used for the menubar
  QPopupMenu *file, *edit, *command;

  bool parse_input(const QString &string, QString &plainText);
  void sirc_write(const QString &str);

  QPopupMenu *user_controls;
  static QPtrList<UserControlMenu> *user_menu;
  int opami;

  QAccel *accel;

  /**
   * The channel name that we belong too.
   */
  //QString channel_name;

  /**
    * Caption at the top of the window.
    */
  QString caption;
  /**
   * Current topic for the channel
   */
  QString m_topic;

  /**
    * Does the window have focus? 1 when yes, 0 when no.
    */
  int have_focus;

  /**
    * Number of time tab has been pressed.  Each time it's pressed
    * roll through the list of matching nicks.
    */
  int tab_pressed;
  /**
   * When tabs pressed save the line for use at a latter date.
   * tab_nick holds the last nick found so when a blank tab is pressed
   * we pop the last niick and ": " up.
    */
  QString tab_saved, tab_nick;
  int tab_start, tab_end;

  // Ticker specific variables.

  /**
    * Main pointer to ksticker.  Caution, it doesn't always exist!
    * Set to 0 ifwhen you delete it.
    */
  KSTicker *ticker;
  /**
    * Size and position of the main window. Main window is returns this
    * this position when it reappears.
    */
  QRect myrect;
  /**
    * Position of the main window
    */
  QPoint mypoint;
  /**
    * Ticker's size and geometry
    */
  QRect tickerrect;
  /**
    * Tickers position
    */
  QPoint tickerpoint;
  QSize  tickersize;
  /**
   * Do we have a kick window open?
   * Remember not to open 2
   */
  bool KickWinOpen;

  /**
   * Hold an internal parser object which does all our parsing for us.
   * Since parsing an intergral part of TopLevel, it's also a friend
   * of ours
   */
  ChannelParser *ChanParser;

  /**
   * QSize maintains size information in case it changes somehow
   */
  QSize current_size;

  /**
   * ID of the timestamp menu item, to use in (un)checking it from slot
   */
  int tsitem;

  /**
   * ID of the filter join/part menu item, to use in (un)checking it from slot
   */
  int fjpitem;

  /**
   *  ID of the beep menu item, to use in (un)checking it from slot
   */
  int beepitem;

  /**
   * ID of the toggle topic menu item, to use in (un)checking it from slot
   */
  int topicitem;

  /**
   * ID of the show/hide ticker menu item
   */
  int tickeritem;

  /**
   * List of nicks already used in nick completion to give them
   * higher priority
   */
  QStringList completeNicks;

  /**
   * The time of the last beep to prevent your soundcard from detonating
   * in high traffic channels
   */
  QTime lastBeep;

  KSircTopic *ksTopic;

  LogFile *logFile;

  KSelectAction *encodingAction;

  static QStringList cmd_menu;

  bool m_gotMsgWithoutFocus;

  KSircChannel m_channelInfo;

};

#endif

// vim: ts=2 sw=2 et
