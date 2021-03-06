/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AliasPREFERENCES_H
#define AliasPREFERENCES_H

#include "kcmodule.h"

typedef QValueList<Kopete::Protocol*> ProtocolList;

class AliasDialogBase;
namespace Kopete { class Protocol; }
class ProtocolItem;
class AliasItem;
class AliasDialog;
namespace Kopete { class Plugin; }

class AliasPreferences : public KCModule
{
	Q_OBJECT

	public:
		AliasPreferences( QWidget *parent = 0, const char *name = 0,
			const QStringList &args = QStringList() );
		~AliasPreferences();

		virtual void save();
		virtual void load();

	private slots:
		void slotAddAlias();
		void slotEditAlias();
		void slotDeleteAliases();
		void slotCheckAliasSelected();
		void slotPluginLoaded( Kopete::Plugin * );

	private:
		AliasDialogBase * preferencesDialog;
		void addAlias( QString &alias, QString &command, const ProtocolList &p, uint id = 0 );
		void loadProtocols( EditAliasDialog *dialog );
		const ProtocolList selectedProtocols( EditAliasDialog *dialog );
		QMap<Kopete::Protocol*,ProtocolItem*> itemMap;
		QMap<QPair<Kopete::Protocol*,QString>, bool> protocolMap;
		QMap<QString,AliasItem*> aliasMap;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

