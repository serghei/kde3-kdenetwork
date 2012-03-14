/***********************************************************************

 IO Controller Object

 $$Id: iocontroller.cpp 621565 2007-01-09 03:13:10Z aseigo $$

 Main io controller.  Reads and writes strings to sirc.  Input
 received in the following 2 formats:

 1. ~window name~<message>
 2. <message>

 each is handled diffrently. The window name is extracted from #1 and
 if the window exists the message portion is sent to it.  If the
 window doesn't exist, or case 2 is found the window is sent to the
 control window "!default".  !default is NOT a constant window but
 rather follows focus arround.  This is the easiest way to solve the
 problem of output to commands that don't have a fixed destination
 window.  (/whois, /help, etc )

 Implementation:

   Friends with KSircProcess allows easy access to TopList.  Makes sence
   and means that IOController has access to TopList, etc.  The two work
   closely together.

   Variables:
      holder: used to hold partital lines between writes.
      *proc: the acutally sirc client process.
      *ksircproc: the companion ksircprocess GUI controller
      stdout_notif: access to SocketNotifier, why is this global to the
                    class?
      counter: existance counter.

   Functions:
   public:
     KSircIOController(KProcess*, KSircProcess*):
       - Object constructor takes two arguements the KProcess
         that holds a running copy of sirc.
       - KSircProcess is saved for access latter to TopList.
       - The receivedStdout signal from KProcess is connected to
         stdout_read and the processExited is connected to the sircDied
	 slot.

     ~KSircIOController: does nothing at this time.

     public slots:
       stdout_read(KProcess  *, _buffer, buflen):
         - Called by kprocess when data arrives.
	 - This function does all the parsing and sending of messages
           to each window.

       stderr_read(KProcess*, _buffer, buflen):
         - Should be called for stderr data, not connected, does
	   nothing.

       stdin_write(QString):
         - Slot that get's connected to by KSircProcess to each
	   window.  QString shold be written un touched! Let the
	   writter figure out what ever he wants to do.

       sircDied:
         - Should restart sirc or something....
	 - Becarefull not to get it die->start->die->... etc

***********************************************************************/

#include <config.h>

#include "ksopts.h"
#include "control_message.h"
#include "iocontroller.h"
#include "ksircprocess.h"
#include "messageReceiver.h"
#include "ksopts.h"

#include <qlistbox.h>
#include <qtextcodec.h>
#include <kcharsets.h>
#include <kglobal.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

int KSircIOController::counter = 0;

KSircIOController::KSircIOController(KProcess *_proc, KSircProcess *_ksircproc)
  : QObject()
{

  counter++;

  proc = _proc;              // save proc
  ksircproc = _ksircproc;    // save ksircproce

  send_buf = 0x0;
  m_debugLB = 0;

  // Connect the data arrived
  // to sirc receive for adding
  // the main text window
  connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)),
          this, SLOT(stdout_read(KProcess*, char*, int)));

  // Connect the stderr data
  // to sirc receive for adding
  // the main text window
  connect(proc, SIGNAL(receivedStderr(KProcess *, char *, int)),
          this, SLOT(stderr_read(KProcess*, char*, int)));

  connect(proc, SIGNAL(processExited(KProcess *)),
	  this, SLOT(sircDied(KProcess *)));
                                              // Notify on sirc dying
  connect(proc, SIGNAL(wroteStdin(KProcess*)),
	  this, SLOT(procCTS(KProcess*)));
  proc_CTS = TRUE;
#if 0
  showDebugTraffic(true);
#endif
}

void my_print(const char *c){
    while(*c != 0x0){
	if(*c & 0x80)
	    fprintf(stderr, "<%02X>", 0xff & *c);
	else
	    fprintf(stderr, "%c", *c);
        c++;
    }
    fprintf(stderr, "\n");
}

void KSircIOController::stdout_read(KProcess *, char *_buffer, int buflen)
{

  /*

    Main reader reads from sirc and ships it out to the right
    (hopefully) window.

    Problem trying to solve:

       _buffer holds upto 1024 (it seems) block of data.  We need to
       take it, split into lines figure out where each line should go,
       and ship it there.

       We watch for broken end of lines, ie partial lines and reattach
       them to the front on the next read.

       We also stop all processing in the windows while writting the
       lines.

    Implementation:

    Variables:
       _buffer original buffer, holds just, icky thing, NOT NULL terminated!
       buf: new clean just the right size buf that is null terminated.
       pos, pos2, pos3 used to cut the string up into peices, etc.
       name: destination window.
       line: line to ship out.

    Steps:
       1. read and copy buffer, make sure it's valid.
       2. If we're holding a broken line, append it.
       3. Check for a broken line, and save the end if it is.
       4. Stop updates in all windows.
       5. Step through the data and send the lines out to the right
       window.
       6. Cleanup and continue.

   */

  int pos,pos2,pos3;
  QCString name, line;

  assert(_buffer != 0);
  assert(buflen > 0);

  QCString buffer(_buffer, buflen+1);
  //fprintf(stderr, "first print: \n");
  //my_print(buffer);
  //kdDebug(5008) << "<-- read: " << buffer << endl;
  name = "!default";


  if(holder.length() > 0){
    buffer.prepend(holder);
    holder.truncate(0);
  }

  if(buffer[buffer.length()-1] != '\n'){
    pos = buffer.findRev('\n');
    if(pos != -1){
      holder = buffer.right(buffer.length()-(pos+1));
      buffer.truncate(pos+1);
    }
    else {
      /* there is _NO_ linefeeds in this line at all, means we're
       * only part of a string, buffer it all!!
       * (lines are linefeed delimeted)
       */
      holder = buffer;
      return;
    }
  }

  pos = pos2 = 0;

  KSircMessageReceiver * rec = ksircproc->TopList["!all"];

  if (0 == rec)
  {
    return;
  }

  rec->control_message(STOP_UPDATES, "");
  if(m_debugLB)
      m_debugLB->setUpdatesEnabled(false);

  do{
    pos2 = buffer.find('\n', pos);

    if(pos2 == -1)
      pos2 = buffer.length();

    line = buffer.mid(pos, pos2 - pos);
    if(m_debugLB){
	QString s = QString::fromUtf8(line);
	m_debugLB->insertItem(s);
    }

    //kdDebug(5008) << "Line: " << line << endl;

    if((line.length() > 0) && (line[0] == '~')){
      pos3 = line.find('~', 1);
      if(pos3 > 1){
	name = line.mid(1,pos3-1).lower();
	name = name.lower();
	line.remove(0, pos3+1);
      }
    }
    QString enc = KGlobal::charsets()->encodingForName( ksopts->channel["global"]["global"].encoding );
    QTextCodec *qtc = KGlobal::charsets()->codecForName( enc );
    QString qsname = qtc->toUnicode(name);
    /*
    char *b = qstrdup(line);
    kdDebug(5008) << "----------------------------------------" << endl;
    kdDebug(5008) << "Line: " << b << endl;
    fprintf(stderr, "My_print: " ); my_print(b);
    fprintf(stderr, "fprintf: %s\n", (const char *)b);
    kdDebug(5008) << "Codec: " << qtc->name() << " (" << ksopts->channel["global"]["global"].encoding << ")" << " Name: " << name << " qsname: " << qsname << endl;
    kdDebug(5008) << "Line(de): " << qtc->toUnicode(b) << endl;
    kdDebug(5008) << "----------------------------------------" << endl;
    */

    if(!(ksircproc->TopList[qsname])){
        // Ignore ssfe control messages with `
        // we left channel, don't open a window for a control message
        bool noticeCreate = true;
        if(ksopts->autoCreateWinForNotice == false && (line[0] == '-' || line[0] == '*'))
            noticeCreate = false;
        if(ksopts->autoCreateWin == TRUE && line[0] != '`' && line[1] != '#' && line[1] != '&' && noticeCreate) {
	    //kdDebug(5008) << "Creating window for: " << qsname << " because of: " << line.data() << endl;
	    ksircproc->new_toplevel(KSircChannel(ksircproc->serverName(), qsname));
        }
        if (!ksircproc->TopList[qsname]) {
            qsname = "!default";
            if(line[0] == '`')
                qsname = "!discard";
        }
        assert(ksircproc->TopList[qsname]);
    }

    ksircproc->TopList[qsname]->sirc_receive(line);


    pos = pos2+1;
  } while((uint) pos < buffer.length());

  ksircproc->TopList["!all"]->control_message(RESUME_UPDATES, "");
  if(m_debugLB){
      m_debugLB->triggerUpdate(true);
      m_debugLB->setContentsPos( 0, m_debugLB->contentsHeight()-m_debugLB->visibleHeight());
      m_debugLB->setUpdatesEnabled(true);
      m_debugLB->triggerUpdate(false);
  }


}

KSircIOController::~KSircIOController()
{
        delete m_debugLB;
}

void KSircIOController::stderr_read(KProcess *p, char *b, int l)
{
  stdout_read(p, b, l);
}

void KSircIOController::stdin_write(QCString s)
{
  if (!proc->isRunning())
  {
     kdDebug(5008) << "writing to a dead process! (" << s << ")\n";
     return;
  }

  //kdDebug(5008) << "--> wrote: " << s;
  buffer += s;
  //fprintf(stderr, "Buffer output: ");
  //my_print(buffer);

  if(proc_CTS == TRUE){
    int len = buffer.length();
    if(send_buf != 0x0){
      qWarning("KProcess barfed in all clear signal again");
      delete[] send_buf;
    }
    send_buf = new char[len];
    memcpy(send_buf, buffer.data(), len);
    if(proc->writeStdin(send_buf, len) == FALSE){
      kdDebug(5008) << "Failed to write but CTS HIGH! Setting low!: " << s << endl;
    }
    else{
	if(m_debugLB){
	    QString s = QString::fromUtf8(buffer);
	    m_debugLB->insertItem(s);
	    m_debugLB->setContentsPos( 0, m_debugLB->contentsHeight());
	}
	buffer.truncate(0);
    }
    proc_CTS = FALSE;
  }

  if(buffer.length() > 5000){
    kdDebug(5008) << "IOController: KProcess barfing again!\n";
  }
  //  write(sirc_stdin, s, s.length());

}

void KSircIOController::sircDied(KProcess *process)
{
  if ( process->exitStatus() == 0 )
      return;
  kdDebug(5008) << "IOController: KProcess died!\n";
  ksircproc->TopList["!all"]->sirc_receive("*E* DSIRC IS DEAD");
  ksircproc->TopList["!all"]->sirc_receive("*E* KSIRC WINDOW HALTED");
  ksircproc->TopList["!all"]->sirc_receive( QCString( "*E* Tried to run: " ) + KGlobal::dirs()->findExe("dsirc").ascii() + QCString( "\n" ) );
  ksircproc->TopList["!all"]->sirc_receive("*E* DID YOU READ THE INSTALL INTRUCTIONS?");
}

void KSircIOController::procCTS ( KProcess *)
{
  proc_CTS = true;
  delete[] send_buf;
  send_buf = 0x0;
  if(!buffer.isEmpty()){
    QCString str = "";
    stdin_write(str);
  }
}

void KSircIOController::showContextMenuOnDebugWindow(QListBoxItem *, const QPoint &pos)
{
    if (!m_debugLB)
        return;

    QPopupMenu popup(m_debugLB);
    popup.insertItem("Save Contents to File...", 1);
    if (popup.exec( pos ) != 1)
        return;

    QString path = KFileDialog::getSaveFileName();
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(IO_WriteOnly))
        return;

    QTextStream stream(&file);

    for (uint i = 0; i < m_debugLB->count(); ++i)
        stream << m_debugLB->text(i) << endl;
}

void KSircIOController::showDebugTraffic(bool show)
{
    kdDebug(5008) << "Got show request: " << show << endl;
    if(m_debugLB == 0 && show == true){
	m_debugLB = new QListBox(0x0, QCString(this->name()) + "_debugWindow");
        m_debugLB->resize(600, 300);
        m_debugLB->show();
        connect(m_debugLB,SIGNAL(contextMenuRequested(QListBoxItem *,const QPoint &)),
                this,SLOT(showContextMenuOnDebugWindow(QListBoxItem *,const QPoint &)));
    }
    else if(m_debugLB != 0 && show == false){
	delete m_debugLB;
        m_debugLB = 0x0;
    }

}

bool KSircIOController::isDebugTraffic()
{
    if(m_debugLB != 0)
	return true;
    else
        return false;
}

void KSircIOController::appendDebug(QString s)
{
    if(m_debugLB){
	m_debugLB->insertItem(s);
	m_debugLB->setContentsPos( 0, m_debugLB->contentsHeight()-m_debugLB->visibleHeight());
    }
}

#include "iocontroller.moc"
