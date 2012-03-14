#include <kapplication.h>

#include "dccManager.h"

class Timer : public QObject
{
public:
    Timer( dccManager *view )
        : m_view( view )
    {
        m_it = m_view->newSendItem("test", "boren", dccItem::dccSentOffer, 123523);
        m_view->newSendItem("test2", "boren", dccItem::dccSentOffer, 123523);
        m_view->newSendItem("test3", "boren", dccItem::dccSentOffer, 123523);
        m_view->newGetItem("test4", "boren", dccItem::dccGotOffer, 123523);
        m_view->newGetItem("test4", "boren", dccItem::dccGotOffer, 123523);
        m_view->newGetItem("test4", "boren", dccItem::dccGotOffer, 123523);
        bytes = 0;
        startTimer( 100 );
    }
    void timerEvent( QTimerEvent * )
        {
            m_it->changeStatus(dccItem::dccSending);
            bytes += 1000;
            m_it->setReceivedBytes(bytes);

        }
private:
    dccManager *m_view;
    int bytes;
    dccItem *m_it;
};


int main(int argc, char **argv){
    KApplication a(argc, argv, "dccMgrTest" );

    dccManager kst(0, "toplevel");
    Timer t(&kst);

    a.setMainWidget(&kst);
    kst.show();
    return a.exec();
}

