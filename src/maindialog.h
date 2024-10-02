// SPDX-License-Identifier: GPL-3.0-only
/*
 * SSHTUNNELGUI <https://github.com/amartin755/sshTunnelGui>
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


#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QList>
#include <QApplication>
#include <QProcess>
#include <QTimer>

#include "ui_mainDialog.h"

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QApplication* theApp, QWidget *parent = nullptr);

private slots:
    void addConnection ();
    void cloneConnection ();
    void editConnection ();
    void deleteConnection ();
    void itemClicked (QTreeWidgetItem *item, int column);
    void editConnection (QTreeWidgetItem *item, int column);
    void processTerminated(int exitCode, QProcess::ExitStatus exitStatus  = QProcess::NormalExit);
    void checkConnections ();
    void shutdown ();
    void connectAll ();
    void disconnectAll ();
    void addItemToList (const QString& name, const QString& localPort, const QString& remotePort,
        const QString& remoteAddress, const QString& server, const QString& url);

private:
    void keyPressEvent (QKeyEvent *e);
    void closeEvent (QCloseEvent *event);
    void saveConnections() const;
    void loadConnections();
    void setURL (QTreeWidgetItem *item, const QString& url);
    void adjustColumnSize ();
    QList<QProcess*> m_connections;
    Ui::Dialog m_gui;
    QTimer m_connectionsWatchdog;

};

#endif
