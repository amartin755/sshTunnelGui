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

#include <QApplication>
#include <QCoreApplication>
#include <QTranslator>
#include <QSettings>
#include <QLibraryInfo>

#include "maindialog.h"


int main (int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName ("sshTunnelGUI");
    QCoreApplication::setOrganizationDomain ("sshtunnelgui.github.io");

    MainDialog dlg (&app);
    dlg.show();
    return app.exec();
}