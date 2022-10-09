/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/assuanserverconnection.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <kleo-assuan.h> // for assuan_fd_t

#include <utils/pimpl_ptr.h>

#include <memory>
#include <string>
#include <vector>

namespace Kleo
{

class AssuanCommandFactory;

class AssuanServerConnection : public QObject
{
    Q_OBJECT
public:
    AssuanServerConnection(assuan_fd_t fd, const std::vector< std::shared_ptr<AssuanCommandFactory> > &factories, QObject *parent = nullptr);
    ~AssuanServerConnection();

public Q_SLOTS:
    void enableCryptoCommands(bool enable = true);

Q_SIGNALS:
    void closed(Kleo::AssuanServerConnection *which);
    void startKeyManagerRequested();
    void startConfigDialogRequested();

public:
    class Private;
private:
    kdtools::pimpl_ptr<Private> d;
};

}

