/* -*- mode: c++; c-basic-offset:4 -*-
    commands/createopenpgpkeyfromcardkeyscommand.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <commands/cardcommand.h>

namespace Kleo
{
namespace Commands
{

class CreateOpenPGPKeyFromCardKeysCommand : public CardCommand
{
    Q_OBJECT
public:
    explicit CreateOpenPGPKeyFromCardKeysCommand(const std::string &serialNumber, const std::string &appName, QWidget *parent = nullptr);
    ~CreateOpenPGPKeyFromCardKeysCommand() override;

    static bool isSupported();

private:
    void doStart() override;
    void doCancel() override;

private:
    class Private;
    inline Private *d_func();
    inline const Private *d_func() const;
    Q_PRIVATE_SLOT(d_func(), void slotDialogAccepted())
    Q_PRIVATE_SLOT(d_func(), void slotDialogRejected())
    Q_PRIVATE_SLOT(d_func(), void slotResult(GpgME::Error))
};

} // namespace Commands
} // namespace Kleo

