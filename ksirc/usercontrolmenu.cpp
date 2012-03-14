#include "usercontrolmenu.h"

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>

QPtrList<UserControlMenu> UserControlMenu::UserMenu;

UserControlMenu::UserControlMenu(const QString& _title,
				 const QString& _action,
				 int _accel,
				 int _type,
				 bool _op_only)
{
  title = _title;
  action = _action;
  accel = _accel;
  type = (itype) _type;
  op_only = _op_only;
}

UserControlMenu::~UserControlMenu()
{
}

QPtrList<UserControlMenu> *UserControlMenu::parseKConfig()
{

  // Reset the UserMenu to nothing before we start.

  UserMenu.clear();

  KConfig *kConfig = kapp->config();
  kConfig->setGroup("UserMenu");
  int items = kConfig->readNumEntry("Number");
  if(items == 0){

    // We found nothing, so let's use some defaults.

    UserMenu.setAutoDelete(TRUE);
    UserMenu.append(new UserControlMenu(i18n("&Refresh Nicks"),
					 "refresh",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu);
    UserMenu.append(new UserControlMenu(i18n("&Follow"),
					 "follow $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu(i18n("&UnFollow"),
					 "unfollow $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a separator
    UserMenu.append(new UserControlMenu(i18n("&Whois"),
					 "/whois $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu(i18n("&Ping"),
					 "/ping $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu(i18n("V&ersion"),
					 "/ctcp $$dest_nick VERSION",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a separator
    UserMenu.append(new UserControlMenu(i18n("&Abuse"),
					 "/me slaps $$dest_nick around with a small 50lb Unix Manual",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a separator
    UserMenu.append(new UserControlMenu(i18n("&Kick"),
					 "/kick $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu(i18n("&Ban"),
					 "/ban $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu(i18n("U&nBan"),
					 "/unban $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu());
    UserMenu.append(new UserControlMenu(i18n("&Op"),
					 "/op $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu(i18n("&Deop"),
					 "/deop $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu());
    UserMenu.append(new UserControlMenu(i18n("&Voice"),
                                         "/mode $$dest_chan +v $$dest_nick",
                                         0,
                                         UserControlMenu::Text,
                                         TRUE));
    UserMenu.append(new UserControlMenu(i18n("Devo&ice"),
                                         "/mode $$dest_chan -v $$dest_nick",
                                         0,
                                         UserControlMenu::Text,
                                         TRUE));
  }
  else{
    QString key, cindex, title, action;
    int accel, type, oponly;
    for(int i = 0; i < items; i++){
      cindex.sprintf("%d", i);
      key = "MenuType-" + cindex;
      type = kConfig->readNumEntry(key);
      if(type == UserControlMenu::Seperator)
	UserMenu.append(new UserControlMenu());
      else if(type == UserControlMenu::Text){
	key = "MenuTitle-" + cindex;
	title = kConfig->readEntry(key);
	key = "MenuAction-" + cindex;
	action = kConfig->readEntry(key);
	key = "MenuAccel-" + cindex;
	accel = kConfig->readNumEntry(key);
	key = "MenuOpOnly-" + cindex;
	oponly = kConfig->readNumEntry(key);
	
	UserMenu.append(new UserControlMenu(title, action, accel, type, (bool) oponly));
      }
    }
  }

  return &UserMenu;

}

void UserControlMenu::writeKConfig()
{
  KConfig *kConfig = kapp->config();
  kConfig->setGroup("UserMenu");

  int items = (int) UserMenu.count();

  kConfig->writeEntry("Number", items);

  QString key;
  QString cindex;
  UserControlMenu *ucm;
  int type;

  for(int i = 0; i < items; i++){
    ucm = UserMenu.at(i);
    cindex.sprintf("%d", i);
    key = "MenuType-" + cindex;
    type = ucm->type;
    kConfig->writeEntry(key, (int) type);
    // Do nothing for a seperator since it defaults across
    if(type == UserControlMenu::Text){
      key = "MenuTitle-" + cindex;
      kConfig->writeEntry(key, ucm->title);
      key = "MenuAction-" + cindex;
      kConfig->writeEntry(key, ucm->action);
      key = "MenuAccel-" + cindex;
      kConfig->writeEntry(key, (int) ucm->accel);
      key = "MenuOpOnly-" + cindex;
      kConfig->writeEntry(key, (int) ucm->op_only);
    }
  }
  kConfig->sync();

}

