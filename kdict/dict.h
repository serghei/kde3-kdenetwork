/* -------------------------------------------------------------

   dict.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias Hölzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   JobData          used for data transfer between Client and Interface
   DictAsyncClient  all network related stuff happens here in an asynchrous thread
   DictInterface    interface for DictAsyncClient, job management

   -------------------------------------------------------------*/

#ifndef _DICT_H_
#define _DICT_H_

#include <pthread.h>
#include <qptrlist.h>
#include <qsocketnotifier.h>

class QSocketNotifier;
struct in_addr;


//********* JobData ******************************************


class JobData
{

public:
  
  enum QueryType {            //type of transaction
    TDefine=0,
    TGetDefinitions,
    TMatch,
    TShowDatabases,
    TShowDbInfo,
    TShowStrategies,
    TShowInfo,
    TUpdate
  };

  enum ErrType {             // error codes
    ErrNoErr=0,
    ErrCommunication,        // display result!
    ErrTimeout,
    ErrBadHost,
    ErrConnect,              // display result!
    ErrRefused,
    ErrNotAvailable,
    ErrSyntax,
    ErrCommandNotImplemented,
    ErrAccessDenied,
    ErrAuthFailed,
    ErrInvalidDbStrat,
    ErrNoDatabases,
    ErrNoStrategies,
    ErrServerError,          // display result!
    ErrMsgTooLong
  };
  
  JobData(QueryType Ntype,bool NnewServer,QString const& Nserver,int Nport,
          int NidleHold, int Ntimeout, int NpipeSize, QString const& Nencoding, bool NAuthEnabled,
          QString const& Nuser, QString const& Nsecret, unsigned int NheadLayout);

  QueryType type;
  ErrType error;
  
  bool canceled;
  int numFetched;
  QString result;
  QStringList matches;

  QString query;
  QStringList defines;

  bool newServer;
  QString server;
  int port, timeout, pipeSize, idleHold;
  QString encoding;
  bool authEnabled;
  QString user, secret;
  QStringList databases,strategies;
  QString strategy;
  unsigned int headLayout;
};


//********* DictAsyncClient ******************************************


class DictAsyncClient
{
  
public:

  DictAsyncClient(int NfdPipeIn, int NfdPipeOut);
  ~DictAsyncClient();

  static void* startThread(void* pseudoThis);

  void insertJob(JobData *newJob);
  void removeJob(); 
  
private:

  void waitForWork();       // main loop
  void define();
  bool getDefinitions();
  bool match();
  void showDatabases();
  void showDbInfo();
  void showStrategies();
  void showInfo();
  void update();

  void openConnection();       // connect, handshake and authorization
  void closeSocket();
  void doQuit();               // send "quit" without timeout, without checks, close connection
  bool waitForRead();          // used by getNextIntoBuffer()
  bool waitForWrite();         // used by sendBuffer() & connect()
  void clearPipe();            // remove start/stop signal

  bool sendBuffer();           // send cmdBuffer to the server
  bool getNextLine();          // set thisLine to next complete line of input
  bool nextResponseOk(int code); // reads next line and checks the response code
  bool getNextResponse(int &code); // reads next line and returns the response code
  void handleErrors();

  void resultAppend(const char* str);
  void resultAppend(QString str);

  JobData *job;
  char *input;
  QCString  cmdBuffer;
  const unsigned int inputSize;
  char *thisLine, *nextLine, *inputEnd;
  int fdPipeIn,fdPipeOut;      //IPC-Pipes to/from async thread
  int tcpSocket,timeout,idleHold;
  QTextCodec *codec;
};


//********* DictInterface *************************************************

class DictInterface : public QObject
{
  Q_OBJECT
 
public:
  
  DictInterface();
  ~DictInterface();

public slots:

  void serverChanged();     // inform the client when server settings get changed
  void stop();              // cancel all pending jobs
   
  void define(const QString &query);
  void getDefinitions(QStringList query);
  void match(const QString &query);
  void showDbInfo(const QString &db); // fetch detailed db info
  void showDatabases();        // fetch misc. info...
  void showStrategies();
  void showInfo();
  void updateServer();         // get info about databases & strategies the server knows
    
signals:

  void infoReady();                   // updateServer done
  void resultReady(const QString &result, const QString &query);     // define done
  void matchReady(const QStringList &result);  // match done
  void started(const QString &message);      // Client is active now, activate indicator
  void stopped(const QString &message);      // Client is now halted, deactivate indicator

private slots:

  void clientDone();

private:

  JobData* generateQuery(JobData::QueryType type, QString query);
  void insertJob(JobData* job);  // insert in job list, if nesscary cancel/remove previous jobs
  void startClient();            // send start signal
  void cleanPipes();             // empty the pipes, so that notifier stops firing

  QSocketNotifier *notifier;
  int fdPipeIn[2],fdPipeOut[2];    //IPC-Pipes to/from async thread
  pthread_t threadID;
  DictAsyncClient *client;
  QPtrList<JobData> jobList;
  bool newServer,clientDoneInProgress;
};

extern DictInterface *interface;

#endif
