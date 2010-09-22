/***************************************************************************
 *   Copyright (C) 2008 - 2009 by Dario Freddi                             *
 *   drf@chakra-project.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef PARTITIONINGPAGE_H
#define PARTITIONINGPAGE_H

#include "../AbstractPage.h"
#include "../InstallationHandler.h"

#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <KPixmapSequence>

class QTimeLine;
class Device;
class QTreeWidgetItem;
class PMActions;
class Partition;
class TribePartition;
class PartitionManagerInterface;

namespace Ui {
    class tribePartition;
}

class PartitionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PartitionDelegate(QObject * parent = 0);
    ~PartitionDelegate();

    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private slots:
    void commitData();
    void slotIndexChanged(const QString&);

private:
    KIcon m_lockIcon;
};

class PartitionViewWidget : public QTreeWidget
{
    Q_OBJECT

public:
    PartitionViewWidget(QWidget* parent = 0);
    virtual ~PartitionViewWidget();

    virtual void paintEvent(QPaintEvent* event);

public slots:
    void setEnabled(bool enabled);

private slots:
    void stopRetainingPaintEvent();

private:
    bool m_retainPaintEvent;
    QTimeLine *m_fadeTimeLine;
    QTimeLine *m_spinnerTimeLine;
    KPixmapSequence m_sequence;
    QPixmap m_backgroundPixmap;
};

class PartitioningPage : public AbstractPage
{
    Q_OBJECT

public:
    enum VisiblePart {
        None = 0,
        TypePart = 1,
        FileSystemPart = 2,
        ShrinkPart = 4
    };
    Q_DECLARE_FLAGS(VisibleParts, VisiblePart)

    enum Mode {
        EasyMode = 1,
        AdvancedMode = 2
    };

    PartitioningPage(QWidget *parent = 0);
    virtual ~PartitioningPage();

private slots:
    virtual void createWidget();
    virtual void aboutToGoToNext();
    virtual void aboutToGoToPrevious();

    void actionLearnMoreTriggered(bool);
    void actionNewPartitionTableTriggered();
    void actionFormatToggled(bool status);
    void actionUndoTriggered(bool);
    void actionResizeTriggered(bool);
    void actionNewTriggered();
    void actionDeleteTriggered(bool);
    void actionUnmountTriggered(bool);

    void populateTreeWidget();
    void setVisibleParts(VisibleParts parts);
    void currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);
    void slotTypeChanged(const QString&);

    void cancelFormat();
    void applyFormat();
    void cancelNew();
    void applyNew();

    void advancedRadioChanged(bool);
    void easyRadioChanged(bool);

private:
    Ui::tribePartition *m_ui;
    PartitionManagerInterface *m_iface;
    Partition *m_newPartition;
    QTreeWidgetItem *createItem(const Partition *p, Device *dev);
    QWidget* m_pmWidget;

    QHash<const Partition*, QString> m_toFormat;
    VisibleParts m_parts;

    Mode m_mode;

    InstallationHandler *m_install;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PartitioningPage::VisibleParts)

#endif /*PARTITIONINGPAGE_H*/
