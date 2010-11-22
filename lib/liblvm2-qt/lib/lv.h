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

#ifndef LV_H
#define LV_H

#include <QObject>
#include <lvm2app.h>

class Lv : public QObject
{
  Q_OBJECT
public:
  Lv(lv_t lvh, QObject* parent = 0);
  ~Lv();
  QString name();
  QString uuid();
  
  bool isActive();
  bool isSuspended();
  
  bool activate();
  bool deactivate();
  
  bool remove();
  
  quint64 size();
  
  bool resize(quint64 size);
  
  //TODO tags line 999 @ lvm2app.h

private:
  lv_t lvh;
  
  friend class Vg;
};

#endif // LV_H
