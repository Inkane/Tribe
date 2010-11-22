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

#ifndef LVM_H
#define LVM_H

#include <QObject>
#include <lvm2app.h>
#include <QHash>
#include "vg.h"

class Lvm: public QObject
{
  Q_OBJECT
public:
  Lvm();
  ~Lvm();
  bool scan();
  bool reloadConfig();
  bool overrideConfig(QString option);
  QString lvNameFromPvid(QString uuid);
  QString lvNameFromDevice(QString device);
private:
  lvm_t libh;
  QHash<QString, Vg*> vgs;
};

#endif // LVM_H
