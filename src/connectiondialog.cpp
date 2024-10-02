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

#include "connectiondialog.h"


ConnectionDialog::ConnectionDialog (QWidget *parent)
    : ConnectionDialog (QString(), QString("443"), QString("443"), QString(), QString(), QString("https://localhost:%p"), parent)
{
}

ConnectionDialog::ConnectionDialog (const QString& name, const QString& remotePort, const QString& localPort, 
    const QString& remoteAddr, const QString& server, const QString& url, QWidget *parent)
    : QDialog (parent)
{
    m_gui.setupUi (this);

    connect (m_gui.chkUseLocalPort, &QCheckBox::toggled, this, &ConnectionDialog::enableLocalPort);
    connect (m_gui.remotePort, &QSpinBox::valueChanged, this, &ConnectionDialog::setLocalPort);

    m_gui.name->setText (name);
    if (!remotePort.isEmpty())
        m_gui.remotePort->setValue (remotePort.toInt());
    if (!localPort.isEmpty())
        m_gui.localPort->setValue (localPort.toInt());
    m_gui.remoteAddress->setText (remoteAddr);
    m_gui.server->setText (server);
    m_gui.url->setText (url);

    m_gui.chkUseLocalPort->setChecked (m_gui.remotePort->value() == m_gui.localPort->value());
}

void ConnectionDialog::enableLocalPort (bool enable)
{
    m_gui.localPort->setEnabled (!enable);
    setLocalPort (m_gui.remotePort->value());
}

void ConnectionDialog::setLocalPort (int val)
{
    if (m_gui.chkUseLocalPort->isChecked())
        m_gui.localPort->setValue (val);
}

const QString ConnectionDialog::getName () const
{
    return m_gui.name->text();
}

const QString ConnectionDialog::getRemotePort () const
{
    return m_gui.remotePort->text();
}

const QString ConnectionDialog::getLocalPort () const
{
    return m_gui.localPort->text();
}

const QString ConnectionDialog::getRemoteAddress () const
{
    return m_gui.remoteAddress->text();
}

const QString ConnectionDialog::getServer () const
{
    return m_gui.server->text();
}

const QString ConnectionDialog::getUrl () const
{
    return m_gui.url->text();
}
