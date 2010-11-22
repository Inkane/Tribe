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

#include "lv.h"
#include <QDebug>

Lv::Lv(lv_t lvh, QObject* parent) : lvh(lvh), QObject(parent)
{
  qDebug()<<"Created LV object:"<<name()<<"with UUID:"<<uuid();
}

Lv::~Lv()
{

}

QString Lv::name()
{
  return lvm_lv_get_name(lvh);
}

QString Lv::uuid()
{
  return lvm_lv_get_uuid(lvh);
}

bool Lv::isActive()
{
  return lvm_lv_is_active(lvh);
}

bool Lv::isSuspended()
{
  return lvm_lv_is_suspended(lvh);
}

bool Lv::activate()
{
  return !lvm_lv_activate(lvh);
}

bool Lv::deactivate()
{
  return !lvm_lv_deactivate(lvh);
}

bool Lv::remove()
{
  return !lvm_vg_remove_lv(lvh);
}

quint64 Lv::size()
{
  return lvm_lv_get_size(lvh);
}

bool Lv::resize(quint64 size)
{
  return !lvm_lv_resize(lvh, size);
}

