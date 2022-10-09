/*  commands/cardcommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cardcommand.h"
#include "cardcommand_p.h"

#include <QWidget>

using namespace Kleo;

CardCommand::Private *CardCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const CardCommand::Private *CardCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

CardCommand::Private::Private(CardCommand *qq, const std::string &serialNumber, QWidget *parent)
    : Command::Private(qq, parent)
    , serialNumber_(serialNumber)
{
}

CardCommand::Private::~Private()
{
}

CardCommand::CardCommand(const std::string &serialNumber, QWidget *parent)
    : Command(new Private(this, serialNumber, parent))
{
}

CardCommand::CardCommand(Private *pp)
    : Command(pp)
{
}

CardCommand::~CardCommand()
{
}

#undef q_func
#undef d_func
