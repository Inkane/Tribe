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

#include "pv.h"
#include <QDebug>

Pv::Pv(pv_t pvh, QObject* parent) : pvh(pvh), QObject(parent)
{
  qDebug()<<"Created PV object:"<<name()<<"with UUID:"<<uuid();
}

Pv::~Pv()
{

}

QString Pv::name()
{
  return lvm_pv_get_name(pvh);
}

QString Pv::uuid()
{
  return lvm_pv_get_uuid(pvh);
}

quint64 Pv::metadataCount()
{
  return lvm_pv_get_mda_count(pvh);
}

quint64 Pv::deviceSize()
{
  return lvm_pv_get_dev_size(pvh);
}

quint64 Pv::size()
{
  return lvm_pv_get_dev_size(pvh);
}

quint64 Pv::freeSize()
{
  return lvm_pv_get_free(pvh);
}

bool Pv::resize(quint64 size)
{
  return !lvm_pv_resize(pvh, size);
}

