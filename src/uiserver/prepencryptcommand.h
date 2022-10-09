/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/prepencryptcommand.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "assuancommand.h"

#include <utils/pimpl_ptr.h>

namespace Kleo
{

class PrepEncryptCommand : public Kleo::AssuanCommandMixin<PrepEncryptCommand>
{
public:
    PrepEncryptCommand();
    ~PrepEncryptCommand() override;
private:
    int doStart() override;
    void doCanceled() override;
public:
    static const char *staticName()
    {
        return "PREP_ENCRYPT";
    }

    class Private;
private:
    kdtools::pimpl_ptr<Private> d;
};

}

