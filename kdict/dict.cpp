/* -------------------------------------------------------------

   dict.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias Hölzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   JobData          used for data transfer between Client and Interface
   DictAsyncClient  all network related stuff happens here in asynchrous thread
   DictInterface    interface for DictAsyncClient, job management

 ------------------------------------------------------------- */

#include <config.h>

#include "application.h"
#include "options.h"
#include "dict.h"

#include <qregexp.h>
#include <qtextcodec.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kmdcodec.h>
#include <kextsock.h>
#include <ksocks.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

//********* JobData ******************************************


JobData::JobData(QueryType Ntype,bool NnewServer,QString const& Nserver,int Nport,
                 int NidleHold, int Ntimeout, int NpipeSize, QString const& Nencoding, bool NAuthEnabled,
                 QString const& Nuser, QString const& Nsecret, unsigned int NheadLayout)
  : type(Ntype), error(ErrNoErr), canceled(false), numFetched(0), newServer(NnewServer),server(Nserver), port(Nport),
    timeout(Ntimeout), pipeSize(NpipeSize), idleHold(NidleHold), encoding(Nencoding), authEnabled(NAuthEnabled),
    user(Nuser), secret(Nsecret), headLayout(NheadLayout)
{}


//********* DictAsyncClient *************************************

DictAsyncClient::DictAsyncClient(int NfdPipeIn, int NfdPipeOut)
: job(0L), inputSize(10000), fdPipeIn(NfdPipeIn),
  fdPipeOut(NfdPipeOut), tcpSocket(-1), idleHold(0)
{
  input = new char[inputSize];
}


DictAsyncClient::~DictAsyncClient()
{
  if (-1!=tcpSocket)
    doQuit();
  delete [] input;
}


void* DictAsyncClient::startThread(void* pseudoThis)
{
  DictAsyncClient* newthis = (DictAsyncClient*) (pseudoThis);

  if (0!=pthread_setcanceltype(PTHREAD_CANCEL_ENABLE,NULL))
    qWarning("pthread_setcanceltype failed!");
  if (0!= pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL))
    qWarning("pthread_setcanceltype failed!");

  signal(SIGPIPE,SIG_IGN);   // ignore sigpipe

  newthis->waitForWork();
  return NULL;
}


void DictAsyncClient::insertJob(JobData *newJob)
{
  if (!job)        // don't overwrite existing job pointer
    job = newJob;
}


void DictAsyncClient::removeJob()
{
  job = 0L;
}


void DictAsyncClient::waitForWork()
{
  fd_set fdsR,fdsE;
  timeval tv;
  int selectRet;
  char buf;

  while (true) {
    if (tcpSocket != -1) {  // we are connected, hold the connection for xx secs
      FD_ZERO(&fdsR);
      FD_SET(fdPipeIn, &fdsR);
      FD_SET(tcpSocket, &fdsR);
      FD_ZERO(&fdsE);
      FD_SET(tcpSocket, &fdsE);
      tv.tv_sec = idleHold;
      tv.tv_usec = 0;
      selectRet = KSocks::self()->select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
      if (selectRet == 0) {
        doQuit();               // nothing happend...
      } else {
        if (((selectRet > 0)&&(!FD_ISSET(fdPipeIn,&fdsR)))||(selectRet == -1))
          closeSocket();
      }
    }

    do {
      FD_ZERO(&fdsR);
      FD_SET(fdPipeIn, &fdsR);
    } while (select(FD_SETSIZE, &fdsR, NULL, NULL, NULL)<0);  // don't get tricked by signals

    clearPipe();

    if (job) {
      if ((tcpSocket!=-1)&&(job->newServer))
        doQuit();

      codec = QTextCodec::codecForName(job->encoding.latin1());
      input[0] = 0;                 //terminate string
      thisLine = input;
      nextLine = input;
      inputEnd = input;
      timeout = job->timeout;
      idleHold = job->idleHold;

      if (tcpSocket==-1)
        openConnection();

      if (tcpSocket!=-1) {        // connection is ready
        switch (job->type) {
        case JobData::TDefine :
          define();
          break;
        case JobData::TGetDefinitions :
          getDefinitions();
          break;
        case JobData::TMatch :
          match();
          break;
        case JobData::TShowDatabases :
          showDatabases();
          break;
        case JobData::TShowDbInfo :
          showDbInfo();
          break;
        case JobData::TShowStrategies :
          showStrategies();
          break;
        case JobData::TShowInfo :
          showInfo();
          break;
        case JobData::TUpdate :
          update();
        }
      }
      clearPipe();
    }
    if (write(fdPipeOut,&buf,1) == -1)   // emit stopped signal
      ::perror( "waitForJobs()" );
  }
}


void DictAsyncClient::define()
{
  QString command;

  job->defines.clear();
  QStringList::iterator it;
  for (it = job->databases.begin(); it != job->databases.end(); ++it) {
    command = "define ";
    command += *it;
    command += " \"";
    command += job->query;
    command += "\"\r\n";
    job->defines.append(command);
  }

  if (!getDefinitions())
    return;

  if (job->numFetched == 0) {
    job->strategy = ".";
    if (!match())
      return;
    job->result = QString::null;
    if (job->numFetched == 0) {
      resultAppend("<body>\n<p class=\"heading\">\n");
      resultAppend(i18n("No definitions found for \'%1'.").arg(job->query));
      resultAppend("</p>\n</html></body>");
    } else {
      // html header...
      resultAppend("<body>\n<p class=\"heading\">\n");
      resultAppend(i18n("No definitions found for \'%1\'. Perhaps you mean:").arg(job->query));
      resultAppend("</p>\n<table width=\"100%\" cols=2>\n");

      QString lastDb;
      QStringList::iterator it;
      for (it = job->matches.begin(); it != job->matches.end(); ++it) {
        int pos = (*it).find(' ');
        if (pos != -1) {
          if (lastDb != (*it).left(pos)) {
            if (lastDb.length() > 0)
              resultAppend("</pre></td></tr>\n");
            lastDb = (*it).left(pos);
            resultAppend("<tr valign=top><td width=25%><pre><b>");
            resultAppend(lastDb);
            resultAppend(":</b></pre></td><td width=75%><pre>");
          }
          if ((*it).length() > (unsigned int)pos+2) {
            resultAppend("<a href=\"http://define/");
            resultAppend((*it).mid(pos+2, (*it).length()-pos-3));
            resultAppend("\">");
            resultAppend((*it).mid(pos+2, (*it).length()-pos-3));
            resultAppend("</a> ");
          }
        }
      }
      resultAppend("\n</pre></td></tr></table>\n</body></html>");
      job->numFetched = 0;
    }
  }
}


QString htmlString(const QString &raw)
{
  unsigned int len=raw.length();
  QString ret;

  for (unsigned int i=0; i<len; i++) {
    switch (raw[i]) {
      case '&' :  ret += "&amp"; break;
      case '<' :  ret+="&lt;"; break;
      case '>' :  ret+="&gt;"; break;
      default  :  ret+=raw[i];
    }
  }

  return ret;
}


QString generateDefineLink(const QString &raw)
{
  QRegExp http("http://[^\\s<>()\"|\\[\\]{}]+");
  QRegExp ftp("ftp://[^\\s<>()\"|\\[\\]{}]+");
  int matchPos=0, matchLen=0;
  bool httpMatch=false;
  QString ret;

  matchPos = http.search(raw);
  matchLen = http.matchedLength();
  if (-1 != matchPos) {
    httpMatch = true;
  } else {
    matchPos = ftp.search(raw);
    matchLen = ftp.matchedLength();
    httpMatch = false;
  }

  if (-1 != matchPos) {
    ret = htmlString(raw.left(matchPos));
    ret += "<a href=\"http://";
    if (httpMatch) {
      ret += "realhttp/";
      ret += raw.mid(matchPos+7, matchLen-7);
    } else {
      ret += "realftp/";
      ret += raw.mid(matchPos+6, matchLen-6);
    }
    ret += "\">";
    ret += htmlString(raw.mid(matchPos, matchLen));
    ret += "</a>";
    ret += htmlString(raw.right(raw.length()-matchLen-matchPos));
  } else {
    ret = "<a href=\"http://define/";
    ret += raw;
    ret += "\">";
    ret += htmlString(raw);
    ret += "</a>";
  }

  return ret;
}


bool DictAsyncClient::getDefinitions()
{
  QCString lastDb,bracketBuff;
  QStrList hashList;
  char *s;
  int defCount,response;

  // html header...
  resultAppend("<body>\n");

  while (job->defines.count()>0) {
    defCount = 0;
    cmdBuffer = "";
    do {
      QStringList::iterator it = job->defines.begin();
      cmdBuffer += codec->fromUnicode(*it);
      defCount++;
      job->defines.remove(it);
    } while ((job->defines.count()>0)&&((int)cmdBuffer.length()<job->pipeSize));

    if (!sendBuffer())
      return false;

    for (;defCount > 0;defCount--) {
      if (!getNextResponse(response))
        return false;
      switch (response) {
      case 552:           // define: 552 No match
        break;
      case 150: {         // define: 150 n definitions retrieved - definitions follow
        bool defineDone = false;
        while (!defineDone) {
          if (!getNextResponse(response))
            return false;
          switch (response) {
          case 151: {      // define: 151 word database name - text follows
            char *db = strchr(thisLine, '\"');
            if (db)
              db = strchr(db+1, '\"');
            char *dbdes = 0;
            if (db) {
              db+=2;                   // db points now on database name
              dbdes = strchr(db,' ');
              if (dbdes) {
                dbdes[0] = 0;          // terminate database name
                dbdes+=2;              // dbdes points now on database description
              }
            } else {
              job->error = JobData::ErrServerError;
              job->result = QString::null;
              resultAppend(thisLine);
              doQuit();
              return false;
            }

            int oldResPos = job->result.length();

            if (((job->headLayout<2)&&(lastDb!=db))||(job->headLayout==2)) {
              lastDb = db;
              resultAppend("<p class=\"heading\">\n");
              if (dbdes)
                resultAppend(codec->toUnicode(dbdes,strlen(dbdes)-1));
              resultAppend(" [<a href=\"http://dbinfo/");
              resultAppend(db);
              resultAppend("\">");
              resultAppend(db);
              resultAppend("</a>]</p>\n");
            } else
              if (job->headLayout==1)
                resultAppend("<hr>\n");

            resultAppend("<pre><p class=\"definition\">\n");

            KMD5 context;
            bool bodyDone = false;
            while (!bodyDone) {
              if (!getNextLine())
                return false;
              char *line = thisLine;
              if (line[0]=='.') {
                if (line[1]=='.')
                  line++;        // collapse double periode into one
                else
                  if (line[1]==0)
                    bodyDone = true;
              }
              if (!bodyDone) {
                context.update(QCString(line));
                if (!bracketBuff.isEmpty()) {
                  s = strchr(line,'}');
                  if (!s)
                    resultAppend(bracketBuff.data());
                  else {
                    s[0] = 0;
                    bracketBuff.remove(0,1);       // remove '{'
                    bracketBuff += line;
                    line = s+1;
                    resultAppend(generateDefineLink(codec->toUnicode(bracketBuff)));
                  }
                  bracketBuff = "";
                }
                s = strchr(line,'{');
                while (s) {
                  resultAppend(htmlString(codec->toUnicode(line,s-line)));
                  line = s;
                  s = strchr(line,'}');
                  if (s) {
                    s[0] = 0;
                    line++;
                    resultAppend(generateDefineLink(codec->toUnicode(line)));
                    line = s+1;
                    s = strchr(line,'{');
                  } else {
                    bracketBuff = line;
                    bracketBuff += "\n";
                    line = 0;
                    s = 0;
                  }
                }
                if (line) {
                  resultAppend(htmlString(codec->toUnicode(line)));
                  resultAppend("\n");
                }
              }
            }
            resultAppend("</p></pre>\n");

            if (hashList.find(context.hexDigest())>=0)         // duplicate??
              job->result.truncate(oldResPos);              // delete the whole definition
            else {
              hashList.append(context.hexDigest());
              job->numFetched++;
            }

            break; }
          case 250: {      // define: 250 ok (optional timing information here)
            defineDone = true;
            break; }
          default: {
            handleErrors();
            return false; }
          }
        }
        break; }
      default:
        handleErrors();
        return false;
      }
    }
  }

  resultAppend("</body></html>\n");
  return true;
}


bool DictAsyncClient::match()
{
  QStringList::iterator it = job->databases.begin();
  int response;
  cmdBuffer = "";

  while (it != job->databases.end()) {
    int send = 0;
    do {
      cmdBuffer += "match ";
      cmdBuffer += codec->fromUnicode(*(it));
      cmdBuffer += " ";
      cmdBuffer += codec->fromUnicode(job->strategy);
      cmdBuffer += " \"";
      cmdBuffer += codec->fromUnicode(job->query);
      cmdBuffer += "\"\r\n";
      send++;
      ++it;
    } while ((it != job->databases.end())&&((int)cmdBuffer.length()<job->pipeSize));

    if (!sendBuffer())
      return false;

    for (;send > 0;send--) {
      if (!getNextResponse(response))
        return false;
      switch (response) {
      case 552:           // match: 552 No match
        break;
      case 152: {         // match: 152 n matches found - text follows
        bool matchDone = false;
        while (!matchDone) {
          if (!getNextLine())
            return false;
          char *line = thisLine;
          if (line[0]=='.') {
            if (line[1]=='.')
              line++;        // collapse double period into one
            else
              if (line[1]==0)
                matchDone = true;
          }
          if (!matchDone) {
            job->numFetched++;
            job->matches.append(codec->toUnicode(line));
          }
        }
        if (!nextResponseOk(250))   // match: "250 ok ..."
          return false;
        break;  }
      default:
        handleErrors();
        return false;
      }
    }
  }

  return true;
}


void DictAsyncClient::showDatabases()
{
  cmdBuffer = "show db\r\n";

  if (!sendBuffer())
    return;

  if (!nextResponseOk(110))  // show db: "110 n databases present - text follows "
    return;

  // html header...
  resultAppend("<body>\n<p class=\"heading\">\n");
  resultAppend(i18n("Available Databases:"));
  resultAppend("\n</p>\n<table width=\"100%\" cols=2>\n");

  bool done(false);
  char *line;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      resultAppend("<tr valign=top><td width=25%><pre><a href=\"http://dbinfo/");
      char *space = strchr(line,' ');
      if (space) {
        resultAppend(codec->toUnicode(line,space-line));
        resultAppend("\">");
        resultAppend(codec->toUnicode(line,space-line));
        resultAppend("</a></pre></td><td width=75%><pre>");
        line = space+1;
        if (line[0]=='"') {
          line++;                  // remove double quote
          char *quote = strchr(line, '\"');
          if (quote)
            quote[0]=0;
        }
      } else {       // hmmm, malformated line...
        resultAppend("\"></a></pre></td><td width=75%>");
      }
      resultAppend(line);
      resultAppend("</pre></td></tr>\n");
    }
  }
  resultAppend("</table>\n</body></html>");

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;
}


void DictAsyncClient::showDbInfo()
{
  cmdBuffer = "show info ";
  cmdBuffer += codec->fromUnicode(job->query);
  cmdBuffer += "\r\n";

  if (!sendBuffer())
    return;

  if (!nextResponseOk(112))     // show info db: "112 database information follows"
    return;

  // html header...
  resultAppend("<body>\n<p class=\"heading\">\n");
  resultAppend(i18n("Database Information [%1]:").arg(job->query));
  resultAppend("</p>\n<pre><p class=\"definition\">\n");

  bool done(false);
  char *line;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      resultAppend(line);
      resultAppend("\n");
    }
  }

  resultAppend("</p></pre>\n</body></html>");

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;
}


void DictAsyncClient::showStrategies()
{
  cmdBuffer = "show strat\r\n";

  if (!sendBuffer())
    return;

  if (!nextResponseOk(111))    // show strat: "111 n strategies present - text follows "
    return;

  // html header...
  resultAppend("<body>\n<p class=\"heading\">\n");
  resultAppend(i18n("Available Strategies:"));
  resultAppend("\n</p>\n<table width=\"100%\" cols=2>\n");

  bool done(false);
  char *line;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      resultAppend("<tr valign=top><td width=25%><pre>");
      char *space = strchr(line,' ');
      if (space) {
        resultAppend(codec->toUnicode(line,space-line));
        resultAppend("</pre></td><td width=75%><pre>");
        line = space+1;
        if (line[0]=='"') {
          line++;                  // remove double quote
          char *quote = strchr(line, '\"');
          if (quote)
            quote[0]=0;
        }
      } else {       // hmmm, malformated line...
        resultAppend("</pre></td><td width=75%><pre>");
      }
      resultAppend(line);
      resultAppend("</pre></td></tr>\n");
    }
  }
  resultAppend("</table>\n</body></html>");

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;
}


void DictAsyncClient::showInfo()
{
  cmdBuffer = "show server\r\n";

  if (!sendBuffer())
    return;

  if (!nextResponseOk(114))     // show server: "114 server information follows"
    return;

  // html header...
  resultAppend("<body>\n<p class=\"heading\">\n");
  resultAppend(i18n("Server Information:"));
  resultAppend("\n</p>\n<pre><p class=\"definition\">\n");

  bool done(false);
  char *line;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      resultAppend(line);
      resultAppend("\n");
    }
  }

  resultAppend("</p></pre>\n</body></html>");

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;
}


void DictAsyncClient::update()
{
  cmdBuffer = "show strat\r\nshow db\r\n";

  if (!sendBuffer())
    return;

  if (!nextResponseOk(111))    // show strat: "111 n strategies present - text follows "
    return;

  bool done(false);
  char *line;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      char *space = strchr(line,' ');
      if (space) space[0] = 0;  // terminate string, hack ;-)
      job->strategies.append(codec->toUnicode(line));
    }
  }

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;

  if (!nextResponseOk(110))  // show db: "110 n databases present - text follows "
    return;

  done = false;
  while (!done) {
    if (!getNextLine())
      return;
    line = thisLine;
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double periode into one
      else
        if (line[1]==0)
          done = true;
    }
    if (!done) {
      char *space = strchr(line,' ');
      if (space) space[0] = 0;  // terminate string, hack ;-)
      job->databases.append(codec->toUnicode(line));
    }
  }

  if (!nextResponseOk(250))   // end of transmission: "250 ok ..."
    return;
}


// connect, handshake and authorization
void DictAsyncClient::openConnection()
{
  if (job->server.isEmpty()) {
    job->error = JobData::ErrBadHost;
    return;
  }

  KExtendedSocket ks;

  ks.setAddress(job->server, job->port);
  ks.setTimeout(job->timeout);
  if (ks.connect() < 0) {
    if (ks.status() == IO_LookupError)
      job->error = JobData::ErrBadHost;
    else if (ks.status() == IO_ConnectError) {
      job->result = QString::null;
      resultAppend(KExtendedSocket::strError(ks.status(), errno));
      job->error = JobData::ErrConnect;
    } else if (ks.status() == IO_TimeOutError)
      job->error = JobData::ErrTimeout;
    else {
      job->result = QString::null;
      resultAppend(KExtendedSocket::strError(ks.status(), errno));
      job->error = JobData::ErrCommunication;
    }

    closeSocket();
    return;
  }
  tcpSocket = ks.fd();
  ks.release();

  if (!nextResponseOk(220))    // connect: "220 text capabilities msg-id"
    return;

  cmdBuffer = "client \"Kdict ";
  cmdBuffer += KDICT_VERSION;
  cmdBuffer += "\"\r\n";

  if (job->authEnabled)
    if (strstr(thisLine,"auth")) {    // skip auth if not supported
      char *msgId = strrchr(thisLine,'<');

      if ((!msgId)||(!job->user.length())) {
        job->error = JobData::ErrAuthFailed;
        closeSocket();
        return;
      }

      KMD5 context;
      context.update(QCString(msgId));
      context.update(job->secret.local8Bit());

      cmdBuffer += "auth " + job->user.local8Bit() + " ";
      cmdBuffer += context.hexDigest();
      cmdBuffer += "\r\n";
    }

  if (!sendBuffer())
    return;

  if (!nextResponseOk(250))    // client: "250 ok ..."
    return;

  if (job->authEnabled)
    if (!nextResponseOk(230))   // auth: "230 Authentication successful"
      return;
}


void DictAsyncClient::closeSocket()
{
  if (-1 != tcpSocket) {
    ::close(tcpSocket);
    tcpSocket = -1;
  }
}


// send "quit" without timeout, without checks, close connection
void DictAsyncClient::doQuit()
{
  fd_set fdsW;
  timeval tv;

  FD_ZERO(&fdsW);
  FD_SET(tcpSocket, &fdsW);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int ret = KSocks::self()->select(FD_SETSIZE, NULL, &fdsW, NULL, &tv);

  if (ret > 0) {    // we can write...
    cmdBuffer = "quit\r\n";
    int todo = cmdBuffer.length();
    KSocks::self()->write(tcpSocket,&cmdBuffer.data()[0],todo);
  }
  closeSocket();
}


// used by getNextLine()
bool DictAsyncClient::waitForRead()
{
  fd_set fdsR,fdsE;
  timeval tv;

  int ret;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn, &fdsR);
    FD_SET(tcpSocket, &fdsR);
    FD_ZERO(&fdsE);
    FD_SET(tcpSocket, &fdsE);
    FD_SET(fdPipeIn, &fdsE);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    ret = KSocks::self()->select(FD_SETSIZE, &fdsR, NULL, &fdsE, &tv);
  } while ((ret<0)&&(errno==EINTR));             // don't get tricked by signals

  if (ret == -1) {     // select failed
    if (job) {
      job->result = QString::null;
      resultAppend(strerror(errno));
      job->error = JobData::ErrCommunication;
    }
    closeSocket();
    return false;
  }
  if (ret == 0) {      // Nothing happend, timeout
    if (job)
      job->error = JobData::ErrTimeout;
    doQuit();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
      doQuit();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsE)||FD_ISSET(fdPipeIn,&fdsE)) {  // broken pipe, etc
      if (job) {
        job->result = QString::null;
        resultAppend(i18n("The connection is broken."));
        job->error = JobData::ErrCommunication;
      }
      closeSocket();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsR))  // all ok
      return true;
  }

  if (job) {
    job->result = QString::null;
    job->error = JobData::ErrCommunication;
  }
  closeSocket();
  return false;
}


// used by sendBuffer() & connect()
bool DictAsyncClient::waitForWrite()
{
  fd_set fdsR,fdsW,fdsE;
  timeval tv;

  int ret;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn, &fdsR);
    FD_SET(tcpSocket, &fdsR);
    FD_ZERO(&fdsW);
    FD_SET(tcpSocket, &fdsW);
    FD_ZERO(&fdsE);
    FD_SET(tcpSocket, &fdsE);
    FD_SET(fdPipeIn, &fdsE);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    ret = KSocks::self()->select(FD_SETSIZE, &fdsR, &fdsW, &fdsE, &tv);
  } while ((ret<0)&&(errno==EINTR));             // don't get tricked by signals

  if (ret == -1) {     // select failed
    if (job) {
      job->result = QString::null;
      resultAppend(strerror(errno));
      job->error = JobData::ErrCommunication;
    }
    closeSocket();
    return false;
  }
  if (ret == 0) {      // nothing happend, timeout
    if (job)
      job->error = JobData::ErrTimeout;
    closeSocket();
    return false;
  }
  if (ret > 0) {
    if (FD_ISSET(fdPipeIn,&fdsR)) {  // stop signal
      doQuit();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsR)||FD_ISSET(tcpSocket,&fdsE)||FD_ISSET(fdPipeIn,&fdsE)) {  // broken pipe, etc
      if (job) {
        job->result = QString::null;
        resultAppend(i18n("The connection is broken."));
        job->error = JobData::ErrCommunication;
      }
      closeSocket();
      return false;
    }
    if (FD_ISSET(tcpSocket,&fdsW))  // all ok
      return true;
  }
  if (job) {
    job->result = QString::null;
    job->error = JobData::ErrCommunication;
  }
  closeSocket();
  return false;
}


// remove start/stop signal
void DictAsyncClient::clearPipe()
{
  fd_set fdsR;
  timeval tv;
  int selectRet;
  char buf;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  do {
    FD_ZERO(&fdsR);
    FD_SET(fdPipeIn,&fdsR);
    if (1==(selectRet=select(FD_SETSIZE,&fdsR,NULL,NULL,&tv)))
      if ( ::read(fdPipeIn, &buf, 1 ) == -1 )
        ::perror( "clearPipe()" );
  } while (selectRet == 1);
}


bool DictAsyncClient::sendBuffer()
{
  int ret;
  int todo = cmdBuffer.length();
  int done = 0;

  while (todo > 0) {
    if (!waitForWrite())
      return false;
    ret = KSocks::self()->write(tcpSocket,&cmdBuffer.data()[done],todo);
    if (ret <= 0) {
      if (job) {
        job->result = QString::null;
        resultAppend(strerror(errno));
        job->error = JobData::ErrCommunication;
      }
      closeSocket();
      return false;
    } else {
      done += ret;
      todo -= ret;
    }
  }
  return true;
}


// set thisLine to next complete line of input
bool DictAsyncClient::getNextLine()
{
  thisLine = nextLine;
  nextLine = strstr(thisLine,"\r\n");
  if (nextLine) {                           // there is another full line in the inputbuffer
    nextLine[0] = 0;  // terminate string
    nextLine[1] = 0;
    nextLine+=2;
    return true;
  }
  unsigned int div = inputEnd-thisLine+1;   // hmmm, I need to fetch more input from the server...
  memmove(input,thisLine,div);      // save last, incomplete line
  thisLine = input;
  inputEnd = input+div-1;
  do {
    if ((inputEnd-input) > 9000) {
      job->error = JobData::ErrMsgTooLong;
      closeSocket();
      return false;
    }
    if (!waitForRead())
      return false;

    int received;
    do {
      received = KSocks::self()->read(tcpSocket, inputEnd, inputSize-(inputEnd-input)-1);
    } while ((received<0)&&(errno==EINTR));       // don't get tricked by signals

    if (received <= 0) {
      job->result = QString::null;
      resultAppend(i18n("The connection is broken."));
      job->error = JobData::ErrCommunication;
      closeSocket();
      return false;
    }
    inputEnd += received;
    inputEnd[0] = 0;  // terminate *char
  } while (!(nextLine = strstr(thisLine,"\r\n")));
  nextLine[0] = 0;  // terminate string
  nextLine[1] = 0;
  nextLine+=2;
  return true;
}


// reads next line and checks the response code
bool DictAsyncClient::nextResponseOk(int code)
{
  if (!getNextLine())
    return false;
  if (strtol(thisLine,0L,0)!=code) {
    handleErrors();
    return false;
  }
  return true;
}


// reads next line and returns the response code
bool DictAsyncClient::getNextResponse(int &code)
{
  if (!getNextLine())
    return false;
  code = strtol(thisLine,0L,0);
  return true;
}


void DictAsyncClient::handleErrors()
{
  int len = strlen(thisLine);
  if (len>80)
    len = 80;
  job->result = QString::null;
  resultAppend(codec->toUnicode(thisLine,len));

  switch (strtol(thisLine,0L,0)) {
    case 420:
    case 421:
      job->error = JobData::ErrNotAvailable;        // server unavailable
      break;
    case 500:
    case 501:
      job->error = JobData::ErrSyntax;             // syntax error
      break;
    case 502:
    case 503:
      job->error = JobData::ErrCommandNotImplemented;    // command not implemented
      break;
    case 530:
      job->error = JobData::ErrAccessDenied;        // access denied
      break;
    case 531:
      job->error = JobData::ErrAuthFailed;           // authentication failed
      break;
    case 550:
    case 551:
      job->error = JobData::ErrInvalidDbStrat;       // invalid strategy/database
      break;
    case 554:
      job->error = JobData::ErrNoDatabases;          // no databases
      break;
    case 555:
      job->error = JobData::ErrNoStrategies;         // no strategies
      break;
    default:
      job->error = JobData::ErrServerError;
  }
  doQuit();
}


void DictAsyncClient::resultAppend(const char* str)
{
  if (job)
    job->result += codec->toUnicode(str);
}


void DictAsyncClient::resultAppend(QString str)
{
  if (job)
    job->result += str;
}



//********* DictInterface ******************************************

DictInterface::DictInterface()
: newServer(false), clientDoneInProgress(false)
{
  if (::pipe(fdPipeIn ) == -1 ) {
    perror( "Creating in pipe" );
    KMessageBox::error(global->topLevel, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }
  if (::pipe(fdPipeOut ) == -1 ) {
    perror( "Creating out pipe" );
    KMessageBox::error(global->topLevel, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }

  if (-1 == fcntl(fdPipeIn[0],F_SETFL,O_NONBLOCK)) {  // make socket non-blocking
    perror("fcntl()");
    KMessageBox::error(global->topLevel, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }

  if (-1 == fcntl(fdPipeOut[0],F_SETFL,O_NONBLOCK)) {  // make socket non-blocking
    perror("fcntl()");
    KMessageBox::error(global->topLevel, i18n("Internal error:\nFailed to open pipes for internal communication."));
    kapp->exit(1);
  }

  notifier = new QSocketNotifier(fdPipeIn[0],QSocketNotifier::Read,this);
  connect(notifier,SIGNAL(activated(int)),this,SLOT(clientDone()));

  // initialize the KSocks stuff in the main thread, otherwise we get
  // strange effects on FreeBSD
  (void) KSocks::self();

  client = new DictAsyncClient(fdPipeOut[0],fdPipeIn[1]);
  if (0!=pthread_create(&threadID,0,&(client->startThread),client)) {
    KMessageBox::error(global->topLevel, i18n("Internal error:\nUnable to create thread."));
    kapp->exit(1);
  }

  jobList.setAutoDelete(true);
}


DictInterface::~DictInterface()
{
  disconnect(notifier,SIGNAL(activated(int)),this,SLOT(clientDone()));

  if (0!=pthread_cancel(threadID))
    kdWarning() << "pthread_cancel failed!" << endl;
  if (0!=pthread_join(threadID,NULL))
    kdWarning() << "pthread_join failed!" << endl;
  delete client;

  if ( ::close( fdPipeIn[0] ) == -1 ) {
    perror( "Closing fdPipeIn[0]" );
  }
  if ( ::close( fdPipeIn[1] ) == -1 ) {
    perror( "Closing fdPipeIn[1]" );
  }
  if ( ::close( fdPipeOut[0] ) == -1 ) {
    perror( "Closing fdPipeOut[0]" );
  }
  if ( ::close( fdPipeOut[1] ) == -1 ) {
    perror( "Closing fdPipeOut[1]" );
  }
}


// inform the client when server settings get changed
void DictInterface::serverChanged()
{
  newServer = true;
}


// cancel all pending jobs
void DictInterface::stop()
{
  if (jobList.isEmpty()) {
    return;
  } else {
    while (jobList.count()>1)  // not yet started jobs can be deleted directly
      jobList.removeLast();

    if (!clientDoneInProgress) {
      jobList.getFirst()->canceled = true;   // clientDone() now ignores the results of this job
      char buf;                             // write one char in the pipe to the async thread
      if (::write(fdPipeOut[1],&buf,1) == -1)
        ::perror( "stop()" );
    }
  }
}


void DictInterface::define(const QString &query)
{
  JobData *newJob = generateQuery(JobData::TDefine,query);
  if (newJob)
    insertJob(newJob);
}


void DictInterface::getDefinitions(QStringList query)
{
  JobData *newjob = new JobData(JobData::TGetDefinitions,newServer,global->server,global->port,
                                global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                                global->user,global->secret,global->headLayout);
  newjob->defines = query;
  newServer = false;
  insertJob(newjob);
}


void DictInterface::match(const QString &query)
{
  JobData *newJob = generateQuery(JobData::TMatch,query);

  if (newJob) {
    if (global->currentStrategy == 0)
      newJob->strategy = ".";        // spell check strategy
    else
      newJob->strategy = global->strategies[global->currentStrategy].utf8();

    insertJob(newJob);
  }
}


// fetch detailed db info
void DictInterface::showDbInfo(const QString &db)
{
  QString ndb = db.simplifyWhiteSpace();   // cleanup query string
  if (ndb.isEmpty())
    return;
  if (ndb.length()>100)               // shorten if necessary
    ndb.truncate(100);
  JobData *newjob = new JobData(JobData::TShowDbInfo,newServer,global->server,global->port,
                                global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                                global->user,global->secret,global->headLayout);
  newServer = false;
  newjob->query = ndb;            // construct job...
  insertJob(newjob);
}


void DictInterface::showDatabases()
{
  insertJob( new JobData(JobData::TShowDatabases,newServer,global->server,global->port,
                         global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                         global->user,global->secret,global->headLayout));
  newServer = false;
}


void DictInterface::showStrategies()
{
  insertJob( new JobData(JobData::TShowStrategies,newServer,global->server,global->port,
                         global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                         global->user,global->secret,global->headLayout));
  newServer = false;
}


void DictInterface::showInfo()
{
  insertJob( new JobData(JobData::TShowInfo,newServer,global->server,global->port,
                         global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                         global->user,global->secret,global->headLayout));
  newServer = false;
}


// get info about databases & stratgies the server knows
void DictInterface::updateServer()
{
  insertJob( new JobData(JobData::TUpdate,newServer,global->server,global->port,
                         global->idleHold,global->timeout,global->pipeSize, global->encoding,global->authEnabled,
                         global->user,global->secret,global->headLayout));
  newServer = false;
}


// client-thread ended
void DictInterface::clientDone()
{
  QString message;

  cleanPipes();      // read from pipe so that notifier doesn´t fire again

  if (jobList.isEmpty()) {
    kdDebug(5004) << "This shouldn´t happen, the client-thread signaled termination, but the job list is empty" << endl;
    return;       // strange..
  }

  clientDoneInProgress = true;
  QStringList::iterator it;
  JobData* job = jobList.getFirst();
  if (!job->canceled) {  // non-interupted job?
    if (JobData::ErrNoErr == job->error) {
      switch (job->type) {
      case JobData::TUpdate :
        global->serverDatabases.clear();
        for (it = job->databases.begin(); it != job->databases.end(); ++it)
          global->serverDatabases.append(*it);
        global->databases = global->serverDatabases;
        for (int i = global->databaseSets.count()-1;i>=0;i--)
          global->databases.prepend(global->databaseSets.at(i)->first());
        global->databases.prepend(i18n("All Databases"));
        global->currentDatabase = 0;

        global->strategies.clear();
        for (it = job->strategies.begin(); it != job->strategies.end(); ++it)
          global->strategies.append(*it);
        global->strategies.prepend(i18n("Spell Check"));
        global->currentStrategy = 0;
        message = i18n(" Received database/strategy list ");
        emit stopped(message);
        emit infoReady();
        break;
      case JobData::TDefine:
      case JobData::TGetDefinitions:
        if (job->type == JobData::TDefine) {
          switch (job->numFetched) {
          case 0:
            message = i18n("No definitions found");
            break;
          case 1:
            message = i18n("One definition found");
            break;
          default:
            message = i18n("%1 definitions found").arg(job->numFetched);
          }
        } else {
          switch (job->numFetched) {
          case 0:
            message = i18n(" No definitions fetched ");
            break;
          case 1:
            message = i18n(" One definition fetched ");
            break;
          default:
            message = i18n(" %1 definitions fetched ").arg(job->numFetched);
          }
        }
        emit stopped(message);
        emit resultReady(job->result, job->query);
        break;
      case JobData::TMatch:
        switch (job->numFetched) {
        case 0:
          message = i18n(" No matching definitions found ");
          break;
        case 1:
          message = i18n(" One matching definition found ");
          break;
        default:
          message = i18n(" %1 matching definitions found ").arg(job->numFetched);
        }
        emit stopped(message);
        emit matchReady(job->matches);
        break;
      default :
        message = i18n(" Received information ");
        emit stopped(message);
        emit resultReady(job->result, job->query);
      }
    } else {
      QString errMsg;
      switch (job->error) {
      case JobData::ErrCommunication:
        errMsg = i18n("Communication error:\n\n");
        errMsg += job->result;
        break;
      case JobData::ErrTimeout:
        errMsg = i18n("A delay occurred which exceeded the\ncurrent timeout limit of %1 seconds.\nYou can modify this limit in the Preferences Dialog.").arg(global->timeout);
        break;
      case JobData::ErrBadHost:
        errMsg = i18n("Unable to connect to:\n%1:%2\n\nCannot resolve hostname.").arg(job->server).arg(job->port);
        break;
      case JobData::ErrConnect:
        errMsg = i18n("Unable to connect to:\n%1:%2\n\n").arg(job->server).arg(job->port);
        errMsg += job->result;
        break;
      case JobData::ErrRefused:
        errMsg = i18n("Unable to connect to:\n%1:%2\n\nThe server refused the connection.").arg(job->server).arg(job->port);
        break;
      case JobData::ErrNotAvailable:
        errMsg = i18n("The server is temporarily unavailable.");
        break;
      case JobData::ErrSyntax:
        errMsg = i18n("The server reported a syntax error.\nThis shouldn't happen -- please consider\nwriting a bug report.");
        break;
      case JobData::ErrCommandNotImplemented:
        errMsg = i18n("A command that Kdict needs isn't\nimplemented on the server.");
        break;
      case JobData::ErrAccessDenied:
        errMsg = i18n("Access denied.\nThis host is not allowed to connect.");
        break;
      case JobData::ErrAuthFailed:
        errMsg = i18n("Authentication failed.\nPlease enter a valid username and password.");
        break;
      case JobData::ErrInvalidDbStrat:
        errMsg = i18n("Invalid database/strategy.\nYou probably need to use Server->Get Capabilities.");
        break;
      case JobData::ErrNoDatabases:
        errMsg = i18n("No databases available.\nIt is possible that you need to authenticate\nwith a valid username/password combination to\ngain access to any databases.");
        break;
      case JobData::ErrNoStrategies:
        errMsg = i18n("No strategies available.");
        break;
      case JobData::ErrServerError:
        errMsg = i18n("The server sent an unexpected reply:\n\"%1\"\nThis shouldn't happen, please consider\nwriting a bug report").arg(job->result);
        break;
      case JobData::ErrMsgTooLong:
        errMsg = i18n("The server sent a response with a text line\nthat was too long.\n(RFC 2229: max. 1024 characters/6144 octets)");
        break;
      case JobData::ErrNoErr:    // make compiler happy
        errMsg = i18n("No Errors");
      }
      message = i18n(" Error ");
      emit stopped(message);
      KMessageBox::error(global->topLevel, errMsg);
    }
  } else {
    message = i18n(" Stopped ");
    emit stopped(message);
  }

  clientDoneInProgress = false;

  client->removeJob();
  jobList.removeFirst();         // this job is now history
  if (!jobList.isEmpty())        // work to be done?
    startClient();                  // => restart client
}


JobData* DictInterface::generateQuery(JobData::QueryType type, QString query)
{
  query = query.simplifyWhiteSpace();   // cleanup query string
  if (query.isEmpty())
    return 0L;
  if (query.length()>300)               // shorten if necessary
    query.truncate(300);
  query = query.replace(QRegExp("[\"\\]"), "");  // remove remaining illegal chars...
  if (query.isEmpty())
    return 0L;

  JobData *newjob = new JobData(type,newServer,global->server,global->port,
                                global->idleHold,global->timeout,global->pipeSize, global->encoding, global->authEnabled,
                                global->user,global->secret,global->headLayout);
  newServer = false;
  newjob->query = query;            // construct job...

  if (global->currentDatabase == 0)     // all databases
    newjob->databases.append("*");
  else {
    if ((global->currentDatabase > 0)&&       // database set
        (global->currentDatabase < global->databaseSets.count()+1)) {
      for (int i = 0;i<(int)global->serverDatabases.count();i++)
        if ((global->databaseSets.at(global->currentDatabase-1))->findIndex(global->serverDatabases[i])>0)
          newjob->databases.append(global->serverDatabases[i].utf8().data());
      if (newjob->databases.count()==0) {
        KMessageBox::sorry(global->topLevel, i18n("Please select at least one database."));
        delete newjob;
        return 0L;
      }
    } else {                               // one database
      newjob->databases.append(global->databases[global->currentDatabase].utf8().data());
    }
  }

  return newjob;
}


void DictInterface::insertJob(JobData* job)
{
  if (jobList.isEmpty()) {     // Client has nothing to do, start directly
    jobList.append(job);
    startClient();
  } else {                      // there are other pending jobs...
    stop();
    jobList.append(job);
  }
}


// start client-thread
void DictInterface::startClient()
{
  cleanPipes();
  if (jobList.isEmpty()) {
    kdDebug(5004) << "This shouldn´t happen, startClient called, but clientList is empty" << endl;
    return;
  }

  client->insertJob(jobList.getFirst());
  char buf;                           // write one char in the pipe to the async thread
  if (::write(fdPipeOut[1],&buf,1) == -1)
    ::perror( "startClient()" );

  QString message;
  switch (jobList.getFirst()->type) {
   case JobData::TDefine:
   case JobData::TGetDefinitions:
   case JobData::TMatch:
     message = i18n(" Querying server... ");
     break;
   case JobData::TShowDatabases:
   case JobData::TShowStrategies:
   case JobData::TShowInfo:
   case JobData::TShowDbInfo:
     message = i18n(" Fetching information... ");
     break;
   case JobData::TUpdate:
     message = i18n(" Updating server information... ");
     break;
  }
  emit started(message);
}


// empty the pipes, so that notifier stops firing
void DictInterface::cleanPipes()
{
  fd_set rfds;
  struct timeval tv;
  int ret;
  char buf;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  do {
    FD_ZERO(&rfds);
    FD_SET(fdPipeIn[0],&rfds);
    if (1==(ret=select(FD_SETSIZE,&rfds,NULL,NULL,&tv)))
      if ( ::read(fdPipeIn[0], &buf, 1 ) == -1 )
        ::perror( "cleanPipes" );
  } while (ret == 1);

  do {
    FD_ZERO(&rfds);
    FD_SET(fdPipeOut[0],&rfds);
    if (1==(ret=select(FD_SETSIZE,&rfds,NULL,NULL,&tv)))
      if ( ::read(fdPipeOut[0], &buf, 1 ) == -1 )
        ::perror( "cleanPipes" );
  } while (ret == 1);
}

//--------------------------------

#include "dict.moc"
