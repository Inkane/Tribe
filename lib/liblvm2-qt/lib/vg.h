/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2010  Andrei Nistor <coder.tux@ceata.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef VG_H
#define VG_H

#include <QObject>
#include <lvm2app.h>
#include <QString>
#include <QHash>
#include "pv.h"
#include "lv.h"

class Vg: public QObject
{
  Q_OBJECT
public:
  Vg(lvm_t libh, QString name, QObject* parent = 0);
  Vg(const Vg& other);
  ~Vg();
  QString name();
  QString uuid();  
  
  bool open(const char* mode = "r", uint32_t flags = 0);
  bool create();
  bool remove();
  bool write();
  bool close();
  
  bool extend(QString device);
  bool reduce(QString device);
  
  bool addTag(QString tag);
  bool removeTag(QString tag);
  
  bool setExtentSize(quint32 size);
  
  bool isClustered();
  bool isExported();
  bool isPartial();
  
  quint64 seqNo();
  
  quint64 size();
  quint64 freeSize();
  quint64 extentSize();
  quint64 extentCount();
  quint64 freeExtentCount();
  quint64 pvCount();
  quint64 maxPvCount();
  quint64 maxLvCount();
  
  //TODO more tags line 823 @ lvm2app.h
  
  /* LV stuff */
  bool createLinearLv(QString name, quint64 size);
private:
  lvm_t libh;
  vg_t vgh;
  QString m_name;
  QString m_uuid;
 
  QHash<QString, Pv*> pvs;
  QHash<QString, Lv*> lvs;
  };

#endif // VG_H
