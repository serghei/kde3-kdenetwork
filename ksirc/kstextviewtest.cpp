#include <kapplication.h>

#include "kstextview.h"


class Timer : public QObject
{
public:
    Timer( KSirc::TextView *view )
	: m_view( view ), m_lines( 0 )
    {
        //startTimer( 5000 );
    }
    void timerEvent( QTimerEvent * )
    {
	m_lines++;
	if ( m_lines > 15 )
	    m_view->removeParag( m_view->firstParag() );
        m_view->appendParag( QString::fromLatin1( "<font color=#0000ff>color test</font><font color=#00ff00>color test</font><font color=#ff0000>color test</font><font color=#000088>color test</font><font color=#008800>color test</font><font color=#880000>color test</font><font color=#0000ff>color test</font><font color=#02ff00>color test</font><font color=#2000ff>color test</font>" ) );
        m_view->appendParag( QString::fromLatin1( "<r>aaaaaa</r> bbbbbb cccccccc ddddddddd eeeeeee " ) );
	m_view->setContentsPos( 0, m_view->contentsHeight() );
    }
private:
    KSirc::TextView *m_view;
    int m_lines;
};

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kstextviewtest" );

    KSirc::TextView view( 0 );
    app.setMainWidget( &view );

    view.show();
    view.appendParag( QString::fromLatin1( "<b>bold</b>Normal<i>Italic hey!</i><r>reverse!</r>" ) );
    view.appendParag( QString::fromLatin1( "bleh foo hey bar schubidu bliep blop" ) );
    view.appendParag( QString::fromLatin1( "more longish paragraphs .. yaddayadda yadda" ) );
    view.appendParag( QString::fromLatin1( "&lt;tronical&gt; hah! &amp;" ) );
    view.appendParag( QString::fromLatin1( "<a href=\"http://www.kde.org/\">url</a>" ) );
    view.appendParag( QString::fromLatin1( "<font color=#0000ff bgcolor=#ff0000>color test</font>" ) );
    view.appendParag( QString::fromLatin1( "<font color=\"#00ff00\">another color test</font>" ) );
    view.appendParag( QString::fromLatin1( "<font color=\"#00ff00\" bgcolor=\"#ff0000\">another color test</font>" ) );
    view.appendParag( QString::fromLatin1( "<i bleh>simple attr test</i><b leh foo=15 doo=\"&amp; > 123 \" oof>complicated attr test</b>" ) );
    view.appendParag( QString::fromLatin1( "\"<b doo=\"&amp; > 123 \" oof>\"third attr test 2</b>\"" ) );
    view.appendParag( QString::fromLatin1( "aaaaaa bbbbbb cccccccc ddddddddd eeeeeee" ) );
    Timer t( &view );

    return app.exec();
}

