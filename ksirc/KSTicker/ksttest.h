#ifndef _KST_STD_IN_
#define _KST_STD_IN_


#include "ksticker.h"

class StdInTicker : public KSTicker
{
  Q_OBJECT
public:
  StdInTicker();
  ~StdInTicker();

protected:
  void closeEvent ( QCloseEvent * );

public slots:
    void readsocket(int socket);
    void end();
  
};

#endif

