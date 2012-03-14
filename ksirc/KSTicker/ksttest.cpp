#include <kapplication.h>
#include <kconfig.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qregexp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <unistd.h>

#include "ksticker.h"
#include "ksttest.h"
#include "../ksopts.h"

KConfig *kConfig;

StdInTicker::StdInTicker()
  : KSTicker()
{
  kConfig->setGroup("defaults");
  QFont font;
  font = kConfig->readFontEntry("font");
  font.setFixedPitch(TRUE);
  setFont(font);
  setSpeed(kConfig->readNumEntry("tick", 30),
	   kConfig->readNumEntry("step", 3));
}

StdInTicker::~StdInTicker()
{
  int tick, step;
  speed(&tick, &step);
  kConfig->setGroup("defaults");
  kConfig->writeEntry("font", KSTicker::font());
  kConfig->writeEntry("tick", tick);
  kConfig->writeEntry("step", step);
  kConfig->writeEntry("text", colorGroup().text() );
  kConfig->writeEntry("background", colorGroup().background() );
  kConfig->sync();
}

void StdInTicker::readsocket(int socket)
{
  char buf[1024];
  int bytes = read(socket, buf, 1024);
  if(bytes){
      QCString str(buf, bytes);
      str.replace(QRegExp("\n"), " // ");
      mergeString(str);
  }
}

void StdInTicker::end()
{
    delete this;
}

void StdInTicker::closeEvent ( QCloseEvent *e )
{
  KSTicker::closeEvent(e);
  delete this;
}


int main(int argc, char **argv){
    KAboutData aboutData( "ksirc", I18N_NOOP("KSirc"),
                          "2.0.0", "", KAboutData::License_Artistic,
                          I18N_NOOP("(c) 1997-2002, Andrew Stanley-Jones"));
    aboutData.addAuthor("Andrew Stanley-Jones",I18N_NOOP("Original Author"), "asj-ksirc@cban.com");
    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication a(argc, argv);

    kConfig = a.config();

    // Options
    KSOptions opts;
    opts.load();

    StdInTicker *kst = new StdInTicker();
    QSocketNotifier *sn = new QSocketNotifier(0, QSocketNotifier::Read);
    QObject::connect(sn, SIGNAL(activated(int)),
                     kst, SLOT(readsocket(int)));
    QObject::connect(kst, SIGNAL(doubleClick()), kst, SLOT(end()));
    QObject::connect(kst, SIGNAL(closing()), kst, SLOT(end()));
    a.setMainWidget(kst);
    kst->show();
    return a.exec();
}

#include "ksttest.moc"

