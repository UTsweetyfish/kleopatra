/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/verifychecksumscontroller.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <crypto/controller.h>

#ifndef QT_NO_DIRMODEL

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>
#include <kmime/kmime_header_parsing.h>

#include <memory>
#include <vector>

namespace Kleo
{
namespace Crypto
{

class VerifyChecksumsController : public Controller
{
    Q_OBJECT
public:
    explicit VerifyChecksumsController(QObject *parent = nullptr);
    explicit VerifyChecksumsController(const std::shared_ptr<const ExecutionContext> &ctx, QObject *parent = nullptr);
    ~VerifyChecksumsController();

    void setFiles(const QStringList &files);

    void start();

public Q_SLOTS:
    void cancel();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotOperationFinished())
};

}
}

#endif // QT_NO_DIRMODEL


