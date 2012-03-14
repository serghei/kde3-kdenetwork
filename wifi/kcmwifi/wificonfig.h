/*
  Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <qstring.h>
#include <qvaluelist.h>

#include "kcmwifi.h"

class KConfig;
class KSimpleConfig;

enum KeyStates { EMPTY=0, INVALID=1, HEX_64=2, HEX_128=3, HEX_256=4, STRING_64=5, STRING_128=6, STRING_256=7 };

class Key
{
  public:
    Key( const QString &key );
    Key();

    QString key() const { return m_key; }
    static KeyStates isValid( QString keyCandidate );
    QString rawKey() const;

    void setKey( const QString &key );

  protected:
    QString m_key;
};

typedef QValueList<Key> KeyList;

class IfConfig
{
  public:
    IfConfig();

    enum Speed { AUTO=0, M1, M2, M55, M6, M9, M11, M12, M18, M24, M36, M48, M54 };
    enum WifiMode { AdHoc=0, Managed, Repeater, Master, Secondary };
    enum PowerMode { AllPackets=0, UnicastOnly, MulticastOnly };
    enum CryptoMode { Open=0, Restricted };

    void load( KConfig *config, int i );
    void save( KConfig *config, int i );

    QString speedAsString();
    QString wifimodeAsString();
    QString cryptomodeAsString();
    QString powermodeAsString();
    int activeKeyId();
    Key activeKey();

    static Speed convertToSpeedFromString( const QString &s );
    static WifiMode convertToWifiModeFromString( const QString &s );
    static PowerMode convertToPowerModeFromString( const QString &s );
    static CryptoMode convertToCryptoModeFromString( const QString &s );

    void speedFromString( const QString &s );
    void wifimodeFromString( const QString &s );
    void powermodeFromString( const QString &s );
    void cryptomodeFromString( const QString &s );

    QString m_networkName;
    QString m_interface;
    WifiMode m_wifiMode;
    Speed m_speed;
    bool m_runScript;
    QString m_connectScript;

    bool m_useCrypto;
    CryptoMode m_cryptoMode;
    int m_activeKey;
    Key m_keys[ 4 ];

    bool m_pmEnabled;
    PowerMode m_pmMode;
    int m_sleepTimeout;
    int m_wakeupPeriod;
};

class WifiConfig : QObject
{
  Q_OBJECT
  public:
    static WifiConfig *instance();
    QString autoDetectInterface();
    ~WifiConfig();

    void load();
    void save();

    IfConfig m_ifConfig[ KCMWifi::vendorBase+5 ];
    bool m_usePreset;
    int m_presetConfig;
    int m_numConfigs;

  private slots:
    void slotTestInterface( KProcIO *proc );

  private:
    WifiConfig();

    KSimpleConfig *m_config;
    static WifiConfig *m_instance;
    QString m_detectedInterface;
};

#endif // WIFICONFIG_H
