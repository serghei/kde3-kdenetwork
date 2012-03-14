
#ifndef KMESSAGERECEIVER_H
#define KMESSAGERECEIVER_H

#include <qstring.h>
#include <qptrlist.h>

class KSircProcess;
class QTextCodec;

struct filterRule {
    const char *desc;
    const char *search;
    const char *from;
    const char *to;
};

typedef QPtrList<filterRule> filterRuleList;

class KSircMessageReceiver
{
public:
  KSircMessageReceiver(KSircProcess *_proc);
  virtual ~KSircMessageReceiver();

    virtual void sirc_receive(QCString str, bool broadcast = false) = 0;

    virtual void control_message(int, QString) = 0;

  bool getBroadcast();
  void setBroadcast(bool bd);

  virtual filterRuleList *defaultRules();

  KSircProcess *ksircProcess() const { return proc; }

private:
  KSircProcess *proc;
  bool broadcast;

};

class UnicodeMessageReceiver : public KSircMessageReceiver
{
public:
    UnicodeMessageReceiver( KSircProcess *process );

    virtual void sirc_receive(QCString str, bool broadcast = false);
    virtual void sirc_receive(QString str, bool broadcast = false) = 0;

protected:
    void setEncoding( const QString &encoding );
    QString encoding() const;

    QTextCodec *encoder() const;

private:
    QString m_encoding;
    QTextCodec *m_encoder;
};

#endif
