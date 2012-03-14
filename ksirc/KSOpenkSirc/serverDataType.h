#ifndef SERVERDATATYPE_H
#define SERVERDATATYPE_H

#include <qptrlist.h>
#include <qstring.h>

class port
{

public:
  port( const QString &portnum ) { p=portnum; }
  QString portnum() const { return p; }

private:
  QString p;

};


class Server
{

public:
    Server( const QString &group,
	    const QString &server,
	    QPtrList<port> ports,
	    const QString &serverdesc,
	    const QString &script,
	    const QString &password  = QString::null,
            bool dossl = false
	) {
          g=group; s=server; p=ports; sd=serverdesc; sc=script;
	  pass=password;
	  ssl = dossl;
	  p.setAutoDelete(TRUE);
          }
  QString group() const      { return g; }
  QString server() const     { return s; }
  QPtrList<port> ports() const      { return p; }
  QString serverdesc() const { return sd; }
  QString script() const     { return sc; }
  QString password() const   { return pass; }
  bool    usessl() const     { return ssl; }

private:
  QString     g;
  QString     s;
  QPtrList<port> p;
  QString     sd;
  QString     sc;
  QString     pass;
  bool        ssl;

};

#endif
