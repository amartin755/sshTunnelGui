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


#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "ui_connectionDialog.h"

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ConnectionDialog(const QString& name, const QString& remotePort, const QString& localPort, 
        const QString& remoteAddr, const QString& server, const QString& url, QWidget *parent);
    
    const QString getName () const;
    const QString getRemotePort () const;
    const QString getLocalPort () const;
    const QString getRemoteAddress () const;
    const QString getServer () const;
    const QString getUrl () const;

private slots:
    void enableLocalPort (bool enable);
    void setLocalPort (int val);

private:
    Ui::dlgConnection m_gui;
};

#endif
