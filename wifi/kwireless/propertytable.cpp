/*
  $ Author: Mirko Boehm $
  $ License: This code is licensed under the LGPL $
  $ Copyright: (C) 1996-2003, Mirko Boehm $
  $ Contact: Mirko Boehm <mirko@kde.org>
         http://www.kde.org
         http://www.hackerbuero.org $
*/

#include <qtable.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qtextstream.h>
#include <kdialogbase.h>
#include <klocale.h>
#include "propertytable.h"
#include "kwirelesswidget.h"

extern "C" {
#include <math.h>
}

PropertyTable::PropertyTable(QWidget *parent, const char* name)
    : PropertyTableBase(parent, name)
{
}

PropertyTable::~PropertyTable()
{
}

PropertiesDialog::PropertiesDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, true,
                  i18n("Wireless Network Device Properties"),
                  KDialogBase::Ok, KDialogBase::Ok, true),
      wait(false)
{
    table = new PropertyTable(this);
    setMainWidget(table);
    table->table->setLeftMargin(0);
    table->table->setSorting(false);
    table->table->setReadOnly(true);
    adjustSize();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
    timer->start(3000, false);
    connect(table->cbDeviceSelector, SIGNAL(activated(int)),
            SLOT(selected(int)));
}

void PropertiesDialog::update(QPtrList<DeviceInfo> *_info)
{
    int selection = table->cbDeviceSelector->currentItem();

    if(wait) return;

    info = _info;
    DeviceInfo *device;
    QPtrListIterator<DeviceInfo> it(*info);

    table->cbDeviceSelector->clear();

    while((device = it.current()) != 0)
    {
        ++it;
        table->cbDeviceSelector->insertItem(device->device());
    }

    if(selection > 0 && selection < table->cbDeviceSelector->count())
    {
        selected(selection);
    } else {
        if(table->cbDeviceSelector->count() == 0)
        {
            selected(-1);
        } else {
            selected(0);
        }
    }

    table->cbDeviceSelector->setEnabled(info->count()>1);

    wait = true;
}

void PropertiesDialog::timeout()
{
    wait = false;
}

void PropertiesDialog::selected(int index)
{
    DeviceInfo temp;
    DeviceInfo *device;
    int count;

    if(index < 0)
    {
        device = &temp;
    } else {
        device = info->at(index);
    }

    QString fields[] =
        {
            i18n("Device:"),
            i18n("ESSID (network name):"),
            i18n("Link quality:"),
            i18n("Signal strength:"),
            i18n("Noise level:"),
            i18n("Bit rate:"),
            i18n("Encryption:")
        };
    const int NoOfFields = sizeof(fields)/sizeof(fields[0]);

    QString values[] =
        {
            device->device(),
            device->essid(),
            device->qualityString(),
            device->signalString(),
            device->noiseString(),
            device->bitrateString(),
            device->encrString()
        };
    const int NoOfValues = sizeof(values)/sizeof(values[0]);

    if(table->table->numRows() == 0)
    {
        table->table->insertRows(0, NoOfFields);
        // HACK (make more rows visible than the default):
        resize(size().width(), (int)(1.8*size().height()));
    }

    for(count = 0; count<NoOfFields; ++count)
    {
       table->table->setText(count, 0, fields[count]);
    }

    for(count = 0; count<NoOfValues; ++count)
    {
        table->table->setText(count, 1, values[count]);
    }

    table->table->adjustColumn(0);
    table->table->adjustColumn(1);

}

#include "propertytable.moc"
