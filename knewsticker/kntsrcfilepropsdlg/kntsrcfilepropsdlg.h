/*
 * kntsrcfilepropsdlg.h
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef KNTSRCFILEPROPSDLG_H
#define KNTSRCFILEPROPSDLG_H

#include <global.h>

#include <klibloader.h>
#include <kpropertiesdialog.h>

class KntSrcFilePropsDlgWidget;
class QListBoxItem;

namespace RSS {
	class Loader;
	class Document;
}

using RSS::Loader;
using RSS::Document;
using RSS::Status;

class KntSrcFilePropsFactory : public KLibFactory
{
	Q_OBJECT
	
	public:
		virtual QObject *createObject(QObject * = 0, const char * = 0,
				const char * = "QObject", const QStringList & = QStringList());
};

class KntSrcFilePropsDlg : public KPropsDlgPlugin
{
	Q_OBJECT
	
	public:
		KntSrcFilePropsDlg(KPropertiesDialog *);
	
	protected slots:
		void slotOpenURL(const QString &);
		void slotConstructUI(Loader *loader, Document doc, Status status);
		void slotGotIcon(const KURL &, const QPixmap &);
		void slotClickedArticle(QListBoxItem *);
	
	private:
		KntSrcFilePropsDlgWidget *m_child;
};

#endif // KNTSRCFILEPROPSDLG_H
