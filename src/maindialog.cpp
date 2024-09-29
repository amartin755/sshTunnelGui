// SPDX-License-Identifier: GPL-3.0-only
/*
 * STECHUHR <https://github.com/amartin755/stechuhr>
 * Copyright (C) 2023 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGlobal>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include "maindialog.h"

enum COLUMN {enabled = 0, localPort, remotePort, remoteAddress, server};

MainDialog::MainDialog(QApplication* theApp, QWidget *parent)
    : QDialog(parent), m_connectionsWatchdog (this)
{
    setWindowFlags (Qt::Window);
    m_gui.setupUi (this);


    connect (&m_connectionsWatchdog, &QTimer::timeout, this, &MainDialog::checkConnections);
    connect (m_gui.btnAdd, &QPushButton::clicked, this, &MainDialog::addItem);
    connect (m_gui.treeWidget, &QTreeWidget::itemClicked, this, &MainDialog::itemClicked);
    loadConnections();
    connect (m_gui.treeWidget, &QTreeWidget::itemChanged, this, &MainDialog::itemChanged);
//    connect (theApp, &QApplication::aboutToQuit, this, &MainDialog::saveState, Qt::DirectConnection);

}
/*
void MainDialog::updateList (const QString& iconPath, const QString& caption, const QDateTime& time)
{
    bool multipleDays = m_wtClock.exceedsDay ();
    QTreeWidgetItem* i = new QTreeWidgetItem (m_gui.treeWidget);
    i->setText (1, multipleDays ? QLocale::system().toString(time, QLocale::ShortFormat) : time.toString ("hh:mm:ss"));
    i->setText (2, caption);
    i->setIcon (0, QIcon (iconPath));
    m_gui.treeWidget->resizeColumnToContents (0);
}
*/
void MainDialog::keyPressEvent (QKeyEvent *e)
{
    if (e->key () != Qt::Key_Escape)
        QDialog::keyPressEvent (e);
}

void MainDialog::closeEvent (QCloseEvent *event)
{
//    m_settings.setGuiState (saveGeometry(), m_gui.splitter->saveState());

    QDialog::closeEvent(event);
}

void MainDialog::addItem ()
{
    QTreeWidgetItem* i = new QTreeWidgetItem (m_gui.treeWidget);

    i->setCheckState(COLUMN::enabled    , Qt::Unchecked);
    i->setFlags(i->flags() | Qt::ItemIsEditable);

    QProcess* proc = new QProcess (this);
    m_connections.append (proc); 
    connect (proc, &QProcess::finished, this, &MainDialog::processTerminated);
}   

void MainDialog::itemClicked (QTreeWidgetItem *item, int column)
{
    if (item && column == COLUMN::enabled)
    {
        int index = m_gui.treeWidget->indexOfTopLevelItem(item);
        Q_ASSERT (index < m_connections.size());
        QProcess* proc = m_connections[index];
        Q_ASSERT (proc);

        if (item->checkState(column) == Qt::Checked)
        {
            if (proc->state() == QProcess::NotRunning)
            {
                QStringList args;
                args << "-N";
                args << "-L";
                args << item->text(COLUMN::localPort) + QString(":") + item->text(COLUMN::remoteAddress) + QString(":") + item->text(COLUMN::remotePort);
                args << item->text(COLUMN::server);
                qInfo() << args;
                proc->start ("ssh", args);
                if (!proc->waitForStarted())
                {
                    QMessageBox::critical (this, "", tr("Could not start ssh client"));
                }
                else
                {
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    if (!m_connectionsWatchdog.isActive())
                        m_connectionsWatchdog.start (2000);
                }
            }
        }
        else
        {
            if (proc->state() == QProcess::Running)
            {
                proc->terminate();
                if (proc->waitForFinished (1000))
                    proc->kill();
            }
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }

        qInfo() << m_gui.treeWidget->indexOfTopLevelItem(item) << ": " << item->checkState(column);
    }
}

void MainDialog::itemChanged (QTreeWidgetItem *item, int column)
{
    saveConnections ();
}

void MainDialog::saveConnections() const
{
    QSettings settings;

    settings.beginWriteArray ("connections");
    settings.remove ("");
    for (int n = 0; n < m_gui.treeWidget->topLevelItemCount(); n++)
    {
        settings.setArrayIndex (n);
        QTreeWidgetItem *item = m_gui.treeWidget->topLevelItem(n);
        settings.setValue ("localPort", item->text(COLUMN::localPort));
        settings.setValue ("remotePort", item->text(COLUMN::remotePort));
        settings.setValue ("remoteAddress", item->text(COLUMN::remoteAddress));
        settings.setValue ("server", item->text(COLUMN::server));
    }
    settings.endArray ();

}

void MainDialog::loadConnections()
{
    QSettings settings;

    int size = settings.beginReadArray ("connections");
    for (int n = 0; n < size; n++)
    {
        settings.setArrayIndex (n);

        QTreeWidgetItem* i = new QTreeWidgetItem (m_gui.treeWidget);
        i->setText (COLUMN::localPort, settings.value ("localPort").toString());
        i->setText (COLUMN::remotePort, settings.value ("remotePort").toString());
        i->setText (COLUMN::remoteAddress, settings.value ("remoteAddress").toString());
        i->setText (COLUMN::server, settings.value ("server").toString());
        i->setCheckState(COLUMN::enabled, Qt::Unchecked);
        i->setFlags(i->flags() | Qt::ItemIsEditable);
    //    m_gui.treeWidget->resizeColumnToContents (0);

        m_connections.append (new QProcess (this)); 
    }
    settings.endArray ();

}

void MainDialog::processTerminated(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (proc)
    {
        for (int n = 0; n < m_connections.size(); n++)
        {
            if (m_connections[n] == proc)
            {
                qInfo() << "connection " << n << " terminated";
                QTreeWidgetItem *item = m_gui.treeWidget->topLevelItem(n);
                item->setCheckState (COLUMN::enabled, Qt::Unchecked);
            }
        }
    }
}

void MainDialog::checkConnections ()
{
    for (int n = 0; n < m_connections.size(); n++)
    {
        QTreeWidgetItem *item = m_gui.treeWidget->topLevelItem(n);
        if (item->checkState (COLUMN::enabled) == Qt::Checked && 
            m_connections[n] &&
            m_connections[n]->state() == QProcess::NotRunning)
        {
            qInfo() << "connection " << n << " terminated";
            item->setCheckState (COLUMN::enabled, Qt::Unchecked);
        }
    }

}