/*  commands/changepincommand.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "cardcommand.h"

namespace GpgME
{
class Error;
}

namespace Kleo
{
namespace Commands
{

class ChangePinCommand : public CardCommand
{
    Q_OBJECT
public:
    enum ChangePinMode {
        NormalMode = 0,
        ResetMode = 1,
        NullPinMode = 2
    };

    explicit ChangePinCommand(const std::string &serialNumber, const std::string &appName, QWidget *parent);
    ~ChangePinCommand() override;

    void setKeyRef(const std::string &keyRef);
    void setMode(ChangePinMode mode = NormalMode);

private:
    void doStart() override;
    void doCancel() override;

private:
    class Private;
    inline Private *d_func();
    inline const Private *d_func() const;
    Q_PRIVATE_SLOT(d_func(), void slotResult(GpgME::Error))
};

} // namespace Commands
} // namespace Kleo


