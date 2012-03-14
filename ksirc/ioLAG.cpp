/**********************************************************************

 The IO LAG Controller

 $$Id: ioLAG.cpp 234524 2003-06-29 15:21:53Z mlaurent $$

**********************************************************************/

#include "ioLAG.h"
#include "control_message.h"
#include "ksircprocess.h"

#include <kdebug.h>

KSircIOLAG::KSircIOLAG(KSircProcess *_proc)
  : QObject(),
    KSircMessageReceiver(_proc)
{
  proc = _proc;
  setBroadcast(FALSE);
  startTimer(30000);
//  startTimer(5000);
//(proc->getWindowList())["!all"]->control_message(SET_LAG, "99");
}


KSircIOLAG::~KSircIOLAG()
{
  killTimers();
}

void KSircIOLAG::sirc_receive(QCString str, bool)
{

  if(str.contains("*L*")){
    int s1, s2;
    s1 = str.find("*L* ") + 4;
    s2 = str.length();
    if(s1 < 0 || s2 < 0){
      kdDebug(5008) << "Lag mesage broken: " << str << endl;
      return;
    }
    QString lag = str.mid(s1, s2 - s1);
    //    cerr << "Lag: " << str << endl;
    //    cerr << "Setting lag to: " << lag << endl;
    (proc->getWindowList())["!all"]->control_message(SET_LAG, lag);
  }

}

void KSircIOLAG::control_message(int, QString)
{
}

void KSircIOLAG::timerEvent ( QTimerEvent * )
{
  QCString cmd = "/lag\n";
  emit outputLine(cmd);
}

#include "ioLAG.moc"
