#ifndef USERCONTROLLERMENU_H
#define USERCONTROLLERMENU_H

#include <qstring.h>
#include <qptrlist.h>

class UserControlMenu {
 public:
  /**
      Basic constructor, everything defaults to a Seperator.  All data
      is public so there is no problems changing it in the future.
      Data is copied internally and deleted when finished.
  */
  UserControlMenu(const QString& _title = 0,
		  const QString& _action = 0,
		  int _accel = 0,
		  int _type = 0,
		  bool _op_only = FALSE);

  ~UserControlMenu();

  static QPtrList<UserControlMenu> UserMenu;

  static QPtrList<UserControlMenu> *parseKConfig();
  static void writeKConfig();

  /**
    * title in the popup menu
  */
  QString title;
  /**
    * Action to preform, must be of the format "blah lbah %s blah"
    * where %s will be expanded into the selected nick.
    */
  QString action;

  /**
    * Accelerator key, currently does nothing.
    */
  int accel;
  /**
    * is this function only available to ops?
    */
  bool op_only;
  /**
    * What type of menu item is this? a Seperator of Text?
    */
  enum itype { Seperator, Text } type;
};


#endif
