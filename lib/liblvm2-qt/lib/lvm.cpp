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

#include "lvm.h"
#include <QDebug>

Lvm::Lvm(){
  libh = lvm_init(NULL);
  if (!libh) qDebug()<<"Couldn't get lvm handle";
  
  scan();
  
  //TODO add UUIDs
  dm_list *vgnames;
  lvm_str_list *strl;
  qDebug()<<"Populating VG list";
  vgnames = lvm_list_vg_names(libh);
  dm_list_iterate_items(strl, vgnames) {
    QString name = strl->str;
    qDebug()<<name;
    Vg *vg = new Vg(libh, name, this);
    vgs.insert(name, vg);
  }
}

Lvm::~Lvm(){
  lvm_quit(libh);
}

bool Lvm::scan()
{
  return !lvm_scan(libh);
}

bool Lvm::reloadConfig()
{
  return !lvm_config_reload(libh);
}

bool Lvm::overrideConfig(QString option)
{
  return !lvm_config_override(libh, option.toLatin1());
}

QString Lvm::lvNameFromPvid(QString uuid)
{
  return lvm_vgname_from_pvid(libh, uuid.toLatin1());
}

QString Lvm::lvNameFromDevice(QString device)
{
  return lvm_vgname_from_device(libh, device.toLatin1());
}
