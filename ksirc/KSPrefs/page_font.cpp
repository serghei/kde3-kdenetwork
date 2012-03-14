#include "page_font.h"
#include "qapplication.h"
PageFont::PageFont( QWidget *parent, const char *name ) : 
  QWidget( parent, name)
{
  layout = new QHBoxLayout(this);
  fontchooser = new KFontChooser(this);
  layout->addWidget(fontchooser);
  connect(fontchooser,SIGNAL(fontSelected ( const QFont&)), this, SLOT(update())); 
}

PageFont::~PageFont( )
{
  
}

void PageFont::update( void ){
  emit modified();
}

void PageFont::saveConfig( void )
{
  ksopts->defaultFont = fontchooser->font();
  QApplication::setFont(fontchooser->font(), true, "KSirc::TextView" );
}

void PageFont::readConfig(  const KSOColors * opts )
{
  /* Just set the font from the preferences */
  fontchooser->setFont( opts->defaultFont );
}

void PageFont::defaultConfig( void )
{
  fontchooser->setFont( ksopts->defaultFont );
}

#include "page_font.moc"
