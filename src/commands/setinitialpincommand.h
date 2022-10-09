/* -*- mode: c++; c-basic-offset:4 -*-
    commands/setinitialpincommand.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2009 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <commands/cardcommand.h>

namespace Kleo
{
namespace Commands
{

class SetInitialPinCommand : public CardCommand
{
    Q_OBJECT
public:
    SetInitialPinCommand(const std::string &serialNumber);
    ~SetInitialPinCommand() override;

    /* reimp */ static Restrictions restrictions()
    {
        return AnyCardHasNullPin;
    }

    QDialog *dialog() const;

private:
    void doStart() override;
    void doCancel() override;

private:
    class Private;
    inline Private *d_func();
    inline const Private *d_func() const;
    Q_PRIVATE_SLOT(d_func(), void slotDialogRejected())
    Q_PRIVATE_SLOT(d_func(), void slotDialogAccepted())
    Q_PRIVATE_SLOT(d_func(), void slotNksPinRequested())
    Q_PRIVATE_SLOT(d_func(), void slotSigGPinRequested())
};

}
}

