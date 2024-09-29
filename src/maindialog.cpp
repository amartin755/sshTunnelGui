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
#include "connectiondialog.h"

enum COLUMN {enabled = 0, localPort, remotePort, remoteAddress, server};

MainDialog::MainDialog(QApplication* theApp, QWidget *parent)
    : QDialog (parent), m_connectionsWatchdog (this)
{
    setWindowFlags (Qt::Window);
    m_gui.setupUi (this);


    connect (&m_connectionsWatchdog, &QTimer::timeout, this, &MainDialog::checkConnections);
    connect (m_gui.btnAdd, &QPushButton::clicked, this, &MainDialog::addItem);
    connect (m_gui.treeWidget, &QTreeWidget::itemClicked, this, &MainDialog::itemClicked);
    connect (m_gui.treeWidget, &QTreeWidget::itemDoubleClicked, this, &MainDialog::editItem);
    connect (theApp, &QApplication::aboutToQuit, this, &MainDialog::shutdown, Qt::DirectConnection);
    loadConnections();

}

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
    ConnectionDialog dlg (this);
    if (dlg.exec () == QDialog::Accepted)
    {
        QTreeWidgetItem* i = new QTreeWidgetItem (m_gui.treeWidget);

        i->setCheckState (COLUMN::enabled, Qt::Unchecked);
        i->setText (COLUMN::localPort, dlg.getLocalPort());
        i->setText (COLUMN::remotePort, dlg.getRemotePort());
        i->setText (COLUMN::remoteAddress, dlg.getRemoteAddress());
        i->setText (COLUMN::server, dlg.getServer());

        QProcess* proc = new QProcess (this);
        m_connections.append (proc); 
        connect (proc, &QProcess::finished, this, &MainDialog::processTerminated);

        saveConnections ();
    }
}   

void MainDialog::editItem (QTreeWidgetItem *item, int column)
{
    if (item->checkState(column) == Qt::Unchecked)
    {
        ConnectionDialog dlg ("", 
            item->text (COLUMN::remotePort),
            item->text (COLUMN::localPort),
            item->text (COLUMN::remoteAddress),
            item->text (COLUMN::server),
            this);
        if (dlg.exec () == QDialog::Accepted)
        {
            item->setText (COLUMN::localPort, dlg.getLocalPort());
            item->setText (COLUMN::remotePort, dlg.getRemotePort());
            item->setText (COLUMN::remoteAddress, dlg.getRemoteAddress());
            item->setText (COLUMN::server, dlg.getServer());
            saveConnections ();
        }
    }
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
            }

        qInfo() << m_gui.treeWidget->indexOfTopLevelItem(item) << ": " << item->checkState(column);
    }
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
    //    m_gui.treeWidget->resizeColumnToContents (0);

        m_connections.append (new QProcess (this)); 
    }
    settings.endArray ();

}

void MainDialog::processTerminated(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED (exitCode);
    Q_UNUSED (exitStatus);
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (proc)
    {
        for (int n = 0; n < m_connections.size(); n++)
        {
            if (m_connections[n] == proc)
            {
                qInfo() << "SIGNAL: connection #" << n << " terminated";
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
            qInfo() << "WATCHDOG: connection #" << n << " terminated";
            item->setCheckState (COLUMN::enabled, Qt::Unchecked);
        }
    }

}

void MainDialog::shutdown ()
{
    // TODO
}