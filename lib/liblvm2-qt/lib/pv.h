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

#ifndef PV_H
#define PV_H
#include <QObject>
#include <lvm2app.h>

class Pv : public QObject
{
  Q_OBJECT
public:
  Pv(pv_t pvh, QObject* parent);
  ~Pv();
  QString name();
  QString uuid();
  
  quint64 metadataCount();
  quint64 deviceSize();
  quint64 size();
  quint64 freeSize();
  bool resize(quint64 size);
private:
  pv_t pvh;
  
  friend class Vg;
};

#endif // PV_H
