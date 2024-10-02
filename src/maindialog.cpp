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

enum COLUMN {enabled = 0, name, remotePort, remoteAddress, localPort, server, url};

MainDialog::MainDialog(QApplication* theApp, QWidget *parent)
    : QDialog (parent), m_connectionsWatchdog (this)
{
    setWindowFlags (Qt::Window);
    m_gui.setupUi (this);
    m_gui.treeWidget->setRootIsDecorated(false);

    connect (&m_connectionsWatchdog, &QTimer::timeout, this, &MainDialog::checkConnections);
    connect (m_gui.btnAdd, &QPushButton::clicked, this, &MainDialog::addConnection);
    connect (m_gui.btnClone, &QPushButton::clicked, this, &MainDialog::cloneConnection);
    connect (m_gui.btnConnect, &QPushButton::clicked, this, &MainDialog::connectAll);
    connect (m_gui.btnDisconnect, &QPushButton::clicked, this, &MainDialog::disconnectAll);
    connect (m_gui.btnEdit, &QPushButton::clicked, this, qOverload<>(&MainDialog::editConnection));
    connect (m_gui.btnDelete, &QPushButton::clicked, this, &MainDialog::deleteConnection);
    connect (m_gui.treeWidget, &QTreeWidget::itemClicked, this, &MainDialog::itemClicked);
    connect (m_gui.treeWidget, &QTreeWidget::itemDoubleClicked, this, qOverload<QTreeWidgetItem*, int>(&MainDialog::editConnection));
    connect (theApp, &QApplication::aboutToQuit, this, &MainDialog::shutdown, Qt::DirectConnection);

    loadConnections();
}

void MainDialog::keyPressEvent (QKeyEvent *e)
{
    if (e->key () != Qt::Key_Escape)
        QDialog::keyPressEvent (e);
    if(e->key() == Qt::Key_Delete)
    {
        deleteConnection ();
    }
}

void MainDialog::closeEvent (QCloseEvent *event)
{
//    m_settings.setGuiState (saveGeometry(), m_gui.splitter->saveState());

    QDialog::closeEvent(event);
}

void MainDialog::addItemToList (const QString& name, const QString& localPort, const QString& remotePort,
    const QString& remoteAddress, const QString& server, const QString& url)
{
    QTreeWidgetItem* i = new QTreeWidgetItem (m_gui.treeWidget);

    i->setCheckState (COLUMN::enabled, Qt::Unchecked);
    i->setText (COLUMN::name, name);
    i->setText (COLUMN::localPort, localPort);
    i->setText (COLUMN::remotePort, remotePort);
    i->setText (COLUMN::remoteAddress, remoteAddress);
    i->setText (COLUMN::server, server);
    setURL (i, url);

    QProcess* proc = new QProcess (this);
    m_connections.append (proc); 
    connect (proc, &QProcess::finished, this, &MainDialog::processTerminated);

    adjustColumnSize ();
}

void MainDialog::addConnection ()
{
    ConnectionDialog dlg (this);
    if (dlg.exec () == QDialog::Accepted)
    {
        addItemToList (dlg.getName(), dlg.getLocalPort(), dlg.getRemotePort(), dlg.getRemoteAddress(), dlg.getServer(), dlg.getUrl());
        saveConnections ();
    }
}   

void MainDialog::cloneConnection ()
{
    auto items = m_gui.treeWidget->selectedItems();
    if (items.size())
    {
        QTreeWidgetItem *item = items.first();
        ConnectionDialog dlg (
            item->text (COLUMN::name), 
            item->text (COLUMN::remotePort),
            item->text (COLUMN::localPort),
            item->text (COLUMN::remoteAddress),
            item->text (COLUMN::server),
            item->text (COLUMN::url),
            this);

        if (dlg.exec () == QDialog::Accepted)
        {
            addItemToList (dlg.getName(), dlg.getLocalPort(), dlg.getRemotePort(), dlg.getRemoteAddress(), dlg.getServer(), dlg.getUrl());
            saveConnections ();
        }
    }
}

void MainDialog::editConnection ()
{
    auto items = m_gui.treeWidget->selectedItems();
    if (items.size())
    {
        QTreeWidgetItem *item = items.first();
        editConnection (item, COLUMN::enabled);
    }
}

void MainDialog::editConnection (QTreeWidgetItem *item, int)
{
    if (item->checkState(COLUMN::enabled) == Qt::Unchecked)
    {
        ConnectionDialog dlg (
            item->text (COLUMN::name), 
            item->text (COLUMN::remotePort),
            item->text (COLUMN::localPort),
            item->text (COLUMN::remoteAddress),
            item->text (COLUMN::server),
            item->text (COLUMN::url),
            this);
        if (dlg.exec () == QDialog::Accepted)
        {
            item->setText (COLUMN::name, dlg.getName());
            item->setText (COLUMN::localPort, dlg.getLocalPort());
            item->setText (COLUMN::remotePort, dlg.getRemotePort());
            item->setText (COLUMN::remoteAddress, dlg.getRemoteAddress());
            item->setText (COLUMN::server, dlg.getServer());
            item->setText (COLUMN::url, dlg.getUrl());
            setURL (item, dlg.getUrl());
            saveConnections ();
            adjustColumnSize ();
        }
    }
}

void MainDialog::deleteConnection ()
{
    auto items = m_gui.treeWidget->selectedItems();
    for (const auto& i : items)
    {
        int index = m_gui.treeWidget->indexOfTopLevelItem(i);
        delete i;

        Q_ASSERT (index < m_connections.size());
        delete m_connections[index];
        m_connections.removeAt (index);
    }
    saveConnections ();
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
        settings.setValue ("name", item->text(COLUMN::name));
        settings.setValue ("localPort", item->text(COLUMN::localPort));
        settings.setValue ("remotePort", item->text(COLUMN::remotePort));
        settings.setValue ("remoteAddress", item->text(COLUMN::remoteAddress));
        settings.setValue ("server", item->text(COLUMN::server));
        settings.setValue ("url", item->text(COLUMN::url));
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
        addItemToList (settings.value ("name").toString(), settings.value ("localPort").toString(),
            settings.value ("remotePort").toString(), settings.value ("remoteAddress").toString(),
            settings.value ("server").toString(), settings.value ("url").toString());
    }
    settings.endArray ();
}

void MainDialog::adjustColumnSize ()
{
    for (int col = 0; col < m_gui.treeWidget->columnCount(); col++)
        m_gui.treeWidget->resizeColumnToContents (col);
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

void MainDialog::setURL (QTreeWidgetItem *item, const QString& url)
{
    QString renderedURL(url);
    renderedURL.replace ("%p", item->text (COLUMN::localPort));
    QLabel* label = new QLabel();
    label->setText ("<a href=\"" + renderedURL + "\">" + renderedURL + "</a>");
    label->setTextFormat (Qt::RichText);
    label->setTextInteractionFlags (Qt::TextBrowserInteraction);
    label->setOpenExternalLinks (true);
    label->setAutoFillBackground (true);
    m_gui.treeWidget->setItemWidget (item, COLUMN::url,label);
    item->setText (COLUMN::url, url);
}

void MainDialog::connectAll ()
{
    for (int n = 0; n < m_gui.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *item = m_gui.treeWidget->topLevelItem (n);
        item->setCheckState (COLUMN::enabled, Qt::Checked);
        itemClicked (item, COLUMN::enabled);
    }
}

void MainDialog::disconnectAll ()
{
    for (int n = 0; n < m_gui.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *item = m_gui.treeWidget->topLevelItem (n);
        item->setCheckState (COLUMN::enabled, Qt::Unchecked);
        itemClicked (item, COLUMN::enabled);
    }
}

void MainDialog::shutdown ()
{
    disconnectAll ();
}
