#ifndef _PAGE_FONT
#define _PAGE_FONT
#include <kfontdialog.h> // For the font selection widget
#include <qlayout.h> // For the layout
#include "ksopts.h"  // For storing the info.

/**
 * A page for the preferences dialog to set the standard font
 *
 * @author Markus Weimer <markus.weimer@web.de>
 */
class PageFont : public QWidget
{
 Q_OBJECT

 public:
  /**
   * Create the Widget
   */
  PageFont( QWidget *parent = 0, const char *name = 0 );


  /*
   * Standard destructor
   */
  ~PageFont();


  /**
   * Save the config set by the user to the global ksopts object.
   *
   */
  void saveConfig( void );


  /**
   * Reset the current user config to the one stored in the global
   * ksopts object.
   *
   */
  void defaultConfig( void );


  /**
   * Read the configuration from the given KSOptions object
   *
   * @param opts the KSOptions object to modify
   */
  void readConfig( const KSOColors* opts=ksopts );

 public slots:
  void update( void );

 signals:
  void modified();

 private:
  KFontChooser* fontchooser; /** The font choosing widget from kdelib */
  QHBoxLayout* layout;
};

#endif
