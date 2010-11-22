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

#include "vg.h"
#include <QDebug>

Vg::Vg(lvm_t libh, QString name, QObject *parent) : libh(libh), m_name(name), QObject(parent)
{
  open("r");
  
  qDebug()<<"Populating PV list for VG"<<this->name();
  dm_list* pvlist;
  lvm_str_list *pvitem;
  pvlist = lvm_vg_list_pvs(vgh);
  dm_list_iterate_items(pvitem, pvlist) {
    Pv *pv = new Pv((pv_t)pvitem->str, this);
    pvs.insert(pv->name(), pv);
  }
  
  qDebug()<<"Populating LV list for VG"<<this->name();
  dm_list* lvlist;
  lvm_str_list *lvitem;
  lvlist = lvm_vg_list_lvs(vgh);
  dm_list_iterate_items(lvitem, lvlist) {
    Lv *lv = new Lv((lv_t)lvitem->str, this);
    lvs.insert(lv->name(), lv);
  }
  close();
}

Vg::~Vg()
{
//  close();
}

QString Vg::name()
{
  //TODO find a way to use lvm_vg_get_name() instead
  return m_name;
}

QString Vg::uuid()
{
  return lvm_vg_get_uuid(vgh);
}

bool Vg::open(const char* mode, uint32_t flags)
{
  vgh = lvm_vg_open(libh, m_name.toLatin1(), mode, flags);
  return vgh;
}

bool Vg::create()
{
  vgh = lvm_vg_create(libh, m_name.toLatin1());
  return vgh;
}

bool Vg::remove()
{
  return !lvm_vg_remove(vgh);
}

bool Vg::write()
{
  return !lvm_vg_write(vgh);
}

bool Vg::close()
{
  return !lvm_vg_close(vgh);
}

bool Vg::extend(QString device)
{
  return !lvm_vg_extend(vgh, device.toLatin1());
}

bool Vg::reduce(QString device)
{
  return !lvm_vg_reduce(vgh, device.toLatin1());

}

bool Vg::addTag(QString tag)
{
  return !lvm_vg_add_tag(vgh, tag.toLatin1());
}

bool Vg::removeTag(QString tag)
{
  return !lvm_vg_remove_tag(vgh, tag.toLatin1());
}

bool Vg::setExtentSize(quint32 size)
{
  return !lvm_vg_set_extent_size(vgh, size);
}

bool Vg::isClustered()
{
  return lvm_vg_is_clustered(vgh);
}

bool Vg::isExported()
{
  return lvm_vg_is_exported(vgh);
}

bool Vg::isPartial()
{
  return lvm_vg_is_partial(vgh);
}

quint64 Vg::seqNo()
{
  return lvm_vg_get_seqno(vgh);
}

quint64 Vg::size()
{
  return lvm_vg_get_size(vgh);
}

quint64 Vg::freeSize()
{
  return lvm_vg_get_free_size(vgh);
}

quint64 Vg::extentSize()
{
  return lvm_vg_get_extent_size(vgh);
}

quint64 Vg::extentCount()
{
  return lvm_vg_get_extent_count(vgh);
}

quint64 Vg::freeExtentCount()
{
  return lvm_vg_get_free_extent_count(vgh);
}

quint64 Vg::pvCount()
{
  return lvm_vg_get_pv_count(vgh);
}

quint64 Vg::maxPvCount()
{
  return lvm_vg_get_max_pv(vgh);
}

quint64 Vg::maxLvCount()
{
  return lvm_vg_get_max_lv(vgh);
}

bool Vg::createLinearLv(QString name, quint64 size)
{
  lv_t lvh = lvm_vg_create_lv_linear(vgh, name.toLatin1(), size);
  if (!lvh) return false;
  Lv *lv = new Lv(lvh, this);
  lvs.insert(lv->name(), lv);
  return true;
}
