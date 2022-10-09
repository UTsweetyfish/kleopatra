/* -*- mode: c++; c-basic-offset:4 -*-
    padwidget.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2018 Intevation GmbH

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>
#include <memory>

namespace Kleo
{
/** The padwidget provides a general I/O area inside of kleopatra
 * and can be used as an alternative view to the tabwidget. */
class PadWidget: public QWidget
{
    Q_OBJECT
public:
    explicit PadWidget(QWidget *parent = Q_NULLPTR);

private:
    class Private;
    std::shared_ptr<Private> d;
};
} // namespace Kleo

